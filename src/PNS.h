#pragma once
#include "common.h"
#include "board.h"
#include "limits.h"
#include <map>

#include "heuristic.h"

enum NodeType : uint8_t {OR, AND};
enum ProofType: uint8_t {PN, DN};

int get_player(const NodeType& type);

struct PNSNode{
    PNSNode(const Board& b, NodeType t);

    PNSNode* children[ACTION_SIZE];
    Board board;

    unsigned int pn = 1;
    unsigned int dn = 1;
    NodeType type;
};

class PNS{
public:
    void search(PNSNode* node);
    void extend(PNSNode* node, unsigned int action);
    unsigned int get_min_children(PNSNode* node, const ProofType type, bool index);
    unsigned int get_sum_children(PNSNode* node, const ProofType type);



    // === Helper Functions ===
    void log_solution(std::string filename);
    void log_solution_min(PNSNode* node, std::ofstream& file);
    void read_solution(std::string filename);
    void add_state(Board& b, PNSNode* node);

    inline std::vector<board_int> get_lines(unsigned int action){
        return heuristic.compressed_lines_per_action[action];
    }
    
    inline std::vector<board_int> get_all_lines(){
        return heuristic.all_compressed_lines;
    }

private :
    std::map<Board, PNSNode*> states;
    Heuristic heuristic;
};