#include "fm_partition.h"

vector<int> oriunlocked;
/****************************************/
FMPartition::~FMPartition()
{
    for (int i = 0; i < nCell; ++i) {
        delete Cells[i];
    }
    for (int i = 0; i < nNet; ++i) {
        delete Nets[i];
    }
}


void FMPartition::parse(string& filename)
{
    //TODO
    ifstream f;
    f.open(filename, ifstream::in);

    if (!f.is_open()) {
       cerr << "Error occur when opening file" << endl;
       exit(1);
    }

    string buf;
    getline(f, buf);

    bf = stod(buf);

    while (f >> buf) {
        if (buf == "NET") {
            f >> buf;
            int netidx = stoi(buf.substr(1));
            if (nMap.count(netidx) == 0) {
                Net* net = new Net(netidx);
                while (f >> buf) {
                    if (buf == ";") { break; }
                    int cidx = stoi(buf.substr(1));
                    if (cMap.count(cidx) == 0) {
                        Cell* cell = new Cell(1, 1, cidx);
                        cMap[cidx] = Cells.size();
                        Cells.push_back(cell);
                        cell->netlist.push_back(netidx);
                    }
                    else {
                        Cells[cMap[cidx]]->netlist.push_back(netidx);
                    }
                    net->clist.push_back(cidx);
                }
                nMap[netidx] = Nets.size();
                Nets.push_back(net);
            }
        }
    }

    nCell = Cells.size();
    nNet = Nets.size();
    upperbound = (1 + bf) / 2 * nCell;
    lowerbound = (1 - bf) / 2 * nCell;
    #ifdef _DEBUG
        cout << "Parse....." << endl;
        for (int i = 0 ; i < nCell; ++i) {
            Cell* c = Cells[i];
            cout << "Cell : " << c->index;
            cout << " netlist: ";
            for (int j = 0; j < c->netlist.size(); ++j) {
                cout << c->netlist[j] << ' ';
            }
            cout << endl;
        }
        for (int i = 0; i < nNet; ++i) {
            Net* n = Nets[i];
            cout << "Net index: " << n->index;
            cout << " clist: ";
            for (int j = 0; j < n->clist.size(); ++j) {
                cout << n->clist[j] << ' ';
            }
            cout << endl;
        }
        cout << endl;
    #endif
}

void FMPartition::initGain()
{
    // randomly put n/2 different cells to another part
    int another = nCell / 2;

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, nCell - 1);

    map<int, int> tmp;
    for (int i = 0; i < another; ++i) {
        int r = dis(gen);
        if (tmp.count(r) == 0) {
            tmp[r] = 1;
            Cells[r]->part ^= 1;
        }
        else {
            --i;
        }
    }
    computeGain();
    for (int i = 0; i < nCell; ++i) {
        unlocked.push_back(Cells[i]->index);
    }
    oriunlocked = unlocked;
    // calculate size of both part
    for (int i = 0; i < nCell; ++i) {
        Cell* tmp = Cells[i];
        if (tmp->part == 0) {
            part0Size += tmp->size;
        }
        else{
            part1Size += tmp->size;
        }
    }
    #ifdef _DEBUG
        cout << "init gain....." << endl;
        printCurrentState();
    #endif
    //moveAndUpdateCellGain(2);
}
void FMPartition::resetGain()
{
    for (int i = 0; i < nCell; ++i) {
        Cells[i]->gain = 0;
    }
}
void FMPartition::computeGain()
{
    for (int i = 0; i < nNet; ++i) { // O(P)
        Net* nn = Nets[i];
        vector<int> fromlist, tolist;
        int from = Cells[cMap[nn->clist[0]]]->part;
        for (int j = 0; j < nn->clist.size(); ++j) {
            if (Cells[cMap[nn->clist[j]]]->part == from) {
                fromlist.push_back(nn->clist[j]);
            }
            else {
                tolist.push_back(nn->clist[j]);
            }
        }
        if (tolist.size() == 0) {
            for (int j = 0; j < fromlist.size(); ++j) {
                Cells[cMap[fromlist[j]]]->gain -= 1;
            }
        }
        else if (tolist.size() == 1) {
            Cells[cMap[tolist[0]]]->gain += 1;
        }
        if (fromlist.size() == 1) {
            Cells[cMap[fromlist[0]]]->gain += 1;
        }

    }
}

