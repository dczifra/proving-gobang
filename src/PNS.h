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

class Logger;

class PNS{
public:
    friend class Play;
    friend class CanonicalOrder;
    friend class Logger;
    struct PNSNode{
        PNSNode(const Board& b, Heuristic& h);

        // === DATA ===
        std::vector<PNSNode*> children;
        const Board board;

        var pn = 1;
        var dn = 1;

        bool extended = false;
        float heuristic_value;
        unsigned int child_num;
      
        unsigned int parent_num = 1;
        NodeType type;

        // === FUNCTIONS ===
        void init_pn_dn();
    };

    PNS(){
        component_cut.resize((int)ACTION_SIZE, std::vector<int>((int)ACTION_SIZE, 0));
    }

    ~PNS(){free_states();}
    void PN_search(PNSNode* node, bool fast_eval);
    void DFPN_search(PNSNode* node);
    void init_PN_search(PNSNode* node);
    void init_DFPN_search(PNSNode* node);

    PNSNode* create_and_eval_node(Board& board, bool eval, bool search_in_states);
    void evaluate_node_with_PNS(PNSNode* node, bool log = false, bool fast_eval = false);
    PNSNode* evaluate_components(Board& base_board);

    void extend_all(PNSNode* node, bool fast_eval);
    Board extend(PNSNode* node, unsigned int action, unsigned int slot, bool fast_eval);
    void delete_all(PNSNode* node);
    void delete_node(PNSNode* node);
    void delete_children(PNS::PNSNode* node);
    static unsigned int get_min_children_index(PNSNode* node, const ProofType type);
    var get_min_children(PNSNode* node, const ProofType type) const;
    var get_sum_children(PNSNode* node, const ProofType type) const;
    void update_node(PNSNode* node);

    // === Helper Functions ===
    void read_solution(std::string filename);
    void free_states();
    void simplify_board(Board& next_state);
    bool game_ended(const Board& b);
    void display_node(PNSNode* node);
    void log_child(PNS::PNSNode* node);
    static bool keep_only_one_child(PNSNode* node);


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
private :
    #if ISOM
    std::unordered_map<std::vector<uint64_t>, PNSNode*, Vector_Hash> states;
    #else
    std::unordered_map<Board, PNSNode*, Board_Hash> states;
    #endif

    std::vector<std::vector<int>> component_cut;
};
