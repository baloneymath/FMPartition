#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include "fm_partition.h"

using namespace std;
//#define _DEBUG
int main(int argc, char* argv[])
{
    assert(argc <= 3);
    // TODO
    string filename = argv[1];

    FMPartition FM;
    FM.parse(filename);
    FM.initGain();
    #ifdef _DEBUG
        cout << "original Cutsize: " << FM.countCutSize() << endl;
    #endif
    int step = -2;
    int counter = 0;
    while (1) {
        ++counter;
        FM.clearGainList();
        FM.oneRound();
        int step = FM.pickBetterResult();
        if (step == -1) {
            break;
        }
        else {
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
    cout << "Cutsize = " << FM.countCutSize() << endl;
    if (argc == 3) {
        string ofile = argv[2];
        FM.outputFile(ofile);
    }
    return 0;
}