void FMPartition::moveAndUpdateCellGain(int cidx)
{
    Cell* cc = Cells[cMap[cidx]];
    int from = cc->part;
    cc->lock = true;
    locked.push_back(cc->index);
    vector<int>::iterator iter = std::find(unlocked.begin(), unlocked.end(), cc->index);
    unlocked.erase(iter);
    for (int i = 0; i < cc->netlist.size(); ++i) {
        Net* net = Nets[nMap[cc->netlist[i]]];
        vector<int> fromlist, tolist;
        for (int j = 0; j < net->clist.size(); ++j) {
            Cell* target = Cells[cMap[net->clist[j]]];
            if (target->part == from) {
                fromlist.push_back(target->index);
            }
            else {
                tolist.push_back(target->index);
            }
        }
        if (tolist.size() == 0) {
            for (int j = 0; j < fromlist.size(); ++j) {
                Cell* tmp = Cells[cMap[fromlist[j]]];
                if (tmp->isFree()) {
                    tmp->gain += 1;
                }
            }
        }
        else if (tolist.size() == 1) {
            Cell* tmp = Cells[cMap[tolist[0]]];
            if (tmp->isFree()) {
                tmp->gain -= 1;
            }
        }

        // F(n) = F(n) - 1, T(n) = T(n) + 1
        vector<int>::iterator it = std::find(fromlist.begin(), fromlist.end(), cc->index);
        fromlist.erase(it);
        tolist.push_back(cc->index);

        if (fromlist.size() == 0) {
            for (int j = 0; j < tolist.size(); ++j) {
                Cell* tmp = Cells[cMap[tolist[j]]];
                if (tmp->isFree()) {
                    tmp->gain -= 1;
                }
            }
        }
        else if (fromlist.size() == 1) {
            Cell* tmp = Cells[cMap[fromlist[0]]];
            if (tmp->isFree()) {
                tmp->gain += 1;
            }
        }

    }
    if (cc->part == 0) {
        part0Size -= cc->size;
        part1Size += cc->size;
    }
    else {
        part1Size -= cc->size;
        part0Size += cc->size;
    }
    cc->part = from ^ 1;
    vector<int> tmp(3, 0);
    tmp[0] = cc->index;
    tmp[1] = cc->gain;
    tmp[2] = abs(part0Size - part1Size);
    recordGain.push_back(tmp);

    #ifdef _DEBUG
        cout << "move and update cell gains....." << endl;
        for (int i = 1; i <= nCell; ++i) {
            cout << "cell: " << Cells[cMap[i]]->index << ' ';
            cout << "part: " << Cells[cMap[i]]->part << ' ';
            cout << "gain: " << Cells[cMap[i]]->gain << ' ';
            cout << "lock: " << Cells[cMap[i]]->lock << endl;
        }
        cout << "unlocked: ";
        for (int i = 0; i < unlocked.size(); ++i) {
            cout << unlocked[i] << ' ';
        }
        cout << endl;
        cout << "locked: ";
        for (int i = 0; i < locked.size(); ++i) {
            cout << locked[i] << ' ';
        }
        cout << endl;
        cout << "part0Size: " << part0Size << endl;
        cout << "part1Size: " << part1Size << endl;
        cout << endl;
    #endif
}


bool FMPartition::balanceAfterMove(int cidx, int size)
{
    Cell* c = Cells[cMap[cidx]];
    int from = c->part;
    int fromSize = 0, toSize = 0;
    if (from == 0) {
        fromSize = part0Size;
        toSize = part1Size;
    }
    else {
        fromSize = part1Size;
        toSize = part0Size;
    }
    if (lowerbound <= (fromSize - c->size) &&
            upperbound >= (fromSize - c->size) &&
            lowerbound <= (toSize + c->size) &&
            upperbound >= (toSize + c->size)) {
        return true;
    }
    else return false;
}

