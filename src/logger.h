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
    void log(Node* node, Heuristic& h);
    void log_solution_min(Node* node, std::ofstream& file, std::string& filebuffer, std::set<Board>& logged);
    void log_node(Node* node, std::string filename);

private:
    std::ofstream logstream;
    std::map<Board, bool> logged_states;
};
