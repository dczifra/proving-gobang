#define DEBUG false


#include "common.h"
#include "heuristic.h"
#include "board.h"
#include "PNS.h"
#include "artic_point.h"
#include "logger.h"
#include "licit.h"
#include "parallel.h"
// =================================================================
//                     TEST THE BOARD's GOODNESS
// =================================================================
Args* args;
int play(Board& b, int player, const Heuristic& heuristic){
    int act;
    while(1){
        //act = b.take_random_action(player);
        if(b.white_win(heuristic.linesinfo_per_field[act])){
            #if DEBUG
                std::cout<<"\nWhite win";
            #endif

            return 1;
        }
        else if(b.black_win()){
            #if DEBUG
                std::cout<<"\nBlack win";
            #endif

            return -1;
        }
        player = -player;
        //display(b, false);
    }
}

/**
 * Description:
 *     2 random player's play, repeated 1000 times
 * */
void random_playes(const Board& basic, int times = 1000){
    Heuristic heuristic;
    auto lines = heuristic.all_linesinfo;
    
    int sum =0;
    for(int i=0;i<times;i++){
        Board b(basic);
        int player = 1;
        sum += (1+play(b,player,heuristic))/2;
        //display(b, true);
    }
    std::cout<<sum<<std::endl;

}

/**
 * Description:
 *     Human player vs random player 
 * */
void human_play(){
    Heuristic heuristic;
    auto lines = heuristic.all_linesinfo;
    
    Board b;
    int act;
    int player = 1;
    while(1){
        if(player == 1){
            std::cin>>act;
            b.move(act, player);
        }
        else{
            //act = b.take_random_action(player);
        }
        if(b.white_win(heuristic.linesinfo_per_field[act])){
            printf("White win\n");
            break;
        }
        else if(b.black_win()){
            printf("Black win\n");
            break;
        }
        player = -player;
        display(b, false);
    }
    display(b, true);
}

// =================================================================
//                     TEST THE BOARD's GOODNESS
// =================================================================

void artic_point(){
    std::vector<int> moves = {0,1,8,10, 29, 24, 25, 30, 5, 2, 17, 21};

    Board b;
    PNS tree(args);

    int player = 1;
    for(auto act: moves){
        b.move(act, player);
        //tree.simplify_board(b);
        //display(b,true);
        player = -player;
    }
    display(b, true);

    //PNSNode* node= new PNSNode(b, tree.heuristic);
    //b = tree.extend(node, 21, false);
    //display(b, true);
    tree.evaluate_components(b);
    
    //Artic_point p(b, tree.heuristic.all_linesinfo, tree.heuristic.linesinfo_per_field);
    //auto comps = p.get_parts();
    //printf("They are the components: %d \n", std::get<0>(comps));
    //display(std::get<1>(comps), true);
    //display(std::get<2>(comps), true);
}

void test_DFPN(){
    Board b;
    Heuristic h;
    int player = 1;
    //choose_problem(b,player);

    PNS tree(args);
    PNSNode* node = new PNSNode(b, args);
    //tree.init_DFPN_search(node);
    
    unsigned int i = 0;
    while(1){
        //tree.DFPN_search(node);
        if(i%10000 == 0){
            tree.stats(node);
        }
        if(node->pn*node->dn==0) break;
        i++;
    }
    tree.stats(node);
}

void canonical_order2(){
    Board b1;
    b1. white = 528401ULL;
    b1.black = 131334ULL;
    b1.move(14, 1);
    b1.move(9, -1);
    Board b2;
    b2.white = 528409ULL;
    b2.black = 131846ULL;

    display(b1, true);
    display(b2, true);

    PNS tree(args);
    tree.isom_machine.get_conversion(b1, b2, tree.heuristic.all_linesinfo);
}

void canonical_order(){
    PNS tree(args);
    Board b1;
    int player = 1;
    std::vector<int> moves = {1,4,8,9, 13, 14, 18, 12, 17, 6, 0, 16, 11, 2, 19, 10};
    for(auto m: moves){b1.move(m, player);player=-player;}
    tree.simplify_board(b1);

    Board b2;
    moves = {0,1,2,3,4,14, 19, 15, 5, 18, 6, 17, 8, 9, 10, 11};
    for(auto m: moves){b2.move(m, player);player=-player;}
    tree.simplify_board(b2);
    b2.black |= ((1ULL)<<12);

    display(b1, true);
    display(b2, true);

    tree.isom_machine.get_conversion(b1, b2, tree.heuristic.all_linesinfo);
}

void get_valids_test(){
    PNS tree(args);
    Board b1;
    Heuristic h;
    int player = 1;
    std::vector<int> moves = {1,4, 2, 5, 3, 6};
    for(auto m: moves){b1.move(m, player);player=-player;}

    //b1.black |= (1ULL << )

    board_int vals = b1.get_valids_without_ondegree(h.all_linesinfo);
    display(vals, true);
}

void test_inner(){
    PNS tree(args);
    Board b;

    Node* node = new PNSNode(b, args);
    tree.extend_all((PNSNode*)node, false);
    for(int i=0;i<node->children.size();i++){
        if(node->children[i]->is_inner()){
            std::cout<<i<<" ";
        }
    }
    std::cout<<std::endl;

    node = node->children[1];
    tree.extend_all((PNSNode*)node, false);

    node = node->children[2];
    tree.extend_all((PNSNode*)node, false);
    display(node->get_board(), true);
    for(int i=0;i<node->children.size();i++){
        if(node->children[i]->is_inner()){
            std::cout<<i<<" ";
        }
    }
    std::cout<<std::endl;
}

