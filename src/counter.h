#pragma once

#include "PNS.h"
#include "board.h"
#include <map>

class Counter{
public:
    Counter(){}
    int count_nodes(PNS::Node* node);
    int update_tree(PNS::Node* node);

private:
    std::map<Board, bool> states;
};
