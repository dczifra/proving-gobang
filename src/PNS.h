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
        PNSNode(const Board& b, unsigned int d, int action, double heur_val, Heuristic& h);

        // === DATA ===
        PNSNode* children[ACTION_SIZE];
        const Board board;

        var pn = 1;
        var dn = 1;
        var pn_th = 1;
        var dn_th = 1;

        unsigned int parent_num = 1;
        unsigned int depth = 0;
        NodeType type;

        // === FUNCTIONS ===
        void init_pn_dn(int child_num, double heur_val);
        inline var theta(){ return (type == OR ? pn : dn);}
        inline var delta(){ return (type == OR ? dn : pn);}
        void set_theta(var val){ type == OR ? pn = val : dn=val;}
        void set_delta(var val){ type == OR ? dn = val : pn=val;}


        inline var theta_th(){ return (type == OR ? pn_th : dn_th);}
        inline var delta_th(){ return (type == OR ? dn_th : pn_th);}
        void set_theta_th(var val){ type == OR ? pn_th = val : dn_th=val;}
        void set_delta_th(var val){ type == OR ? dn_th = val : pn_th=val;}
    };

    PNS(){
      component_cut.resize((int)ACTION_SIZE, std::vector<int>((int)ACTION_SIZE, 0));
    }

    ~PNS(){free_states();}
    void PN_search(PNSNode* node, bool fast_eval);
    void DFPN_search(PNSNode* node);
    void init_PN_search(PNSNode* node);
    void init_DFPN_search(PNSNode* node);

    PNSNode* create_and_eval_node(Board& board, int base_depth, bool eval, bool search_in_states);
    void evaluate_node_with_PNS(PNSNode* node, bool log = false, bool fast_eval = false);
    PNSNode* evaluate_components(Board& base_board, const int base_depth);

    Board extend(PNSNode* node, unsigned int action, bool fast_eval);
    void delete_all(PNSNode* node);
    void delete_node(PNSNode* node);
    void delete_children(PNS::PNSNode* node);
    unsigned int get_min_children_index(PNSNode* node, const ProofType type) const;
    var get_min_children(PNSNode* node, const ProofType type) const;
    var get_sum_children(PNSNode* node, const ProofType type) const;

    // === Helper Functions ===
    void log_solution_min(PNSNode* node, std::ofstream& file, std::set<Board>& logged);
    void read_solution(std::string filename);
    void free_states();
    void simplify_board(Board& next_state, const unsigned int action, int depth);
    bool game_ended(const Board& b, int action);
    

    // === DFPN Helper ===
    unsigned int update_threshhold(PNSNode* node);
    var get_min_delta_index(PNSNode* node, int& second) const;

    
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

    void component_stats() {
      std::cout<< "COMPONENT CUT"<<std::endl;
      for (int depth=1; depth <= ACTION_SIZE; depth++) {
        int cnt = 0;
        int gainsum = 0;
        std::cout<<"Depth "<< depth <<": ";
        for (int j=0; j < ACTION_SIZE - depth; j++) {
          int freq = component_cut[ACTION_SIZE-depth][j];
          cnt += freq;
          gainsum += freq * j;
          std::cout<< freq << " ";
        }
        if (cnt > 0) {
            std::cout<<"---> "<< (float) gainsum / (float) cnt << std::endl;
          }
        else {
            std::cout<<"---> "<< 0 << std::endl;
        }
      }
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
    //std::map<std::vector<uint64_t>, PNSNode*> states;
    std::unordered_map<std::vector<uint64_t>, PNSNode*, Vector_Hash> states;
    #else
    std::unordered_map<Board, PNSNode*, Board_Hash> states;
    #endif

    std::vector<std::vector<int>> component_cut;
};
