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

#include "einstein.hpp"

inline void flip_bit(bool &x) { x = !x; }

std::fstream flog;

void logger(std::string logfile) {
    flog.open(logfile, std::fstream::out);
    if (!flog.is_open()) {
        std::runtime_error("Error opening file\n");
    }
}

#ifdef GREEDY
#define WIN 100
#define DRAW 50

double eval(Board b, Move m) {
    Color us = b.side_to_move();
    double value = 0;

    if (b.get_move_type(m) == CAPTURE_OP) {
        value = 5.0;
    } else if (b.get_move_type(m) == NORMAL) {
        value = 1.0;
        Square from = from_sq(m);
        Square to = to_sq(m);
        Rank from_r = rank_of(from);
        File from_f = file_of(from);
        Rank to_r = rank_of(to);
        File to_f = file_of(to);
        if (us == RED) {
            value += 1 - (from_r + from_f) / 10;
        } else {
            value += (from_r + from_f) / 10;
        }
        if (to_r == RANK_1 || to_r == RANK_6 || to_f == FILE_A || to_f == FILE_F) {
            value += 0.6;
        }
    } else {
        value = -5;
    }

    b.do_move(m);
    b.update_status();
    if (b.is_terminal()) {
        if (b.who_won() == b.side_to_move()) {
            return -WIN;
        } else if (b.who_won() == COLOR_NONE) {
            return DRAW;
        } else {
            return WIN;
        }
    }

    MoveList eval_mL;
    int mL_size;
    if (b.side_to_move() == RED) {
        mL_size = b.legal_actions<RED>(eval_mL);
    } else {
        mL_size = b.legal_actions<BLUE>(eval_mL);
    }
    for (int i = 0; i < mL_size; ++i) {
        Move next_m = eval_mL[i];
        if (b.get_move_type(next_m) == CAPTURE_OP) {
            value -= 0.5;
        }
    }

    return value;
}
#endif

int main() {
#ifdef CONSERVATIVE
    logger(".log.conservative");
#elif GREEDY
    logger(".log.greedy");
#else
    logger(".log.random");
#endif
    Board b;
    MoveList mL;
    Move move;
    int mL_size;
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

        for (myTurn = (start == 'f'); !b.is_terminal(); flip_bit(myTurn)) {
            if (myTurn) {
                if (b.side_to_move() == RED) {
                    mL_size = b.legal_actions<RED>(mL);
                } else {
                    mL_size = b.legal_actions<BLUE>(mL);
                }
#ifdef CONSERVATIVE
                int best_score = -1;
                for (int i = 0; i < mL_size; ++i) {
                    Move m = mL[i];
                    if (b.get_move_type(m) == CAPTURE_OP && best_score < 2) {
                        move = m;
                        best_score = 2;
                        break;
                    } else if (b.get_move_type(m) == NORMAL && best_score < 1) {
                        move = m;
                        best_score = 1;
                    }
                }
                if (best_score == -1) {
                    move = mL[rand() % mL_size];
                }
#elif GREEDY
                double max_score = -10000, score;
                for (int i = 0; i < mL_size; ++i) {
                    Move m = mL[i];
                    score = eval(b, m);
                    if (score > max_score) {
                        move = m;
                        max_score = score;
                    }
                }
#else
                move = mL[rand() % mL_size];
#endif
                move_str = b.move_to_str(move);
                flog << "send move ";
                flog << myTurn << move_str << std::endl;
                std::cout << move_str << std::flush;
                b.do_move(move);
                b.update_status();
                flog << b;
            } else {
                num = getchar();
                dir = getchar();
                flog << "receive move ";
                flog << myTurn << num << dir << std::endl;
                b.do_move(num, dir);
                b.update_status();
                flog << b;
            }
        }
        flog << "winner: " << b.who_won() << std::endl;
    } while (getchar() == 'y');

    return 0;
}
