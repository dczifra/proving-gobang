#include "common.h"
#include "gobanggame.h"


#include <map>

class MCTS{
public:
    MCTS(){

    }

    std::vector<int> get_action_prob(mtx<int>& board, int curPlayer){

    }

    void search(mtx<int>& raw_board, int curPlayer){

    }


private:
    GobangGame game;
    Args args;
    const int ACTION_SIZE = ROW*COL;
    std::map<mtx<int>, std::vector<float>> Qsa;
    std::map<mtx<int>, std::vector<int>> Nsa;
    std::map<mtx<int>, int> Ns;
    std::map<mtx<int>, bool> Es;
    std::map<mtx<int>, float> Vs;
    std::map<mtx<int>, std::vector<float>> Ps;

};