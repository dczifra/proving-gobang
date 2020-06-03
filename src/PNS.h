#include "common.h"
#include "board.h"
#include "limits.h"
#include <map>

#include "heuristic.h"

enum NodeType : uint8_t {OR, AND};
enum ProofType: uint8_t {PN, DN};


struct PNSNode{

    PNSNode(const Board& b, NodeType t):children(), board(b), type(t){
        unsigned int sum = 0;
        for(int i=0;i<ACTION_SIZE;i++){
            if(b.is_valid(i)) ++sum;
        }

        // === Init proof and disploof number ===
        if(sum == 0){ // Game if over, black wins
            pn = UINT_MAX;
            dn = 0;
        }
        else{
            pn = (t==OR ? 1:sum);
            dn = (t==AND ? 1:sum);
        }
    }

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
    inline std::vector<board_int> lines(unsigned int action){
        return heuristic.compressed_lines_per_action[action];
    }
    bool valid(){return true;}

private :
    std::map<Board, PNSNode*> states;
    Heuristic heuristic;
};