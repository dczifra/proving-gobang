#include "board.h"
#include "assert.h"

#define REC 1

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

//bool operator<(const Board& b1, const Board& b2) {
//    return (b1.white<b2.white) || (b1.white == b2.white && b1.black<b2.black);
//}

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

// === Policy ===
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

double Board::heuristic_val(const std::vector<Line_info> &lines) const
{
    double sum = 0.0;
    for (auto line : lines)
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

// ==============================================
//                 REMOVE DEAD 2 LINE
// ==============================================
void Board::remove_dead_fields_line(const Line_info &line, const std::vector<unsigned int> &field_linesum)
{
    for (auto field : line.points)
    {
        if (field_linesum[field] == 1)
        {
            set_black(field);
        }
    }
}

void Board::remove_dead_fields_all(const std::vector<Line_info> &all_line)
{
    std::vector<bool> dead(ACTION_SIZE, true);
    // === For all lines, which cross the action ===
    for (auto line : all_line)
    {
        //  === Skip for not empty line (empty: except action) ===
        if (line.line_board & black) continue;

        // === For every field on the line ===
        for (auto field : line.points)
        {
            dead[field] = false;
        }
    }

    for(int i=0;i<ACTION_SIZE;i++){
        if(dead[i]) set_black(i);
    }
}

void Board::remove_dead_fields(const std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo_per_field,
                               const int action)
{
    // === For all lines, which cross the action ===
    for (auto line : linesinfo_per_field[action])
    {
        //  === Skip for not empty line (empty: except action) ===
        board_int new_black = (black ^ (1ULL) << action);
        if (line.line_board & new_black)
        { // Line not empty
            continue;
        }

        // === For every field on the line ===
        for (auto field : line.points)
        {
            // === If field not empty, continue
            //if((black & ((1ULL) << field)) || (white & ((1ULL) << field))) continue;
            if (black & ((1ULL) << field))
                continue;

            unsigned int free_lines = 0;
            for (auto side_line : linesinfo_per_field[field])
            {
                bool is_free = !(side_line.line_board & black);
                if (is_free)
                    free_lines++;
            }
            if (free_lines == 0)
            {
                set_black(field);
            }
        }
    }
}

void Board::remove_2lines_all(const std::vector<Line_info> &all_line)
{
    std::vector<unsigned int> free_num(ACTION_SIZE, 0);

    for (auto line : all_line)
    {
        bool is_free = !(line.line_board & black);
        if (is_free)
        {
            for (auto field : line.points)
            {
                free_num[field] += 1;
            }
        }
    }

    bool rerun = false;
    for (auto line : all_line)
    {
        bool is_free = !(line.line_board & black);
        int emptynum = line.size - __builtin_popcountll(line.line_board & white);
        if (is_free && (emptynum == 2))
        {
            for (auto field : line.points)
            {
                if ((free_num[field] == 1) && !(white & (1ULL << field)))
                {
                    int other_empty = find_empty(line, field);
                    move(other_empty, 1);
                    move(field, -1);
                    remove_dead_fields_line(line, free_num);
                    rerun = true;
                    //remove_2lines_all(all_line);
                    //return;
                }
            }
        }
    }
#ifdef REC
    if (rerun)
        remove_2lines_all(all_line);
//remove_2lines_all(all_line);
#endif
}

void Board::remove_2lines(const std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo_per_field,
                          const int action)
{
    // === For all lines, which cross the action ===
    for (auto line : linesinfo_per_field[action])
    {
        // === If the line was dead before "action", continue ===
        board_int new_black = (black ^ (1ULL) << action);
        if (line.line_board & new_black)
        { // Line not empty
            continue;
        }

        for (auto field : line.points)
        {
            if ((white & (1ULL << field)))
                continue;

            unsigned int free_lines = 0;
            int emptynum = 0;
            Line_info act_line;
            for (auto side_line : linesinfo_per_field[field])
            {
                bool is_free = !(side_line.line_board & black);
                if (is_free)
                {
                    emptynum = side_line.size - __builtin_popcountll(side_line.line_board & white);
                    free_lines++;
                    act_line = side_line;
                }
            }

            if (free_lines == 1 && (emptynum == 2))
            {
                // Delete field
                // Find other and move there one step, and call remove_2_lines
                int other_empty = find_empty(act_line, field);
                move(other_empty, 1);
                move(field, -1);
                remove_dead_fields(linesinfo_per_field, field);
                remove_2lines(linesinfo_per_field, other_empty);
            }
        }
    }
}

// ==============================================
//       REMOVE LINES WITH 2 1-DEGREE NODES
// ==============================================
void Board::remove_lines_with_two_ondegree(const std::vector<Line_info> &all_line)
{
    std::vector<unsigned int> free_num(ACTION_SIZE, 0);

    for (auto line : all_line)
    {
        bool is_free = !(line.line_board & black);
        if (is_free)
        {
            for (auto field : line.points)
            {
                free_num[field] += 1;
            }
        }
    }

    bool rerun = false;
    for (auto line : all_line)
    {
        bool is_free = !(line.line_board & black);
        int emptynum = line.size - __builtin_popcountll(line.line_board & white);
        if (is_free)
        {
            int deg_1 = -1;
            for (auto field : line.points)
            {
                if ((free_num[field] == 1) && !(white & (1ULL << field)))
                {
                    if (deg_1 > -1)
                    {
                        move(field, 1);
                        move(deg_1, -1);
                        remove_dead_fields_line(line, free_num);
                        rerun = true;
                    }
                    else
                    {
                        deg_1 = field;
                    }
                }
            }
        }
    }

#ifdef REC
    if (rerun)
        remove_lines_with_two_ondegree(all_line);
#endif
}

// ==============================================
//                STEP INTO COMP
// ==============================================
void Board::start_search(std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo_per_field, std::vector<int> &status, int from)
{
    status[from] = 1;
    for (auto line : linesinfo_per_field[from])
    {
        if (line.line_board & black)
            continue;

        for (auto field : line.points)
        {
            if (status[field] == -1)
            {
                if (is_valid(field))
                    start_search(linesinfo_per_field, status, field);
                else if (white & ((1ULL) << field))
                {
                    status[field] = 0;
                }
            }
        }
    }
}

void Board::keep_comp(std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo_per_field, int action)
{
    std::vector<int> status(ACTION_SIZE, -1); // -1 undiscovered 0 discoverd 1 processed

    start_search(linesinfo_per_field, status, action);
    for (int i = 0; i < ACTION_SIZE; i++)
    {
        if (status[i] >= 0)
            continue;
        else
        {
            // === Delete from white board ===
            if ((white & ((1ULL) << i)) > 0)
            {
                white = white ^ ((1ULL) << i);
            }
            black |= ((1ULL) << i);
        }
    }
}

// ==============================================
//               SPLIT TO COMPONENTS
// ==============================================
bool valid_action(int action)
{
    return action >= 0 && action < ACTION_SIZE;
}

void get_component(const std::vector<std::array<bool, 11>> &adjacent_nodes,
                   std::vector<int> &node_component,
                   int start, int act_component)
{
    for (int dir = 0; dir < 11; dir++)
    {
        int neigh = start + dir - 5; // dir = node - neigh + 5
        if (adjacent_nodes[start][dir] && node_component[neigh] == -1)
        {
            node_component[neigh] = act_component;
            get_component(adjacent_nodes, node_component, neigh, act_component);
        }
    }
}

std::vector<double> get_component_sum(const std::vector<Line_info> &all_lines,
                                      const std::vector<int> &emptynum_in_line,
                                      const std::vector<int> &first_field_in_line,
                                      const std::vector<int> &node_component,
                                      const int num_component)
{
    std::vector<double> component_sum(num_component);
    for (int i = 0; i < all_lines.size(); i++)
    {
        int act_comp = node_component[first_field_in_line[i]];
        int empty_num = emptynum_in_line[i];
        if (empty_num > -1)
        {
            component_sum[act_comp] += std::pow(2.0, -empty_num);
        }
    }
    return component_sum;
}

std::vector<int> Board::get_all_components(const std::vector<std::array<bool, 11>> &adjacent_nodes, const std::vector<bool> &free_node, int &num_component)
{
    std::vector<int> node_component(ACTION_SIZE, -1);
    // === Get components ===
    for (int ind = 0; ind < ACTION_SIZE; ind++)
    {
        if (!free_node[ind] || node_component[ind] != -1 || !is_valid(ind))
            continue;
        else
        {
            node_component[ind] = num_component;
            get_component(adjacent_nodes, node_component, ind, num_component);
            num_component++;
        }
    }
    return node_component;
}

void Board::get_fields_and_lines(const std::vector<Line_info> &all_lines,
                                 std::vector<int> &emptynum_in_line,
                                 std::vector<int> &first_field_in_line,
                                 std::vector<bool> &free_node,
                                 std::vector<std::array<bool, 11>> &adjacent_nodes)
{
    // ======== ADJACENCY TABLE ==========
    // node1-node2 : if node1 > node2 [+5]
    // -5 -1  3       0  4  8
    // -4  #  4  ==>  1  #  9
    // -3  1  5       2  6  10black |= ((1ULL) << i);

    int iter = 0;
    // === Iterate on lines ===
    for (auto line : all_lines)
    {
        bool is_free = !(line.line_board & black);

        if (!is_free)
        {
            emptynum_in_line[iter] = -1;
        }
        else
        {
            // === This line is free, update it's fileds
            int emptynum = line.size - __builtin_popcountll(line.line_board & white);
            emptynum_in_line[iter] = emptynum;
            first_field_in_line[iter] = line.points[0];

            free_node[line.points[0]] = true;
            for (int i = 1; i < line.points.size(); i++)
            { // We doesn't matter with lines with length 1
                int act = line.points[i];
                int prev = line.points[i - 1];
                free_node[act] = true;

                int direction = prev - act + 5; // Only for 4xn table
                adjacent_nodes[act][direction] = true;
                direction = act - prev + 5; // Only for 4xn table
                adjacent_nodes[prev][direction] = true;
            }
        }
        iter += 1;
    }
}

void Board::remove_small_components(const std::vector<Line_info> &all_lines)
{
    std::vector<int> emptynum_in_line(all_lines.size());        // -1 if not empty
    std::vector<int> first_field_in_line(all_lines.size(), -1); // -1 if ther is none
    std::vector<bool> free_node(ACTION_SIZE, 0);
    std::vector<std::array<bool, 11>> adjacent_nodes(ACTION_SIZE); // 3 and 7 unused

    // === INIT line and field features ===
    get_fields_and_lines(all_lines, emptynum_in_line, first_field_in_line, free_node, adjacent_nodes);

    // === Split to components ===
    int num_component = 0;
    std::vector<int> node_component = get_all_components(adjacent_nodes, free_node, num_component);

    // === SUM lines on components ===
    std::vector<double> component_sum = get_component_sum(all_lines, emptynum_in_line, first_field_in_line, node_component, num_component);

    // === Delete small components ===
    for (unsigned int i = 0; i < ACTION_SIZE; i++)
    {
        int act_component = node_component[i];
        // === If small component, make all values black ===
        if (act_component >= 0 && component_sum[act_component] >= 1.0)
        {
            // filed is in a big component
        }
        else
        {
            if ((white & ((1ULL) << i)) > 0)
            {
                white = white ^ ((1ULL) << i);
            }
            //move(i,-1);
            black |= ((1ULL) << i);
        }
    }

    if (0)
    {
        // === Print Comp ===
        for (int i = 0; i < ROW; i++)
        {
            for (int j = 0; j < COL; j++)
            {
                std::cout << node_component[j * ROW + i] << " ";
            }
            std::cout << std::endl;
        }

        for (auto comp_size : component_sum)
        {
            std::cout << comp_size << " ";
        }
        std::cout << std::endl;
    }
}

// =======================================
//             ARTICULATION POINT
// =======================================
void Board::get_one_artic_point(Heuristic &h)
{
    Board b(*this);
    for (int act = 0; act < ACTION_SIZE; act++)
    {
        if (is_valid(act))
        {
            /*std::vector<int> parent(ACTION_SIZE, -1);
            std::vector<int> depth(ACTION_SIZE, -1);
            std::vector<int> low(ACTION_SIZE, -1);

            int artic = get_articulation_point(act, 0, parent, depth, low, h.linesinfo_per_field);
            if(artic > -1){
                display(*this, true, {artic});
                std::cout<<artic<<std::endl;
                //print_v(depth);
                //print_v(low);
                return;
            }*/
            Artic_point p(&b, h.all_linesinfo, h.linesinfo_per_field);
            auto mypair = p.get_articulation_point_bipartite(2, 0);
            std::cout << mypair.first << " " << mypair.second << std::endl;
        }
    }
}

int Board::get_articulation_point(int node, int d,
                                  std::vector<int> &parent, std::vector<int> &depth, std::vector<int> &low,
                                  std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo_per_field) const
{
    /**
     * Description:
     *     Returns: the first discovered acticulation point: cut point of a 2
     *              connected component
     * Params:
     *     parent: init to -1
     *     depth: init to -1, because it is used also as visited indicator
     * */
    depth[node] = d;
    low[node] = d;
    unsigned int child_num = 0;
    bool is_articulation = false;

    for (auto line : linesinfo_per_field[node])
    {
        // If line not active, continue
        if (line.line_board & black)
            continue;

        for (auto next_node : line.points)
        {
            // If field is not valid, continue
            if (!is_valid(next_node))
                continue;

            if (depth[next_node] == -1)
            {
                parent[next_node] = node;
                int artic_point = get_articulation_point(next_node, d + 1, parent, depth, low, linesinfo_per_field);
                if (artic_point > -1)
                    return artic_point;

                child_num++;
                if (low[next_node] >= depth[node])
                {
                    is_articulation = true;
                    //std::cout<<"Artic point: "<<node<<std::endl;
                }
                low[node] = std::min(low[node], low[next_node]);
            }
            else if (next_node != parent[node])
            {
                low[node] = std::min(low[node], depth[next_node]);
            }
        }
    }

    if (is_articulation && (parent[node] != -1 || child_num > 1))
    {
        return node;
    }
    else
        return -1;
}

std::pair<int, bool> Board::Artic_point::get_articulation_point_bipartite(int node, int d)
{
    /**
     * Description:
     *     Returns: the first discovered acticulation point: cut point of a 2
     *              connected component
     * Params:
     *     parent: init to -1
     *     depth: init to -1, because it is used also as visited indicator
     * */
    depth[node] = reach_time;
    low[node] = reach_time;
    ++reach_time;
    ++reached_nodes;

    for (auto line : linesinfo_per_field[node])
    {
        // If line not active, continue
        if (line.line_board & board->black)
            continue;

        const unsigned int line_index = line.index;
        if (depth_line[line_index] == -1)
        {
            parent_line[line_index] = node;
            std::pair<int, bool> artic_info = get_articulation_point_bipartite_line(line, d + 1);
            if (artic_info.first > -1)
                return artic_info;

            if (low_line[line_index] >= depth[node])
            {
                //std::cout << "Artic point: " << node << std::endl;
                return {node, false};
            }
            low[node] = std::min(low[node], low_line[line_index]);
        }
        else if (line_index != parent[node])
        {
            low[node] = std::min(low[node], depth_line[line_index]);
        }
    }

    return {-1, false};
}

std::pair<int, bool> Board::Artic_point::get_articulation_point_bipartite_line(Line_info &line, int d)
{
    /**
     * Description:
     *     Returns: the first discovered acticulation point: cut point of a 2
     *              connected component
     * Params:
     *     parent: init to -1
     *     depth: init to -1, because it is used also as visited indicator
     * */
    const unsigned int line_index = line.index;
    depth_line[line_index] = reach_time;
    low_line[line_index] = reach_time;
    ++reach_time;

    for (auto next_node : (line.points))
    {
        // If field is not valid, continue
        if (!board->is_valid(next_node))
            continue;

        if (depth[next_node] == -1)
        {
            parent[next_node] = line_index;
            std::pair<int, bool> artic_info = get_articulation_point_bipartite(next_node, d + 1);
            if (artic_info.first > -1)
                return artic_info;

            if ((low[next_node] >= depth_line[line_index]) && 0)
            {
                std::cout << "Artic line: " << line_index << std::endl;
                display(*board, true, line.points);
                return {line_index, true};
            }
            low_line[line_index] = std::min(low_line[line_index], low[next_node]);
        }
        else if (next_node != parent_line[line_index])
        {
            low_line[line_index] = std::min(low_line[line_index], depth[next_node]);
        }
    }

    return {-1, true};
}

Board::Artic_point::Artic_point(const Board *b, std::vector<Line_info>& all_linesinfo0,
    std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo, int no_lines) : board(b), all_linesinfo(all_linesinfo0), linesinfo_per_field(linesinfo)
{
    empty_nodes = __builtin_popcountll(board->get_valids());

    parent.resize(ACTION_SIZE, -1);
    depth.resize(ACTION_SIZE, -1);
    low.resize(ACTION_SIZE, -1);
}

Board::Artic_point::Artic_point(const Board *b, std::vector<Line_info>& all_linesinfo0,
    std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo) : Artic_point(b, all_linesinfo0, linesinfo, -1)
{
    unsigned int line_size = all_linesinfo.size();
    parent_line.resize(line_size, -1);
    depth_line.resize(line_size, -1);
    low_line.resize(line_size, -1);
}

std::tuple<int, board_int, board_int> Board::Artic_point::get_parts(){
    Line_info empty_line;
    int node = -1;
    // === Find first empty line ===
    for (auto line : all_linesinfo){
        if (line.line_board & board->black) continue;

        // === For every field on the line ===
        for (auto field : line.points)
        {
            if(board->is_valid(field)){
                node = field;
            }
        }
    }

    if(node == -1){
        display(*board, true);
    }
    assert(node >= 0);
    depth[node] = reach_time;
    low[node] = reach_time;
    ++reach_time;
    ++reached_nodes;

    // === Get first empty line ===
    for(auto line : linesinfo_per_field[node]){
        // If line not active, continue
        if (line.line_board & board->black){
            continue;
        }
        else{
            auto artic_pair = get_articulation_point_bipartite_line(line, 1);
            
            if(artic_pair.first == -1 && reached_nodes < empty_nodes)
                artic_pair.first = node;

            if(artic_pair.first > -1){
                board_int comp1 = 0;
                board_int comp2 = 0;

                int artic_reach_time = depth[artic_pair.first];

                for(int n=0;n<ACTION_SIZE;n++){
                    if(!board->is_valid(n) || n == artic_pair.first) continue;

                    if(depth[n] >= artic_reach_time) comp1 |= ((1ULL)<<n);
                    else comp2 |= ((1ULL)<<n);
                }
                if(__builtin_popcountll(comp1) < __builtin_popcountll(comp2)){
                    return std::make_tuple(artic_pair.first, comp1, comp2);
                }
                else{
                    return std::make_tuple(artic_pair.first, comp2, comp1);
                }
            }
            else{
                return std::make_tuple(-1, (0ULL), (0ULL));
            }
            break;
        }
    }
    // === Search for components ===
}