void
FMPartition::buildBucket()
{
    GainList = map<int, vector<int>>();
    for (int i = 0; i < nCell; ++i) {
        Cell* c = Cells[i];
        if (!c->isFree()) continue;
        if (GainList.count(c->gain) == 0) {
            vector<int> tmp;
            tmp.push_back(c->index);
            GainList[c->gain] = tmp;
        }
        else {
            GainList[c->gain].push_back(c->index);
        }
    }
    #ifdef _DEBUG
        cout << endl;
        cout << "build gain bucket....." << endl;
        cout << "GainList: " << endl;
        for (auto it = GainList.rbegin(); it != GainList.rend(); ++it) {
            cout << (*it).first << ": ";
            for (int i = 0; i < (*it).second.size(); ++i) {
                cout << (*it).second[i] << ' ';
            }
            cout << endl;
        }
        cout << "max gain: " << (*GainList.rbegin()).first << endl << endl;
    #endif
}

int FMPartition::findNextMoveCell()
{
    int target = -1;
    for (auto it = GainList.rbegin(); it != GainList.rend(); ++it) {
        vector<int> tmp = (*it).second;
        for (int i = 0; i < tmp.size(); ++i) {
            int cidx = tmp[i];
            if (balanceAfterMove(cidx, Cells[cMap[cidx]]->size)) {
                target = cidx;
                break;
            }
        }
        if (target != -1) {
            break;
        }
    }
    #ifdef _DEBUG
    cout << "next target: " << target << endl;
    #endif
    return target;
}

void FMPartition::freeAllCell()
{
    for (int i = 0; i < nCell; ++i) {
        Cells[i]->lock = false;
    }
    locked = vector<int>();
    unlocked = oriunlocked;
}
void FMPartition::resetRecord()
{
    recordGain = vector<vector<int>>();
}

int FMPartition::pickBetterResult()
{
    vector<int> accumGain(recordGain.size(), 0) ;
    accumGain[0] = recordGain[0][1];
    for (int i = 1; i < recordGain.size(); ++i) {
        accumGain[i] = accumGain[i - 1] + recordGain[i][1];
    }
    int Max = accumGain[0];
    for (int i= 1; i < accumGain.size(); ++i) {
        if (Max < accumGain[i]) {
            Max = accumGain[i];
        }
    }
    if (Max <= 0) {
        #ifdef _DEBUG
            cout << "No more better move, Stop iteration!!!!!!" << endl;
            printCurrentState();
        #endif
        return -1;
    }
    vector<int> maxResultStep;
    for (int i = 0; i < accumGain.size(); ++i) {
        if (accumGain[i] == Max) {
            maxResultStep.push_back(i);
        }
    }
    // find the step which results best balance in maxResultStep
    int targetStep = maxResultStep[0];
    int bestBalance = recordGain[maxResultStep[0]][2];
    for (int i = 1; i < maxResultStep.size(); ++i) {
        int tmp = recordGain[maxResultStep[i]][2];
        if (tmp < bestBalance) {
            bestBalance = tmp;
            targetStep = maxResultStep[i];
        }
    }

    #ifdef _DEBUG
        cout << endl;
        cout << "move cell: ";
        for (int i = 0; i < recordGain.size(); ++i) {
            cout << 'c' << recordGain[i][0] << ' ';
        }
        cout << endl;
        cout << "gain:      ";
        for (int i = 0; i < recordGain.size(); ++i) {
            cout << recordGain[i][1] << ' ';
        }
        cout << endl;
        cout << "accumGain: ";
        for (int i = 0; i < recordGain.size(); ++i) {
            cout << accumGain[i] << ' ';
        }
        cout << endl;
        cout << "targetStep: " << targetStep << endl;
        cout << endl;
    #endif
    return targetStep;
}

void FMPartition::oneRound()
{
    #ifdef _DEBUG
        printCurrentState();
    #endif
    while (!unlocked.empty()) {
        buildBucket();
        int next = findNextMoveCell();
        moveAndUpdateCellGain(next);
    }
    freeAllCell();
}

int FMPartition::countCutSize()
{
    int cutsize = 0;
    for (int i = 0; i < nNet; ++i) {
        Net* n = Nets[i];
        int from = Cells[cMap[n->clist[0]]]->part;
        bool cutted = false;
        for (int j = 1; j < n->clist.size(); ++j) {
            Cell* c = Cells[cMap[n->clist[j]]];
            if (from != c->part) {
                cutted = true;
            }
        }
        if (cutted == true) {
            cutsize += 1;
        }
    }
    return cutsize;
}

