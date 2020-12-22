#include "board.h"
#include "assert.h"

#define REC 1

// ==============================================
//                 EXTRA
// ==============================================
NodeType operator!(const NodeType &type)
{
    return (type == OR ? AND : OR);
}

std::istream &operator>>(std::istream &i, NodeType &type)
{
    unsigned int inp;
    i >> inp;
    type = (inp == 0 ? OR : AND);
    return i;
}

int get_player(const NodeType &type)
{
    return (type == OR ? 1 : -1);
}

bool operator<(const Board &b1, const Board &b2)
{
    return (b1.node_type < b2.node_type) || ((b1.node_type == b2.node_type) && b1.white < b2.white) || ((b1.node_type == b2.node_type) && b1.white == b2.white && b1.black < b2.black);
}

// ==============================================
//                  BOARD ACTIONS
// ==============================================
Board::Board(){
    init();
}
Board::Board(const Board& b){
    white = b.white;
    black = b.black;
    //blocled_line = b.blocked_lines;
    node_type = b.node_type;
}

Board::Board(const Board& b, int action, int player){
    white = b.white;
    black = b.black;
    //blocked_lines = b.blocked_lines;
    node_type = b.node_type;
    move(action, player);
}

bool Board::operator==(const Board& b) const{
    return (white == b.white) && (black == b.black) && node_type == b.node_type; 
}

void Board::move(std::vector<int> actions, int& player){
    for(auto& act: actions){
        move(act, player);
        player = -player;
    }
}

unsigned int Board::find_empty(Line_info& line, int skip_field){
    for(auto field: line.points){
        if(field == skip_field) continue;

        if(!(white & ((1ULL)<<field))){
            return field;
        }
    }

    std::cout<<"Shouldnt be here, line should contain 1 empty field!!!\n";
    assert(0);
}

int Board::get_active_line_num(const std::vector<Line_info> & lines) const{
    int sum = 0;
    for(auto line: lines){
        bool is_free = !(line.line_board & black);
        if(is_free) ++sum;
    }
    return sum;
}

// === GAME OVER FUNCTIONS===
bool Board::white_win(const std::vector<Line_info> & lines) const {
    for(auto line: lines){
        bool blocked = (line.line_board & black);
        if(!blocked && (__builtin_popcountll(line.line_board & white)==line.size)){
            return true;
        }
    }
    return false;
}

int Board::get_winner(const std::vector<Line_info>& lines) const {
    if(white_win(lines)) return 1;
    else if (black_win())
    {
        return -1;
    }
    else return 0;
}

// === TODO with saved constant array ===
bool Board::no_free_lines(const std::vector<Line_info>& all_lines) const{
    for(auto line: all_lines){
        bool is_free = !(line.line_board & black);
        if(is_free) return false;
    }
    return true;
}

board_int Board::get_valids_without_ondegree(const std::vector<Line_info> & all_lines) const{
    std::vector<int> degree(ACTION_SIZE, 0);
    // === Compute degree for all fields ===
    for (auto line : all_lines){
        bool is_free = !(line.line_board & black);
        if (!is_free) continue;
        else{
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            for (int field : line.points){
                // If the line lenght is one, that's still a valid move
                if (emptynum == 1) degree[field] += 2;
                else degree[field]++;
            }
        }
    }
    // === Delete all 1 degree field from valids ===
    board_int valids = ~(white | black) & FULL_BOARD;
    for(int i=0;i<ACTION_SIZE;i++){
        if(white & (1ULL << i)) continue;
        else if(degree[i] == 1){
            valids = valids ^ ((1ULL) << i);
        }
    }

    return valids;
}


bool Board::heuristic_stop(const std::vector<Line_info> &all_lines) const
{
    double sum = 0;
    for (auto line : all_lines)
    {
        bool is_free = !(line.line_board & black);
        if (!is_free)
            continue;
        else
        {
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            sum += std::pow(2.0, -emptynum);
            if (sum >= 1.0)
                return false;
        }
    }

    // The sum is under 1, the game is over
    return true;
}

double Board::heuristic_value(const std::vector<Line_info> &all_lines) const
{
    double sum = 0;
    for (auto line : all_lines)
    {
        bool is_free = !(line.line_board & black);
        if (!is_free)
            continue;
        else
        {
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            sum += std::pow(2.0, -emptynum);
        }
    }

    return sum;
}

std::string Board::heuristic_layers(const std::vector<Line_info>& all_lines) const{
    std::vector<int> layers(LAYERNUM+1, 0);
    for (auto line : all_lines){
        bool is_free = !(line.line_board & black);
        if (!is_free)
            continue;
        else{
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            layers[emptynum]+=1;
        }
    }

    std::string ret = "";
    for(int i=0;i<LAYERNUM+1;i++) ret += std::to_string(layers[i])+" ";
    return ret;
}

