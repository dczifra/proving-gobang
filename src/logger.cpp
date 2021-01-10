#include "logger.h"
#include "counter.h"
#include "common.h"

void Logger::init(bool disproof){
    #if LOG
    std::string folder = (disproof ? "../logs/disproof_" : "../logs/proof_");
    std::string filename =  std::to_string(ROW)+"x"+std::to_string(COL)+".csv";
    logstream.open(folder + filename);
    logstream<<"white black current_player pn dn empty_cells potential node_count l0 l1 l2 l3 l4 l5 l6 l7"<<std::endl;
    #endif
}

void Logger::log(PNS::Node* base_node, Heuristic& h){
    PNS::PNSNode* node = dynamic_cast<PNS::PNSNode*>(base_node);
    if(node == nullptr) return;

    if(logged_states.find(node->board) != logged_states.end()){
        return;
    }
    else{
        logged_states[node->board] = true;
    }

    Counter counter;

    logstream<<node->board.white<<" "<<node->board.black<<" "<<node->board.node_type<<" ";
    logstream<<node->pn<<" "<<node->dn<<" ";
    logstream<<node->board.get_valids_num()<<" ";
    logstream<<node->heuristic_value<<" ";
    logstream<<counter.count_nodes(node)<<" ";
    logstream<<node->board.heuristic_layers(h.all_linesinfo);
    logstream<<std::endl;
}

void Logger::log_solution_min(PNS::Node* node, std::ofstream& file, std::set<Board>& logged){
    PNS::PNSNode* heur_node = dynamic_cast<PNS::PNSNode*>(node);
    if(node == nullptr) return;
    else if(heur_node == nullptr || logged.find(heur_node->board) == logged.end()){
        if(heur_node != nullptr){
            logged.insert(heur_node->board);
            file<<heur_node->board.white<<" "<<heur_node->board.black<<" "<<heur_node->board.node_type<<" "<<node->pn<<" "<<node->dn<<std::endl;
        }

        if(PNS::keep_only_one_child(node)){
            ProofType proof_type = (node->pn == 0 ? PN:DN);
            unsigned int min_ind = PNS::get_min_children_index(node, proof_type);
            if (min_ind == UINT_MAX) return;
            else log_solution_min(node->children[min_ind], file, logged);
        }
        else{
            for(int i=0;i<node->child_num;i++){
                log_solution_min(node->children[i], file, logged);
            }
        }
    }
}
