/*
 * \file types.hpp
 * \brief game related types
 * \author Maria Elsa (elsa)
 * \course Theory of Computer Game (TCG)
 */

#ifndef TYPES_HPP
#define TYPES_HPP

#include <array>
#include <random>
#include <algorithm>
#include <chrono>

#include "color.hpp"

enum Move : int {
    MOVE_NONE,
    MOVE_NULL = 37,
    MOVE_PASS = 74
};

enum MoveType : int {
    NORMAL,
    CAPTURE_SELF,
    CAPTURE_OP,
    ALL
};

enum class Status {
    RedPlay,
    BluePlay,
    RedWin,
    BlueWin,
    Draw
};

enum Color : int {
    RED, BLUE, COLOR_NONE,
    COLOR_NB = 2
};

enum Piece : int {
    R_1, R_2, R_3, R_4, R_5, R_6,
    B_1, B_2, B_3, B_4, B_5, B_6,
    NO_PIECE,
    PIECE_NB = 12
};

enum PieceType : int {
    P_1, P_2, P_3, P_4, P_5, P_6,
    NO_PIECE_TYPE,
    PIECE_TYPE_NB = 6
};

enum Square : int {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6,
    SQ_NONE,

    SQUARE_NB = 36
};

enum Direction : int {
    NORTH = 6,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};

enum File : int {
    FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_NB = 6
};

enum Rank : int {
    RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_NB = 6
};

inline std::istream& operator>>(std::istream &is, Square &s) {
    int input;
    is >> input;
    s = static_cast<Square>(input);

    return is;
}

#define ENABLE_INCR_OPERATORS_ON(T)                             \
inline T& operator++(T& d) { return d = T(int(d) + 1); }        \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

ENABLE_INCR_OPERATORS_ON(PieceType)
ENABLE_INCR_OPERATORS_ON(Piece)
ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(File)
ENABLE_INCR_OPERATORS_ON(Rank)
ENABLE_INCR_OPERATORS_ON(Color)

#undef ENABLE_INCR_OPERATORS_ON

// Additional operators to add a Direction to a Square
constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square &s, Direction d) { return s = s + d; }
inline Square& operator-=(Square &s, Direction d) { return s = s - d; }

constexpr Color operator~(Color c) {
    return Color(c ^ BLUE); // Toggle color
}

constexpr Square make_square(File f, Rank r) {
    return Square((r * 6) + f);
}

constexpr Piece make_piece(Color c, PieceType pt) {
    return Piece((c * 6) + pt);
}

inline PieceType type_of(Piece pc) {
    return PieceType(pc % PIECE_TYPE_NB);
}

inline Color color_of(Piece pc) {
    return Color(pc / PIECE_TYPE_NB);
}

constexpr bool is_ok(Square s) {
    return s >= SQ_A1 && s <= SQ_F6;
}

constexpr File file_of(Square s) {
    return File(s % FILE_NB);
}

constexpr Rank rank_of(Square s) {
    return Rank(s / RANK_NB);
}

constexpr Square from_sq(Move m) {
    return Square(m / SQUARE_NB);
}

constexpr Square to_sq(Move m) {
    return Square(m % SQUARE_NB);
}

constexpr Move make_move(Square from, Square to) {
    return Move((from * SQUARE_NB) + to);
}

constexpr Move reverse_move(Move m) {
    return make_move(to_sq(m), from_sq(m));
}

constexpr bool is_move_ok(Move m) {
    // catch MOVE_NULL and MOVE_NONE
    return from_sq(m) != to_sq(m) && is_ok(from_sq(m)) && is_ok(to_sq(m));
}

const std::array<std::string, 10> Icon
{"０", "１", "２", "３", "４", "５", "６", "７", "８", "９"};

inline std::ostream& operator<<(std::ostream &os, const Piece &pc) {
    switch (color_of(pc)) {
        case RED:
            os << Ansi::BOLD << Ansi::BG_RED << Ansi::FG_WHITE;
            break;
        case BLUE:
            os << Ansi::BOLD << Ansi::BG_BLUE << Ansi::FG_WHITE;
            break;
        default:
            os << Ansi::BG_WHITE;
            break;
    }
    if (pc == NO_PIECE) {
        os << "  " << Ansi::CLEAR;
    } else {
        os << Icon[type_of(pc)] << Ansi::CLEAR;
    }
    return os;
}

// clear n lines
#define CLR_LINES(n) do {\
    for (int cnt=(n); cnt--;) {\
        std::cout << "\033[1F\033[2K";\
    }\
} while(0)

#define CLR_SCREEN std::cout << "\033[2J\033[1;1H"

#define MAX_MOVES 18 // 3 x 6
using MoveList = std::array<Move, MAX_MOVES>;
using PieceList = std::array<int, PIECE_TYPE_NB>;

Square init_sq_pos[COLOR_NB][PIECE_TYPE_NB] = {
    {SQ_A1, SQ_B1, SQ_C1, SQ_A2, SQ_B2, SQ_A3},
    {SQ_F4, SQ_E5, SQ_F5, SQ_D6, SQ_E6, SQ_F6}
};

PieceList init_piece[COLOR_NB] = {
    {0, 1, 3, 2, 4, 5},
    {5, 4, 2, 3, 0, 1}
};

struct State
{
    Color sideToMove;
    Status status;
    int gameLength;
    Piece board[SQUARE_NB];
    int num_pieces[COLOR_NB];

    bool operator==(const State &other) const{
        if (status == other.status && sideToMove == other.sideToMove && gameLength == other.gameLength){
            for(int i = 0; i < SQUARE_NB; i++){
                if(board[i] != other.board[i])
                    return false;
            }
            for(int i = 0; i < COLOR_NB; i++){
                if(num_pieces[i] != other.num_pieces[i])
                    return false;
            }
            return true;
        }
        return false;
    }
};

std::pair<std::string, std::string> shuffle_piece() {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::string res[2];
    for (int i = 0; i < COLOR_NB; ++i) {
        std::shuffle(init_piece[i].begin(), init_piece[i].end(),
            std::default_random_engine(seed));
        for (int j = 0; j < PIECE_TYPE_NB; ++j) {
            res[i] += (char)init_piece[i][j] + '0';
        }
    }
    return std::make_pair(res[0], res[1]);
}

#endif // #ifndef TYPES_HPP
