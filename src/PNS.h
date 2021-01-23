#pragma once
#include "common.h"
#include "board.h"
#include "limits.h"
#include "heuristic.h"
#include "canonicalorder.h"
#include "licit.h"
#include "node.h"

#include <map>
#include <unordered_map>
#include <set>


class Logger;
enum ProofType: uint8_t {PN, DN};

bool operator<(const Board& b1, const Board& b2);

class PNS{
public:
    friend class Play;
    friend class CanonicalOrder;
    friend class Logger;
    friend class Node;
    friend class LicitNode;

    PNS(Args* inp_args){
        component_cut.resize((int)ACTION_SIZE, std::vector<int>((int)ACTION_SIZE, 0));
	    args = inp_args;
    }

    ~PNS(){free_states();}
    void PN_search(Node* node, bool fast_eval);
    void PN_search_square(Node* node, bool fast_eval);
    void DFPN_search(PNSNode* node);
    void init_PN_search(PNSNode* node);
    void init_DFPN_search(PNSNode* node);

    PNSNode* create_and_eval_node(Board& board, bool eval);
    void evaluate_node_with_PNS(PNSNode* node, bool log, bool fast_eval);
    void evaluate_node_with_PNS_square(PNSNode* node, bool log, bool fast_eval);
    PNSNode* evaluate_components(Board& base_board);
    void search_and_keep_one_layer(Node* node, bool fast_eval, int iter = -1);

    void extend_all(PNSNode* node, bool fast_eval);
    void extend(PNSNode* node, unsigned int action, unsigned int slot, bool fast_eval);
    void delete_all(Node* node);
    void delete_node(Node* node);
    void delete_and_log(Node* node);
    //void delete_children(Node* node);
    static unsigned int get_min_children_index(Node* node, const ProofType type);
    static var get_min_children(Node* node, const ProofType type);
    static var get_sum_children(Node* node, const ProofType type);
    static void update_node(Node* node);

    // === Helper Functions ===
    void read_solution(std::string filename);
    void free_states();
    Node* licit_for_defender_move(const Board& next_state, int action);
    static void defender_get_favour_points(Board& next_state, int action);
    static void simplify_board(Board& next_state);
    static bool game_ended(const Board& b);
    static void display_node(PNSNode* node);
    static bool keep_only_one_child(Node* node);


    // === DFPN Helper ===
    unsigned int update_threshhold(PNSNode* node);
    var get_min_delta_index(PNSNode* node, int& second) const;
    
    inline std::vector<Line_info> get_lines(unsigned int action) const {
        return heuristic.linesinfo_per_field[action];
    }
    
    inline std::vector<Line_info> get_all_lines() const{
        return heuristic.all_linesinfo;
    }

    void stats(PNSNode* node, bool end = false);
    void component_stats();
    
    // === MAP ===
    //bool has_board(const Board& board);
    void add_board(const Board& board, PNSNode* node);
    void delete_from_map(const Board& board);
    PNSNode* get_states(const Board& board);

    static Heuristic heuristic;
    static CanonicalOrder isom_machine;
    static Logger* logger;
    static Licit licit;

    int total_state_size = 0;
    
private :
    #if ISOM
    std::unordered_map<std::vector<uint64_t>, PNSNode*, Vector_Hash> states;
    #else
    std::unordered_map<Board, PNSNode*, Board_Hash> states;
    #endif

    Args* args;
    bool print=false;
    std::vector<std::vector<int>> component_cut;
};