#pragma once
#include "common.h"
#include "board.h"
#include "limits.h"
#include <map>

#include "heuristic.h"

enum ProofType: uint8_t {PN, DN};

struct PNSNode{
    PNSNode(const Board& b, unsigned int d, int heur_val);

    // === DATA ===
    PNSNode* children[ACTION_SIZE];
    const Board board;

    unsigned int pn = 1;
    unsigned int dn = 1;
    unsigned int pn_th = 1;
    unsigned int dn_th = 1;

    unsigned int parent_num = 1;
    unsigned int depth = 0;
    NodeType type;

    // === FUNCTIONS ===
    inline unsigned int& theta(){ type == OR ? pn : dn;}
    inline unsigned int& delta(){ type == OR ? dn : pn;}
    inline unsigned int& set_theta(unsigned int val){ type == OR ? pn = val : dn=val;}
    inline unsigned int& set_delta(unsigned int val){ type == OR ? dn = val : pn=val;}


    inline unsigned int& theta_th(){ type == OR ? pn_th : dn_th;}
    inline unsigned int& delta_th(){ type == OR ? dn_th : pn_th;}
    inline unsigned int& set_theta_th(unsigned int val){ type == OR ? pn_th = val : dn_th=val;}
    inline unsigned int& set_delta_th(unsigned int val){ type == OR ? dn_th = val : pn_th=val;}
};

class PNS{
public:
    ~PNS(){free_states();}
    void PN_search(PNSNode* node);
    void DFPN_search(PNSNode* node);
    void init_PN_search(PNSNode* node);
    void init_DFPN_search(PNSNode* node);

    void extend(PNSNode* node, const unsigned int action);
    void delete_all(PNSNode* node);
    void delete_node(PNSNode* node);
    unsigned int get_min_children(PNSNode* node, const ProofType type, bool index) const;
    unsigned int get_sum_children(PNSNode* node, const ProofType type) const;

    // === Helper Functions ===
    void log_solution(std::string filename);
    void log_solution_min(PNSNode* node, std::ofstream& file);
    void read_solution(std::string filename);
    void add_state(const Board& b, PNSNode* node);
    void free_states();
    void simplify_board(Board& next_state, const unsigned int action);

    // === DFPN Helper ===
    unsigned int update_threshhold(PNSNode* node);
    unsigned int get_min_delta_index(PNSNode* node, int& second) const;

    
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
