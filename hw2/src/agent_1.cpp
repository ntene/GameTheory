/*
 * \file baseline.cpp
 * \brief baseline agents
 * \author Maria Elsa (elsa)
 * \course Theory of Computer Game (TCG)
 */

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <time.h>
#include <math.h>
#include <cstring>
#include <chrono>

#include "einstein.hpp"

inline void flip_bit(bool &x) { x = !x; }

std::fstream flog;

void logger(std::string logfile) {
    flog.open(logfile, std::fstream::out);
    if (!flog.is_open()) {
        std::runtime_error("Error opening file\n");
    }
}

#define MAXNODES 500000
#define C 1.18
#define C1 1.18
#define C2 0.01
#define parent(ptr) (nodes[ptr].p_id)
#define child(ptr, i) (nodes[ptr].c_id[i])
#define min(a, b) (a < b ? a : b)
#define Rd 2
#define Sd_e 0.2

struct Node
{
    Move ply;
    int p_id;
    int c_id[MAX_MOVES];
    int depth;
    int Nchild;
    int Ntotal;
    double sqrtlogNtotal;
    double sqrtNtotal;
    double Scores;
    double Sum2;
    double Average;
    double Variance;
    double sd;
    bool pruned = false;
    State state;
} nodes[MAXNODES];


double UCB(int id){
    double range = 14.5;
    double SR = nodes[id].Average / range;
    double Vi = nodes[id].Variance + (C1 * nodes[parent(id)].sqrtlogNtotal) / nodes[id].sqrtNtotal ;
    double temp =  C * (nodes[parent(id)].sqrtlogNtotal / nodes[id].sqrtNtotal) * min(Vi, C2);
    return (nodes[id].depth%2) ?  (SR + temp) : (1.0 - SR + temp);
}

void update_score(int s_now, double deltaS, double deltaS2, int deltaN, int current){
    while(1)
    {
        nodes[s_now].Ntotal += deltaN;
        nodes[s_now].sqrtNtotal = sqrt((double) nodes[s_now].Ntotal); 
        nodes[s_now].sqrtlogNtotal = sqrt(log((double) nodes[s_now].Ntotal));
        nodes[s_now].Scores += deltaS;
        nodes[s_now].Sum2 += deltaS2;
        nodes[s_now].Average = nodes[s_now].Scores / (double) nodes[s_now].Ntotal;
        nodes[s_now].Variance = (nodes[s_now].Sum2 - (nodes[s_now].Scores*nodes[s_now].Scores/nodes[s_now].Ntotal))/nodes[s_now].Ntotal;
        nodes[s_now].sd = sqrt(nodes[s_now].Variance);
        if (s_now == current)
            break;
        s_now = parent(s_now);     
        //flog << "update score " << s_now << " current "  << current << std::endl;
    }
}

double calculate_scores(Board sb){
    double scores = 0;
    Color won = sb.who_won() == RED ? RED : BLUE;
    scores += (sb.num_pieces[won] - sb.num_pieces[~won]);
    if (sb.num_pieces[won] - sb.num_pieces[~won] == 0){
        if (won == RED && sb.board[0] != NO_PIECE)
            scores += (sb.board[35] - sb.board[0]);
        else if(won == BLUE && sb.board[35] != NO_PIECE)
            scores += (sb.board[0] - sb.board[35]);
        //scores += won == RED ? sb.board[35] : sb.board[0];
    }
    else{
        scores += 6;
    }
    int avg_pc = 0;
    for(int k = 0; k < 36; k++){
        if (sb.board[k] != NO_PIECE){
            if (color_of(sb.board[k]) == won){
                avg_pc += type_of(sb.board[k]);
            }
        }
    }
    scores += double (avg_pc / sb.num_pieces[won]);
    //flog << "calculate_scores " << std::endl;
    return scores;
}

