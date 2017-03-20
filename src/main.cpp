#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include "fm_partition.h"

using namespace std;
#define _DEBUG
int main(int argc, char* argv[])
{
    assert(argc <= 3);
    // TODO
    string filename = argv[1];

    FMPartition FM, FM_before;
    FM.parse(filename);
    FM.initGain();
    #ifdef _DEBUG
        cout << "original Cutsize: " << FM.countCutSize() << endl;
    #endif
    int step = -2;
    int counter = 0;
    while (1) {
        ++counter;
        vector<int> oriPart = FM.storePart();
        vector<int> oriGain = FM.storeGain();
        vector<list<int>> oriGList = FM.storeGainList();
        int orip0 = FM.getPart0Size();
        int orip1 = FM.getPart1Size();
        FM.clearGainList();
        FM.oneRound();
        int step = FM.pickBetterResult();
        FM.restoreALL(oriPart, oriGain, orip0, orip1, oriGList);
        if (step == -1) {
            break;
        }
        else {
            FM.clearGainList();
            FM.moveToStep(step);
            FM.resetRecord();
            FM.resetGain();
            FM.computeGain();
            #ifdef _DEBUG
                cout << "Round-" << counter << ": ";
                cout << FM.countCutSize() << endl;
            #endif
        }
    }
    int p0size = FM.getPart0Size();
    int p1size = FM.getPart1Size();
    cout << "Cutsize = " << FM.countCutSize() << endl;
    if (argc == 3) {
        string ofile = argv[2];
        FM.outputFile(ofile);
    }
    return 0;
}
