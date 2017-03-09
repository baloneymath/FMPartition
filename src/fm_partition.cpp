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
    nCell = n;
}
Net::~Net() {}

/****************************************/
FMPartition::FMPartition() {}
FMPartition::~FMPartition() {}

void FMPartition::initialize(string& filename)
{
    //TODO
    initGain();
}

void FMPartition::initGain()
{
    //TODO
}
