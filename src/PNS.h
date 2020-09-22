#pragma once
#include "common.h"
#include "board.h"
#include "limits.h"
#include <map>
#include <unordered_map>

#include "heuristic.h"

enum ProofType: uint8_t {PN, DN};

bool operator<(const Board& b1, const Board& b2);
struct Board_Hash{
    std::size_t operator()(Board const& b) const noexcept{
        std::size_t h1 = std::hash<uint64_t>{}(b.white);
        std::size_t h2 = std::hash<uint64_t>{}(b.black);
        std::size_t h3 = std::hash<uint8_t>{}(b.node_type);
        return  h1 ^ h2 ^ h3;
    }
};

class PNS{
public:
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
    void PN_search(PNSNode* node);
    void DFPN_search(PNSNode* node);
    void init_PN_search(PNSNode* node);
    void init_DFPN_search(PNSNode* node);

    void evalueate_node_with_PNS(PNSNode* node, bool log = false);
    void evaluate_components(PNSNode* node);

    void extend(PNSNode* node, unsigned int action);
    void delete_all(PNSNode* node);
    void delete_node(PNSNode* node);
    unsigned int get_min_children(PNSNode* node, const ProofType type, bool index) const;
    unsigned int get_sum_children(PNSNode* node, const ProofType type) const;

    // === Helper Functions ===
    void log_solution(std::string filename);
    void log_solution_min(PNSNode* node, std::ofstream& file);
    void read_solution(std::string filename);
    void add_state(const Board& b, PNSNode* node);
    void add_state(PNS::PNSNode* node);
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
        std::cout<<"\rPN: "<<node->pn<<" DN: "<<node->dn<<" States size: "<<states.size()<<std::flush;
        if(end) std::cout<<std::endl;
    }

    static Heuristic heuristic;
private :
    //std::map<Board, PNSNode*> states;
    std::unordered_map<Board, PNSNode*, Board_Hash> states;
};
