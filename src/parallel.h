#include "common.h"
#include "board.h"
#include "PNS.h"
#include "play.h"

void add_descendents(Node* node, PNS& tree, int depth, int maxdepth,
                    std::map<Board, std::pair<int,int>>& ors,
                    std::map<Board, std::pair<int,int>>& ands){
    if(!node->extended) tree.extend_all((PNSNode*) node, false);

    for(Node* child: node->children){
        if(child==nullptr) assert(0);

        Board act_board(child->get_board());
        // === Search deeper ===
        if(child->is_inner() || (child->type == OR && depth < maxdepth)){
            add_descendents(child, tree, depth+1, maxdepth, ors, ands);
        }
        else{
            assert(!child->is_inner());

            // === Add to the discovered nodes ===
            std::map<Board, std::pair<int,int>>& discovered = (act_board.node_type == OR) ? ors:ands;
            if(discovered.find(act_board) != discovered.end()) discovered[act_board].second+=1;
            else discovered[act_board] = {depth,1};
        }
    }
}

void prove_node(Args& args){
    int depth, times;
    Board b;
    std::cin>>b.white>>b.black>>b.score_left>>b.score_right>>b.node_type;
    std::cin>>b.forbidden_all>>depth>>times;
    
    display(b, true);   
    PNS tree(&args);
    PNSNode* node = new PNSNode(b, &args);

    if(args.PNS_square){
        std::cout<<"PNS2"<<std::endl;
        tree.evaluate_node_with_PNS_square(node, args.log, false);
    }
    else tree.evaluate_node_with_PNS(node, args.log, false);

    PNS::logger->log_node(node, "data/board_sol/"+b.to_string()+".sol");
    tree.stats(node, true);
}

struct Descendents{
    std::string filename;
    std::map<Board, std::pair<int,int>> boards; // Depth and occurance
    Descendents(std::string fname):filename(fname){
        
    }
};

void generate_roots_descendents(Args& args, int depth = 3){
    PNS tree(&args);
    Board b;
    int player = 1;
    Play::choose_problem(b,player,false,&args);
    PNSNode* node = new PNSNode(b, &args);
    //tree.extend_all(node, false);
    //node = (PNSNode*)node->children[0];

    // === Add all decendents above the given depth ===
    Descendents ors("../ors.txt");
    Descendents ands("../ands.txt");
    add_descendents(node, tree, 0, 2, ors.boards, ands.boards);
    std::cout<<ors.boards.size()<<" "<<ands.boards.size()<<" "<<tree.get_states_size()<<std::endl;

    // === Log the descendents ===
    for(auto log: {ors, ands}){
        std::ofstream log_file(log.filename);
        log_file<<"white black common type score_left score_right depth intersection\n";
        for(auto& p: log.boards){
            const Board& b(p.first);
            log_file<<b.white<<" "<<b.black<<" "<<b.score_left<<" "<<b.score_right<<" "<<b.node_type;
            log_file<<" "<<b.forbidden_all<<" "<<p.second.first<<" "<<p.second.second<<std::endl;
        }
        log_file.close();
    }
}
