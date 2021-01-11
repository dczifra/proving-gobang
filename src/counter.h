#pragma once

#include "PNS.h"
#include "board.h"
#include <map>

class Counter{
public:
    Counter(){}
    int count_nodes(Node* node);
    int update_tree(Node* node);

private:
    std::map<Board, bool> states;
};