void Board::flip()
{
    board_int w = 0, b = 0;
    board_int col = 0x0f;

    for (int i = 0; i < COL; i++)
    {
        board_int old_w = (white & (col << (4 * i)));
        board_int old_b = (black & (col << (4 * i)));
        int move = (COL - 2 * (i + 1) + 1) * 4;
        if (move >= 0)
        {
            w |= (old_w << move);
            b |= (old_b << move);
        }
        else
        {
            move = -move;
            w |= (old_w >> move);
            b |= (old_b >> move);
        }
    }
    white = w;
    black = b;
    //white = static_cast<board_int>(flip_bit(white))>>FLIP_SIZE;
    //black = static_cast<board_int>(flip_bit(black))>>FLIP_SIZE;
}

std::array<float, ACTION_SIZE> Board::heuristic_mtx(const std::vector<Line_info> &lines) const
{
    // Returns a heuristic value for every possible action
    std::array<float, ACTION_SIZE> mtx = {0};

    for (auto line : lines)
    {
        bool is_free = !(line.line_board & black);
        if (!is_free)
            continue;
        else
        {
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            for (int field : line.points)
            {
                mtx[field] += std::pow(2.0, -emptynum);
            }
        }
    }
    return mtx;
}

// ==============================================
//                 SIMPLIFY BOARD
// ==============================================
int Board::one_way(const std::vector<Line_info> &all_lines) const
{
    std::vector<bool> two_line(ACTION_SIZE, 0);

    for (auto line : all_lines)
    {
        bool is_free = !(line.line_board & black);
        if (!is_free)
            continue;
        else
        {
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            if (emptynum >= 3 || (emptynum == 2 && node_type == AND))
                continue;

            for (int field : line.points)
            {
                if (white & (1ULL << field))
                    continue;

                if (emptynum == 1)
                {
                    return field;
                }
                else if (emptynum == 2)
                {
                    //else if(emptynum == 2 && node_type == OR){
                    if (two_line[field])
                        return field;
                    else
                        two_line[field] = true;
                }
            }
        }
    }

    // No obvious action
    return -1;
}

void Board::remove_dead_fields_all(const std::vector<Line_info> &all_line, board_int forbidden){
    std::vector<bool> dead(ACTION_SIZE, true);
    // === For all lines, which cross the action ===
    for (auto line : all_line){
        //  === Skip for not empty line (empty: except action) ===
        if (line.line_board & black) continue;
        // === For every field on the line ===
        for (auto field : line.points){
            dead[field] = false;
        }
    }

    for(int i=0;i<ACTION_SIZE;i++){
        if(dead[i] && ((1ULL << i) & forbidden)==0) set_black(i);
    }
}

void Board::remove_2lines_all(const std::vector<Line_info> &all_line, board_int forbidden){
    std::vector<unsigned int> degree(ACTION_SIZE, 0);

    for (auto line : all_line){
        bool is_free = !(line.line_board & black);
        if(is_free){
            for(auto field : line.points){
                degree[field] += 1;
            }
        }
    }

    bool rerun = false;
    for(auto line : all_line){
        bool is_free = !(line.line_board & black);
        int emptynum = line.size - __builtin_popcountll(line.line_board & white);
        if (is_free && (emptynum == 2)){
            for (auto field : line.points){
                if ((degree[field] == 1) && !(white & (1ULL << field))){
                    int other_empty = find_empty(line, field);
                    // Attacker choses the >= 2 degree field, that cannot be forbidden
                    if(!(forbidden & (1ULL << other_empty)) && !(forbidden & (1ULL << field))){
                        move(other_empty, 1);
                        move(field, -1);
                        rerun = true;
                    }

                }
            }
        }
    }
#ifdef REC
    if (rerun)
        remove_2lines_all(all_line, forbidden);
#endif
}

void Board::remove_lines_with_two_ondegree(const std::vector<Line_info> &all_line, board_int forbidden)
{
    std::vector<unsigned int> degree(ACTION_SIZE, 0);

    for (auto line : all_line){
        bool is_free = !(line.line_board & black);
        if (is_free){
            for (auto field : line.points){
                degree[field] += 1;
            }
        }
    }

    bool rerun = false;
    for (auto line : all_line){
        bool is_free = !(line.line_board & black);
        int emptynum = line.size - __builtin_popcountll(line.line_board & white);
        if (is_free){
            int deg_1 = -1;
            for (auto field : line.points){
                // Cannot be any of the two forbidden ==> attacker will choose that
                if ((degree[field] == 1) && !(white & (1ULL << field)) && !(forbidden & (1ULL << field))){
                    if (deg_1 > -1){
                        move(field, 1);
                        move(deg_1, -1);
                        rerun = true;
                    }
                    else{
                        deg_1 = field;
                    }
                }
            }
        }
    }

#ifdef REC
    if (rerun)
        remove_lines_with_two_ondegree(all_line, forbidden);
#endif
}