#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <fstream>
#include "fm_partition.h"

using namespace std;

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
    int counter = 1;
    while (1) {
        #ifdef _DEBUG
            cout << "*************************************" << endl;
            cout << "*            Round-" << counter;
            cout << "                *" << endl;
            cout << "*************************************" << endl;
            ++counter;
        #endif
        vector<int> oriPart = FM.storePart();
        vector<int> oriGain = FM.storeGain();
        int orip0 = FM.getPart0Size();
        int orip1 = FM.getPart1Size();
        vector<int> oriULK = FM.storeUnlocked();
        FM.oneRound();
        int step = FM.pickBetterResult();
        FM.restoreALL(oriPart, oriGain, orip0, orip1, oriULK);
        if (step == -1) {
            break;
        }
        else {
            FM.moveToStep(step);
            FM.resetRecord();
            FM.resetGain();
            FM.computeGain();
        }
    }
    int p0size = FM.getPart0Size();
    int p1size = FM.getPart1Size();
    cout << "Cutsize = " << FM.countCutSize() << endl;
    cout << "G1 " << p0size << endl;
    FM.printPart0Cell();
    cout << "G2 " << p1size << endl;
    FM.printPart1Cell();
    if (argc == 3) {
        string ofile = argv[2];
        FM.outputFile(ofile);
    }
    return 0;
}