void testPNS2(){
    PNS tree(args);
    Board b;
    PNSNode* node = new PNSNode(b, args);
    tree.extend_all(node, false);
    tree.search_and_keep_one_layer(node, false, 100);
    node->children[0]->pn = 20.0;
    tree.PN_search_square(node, false);
}

void print_inner(Node* node){
    if(!node->is_inner()){
        display(node->get_board(), true);
        node->type == OR ? std::cout<<"OR\n":std::cout<<"AND\n";
    }
    else{
        for(int i=0;i<node->children.size();i++){
            if(!node->children[i]->is_inner()) display(node->children[i]->get_board(), true);
            else{
                std::cout<<"=== Begin ===\n";
                print_inner(node->children[i]);
                std::cout<<"=== END ===\n";
            }
            node->children[i]->type == OR ? std::cout<<"OR\n":std::cout<<"AND\n";
        }
    }
}

void test_strategy(){
    PNS tree(args);
    Board b;
    Node* node = new PNSNode(b, args);
    tree.extend_all((PNSNode*)node, false);

    node = node->children[0];
    //print_inner(node);
    node = node->children[1];
    tree.extend_all((PNSNode*)node, false);
    node = node->children[0];
    print_inner(node);
}

void test_extend(){
    PNS tree(args);
    Board b;
    b.white = 72ULL;
    b.black = 281474976710658ULL;
    b.node_type = OR;
    b.score_left = -1;
    b.score_right = 1;
    b.forbidden_all = 362838837166410ULL;
    PNSNode* node0 = new PNSNode(b, args);
    node0->pn = 0;
    node0->dn = var_MAX;
    tree.add_board(b, node0);

    display(b, true);

    Board base;
    Node* node = new PNSNode(base, args);
    tree.extend_all((PNSNode*)node, false);
    node = (PNSNode*)node->children[0]->children[0];
    tree.extend_all((PNSNode*)node, false);
    node = (PNSNode*)node->children[34]->children[0];
    display(node->get_board(), true);

    tree.extend_all((PNSNode*)node, false);
    tree.stats(node, true);

    node = (PNSNode*)node->children[0]->children[1]->children[0];
    std::cout<<node->is_inner()<<std::endl;
    display(node->get_board(), true);
    std::cout<<node->get_board().white<<" "<<node->get_board().black<<" "<<node->get_board().node_type<<" "<<node->get_board().forbidden_all<<"\n";

    Board b1(node->get_board());
    Board b2(node0->get_board());
    std::cout<<( b1 == b2)<<std::endl;
}

void test_extend2(){
    PNS tree(args);
    Board b;
    PNSNode* node = new PNSNode(b, args);
    tree.extend_all(node, false);
    display(node->board, true);

    //for(auto child: node->children){
    //    if(!child->is_inner()) display(child->get_board(), true);
    //}

    node = (PNSNode*) node->children[4];
    display(node->board, true);
    tree.extend_all(node, false);
    //std::cout<<node->children[0]->is_inner()<<std::endl;
    //display(node->board.forbidden_all, true);

    node = (PNSNode*)node->children[0]->children[0];
    display(node->board, true);
    //tree.extend_all((PNSNode*)node, false);

}


void parallel_search(Node* node, PNS& tree, int depth, int maxdepth,
                    std::map<Board, std::pair<int,int>>& ors,
                    std::map<Board, std::pair<int,int>>& ands){
    if(!node->extended) tree.extend_all((PNSNode*) node, false);

    for(int i=0;i<node->children.size();i++){
        Node* child = node->children[i];
        if(child==nullptr) assert(0);

        Board act_board(child->get_board());
        //if(depth==0) display(act_board, true);

        if(child->is_inner() || (child->type == OR && depth < maxdepth)){
            parallel_search(child, tree, depth+1, maxdepth, ors, ands);
            if(act_board.node_type == OR){
                if(ors.find(act_board) != ors.end()) ors[act_board].second+=1;
                else ors[act_board] = {depth,1};
            }
            else{
                if(ands.find(act_board) != ands.end()) ands[act_board].second+=1;
                else ands[act_board] = {depth,1};
            }
        }
        else{
            assert(!child->is_inner());
            if(act_board.node_type == OR){
                if(ors.find(act_board) != ors.end()) ors[act_board].second+=1;
                else ors[act_board] = {depth,1};
            }
            else{
                if(ands.find(act_board) != ands.end()) ands[act_board].second+=1;
                else ands[act_board] = {depth,1};
            }
        }
    }
}


Heuristic PNS::heuristic;
CanonicalOrder PNS::isom_machine;
Logger* PNS::logger = new Logger();
Licit PNS::licit;
board_int Board::base_forbidden = PNS::heuristic.forbidden_all;

int main() {
    args = new Args();
    std::cout<<"### TEST ###"<<std::endl;

    //test_components();
    //test_DFPN();
    //artic_point();
    //canonical_order();
    //canonical_order();
    //PNSNode* node = new PNSNode(b, tree.heuristic);
    //tree.evalueate_node_with_PNS(node, true, false);
    //tree.stats(node);
    //get_valids_test();
    //testPNS2();
    //test_strategy();
    //test_extend2();

    PNS tree(args);
    Board b;
    PNSNode* node = new PNSNode(b, args);

    //read_descendents(node, tree, 0, 2,"data/board_sol");
    //Play::read_solution("data/board_sol/9070970929408_586589453435568_0_0.sol", tree);
    std::cout<<tree.get_states_size()<<std::endl;

    b.white = 38483443844101;
    b.black = 589922348057136;
    display(b, true);

    //print_failed("../fail1.txt");

    return 0;
}
