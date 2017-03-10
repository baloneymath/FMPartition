#ifndef FM_PARTITION_H
#define FM_PARTITION_H

#include <list>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <fstream>
using namespace std;

class Net;

class Cell {
    public:
        Cell();
        Cell(int&, int&);
        ~Cell();
        bool isFree();
    private:
        vector<int> netlist; // nets containing Cell i
        int         index; // index of Cell i
        int         part; // which partition that Cell i is in
        int         size; // size of Cell i
        int         gain; // the gain of Cell
        int         gainByMove; // the gain from moving and locking
        bool        lock; // if Cell i is locked
};

class Net {
    public:
        Net();
        Net(int&);
        ~Net();
    private:
        vector<int> clist; // cell list on the net
        int         nCell; // # of cells on net
        int         lock[2]; // # of locked cells in two partitions
        int         unclock[2]; // # of unclocked cells in two partitions
        int         index; // index of the net
};

class FMPartition {
    public:
        FMPartition();
        ~FMPartition();
        void    initialize(string&); // load from file
        void    initGain(); // initialize the gain


    private:
        int         nNet; // total # of nets
        int         nCell; // total # of cells
        double      bf; // balance factor of the size of two partition
        vector<Cell*> Cells;
        vector<Net*>  Nets;
        map<int, int> cMap; // first: cell index, second: index in vector
        map<int, int> nMap;
        int         upperbound; // the upperbound of balance
        int         lowerbound; // the lowerbound of balance
        int         Smax; // max size of Cell

};


#endif