int main() {
    logger(".log.agenta");
    Board b;
    Move move;
    bool myTurn;
    char start, num, dir;
    std::string move_str;
    char init[2][PIECE_TYPE_NB + 1];

    // Initialize random seed
    srand(time(NULL));
    do {
        // Get initial positions
        for (int i = 0; i < COLOR_NB; ++i) {
            for (int j = 0; j < PIECE_TYPE_NB; ++j) {
                init[i][j] = getchar();
            }
        }

        init[0][PIECE_TYPE_NB] = init[1][PIECE_TYPE_NB] = '\0';
        start = getchar();

        //flog << init[0] << " " << init[1] << std::endl;
        //flog << start << std::endl;
        b.init(init[0], init[1]);

        memset(&nodes, 0, sizeof nodes);
        int count = 1;
        int current = 0;
        int pruned = 0;
        nodes[0].state = b.copy_board();

        //flog << b;

        for (myTurn = (start == 'f'); !b.is_terminal(); flip_bit(myTurn)) {
            if (myTurn) {

                //start UCT
                //flog << "current : " << current << std::endl;
                //flog << "count " << count << std::endl;
                /*for(int i = 0; i < nodes[current].Nchild; i++){
                    flog << "^ " << nodes[current].c_id[i] << std::endl;
                }*/
                int Simulations = 0;
                while(Simulations < 300000){
                    //select
                    //flog << "simulation start " << std::endl;
                    if (count > MAXNODES - 19){
                        State temp = nodes[current].state;
                        memset(&nodes, 0, sizeof nodes);
                        current = 0;
                        count = 1;
                        nodes[current].state = temp;
                        //flog << "trigger max nodes" << std::endl;
                    }
                    int ptr = current;
                    while(nodes[ptr].Nchild > 0){
                        int maxchild = child(ptr, 0);
                        double maxV = UCB(maxchild);
                        for(int i = 1; i < nodes[ptr].Nchild; i++){
                            int ctemp = child(ptr, i);
                            double temp = UCB(ctemp);
                            if (temp > maxV && !nodes[ctemp].pruned){
                                maxV = temp;
                                maxchild = ctemp;
                            }

                        }
                        ptr = maxchild;
                        //flog << "---> ptr " << ptr << std::endl;
                    }
                    //expansion
                    bool jump = false;
                    if(nodes[ptr].Nchild == 0){
                        MoveList statemL;
                        Board state_board;
                        state_board.copy_state(nodes[ptr].state);
                        if (state_board.is_terminal()){
                            jump = true;
                            int deltaN = 30;
                            Simulations += deltaN;

                            double scores = calculate_scores(state_board);
                            update_score(ptr, deltaN*scores, deltaN * scores * scores, deltaN, current);
                            //flog << "this is why" << std::endl;
                        }
                        else{
                            int state_mL_size;
                            if (state_board.side_to_move() == RED){
                                state_mL_size = state_board.legal_actions<RED>(statemL);
                            }
                            else{
                                state_mL_size = state_board.legal_actions<BLUE>(statemL);
                            }
                            for(int i = 0; i < state_mL_size; i++){
                                state_board.copy_state(nodes[ptr].state);
                                nodes[count].p_id = ptr;
                                nodes[count].ply = statemL[i];
                                nodes[count].depth = nodes[ptr].depth + 1;
                                state_board.do_move(statemL[i]);
                                state_board.update_status();
                                //flog << "test break , current"<< current << std::endl;
                                nodes[count].state = state_board.copy_board();
                                nodes[ptr].c_id[i] = count;
                                count++;
                            }
                            nodes[ptr].Nchild = state_mL_size;
                        }
                    }

                    if (!jump){
                        //simulation
                        for(int i = 0; i < nodes[ptr].Nchild; i++){
                            double deltaS = 0;
                            double deltaS2 = 0;
                            int deltaN = 30;
                            int s_now = nodes[ptr].c_id[i];
                            if (!nodes[s_now].pruned){
                                for(int j = 0; j < deltaN; j++){
                                    Board sb;
                                    sb.copy_state(nodes[s_now].state);
                                    while(!sb.is_terminal()){
                                        MoveList s_mL;
                                        int s_mL_size;
                                        if(sb.side_to_move() == RED){
                                            s_mL_size = sb.legal_actions<RED>(s_mL);
                                        }
                                        else{
                                            s_mL_size = sb.legal_actions<BLUE>(s_mL);
                                        }
                                        Move s_move = s_mL[rand() % s_mL_size];
                                        //if (s_mL_size == 0)
                                        //    flog << "###############" << std::endl;
                                        sb.do_move(s_move);
                                        sb.update_status();
                                    }
                                    if ((sb.who_won() == RED && start == 'f' )|| (sb.who_won() == BLUE && start != 'f') ){
                                        double scores = calculate_scores(sb);
                                        deltaS +=  scores;
                                        deltaS2 += scores*scores;
                                    }
                                }
                                Simulations += deltaN;
                                //backup
                                update_score(s_now, deltaS, deltaS2, deltaN, current);
                            }                        
                        }
                    }

                    if (nodes[current].Ntotal > 1000){
                        for(int k = 0; k < nodes[current].Nchild; k++){
                            int id = nodes[current].c_id[k]; 
                            double l_o = nodes[id].Average - Rd * nodes[id].sd;
                            double r_o = nodes[id].Average + Rd * nodes[id].sd;
                            for(int l = k + 1; l < nodes[current].Nchild; l++){
                                int id_1 = nodes[current].c_id[l];
                                double l_o2 = nodes[id_1].Average - Rd * nodes[id_1].sd;
                                double r_o2 = nodes[id_1].Average + Rd * nodes[id_1].sd;
                                if (r_o < l_o2 && nodes[id].sd < Sd_e && nodes[id_1].sd < Sd_e){
                                    nodes[id].pruned = true;
                                    pruned++;
                                }
                                else if(r_o2 < l_o && nodes[id].sd < Sd_e && nodes[id_1].sd < Sd_e){
                                    nodes[id_1].pruned = true;
                                    pruned++;
                                }

                            }
                        }
                    }
                }

                double maxW = nodes[nodes[current].c_id[0]].Average;
                int maxchild = nodes[current].c_id[0];
                for(int i = 1; i < nodes[current].Nchild; i++){
                    int child = nodes[current].c_id[i];
                    if(nodes[child].Average > maxW){
                        maxW = nodes[child].Average;
                        maxchild = child;
                    }
                }
                current = maxchild;

                move = nodes[maxchild].ply;
                move_str = b.move_to_str(move);
                flog << "send move ";
                flog << myTurn << " " << move_str << "scores: " << nodes[maxchild].Average << " UCB " << UCB(maxchild) <<  std::endl;
                std::cout << move_str << std::flush;
                b.do_move(move);
                //flog << "do move " << std::endl;
                b.update_status();
            } else {
                num = getchar();
                dir = getchar();
                flog << "receive move ";
                flog << myTurn << " " << num << " " << dir << std::endl;
                b.do_move(num, dir);
                b.update_status();
                State temp_state;
                temp_state = b.copy_board();
                bool new_state = true;
                for(int i = 0; i < nodes[current].Nchild; i++){
                    int temp_c_id = nodes[current].c_id[i];
                    if(temp_state == nodes[temp_c_id].state){
                        current = temp_c_id;
                        new_state = false;
                        break;
                    }
                }
                if (new_state){
                    memset(&nodes, 0, sizeof nodes);
                    current = 0;
                    count = 1;
                    nodes[current].state = b.copy_board();
                    //flog << "*****************************************" << std::endl;
                }
                //flog << b;
            }
        }
        flog << "winner: " << b.who_won() << std::endl;
        flog << "tree size: " << count;
        int temp_depth = 0;
        int temp_b = 0;
        int max_depth = 0;
        for(int i = 0; i < count; i++){
            temp_depth += nodes[i].depth;
            temp_b += nodes[i].Nchild;
            if (nodes[i].depth > max_depth)
                max_depth = nodes[i].depth;
        } 
        flog <<" max depth: " << max_depth << " avg depth " << temp_depth/count << "pv depth " << nodes[current].depth << " avg branching " << (double) temp_b/count;
        flog << " variance " << nodes[current].Variance << " pruned :" << pruned << std::endl;

    } while (getchar() == 'y');

    return 0;
}
