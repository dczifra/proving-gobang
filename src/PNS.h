#pragma once
#include "common.h"
#include "board.h"
#include "limits.h"
#include <map>

#include "heuristic.h"

enum ProofType: uint8_t {PN, DN};



struct PNSNode{
    PNSNode(const Board& b, NodeType t, unsigned int d, int heur_val);

    PNSNode* children[ACTION_SIZE];
    const Board board;

    unsigned int pn = 1;
    unsigned int dn = 1;
    unsigned int parent_num = 1;
    unsigned int depth = 0;
    NodeType type;
};

class PNS{
public:
    ~PNS(){free_states();}
    void search(PNSNode* node);
    void extend(PNSNode* node, const unsigned int action);
    void delete_all(PNSNode* node);
    void delete_node(PNSNode* node);
    unsigned int get_min_children(PNSNode* node, const ProofType type, bool index) const;
    unsigned int get_sum_children(PNSNode* node, const ProofType type) const;

    // === Helper Functions ===
    void log_solution(std::string filename);
    void log_solution_min(PNSNode* node, std::ofstream& file);
    void read_solution(std::string filename);
    void add_state(Board& b, PNSNode* node);
    void free_states();

    inline std::vector<Line_info> get_lines(unsigned int action) const{
        return heuristic.linesinfo_per_field[action];
    }
    
    inline std::vector<Line_info> get_all_lines() const{
        return heuristic.all_linesinfo;
    }

    void stats(PNSNode* node, bool end = false){
        std::cout<<"\rPN: "<<node->pn<<" DN: "<<node->dn<<" States size: "<<states.size()<<std::flush;
        if(end) std::cout<<std::endl;
    }

private :
    std::map<Board, PNSNode*> states;
    Heuristic heuristic;
};