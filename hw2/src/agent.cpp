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

#define MAXNODES 100000
#define C 1.18f
#define parent(ptr) (nodes[ptr].p_id)
#define child(ptr, i) (nodes[ptr].c_id[i])

struct Node
{
	Move ply;
	int p_id;
	int c_id[MAX_MOVES];
	int depth;
	int Nchild;
	int Ntotal;
	double CsqrtlogNtotal;
	double sqrtNtotal;
	int Wins;
	double WR;
	State state;
} nodes[MAXNODES];

double UCB(int id){
    double temp = nodes[parent(id)].CsqrtlogNtotal/nodes[id].sqrtNtotal;
	return (nodes[id].depth % 2) ? (nodes[id].WR+temp) : (1.0 - nodes[id].WR+temp);
}

int main() {
    logger(".log.agent");
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

        flog << init[0] << " " << init[1] << std::endl;
        flog << start << std::endl;
        b.init(init[0], init[1]);

        memset(&nodes, 0, sizeof nodes);
        int count = 1;
        int current = 0;
        nodes[0].state = b.copy_board();

        //flog << b;
        
        /*nodes[0].state.sideToMove = RED;
        nodes[0].state.status = Status::RedPlay;
        nodes[0].state.gameLength = 0;
        std::copy(std::begin(b.temp), std::end(b.temp), std::begin(nodes[0].state.board));
		*/

        for (myTurn = (start == 'f'); !b.is_terminal(); flip_bit(myTurn)) {
            if (myTurn) {

                //start UCT
                //flog << "current : " << current << std::endl;
                //flog << "count " << count << std::endl;
                /*for(int i = 0; i < nodes[current].Nchild; i++){
                	flog << "^ " << nodes[current].c_id[i] << std::endl;
                }*/
                int Simulations = 0;
                while(Simulations < 150000){
                	//select
                	int ptr = current;
                	//flog <<  "-------" << std::endl << "Child  "  << nodes[current].Nchild << " Ntotal  " << nodes[current].Ntotal << "  depth: " << nodes[current].depth << std::endl;
                	/*flog <<  "Tree ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
                	for(int i = 0; i < count; i++){
                		flog << "id " << i << "  ";
                		for(int j = 0; j < nodes[i].Nchild; j++)
                			flog << nodes[i].c_id[j] << " ";
                		flog << std::endl;
                	}
                	flog <<  "Tree ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;*/
                	while(nodes[ptr].Nchild > 0){
                		int maxchild = child(ptr, 0);
                		double maxV = UCB(maxchild);
                		for(int i = 1; i < nodes[ptr].Nchild; i++){
                			int ctemp = child(ptr, i);
                			double temp = UCB(ctemp);
                			if (temp > maxV){
                				maxV = temp;
                				maxchild = ctemp;
                			}

                		}
                		ptr = maxchild;
                		//flog << "---> ptr " << ptr << std::endl;
                	}
                	//flog << "ptr == " << ptr <<  "  ptr child " << nodes[ptr].Nchild << "  Win rate  " << UCB(ptr) << std::endl;
                	//expansion
            		if(nodes[ptr].Nchild == 0){
            			MoveList statemL;
            			Board state_board;
            			state_board.copy_state(nodes[ptr].state);
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
            				nodes[count].state = state_board.copy_board();
            				nodes[ptr].c_id[i] = count;
            				count++;
            			}
            			nodes[ptr].Nchild = state_mL_size;
            		}

                	//simulation
                	for(int i = 0; i < nodes[ptr].Nchild; i++){
                		int deltaW = 0;
                		int deltaN = 100;
						int s_now = nodes[ptr].c_id[i];
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
                				if (s_mL_size == 0)
                					flog << "###############" << std::endl;
                				sb.do_move(s_move);
                				sb.update_status();
                			}
                			if ((sb.who_won() == RED && start == 'f' )|| (sb.who_won() == BLUE && start != 'f') ){
                				deltaW++;
                			}
                		}
            			//flog << "finish simulations " << Simulations << std::endl;
                		Simulations += deltaN;

                		/*only for print ply
                		Board sb;
                		sb.copy_state(nodes[ptr].state);
                		flog << "s now   " << s_now << " move " << sb.move_to_str(nodes[s_now].ply) << "  win " << deltaW  << "   Ntotal " << nodes[current].Ntotal << std::endl;*/

    		            //backup
	                	while(1)
	                	{
	                		nodes[s_now].Ntotal += deltaN;
	                		nodes[s_now].CsqrtlogNtotal = C * sqrt(log((double) nodes[ptr].Ntotal));
	                		nodes[s_now].Wins += deltaW;
	                		nodes[s_now].WR = (double) nodes[s_now].Wins / (double) nodes[s_now].Ntotal;
	                		nodes[s_now].sqrtNtotal = sqrt((double) nodes[s_now].Ntotal);
	                		if (s_now == current)
	                			break;
	                		s_now = parent(s_now);
	                		//flog << "s_now " << s_now << "current " << current << std::endl;
	                		
	                	}
	                	//flog << "backup to current " << std::endl;
                	}	
                }

                double maxW = nodes[nodes[current].c_id[0]].WR;
                int maxchild = nodes[current].c_id[0];
                for(int i = 1; i < nodes[current].Nchild; i++){
                	int child = nodes[current].c_id[i];
                	if(nodes[child].WR > maxW){
                		maxW = nodes[child].WR;
                		maxchild = child;
                	}
                }
                current = maxchild;

                move = nodes[maxchild].ply;
                move_str = b.move_to_str(move);
                flog << "send move ";
                flog << myTurn << " " << move_str << "win rate: " << nodes[maxchild].WR << " UCB " << UCB(maxchild) <<  std::endl;
                std::cout << move_str << std::flush;
                b.do_move(move);
                //flog << "do move " << std::endl;
                b.update_status();
                //flog << b;
                //flog << "update b status" << std::endl;
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
                	flog << "*****************************************" << std::endl;
                }
                //flog << b;
            }
        }
        flog << "winner: " << b.who_won() << std::endl;
        flog << "tree size: " << count << " max depth: " << nodes[count - 1].depth;
        int temp_depth = 0;
        int temp_b = 0;
        for(int i = 0; i < count; i++){
        	temp_depth += nodes[i].depth;
        	temp_b += nodes[i].Nchild;
        } 
        flog << " avg depth " << temp_depth/count << "pv depth " << nodes[current].depth << " avg branching " << temp_b/count << std::endl;

    } while (getchar() == 'y');

    return 0;
}
