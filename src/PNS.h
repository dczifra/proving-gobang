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
    inline std::vector<board_int> get_lines(unsigned int action){
        return heuristic.compressed_lines_per_action[action];
    }

private :
    std::map<Board, PNSNode*> states;
    Heuristic heuristic;
};