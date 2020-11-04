#pragma once

#include "PNS.h"
#include "board.h"
#include <map>

class Counter{
public:
    Counter(){}
    int count_nodes(PNS::PNSNode* node);

private:
    std::map<Board, bool> counted_states;
};
