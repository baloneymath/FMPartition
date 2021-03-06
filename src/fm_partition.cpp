#include "fm_partition.h"

/****************************************/
FMPartition::~FMPartition()
{
    for (int i = 1; i <= nCell; ++i) {
        delete Cells[i];
    }
    for (int i = 1; i <= nNet; ++i) {
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
    int netidx = 0;
    while (f >> buf) {
        if (buf == "NET") {
            f >> buf;
            ++netidx;
            while (f >> buf) {
                if (buf == ";") break;
                nCell = max(nCell, stoi(buf.substr(1)));
            }
        }
    }
    nNet = netidx;
    netidx = 0;
    f.close();

    Cells.resize(nCell + 1);
    for (int i = 0; i <= nCell; ++i) {
        Cell* cell = new Cell(1, 1, i);
        Cells[i] = cell;
    }
    Nets.resize(nNet + 1);
    f.open(filename, ifstream::in);
    if (!f.is_open()) {
       cerr << "Error occur when opening file" << endl;
       exit(1);
    }
    while (f >> buf) {
        if (buf == "NET") {
            f >> buf;
            ++netidx;
            Net* net = new Net(netidx);
            set<int> tmp;
            while (f >> buf) {
                if (buf == ";") break;
                int cidx = stoi(buf.substr(1));
                if (tmp.count(cidx) == 0) {
                    Cells[cidx]->netlist.push_back(netidx);
                    net->clist.push_back(cidx);
                    tmp.insert(cidx);
                }
            }
            Nets[netidx] = net;
        }
    }
    f.close();
    upperbound = (1 + bf) / 2 * nCell;
    lowerbound = (1 - bf) / 2 * nCell;
    #ifdef _DEBUG
        cout << "Parse....." << endl;
        for (int i = 1; i <= nNet; ++i) {
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
    int another = 0;
    for (int i = 1; i <= nCell; ++i) {
        if (Cells[i]->netlist.size() != 0) {
            ++another;
        }
    }
    another /= 2;

    vector<Cell*> tmp = Cells;
    for (int i = 1; i <= nCell; ++i) {
        int m = 0;
        Cell* c = Cells[i];
        for (int j = 0; j < c->netlist.size(); ++j) {
            m = max((size_t)m, Nets[c->netlist[j]]->clist.size());
        }
        c->longestNet = m;
    }

    std::sort(tmp.begin(), tmp.end(), Cell());
    int j = 1;
    for (int i = 0; i < another; ++i) {
        Cell* c = tmp[j];
        ++j;
        if (c->netlist.size() == 0) {
            --i;
            continue;
        }
        c->part ^= 1;
    }
//  random_device rd;
//  mt19937 gen(rd());
//  uniform_int_distribution<> dis(1, nCell);

//  map<int, int> tmp;
//  for (int i = 0; i < another; ++i) {
//      int r = dis(gen);
//      if (Cells[r]->netlist.size() == 0) {
//          --i;
//          continue;
//      }
//      if (tmp.count(r) == 0) {
//          tmp[r] = 1;
//          Cells[r]->part ^= 1;
//      }
//      else {
//          --i;
//      }
//  }
    correctNetP0P1();
    computeGain();
    // calculate size of both part
    for (int i = 1; i <= nCell; ++i) {
        Cell* tmp = Cells[i];
        if (tmp->netlist.size() == 0) continue;
        if (tmp->part == 0) {
            part0Size += tmp->size;
        }
        else{
            part1Size += tmp->size;
        }
    }
    MaxP = 0;
    for(auto it = ++Cells.begin(); it != Cells.end(); ++it) {
        MaxP = max((int)(*it)->netlist.size(), MaxP);
    }
    GainList.resize(2 * MaxP + 1);
    MaxGain = buildGainList();

    #ifdef _DEBUG
        cout << "init gain....." << endl;
        printCurrentState();
    #endif
}
void FMPartition::correctNetP0P1()
{
    for (int i = 1; i <= nNet; ++i) {
        Net* nn = Nets[i];
        nn->nP0 = nn->nP1 = 0;
        if (nn->clist.size() <= 1) continue;
        for (int j = 0; j < nn->clist.size(); ++j) {
            if (Cells[nn->clist[j]]->part == 0) {
                ++nn->nP0;
            }
        }
        nn->nP1 = nn->clist.size() - nn->nP0;
    }
}

void FMPartition::resetGain()
{
    for (int i = 1; i <= nCell; ++i) {
        Cells[i]->gain = 0;
    }
}
void FMPartition::computeGain()
{
    for (int i = 1; i <= nNet; ++i) { // O(P)
        Net* nn = Nets[i];
        if (nn->clist.size() <= 1) continue;
        int from = Cells[nn->clist[0]]->part;

        int nfrom = (from == 0)? nn->nP0 : nn->nP1;
        int nto = nn->clist.size() - nfrom;
        if (nto == 0) {
            for (int j = 0; j < nn->clist.size(); ++j) {
                Cells[nn->clist[j]]->gain -= 1;
            }
        }
        else if (nto == 1) {
            for (int j = 0; j < nn->clist.size(); ++j) {
                Cell* c = Cells[nn->clist[j]];
                if (c->part == from ^ 1) {
                    c->gain += 1;
                    break;
                }
            }
        }
        if (nfrom == 1) {
            Cells[nn->clist[0]]->gain += 1;
        }

    }
}

int FMPartition::buildGainList() {
    for (int i = 1; i <= nCell; ++i) {
        Cell* tmp = Cells[i];
        if (tmp->netlist.size() == 0) continue;
        auto where = GainList[tmp->gain + MaxP].begin();
        tmp->place = GainList[tmp->gain + MaxP].insert(where, tmp->index);
    }
    int maxGain = 0;
    for (int i = 2 * MaxP; i >= 0 ; --i) {
        if (!GainList[i].empty()) {
            maxGain = i - MaxP;
            break;
        }
    }
    return maxGain;
}

void FMPartition::clearGainList() {
    for (int i = 2 * MaxP; i >= 0; --i) {
        if (!GainList[i].empty()) {
            GainList[i].clear();
        }
    }
}

void FMPartition::moveAndUpdateCellGain(int cidx)
{
    if (cidx == -1) {
        return;
    }
    Cell* cc = Cells[cidx];
    int from = cc->part;
    cc->lock = true;
    GainList[cc->gain + MaxP].erase(cc->place);
    for (int i = 0; i < cc->netlist.size(); ++i) {
        Net* net = Nets[cc->netlist[i]];
        if (net->clist.size() <= 1) continue;

        int nfrom = (from == 0)? net->nP0 : net->nP1;
        int nto = net->clist.size() - nfrom;
        // update the target cell first
        if (nto == 0) {
            for (int j = 0; j < net->clist.size(); ++j) {
                Cell* tmp = Cells[net->clist[j]];
                if (!tmp->isFree()) continue;
                GainList[tmp->gain + MaxP].erase(tmp->place);
                tmp->gain += 1;
                MaxGain = max(MaxGain, tmp->gain);
                auto where = GainList[tmp->gain + MaxP].begin();
                tmp->place = GainList[tmp->gain + MaxP].insert(where, tmp->index);
            }
        }
        else if (nto == 1) {
            Cell* tmp = 0;
            for (int j = 0; j < net->clist.size(); ++j) {
                Cell* target = Cells[net->clist[j]];
                if (target->part == from ^ 1) {
                    tmp = target;
                    break;
                }
            }
            if (tmp != 0 && tmp->isFree()) {
                GainList[tmp->gain + MaxP].erase(tmp->place);
                tmp->gain -= 1;
                auto where = GainList[tmp->gain + MaxP].begin();
                tmp->place = GainList[tmp->gain + MaxP].insert(where, tmp->index);
            }
        }
        // F(n) = F(n) - 1, T(n) = T(n) + 1
        --nfrom;
        ++nto;
        if (from == 0) {
            --net->nP0;
            ++net->nP1;
        }
        else {
            ++net->nP0;
            --net->nP1;
        }

        if (nfrom == 0) {
            for (int j = 0; j < net->clist.size(); ++j) {
                Cell* tmp = Cells[net->clist[j]];
                if (!tmp->isFree()) continue;
                GainList[tmp->gain + MaxP].erase(tmp->place);
                tmp->gain -= 1;
                auto where = GainList[tmp->gain + MaxP].begin();
                tmp->place = GainList[tmp->gain + MaxP].insert(where, tmp->index);
            }
        }
        else if (nfrom == 1) {
            Cell* tmp = 0; 
            for (int j = 0; j < net->clist.size(); ++j) {
                Cell* target = Cells[net->clist[j]];
                if (target->part == from && target->index != cc->index) {
                    tmp = target;
                    break;
                }
            }
            if (tmp != 0 && tmp->isFree()) {
                GainList[tmp->gain + MaxP].erase(tmp->place);
                tmp->gain += 1;
                MaxGain = max(MaxGain, tmp->gain);
                auto where = GainList[tmp->gain + MaxP].begin();
                tmp->place = GainList[tmp->gain + MaxP].insert(where, tmp->index);
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
            cout << "cell: " << Cells[i]->index << ' ';
            cout << "part: " << Cells[i]->part << ' ';
            cout << "gain: " << Cells[i]->gain << ' ';
            cout << "lock: " << Cells[i]->lock << endl;
        }
        cout << "part0Size: " << part0Size << endl;
        cout << "part1Size: " << part1Size << endl;
        cout << endl;
    #endif
}


bool FMPartition::balanceAfterMove(int cidx, int size)
{
    Cell* c = Cells[cidx];
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


int FMPartition::findNextMoveCell()
{
    int target = -1;
    for (int i = MaxGain + MaxP; i >=0; --i) {
        for (auto& j : GainList[i]) {
            int cidx = j;
            if (!Cells[cidx]->isFree()) continue;
            if (balanceAfterMove(cidx, Cells[cidx]->size)) {
                target = cidx;
                break;
            }
        }
        if (target != -1) {
            break;
        }
    }
    #ifdef _DEBUG
    for (int i = 2 * MaxP; i >=0; --i) {
        cerr << i - MaxP << ": ";
        for (auto j = GainList[i].begin(); j != GainList[i].end(); ++j) {
            cerr << *j << ' ';
        }
        cerr << endl;
    }
    cout << "next target: " << target << endl;
    #endif
    return target;
}

void FMPartition::freeAllCell()
{
    for (int i = 1; i <= nCell; ++i) {
        Cells[i]->lock = false;
    }
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
    //cerr << "Gain: " << Max << ' ';
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
    //cerr << "targetstep: " << targetStep << ' ';
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


int FMPartition::countCutSize()
{
    int cutsize = 0;
    for (int i = 1; i <= nNet; ++i) {
        Net* n = Nets[i];
        if (n->clist.size() <= 1) continue;
        int from = Cells[n->clist[0]]->part;
        bool cutted = false;
        for (int j = 1; j < n->clist.size(); ++j) {
            Cell* c = Cells[n->clist[j]];
            if (from != c->part) {
                cutted = true;
                break;
            }
        }
        if (cutted == true) {
            cutsize += 1;
        }
    }
    return cutsize;
}

void FMPartition::oneRound()
{
    MaxGain = buildGainList();
    for (int i = 0; i < nCell; ++i) {
        int next = findNextMoveCell();
        moveAndUpdateCellGain(next);
    }
    freeAllCell();
    resetGain();
    computeGain();
}

void FMPartition::moveToStep(int& step)
{
    for (int i = recordGain.size() - 1; i > step; --i) {
        Cell* c = Cells[recordGain[i][0]];
        int from = c->part;
        c->part ^= 1;
        if (from == 0) {
            --part0Size;
            ++part1Size;
        }
        else {
            ++part0Size;
            --part1Size;
        }
    }
    correctNetP0P1();
}

void FMPartition::printCurrentState()
{
    for (int i = 1; i <= nCell; ++i) {
        cout << "cell: " << Cells[i]->index << ' ';
        cout << "part: " << Cells[i]->part << ' ';
        cout << "gain: " << Cells[i]->gain << ' ';
        cout << "lock: " << Cells[i]->lock << endl;
    }
    cout << endl;
    cout << "part0Size: " << part0Size << endl;
    cout << "part1Size: " << part1Size << endl << endl;
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
    for (int i = 1; i <= nCell; ++i) {
        Cell* c = Cells[i];
        if (c->netlist.size() == 0) continue;
        if (c->part == 0) {
            p0.push_back(c->index);
        }
    }
    for (int i = 0; i < p0.size(); ++i) {
        ofile << 'c' << p0[i] << ' ';
    }
    ofile << ';' << endl;
    ofile << "G2 " << part1Size << endl;
    for (int i = 1; i <= nCell; ++i) {
        Cell* c = Cells[i];
        if (c->netlist.size() == 0) continue;
        if (c->part == 1) {
            p1.push_back(c->index);
        }
    }
    for (int i = 0; i < p1.size(); ++i) {
        ofile << 'c' << p1[i] << ' ';
    }
    ofile << ';' << endl;
}
