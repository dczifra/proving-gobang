#pragma once
#include "common.h"
#include "board.h"
#include "limits.h"
#include "heuristic.h"
#include "canonicalorder.h"

#include <map>
#include <unordered_map>
#include <set>

enum ProofType: uint8_t {PN, DN};

bool operator<(const Board& b1, const Board& b2);

class PNS{
public:
    friend class Play;
    friend class CanonicalOrder;
    struct PNSNode{
        PNSNode(const Board& b, unsigned int d, int action, int heur_val, Heuristic& h);

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
        inline unsigned int theta(){ return (type == OR ? pn : dn);}
        inline unsigned int delta(){ return (type == OR ? dn : pn);}
        void set_theta(unsigned int val){ type == OR ? pn = val : dn=val;}
        void set_delta(unsigned int val){ type == OR ? dn = val : pn=val;}


        inline unsigned int theta_th(){ return (type == OR ? pn_th : dn_th);}
        inline unsigned int delta_th(){ return (type == OR ? dn_th : pn_th);}
        void set_theta_th(unsigned int val){ type == OR ? pn_th = val : dn_th=val;}
        void set_delta_th(unsigned int val){ type == OR ? dn_th = val : pn_th=val;}
    };

    ~PNS(){free_states();}
    void PN_search(PNSNode* node, bool fast_eval);
    void DFPN_search(PNSNode* node);
    void init_PN_search(PNSNode* node);
    void init_DFPN_search(PNSNode* node);

    PNSNode* create_and_eval_node(Board& board, int base_depth, bool eval, bool search_in_states);
    void evalueate_node_with_PNS(PNSNode* node, bool log = false, bool fast_eval = false);
    PNSNode* evaluate_components(Board& base_board, const int base_depth);

    Board extend(PNSNode* node, unsigned int action, bool fast_eval);
    void delete_all(PNSNode* node);
    void delete_node(PNSNode* node);
    void delete_children(PNS::PNSNode* node);
    unsigned int get_min_children(PNSNode* node, const ProofType type, bool index) const;
    unsigned int get_sum_children(PNSNode* node, const ProofType type) const;

    // === Helper Functions ===
    void log_solution_min(PNSNode* node, std::ofstream& file, std::set<Board>& logged);
    void read_solution(std::string filename);
    void free_states();
    void simplify_board(Board& next_state, const unsigned int action, int depth);
    bool game_ended(const Board& b, int action);
    

    // === DFPN Helper ===
    unsigned int update_threshhold(PNSNode* node);
    unsigned int get_min_delta_index(PNSNode* node, int& second) const;

    
    inline std::vector<Line_info> get_lines(unsigned int action) const {
        return heuristic.linesinfo_per_field[action];
    }
    
    inline std::vector<Line_info> get_all_lines() const{
        return heuristic.all_linesinfo;
    }

    void stats(PNSNode* node, bool end = false){
        if(node != nullptr) std::cout<<"\rPN: "<<node->pn<<" DN: "<<node->dn<<" ";
        std::cout<<"States size: "<<states.size()<<"        "<<std::flush;
        if(end) std::cout<<std::endl;
    }

    // === MAP ===
    //bool has_board(const Board& board);
    void add_board(const Board& board, PNSNode* node);
    void delete_from_map(const Board& board);
    PNSNode* get_states(const Board& board);

    static Heuristic heuristic;
    static CanonicalOrder isom_machine;
private :
    //std::map<Board, PNSNode*> states;
    #if ISOM
    std::map<std::vector<uint64_t>, PNSNode*> states;
    #else
    std::unordered_map<Board, PNSNode*, Board_Hash> states;
    #endif
};
