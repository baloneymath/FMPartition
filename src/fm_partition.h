#ifndef FM_PARTITION_H
#define FM_PARTITION_H

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
        //Cell() {lock = false;}
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

        vector<int> netlist; // nets containing Cell i
        int         index; // index of Cell i
        int         size; // size of Cell i
        int         part; // which partition that Cell i is in
        int         gain; // the gain of Cell
        bool        lock; // if Cell i is locked
        list<int>::iterator       place; // place in the GainList
};

class Net {
    public:
        Net() {}
        Net(int n) {index = n;}
        ~Net() {}

        vector<int> clist; // cell list on the net
        int         index; // index of the net
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
        void    buildGainList();
        void    computeGain();
        void    resetGain();
        int     findNextMoveCell(); // return the index of max gain cell
        int     pickBetterResult();
        void    moveToStep(int); // moveAndUpdateCellGain() till a step
        void    oneRound(); // start the process 1 round
        int     countCutSize();
        vector<int> storePart(); // store the current state of FM
        vector<int> storeGain(); // store the current state of FM
        vector<int> storeUnlocked();
        vector<list<int>> storeGainList();
        void    restoreALL(vector<int>&, vector<int>&,
                        int, int, vector<int>&, vector<list<int>>&); // restore state
        void    resetRecord();
        int     getPart0Size();
        int     getPart1Size();
        void    printCurrentState();
        void    printPart0Cell();
        void    printPart1Cell();
        void    outputFile(string&);
        // helper func
        bool    balanceAfterMove(int, int); // check if is balanced after move
        void    freeAllCell(); // unlock all cells
        void    restorePart(vector<int>&);
        void    restoreGain(vector<int>&);
        void    moveAndUpdateCellGain(int); // updating cell gains

    private:
        int             nNet; // total # of nets
        int             nCell; // total # of cells
        double          bf; // balance factor of the size of two partition
        vector<Cell*>   Cells;
        vector<Net*>    Nets;
        map<int, int>   cMap; // first: cell index, second: index in vector
        map<int, int>   nMap;
        double          upperbound; // the upperbound of balance
        double          lowerbound; // the lowerbound of balance
        int             MaxP;
        vector<int>     locked;
        vector<int>     unlocked;
        //map<int, vector<int>> GainList;
        vector<list<int>> GainList;
        int             part0Size;
        int             part1Size;
        vector<vector<int>>  recordGain; // 0:cidx, 1:cgain, 2: balance


};


#endif
