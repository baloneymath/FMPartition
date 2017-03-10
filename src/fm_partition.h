#ifndef FM_PARTITION_H
#define FM_PARTITION_H

#include <list>
#include <vector>
#include <string>
using namespace std;

class Net;

class Cell {
    public:
        Cell();
        Cell(int&, int&);
        ~Cell();
        bool isFree();
    private:
        list<Net*>  netlist; // nets containing Cell i
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
        list<Cell*> clist; // cell list on the net
        int         nCell; // # of cells on net
        int         lock[2]; // # of locked cells in two partitions
        int         unclock[2]; // # of unclocked cells in two partitions
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
        vector<Cell*> cells;
        vector<Net*>  nets;
        int         upperbound; // the upperbound of balance
        int         lowerbound; // the lowerbound of balance
        int         Smax; // max size of Cell

};


#endif
