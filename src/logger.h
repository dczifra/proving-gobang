#pragma once

#include <fstream>
#include <iostream>

#include "common.h"
#include "PNS.h"
#include "board.h"
#include <map>

class Logger{
public:
    Logger(){}
    void init(bool disproof);
    void log(PNS::PNSNode* node, Heuristic& h);
    void log_solution_min(PNS::PNSNode* node, std::ofstream& file, std::set<Board>& logged);

private:
    std::ofstream logstream;
    std::map<Board, bool> logged_states;
};