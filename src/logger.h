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
    void log(Node* node, std::ofstream& file);
    void log_solution_min(Node* node, std::ofstream& file, std::string& filebuffer, std::set<Board>& logged, int depth);
    void log_node(Node* node, std::string filename);
    void log_states(PNS& tree, std::string filename);
    void log_data(Node* node, std::set<Board>& logged, int depth, std::ofstream& file);
    void log_statistics(Node* node, std::string filename);

private:
    std::ofstream logstream;
    std::map<Board, bool> logged_states;
};
