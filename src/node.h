#pragma once

#include "common.h"
#include "board.h"

class PNS;
class PNSNode;
class LicitSwitchNode;

class Node{
public:
    Node(int childnum): children(childnum), child_num(childnum){} // abstract class!

    NodeType type;
    var pn = 1;
    var dn = 1;

    std::vector<Node*> children;
    unsigned int parent_num = 1;    
    unsigned int child_num = 0;
    bool extended = false;

    virtual void extend_all()=0; // TODO
    virtual bool is_inner()=0;   //      :D
    virtual Board get_board()=0; // TODO

    // === Helper Functions ===
    int get_licit_limit(PNS* tree, const Board& act_board, int action);
    void handle_collision(PNS* tree, PNSNode*& node, const Board& board);
    PNSNode* get_defender_side(PNS* tree, const Board& act_board, int action);
    PNSNode* add_neighbour_move(PNS* tree, const Board& act_board, int action);
    static void nullify_scores(Board& board);
    static bool is_empty_side(int action, const Board& b);
};

class PNSNode: public Node{
public:
    PNSNode(const Board& b) : Node(0){
        type = b.node_type;
        parent_num = 1;
        heuristic_value = -1;
    }
    PNSNode(const Board& b, Args* args);

    // === DATA ===
    const Board board;

    float heuristic_value;
    
    // === FUNCTIONS ===
    void init_pn_dn();
    virtual void extend_all(){}
    virtual bool is_inner(){ return false;}
    virtual Board get_board(){ return board;}
};

class InnerNode: public Node{
public:
    InnerNode(int childnum, NodeType t);
    virtual void extend_all(){}
    virtual bool is_inner(){ return true;}
    virtual Board get_board(){
        Board b;
        b.white = -1;
        b.black = -1;
        assert(0);
        return b;
    }
};

class AttackerOnForbidden: public InnerNode{
public:
    AttackerOnForbidden(PNS* tree, const Board& act_board, int action);
    virtual void extend_all();
};

class LicitSwitchNode: public InnerNode{
public:
    LicitSwitchNode(PNS* tree, const Board& act_board, int action, int licit_limit);
};

class LicitNode: public InnerNode{
public:
    LicitNode(PNS* tree, const Board& act_board, int action, int licit);
};
