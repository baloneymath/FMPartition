#include "fm_partition.h"

/****************************************/
Cell::Cell() {}
Cell::Cell(int& p, int& s)
{
    part = p;
    size = s;
}
Cell::~Cell() {}

bool Cell:isFree()
{
    return !lock;
}

/****************************************/
Net::Net() {}
Net::Net(int& n)
{
    index = n;
}
Net::~Net() {}

/****************************************/
FMPartition::FMPartition() {}
FMPartition::FMPartition(string& filename)
{
    initialize(filename);
}
FMPartition::~FMPartition() {}

void split(const string& s, const string& delim, vector<string>& v)
{
    size_t pos = 0;
    string token;
    while ((pos = s.find(delim)) != string::npos) {
        token = s.substr(0, pos);
        v.push_back(token);
        cout << token << endl;
        s.erase(0, pos + strlen(delim));
    }
}

void FMPartition::initialize(string& filename)
{
    //TODO
    ifstream f;
    f.open(filename&, ifstream::in);

    if (!f.is_open()) {
        cerr << "Error occur when opening file" << endl;
    }

    string buf;
    getline(f, buf);

    bf = stod(buf);
    nNet = 0;
    nCell = 0;

    while (getline(f, buf)) {
        vector<string> tokens;
        split(buf, " ;", tokens);

        Net *net = new Net(int(tokens[1][tokens[1].size() - 1] - '0'));
        for (int i = 2; i < tokens.size(); ++i) {
            idx = int(tokens[i][tokens[i].size() - 1] - '0');
            if (cMap.count(idx) == 0) {
                Cell *cell = new Cell(1, 1);
                cell->index = idx;
                cell->net
                cMap[idx] = net->Cells.size();
                Cells.push_back(cell);
            }
            net->clist.push_back(idx);
        }
    }

    initGain();
}

void FMPartition::initGain()
{
    //TODO
}
