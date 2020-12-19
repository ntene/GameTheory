#include <iostream>
#include <fstream>
#include <getopt.h>
#include <tuple>

#include <sys/stat.h>

#include "einstein.hpp"
#include "io.hpp"

#define EINSTEIN_WRONG_ARG do {\
    std::cout << "Usage: ./game [-r round_num] [-l logfile] [-f path_to_p0] [-s path_to_p1] [-g]\n";\
    std::cout << " -r [round_num] (default 10)\n";\
    std::cout << " -l [path to logfile] (default \'log\')\n";\
    std::cout << " -f [path to first player] (default human)\n";\
    std::cout << " -s [path to second player] (default human)";\
    std::cout << " -g [0/1]: deactivate/activate GUI (default activated)\n";\
    return(1);\
} while(0)

std::fstream flog;

void announce_winner(Color winner, int first_player, int *win_count, bool gui) {
    if (winner == RED) {
        if (gui) {
            std::cout << Ansi::FG_RED << "Red Player Wins!!!\n\n";
        }
        win_count[first_player]++;
    } else if (winner == BLUE) {
        if (gui) {
            std::cout << Ansi::FG_BLUE << "Blue Player Wins!!!\n\n";
        }
        win_count[!first_player]++;
    } else {
        if (gui) {
            std::cout << Ansi::FG_GREEN << "Draw ~ OAO\n\n";
        }
        win_count[2]++;
    }
    if (gui) std::cout << Ansi::CLEAR;
}

void header(int first_player, int *win_count) {
    std::cout << Ansi::BOLD << Ansi::FG_CYAN << "Einstein Würfelt Nicht! (Dame) 6x6\n\n" << Ansi::CLEAR;
    std::cout << (first_player==0?Ansi::FG_RED:Ansi::FG_BLUE) << "Player 0: " << win_count[0] << std::endl;
    std::cout << (first_player==1?Ansi::FG_RED:Ansi::FG_BLUE) << "Player 1: " << win_count[1] << std::endl;
    std::cout << Ansi::FG_GREEN << "Draw: " << win_count[2] << "\n\n" << Ansi::CLEAR;
}

void final_score(int *win_count) {
    std::cout << Ansi::BOLD << Ansi::FG_GREEN;
    std::cout << "Final Score:\n";
    std::cout << "Player 0: " << win_count[0] << "\n";
    std::cout << "Player 1: " << win_count[1] << "\n";
    std::cout << "Draw: " << win_count[2] << "\n\n";
    std::cout << Ansi::CLEAR;
}

void flip_bit(int &x) { x = 1 - x; }

bool executable(const char *file) {
    //https://stackoverflow.com/questions/5719694/how-to-check-if-file-is-executable-in-c
    struct stat st;
    if (stat(file, &st) < 0) return false;
    if (st.st_mode & S_IEXEC) return true;
    return false;
}

int main(int argc, char **argv) {
    Agent *player[2];
    int opt, round = 10, gui = 1;
    float speed = 0.05;
    int round_cnt = 1;
    int first_player = 0;
    std::string first, second, logfile = ".log.game";
    int win_count[3] = {0, 0, 0};
    bool restart = 1;
    std::string topLeft, bottomRight;

    while ((opt = getopt(argc, argv, "r:f:s:l:g:")) != -1) {
        switch (opt) {
            case 'r':
                round = std::stoi(optarg);
                break;
            case 'l':
                logfile = optarg;
                break;
            case 'f':
                first = optarg;
                break;
            case 's':
                second = optarg;
                break;
            case 'g':
                gui = std::stoi(optarg);
                break;
            default:
                EINSTEIN_WRONG_ARG;
        }
    }

    if (round <= 0 || gui < 0) EINSTEIN_WRONG_ARG;
    if (first.length() > 0 && !executable(first.c_str())) {
        throw std::runtime_error("First player is not executable\n");
    }
    if (second.length() > 0 && !executable(second.c_str())) {
        throw std::runtime_error("Second player is not executable\n");
    }
    flog.open(logfile, std::fstream::out);
    if (!flog.is_open()) {
        throw std::runtime_error("Error opening logfile\n");
    }
    if (gui == 0) {
        speed = 0;
    }

    if (first.length() == 0) {
        player[0] = new Agent();
    } else {
        player[0] = new Agent(first);
    }
    if (second.length() == 0) {
        player[1] = new Agent();
    } else {
        player[1] = new Agent(second);
    }

    round = round * 2;

    while (restart && round_cnt <= round) {
        flog << "round: " << round_cnt << std::endl;
        Board b;
        std::tie(topLeft, bottomRight) = shuffle_piece();
        b.init(topLeft, bottomRight);
        player[0]->init(topLeft, bottomRight, (first_player == 0) ? 'f' : 's');
        player[1]->init(topLeft, bottomRight, (first_player == 1) ? 'f' :'s');
        flog << "init: " << topLeft << bottomRight << std::endl;
        flog << b;

        if (gui) {
            CLR_SCREEN;
            header(first_player, win_count);
            std::cout << b << "\n\n";
        }
        usleep(speed * 1000000);
        if (gui) {
            CLR_LINES(FILE_NB + 7);
        }
        int now_turn = first_player;
        flog << "turn: " << first_player << std::endl;
        while (!b.is_terminal()) {
            std::string m = player[now_turn]->get_move();
            flog << now_turn << m << std::endl;
            b.do_move(m[0], m[1]);
            flog << b;
            player[!now_turn]->give_move(m);
            b.update_status();
            flip_bit(now_turn);

            if (gui) {
                CLR_SCREEN;
                header(first_player, win_count);
                std::cout << b << "\n\n";
            }
            usleep(speed * 1000000);
            if (gui && !b.is_terminal()) CLR_LINES(FILE_NB + 7);
        }

        announce_winner(b.who_won(), first_player, win_count, gui);
        flog << "winner: " << b.who_won() << std::endl;
        flog << "game length: " << b.get_gameLength() << std::endl;

        if (round_cnt == round) break;
        player[0]->restart(restart ? 'y' : 'n');
        player[1]->restart(restart ? 'y' : 'n');

        flip_bit(first_player);
        round_cnt++;
    }
    final_score(win_count);

    return 0;
}