void FMPartition::moveToStep(int step)
{
    for (int i = 0; i <= step; ++i) {
        buildBucket();
        int next = findNextMoveCell();
        moveAndUpdateCellGain(next);
    }
    freeAllCell();
}


vector<int> FMPartition::storePart()
{
    vector<int> parts;
    for (int i = 0; i < nCell; ++i) {
        parts.push_back(Cells[i]->part);
    }
    return parts;
}

vector<int> FMPartition::storeGain()
{
    vector<int> gains;
    for (int i = 0; i < nCell; ++i) {
        gains.push_back(Cells[i]->gain);
    }
    return gains;
}

vector<int> FMPartition::storeUnlocked()
{
    return unlocked;
}

int FMPartition::getPart0Size()
{
    return part0Size;
}

int FMPartition::getPart1Size()
{
    return part1Size;
}

void FMPartition::restorePart(vector<int>& parts)
{
    for (int i = 0; i < nCell; ++i) {
        Cells[i]->part = parts[i];
    }
}

void FMPartition::restoreGain(vector<int>& gains)
{
    for (int i = 0; i < nCell; ++i) {
        Cells[i]->gain = gains[i];
    }
}

void FMPartition::restoreALL(vector<int>& parts, vector<int>& gains,
        int p0, int p1, vector<int>& ulk)
{
    restoreGain(gains);
    restorePart(parts);
    part0Size = p0;
    part1Size = p1;
    unlocked = ulk;
    locked = vector<int>();
    freeAllCell();
    resetRecord();
}

void FMPartition::printCurrentState()
{
    for (int i = 1; i <= nCell; ++i) {
        cout << "cell: " << Cells[cMap[i]]->index << ' ';
        cout << "part: " << Cells[cMap[i]]->part << ' ';
        cout << "gain: " << Cells[cMap[i]]->gain << ' ';
        cout << "lock: " << Cells[cMap[i]]->lock << endl;
    }
    cout << "unlocked: ";
    for (int i = 0; i < unlocked.size(); ++i) {
        cout << unlocked[i] << ' ';
    }
    cout << endl;
    cout << "locked: ";
    for (int i = 0; i < locked.size(); ++i) {
        cout << locked[i] << ' ';
    }
    cout << endl;
    cout << "part0Size: " << part0Size << endl;
    cout << "part1Size: " << part1Size << endl << endl;
}

void FMPartition::printPart0Cell()
{
    vector<int> p0;
    for (int i = 0; i < nCell; ++i) {
        Cell* c = Cells[i];
        if (c->part == 0) {
            p0.push_back(c->index);
        }
    }
    std::sort(p0.begin(), p0.end());
    for (int i = 0; i < p0.size(); ++i) {
        cout << 'c' << p0[i] << ' ';
    }
    cout << ';' << endl;
}

void FMPartition::printPart1Cell()
{
    vector<int> p1;
    for (int i = 0; i < nCell; ++i) {
        Cell* c = Cells[i];
        if (c->part == 1) {
            p1.push_back(c->index);
        }
    }
    std::sort(p1.begin(), p1.end());
    for (int i = 0; i < p1.size(); ++i) {
        cout << 'c' << p1[i] << ' ';
    }
    cout << ';' << endl;
}

void FMPartition::outputFile(string& filename)
{
    ofstream ofile;
    ofile.open(filename, ofstream::out);
    if (!ofile.is_open()) {
        cerr << "Error writing output file!!" << endl;
    }
    ofile << "Cutsize = " << countCutSize() << endl;
    ofile << "G1 " << part0Size << endl;
    vector<int> p0, p1;
    for (int i = 0; i < nCell; ++i) {
        Cell* c = Cells[i];
        if (c->part == 0) {
            p0.push_back(c->index);
        }
    }
    std::sort(p0.begin(), p0.end());
    for (int i = 0; i < p0.size(); ++i) {
        ofile << 'c' << p0[i] << ' ';
    }
    ofile << ';' << endl;
    ofile << "G2 " << part1Size << endl;
    for (int i = 0; i < nCell; ++i) {
        Cell* c = Cells[i];
        if (c->part == 1) {
            p1.push_back(c->index);
        }
    }
    std::sort(p1.begin(), p1.end());
    for (int i = 0; i < p1.size(); ++i) {
        ofile << 'c' << p1[i] << ' ';
    }
    ofile << ';' << endl;
}
