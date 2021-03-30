#include "artic_point.h"

Artic_point::Artic_point(const Board& b, std::vector<Line_info>& all_linesinfo0,
    std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo, int no_lines) : board(b), all_linesinfo(all_linesinfo0), linesinfo_per_field(linesinfo)
{
    empty_nodes = board.get_valids_num();

    parent.resize(ACTION_SIZE, -1);
    depth.resize(ACTION_SIZE, -1);
    low.resize(ACTION_SIZE, -1);
}

Artic_point::Artic_point(const Board& b, std::vector<Line_info>& all_linesinfo0,
    std::array<std::vector<Line_info>, ACTION_SIZE> &linesinfo) : Artic_point(b, all_linesinfo0, linesinfo, -1)
{
    unsigned int line_size = all_linesinfo.size();
    parent_line.resize(line_size, -1);
    depth_line.resize(line_size, -1);
    low_line.resize(line_size, -1);
}

std::pair<int, bool> Artic_point::get_articulation_point_bipartite(int node, int d)
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
        if (line.line_board & board.black)
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

std::pair<int, bool> Artic_point::get_articulation_point_bipartite_line(Line_info &line, int d)
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
        if (!board.is_valid(next_node))
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
                display(board, true, line.points);
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

std::tuple<int, Board, Board> Artic_point::get_parts(){
    Line_info empty_line;
    int node = -1;
    // === Find first empty line ===
    for (auto line : all_linesinfo){
        if (line.line_board & board.black) continue;

        // === For every field on the line ===
        for (auto field : line.points){
            if(board.is_valid(field)){
                node = field;
            }
        }
    }

    if(node == -1){
        Board b(board);
        display(b.white, true);
        display(b.black, true);
        display(b, true);
    }
    assert(node >= 0);
    depth[node] = reach_time;
    low[node] = reach_time;
    ++reach_time;
    ++reached_nodes;

    // === Get first empty line ===
    for(auto line : linesinfo_per_field[node]){
        // If line not active, continue
        if (line.line_board & board.black){
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
                    if(!board.is_valid(n) || n == artic_pair.first) continue;

                    if(depth[n] >= artic_reach_time) comp1 |= ((1ULL)<<n);
                    else comp2 |= ((1ULL)<<n);
                }

                Board b1(board);
                b1.black |= comp2;
                //b1.white &= comp1;
                Board b2(board);
                b2.black |= comp1;
                //b2.white &= comp2;
                //display(comp1, true);
                //display(comp2, true);
                //b1.remove_dead_fields_all(all_linesinfo);
                //b2.remove_dead_fields_all(all_linesinfo);

                if(__builtin_popcountll(comp1) < __builtin_popcountll(comp2)){

                    return std::make_tuple(artic_pair.first, b1, b2);
                }
                else{
                    return std::make_tuple(artic_pair.first, b2, b1);
                }
            }
            else{
                Board b;
                return std::make_tuple(-1, b, b);
                //return std::make_tuple(-1, (0ULL), (0ULL));
            }
            break;
        }
    }
    // === Search for components ===
    Board b;
    return std::make_tuple(-1, b, b);
    //return std::make_tuple(-1, (0ULL), (0ULL));
}
