#include "common.h"

#include <map>

struct Args{
    int simulationNum = 10000;
};

class MCTS{
public:
    MCTS(){

    }

    std::vector<int> get_action_prob(mtx<int>& board, int curPlayer){
        std::vector<int> v;
        return v;
    }

    double search(mtx<int>& raw_board, int curPlayer, int round){
        auto index = Es.find(raw_board);
        if(index==Es.end()){
            Es[raw_board]=heuristic.getGameEnded(raw_board, curPlayer, round);
            index=Es.find(raw_board);
        }

        if(index->second!=0){
            return -(index->second);
        }

        if(Ps.find(raw_board)==Ps.end()){

        }

    }


private:
    Args args;
    const int ACTION_SIZE = ROW*COL;
    std::map<mtx<int>, std::vector<float>> Qsa;
    std::map<mtx<int>, std::vector<int>> Nsa;
    std::map<mtx<int>, int> Ns;
    std::map<mtx<int>, int> Es;
    std::map<mtx<int>, float> Vs;
    std::map<mtx<int>, std::vector<float>> Ps;
    Heuristic heuristic;

};