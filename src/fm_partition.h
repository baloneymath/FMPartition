#ifndef FM_PARTITION_H
#define FM_PARTITION_H

#include <set>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <utility>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <random>
#include <algorithm>
#include <cassert>
//#include <boost/algorithm/string.hpp>a

//#define _DEBUG
#define INT_MIN -999999999

using namespace std;

class Net;

class Cell {
    public:
        Cell() {}
        Cell(int p, int s, int idx)
        {
            part = p;
            size = s;
            index = idx;
            gain = 0;
            lock = false;
        }
        ~Cell() {};
        bool isFree() {return !lock;}
        bool operator() (Cell* c1, Cell* c2) {
            return c1->longestNet < c2->longestNet;
        }

        vector<int> netlist; // nets containing Cell i
        int         index; // index of Cell i
        int         size; // size of Cell i
        int         part; // which partition that Cell i is in
        int         gain; // the gain of Cell
        bool        lock; // if Cell i is locked
        int         longestNet;
        list<int>::iterator       place; // place in the GainList
};

class Net {
    public:
        Net() {}
        Net(int n) {
            index = n;
            nP0 = 0;
            nP1 = 0;
        }
        ~Net() {}

        vector<int> clist; // cell list on the net
        int         index; // index of the net
        int         nP0;
        int         nP1;
};

class FMPartition {
    public:
        FMPartition(){
            nNet = nCell = 0;
            upperbound = lowerbound = 0.0;
            part0Size = part1Size = 0;
        }
        FMPartition(string&);
        ~FMPartition();
        void    parse(string&); // load from file
        void    initGain(); // initialize the gain
        int     buildGainList();
        void    clearGainList();
        void    computeGain();
        void    resetGain();
        int     findNextMoveCell(); // return the index of max gain cell
        int     pickBetterResult();
        void    moveToStep(int&); // moveAndUpdateCellGain() till a step
        void    oneRound(); // start the process 1 round
        int     countCutSize();
        void    resetRecord();
        void    printCurrentState();
        void    outputFile(string&);
        // helper func
        bool    balanceAfterMove(int, int); // check if is balanced after move
        void    freeAllCell(); // unlock all cells
        void    moveAndUpdateCellGain(int); // updating cell gains
        void    correctNetP0P1();

    private:
        int             nNet; // total # of nets
        int             nCell; // total # of cells
        double          bf; // balance factor of the size of two partition
        vector<Cell*>   Cells;
        vector<Net*>    Nets;
        double          upperbound; // the upperbound of balance
        double          lowerbound; // the lowerbound of balance
        int             MaxP;
        int             MaxGain;

        vector<list<int>> GainList;
        int             part0Size;
        int             part1Size;
        vector<vector<int>>  recordGain; // 0:cidx, 1:cgain, 2: balance


};


#endif
