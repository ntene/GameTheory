#ifndef MYAI_INCLUDED
#define MYAI_INCLUDED 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <cstdint>
#include <random>
#include <unordered_map>
#include <algorithm>

#define RED 0
#define BLACK 1
#define CHESS_COVER 15
#define CHESS_EMPTY 0
#define COMMAND_NUM 18

class MyAI  
{
	const char* commands_name[COMMAND_NUM] = {
		"protocol_version",
		"name",
		"version",
		"known_command",
		"list_commands",
		"quit",
		"boardsize",
		"reset_board",
		"num_repetition",
		"num_moves_to_draw",
		"move",
		"flip",
		"genmove",
		"game_over",
		"ready",
		"time_settings",
		"time_left",
  	"showboard"
	};
public:
	MyAI(void);
	~MyAI(void);

	// commands
	bool protocol_version(const char* data[], char* response);// 0
	bool name(const char* data[], char* response);// 1
	bool version(const char* data[], char* response);// 2
	bool known_command(const char* data[], char* response);// 3
	bool list_commands(const char* data[], char* response);// 4
	bool quit(const char* data[], char* response);// 5
	bool boardsize(const char* data[], char* response);// 6
	bool reset_board(const char* data[], char* response);// 7
	bool num_repetition(const char* data[], char* response);// 8
	bool num_moves_to_draw(const char* data[], char* response);// 9
	bool move(const char* data[], char* response);// 10
	bool flip(const char* data[], char* response);// 11
	bool genmove(const char* data[], char* response);// 12
	bool game_over(const char* data[], char* response);// 13
	bool ready(const char* data[], char* response);// 14
	bool time_settings(const char* data[], char* response);// 15
	bool time_left(const char* data[], char* response);// 16
	bool showboard(const char* data[], char* response);// 17

private:
	int Color;
	int Red_Time, Black_Time;
	int Board[32];
	int CoverChess[16];
	int Red_Chess_Num, Black_Chess_Num;
	int node;
	int onode;
	int Game_ply = 0;

	int Max_Board[32];
	int Max_CoverChess[14];
	//int DEPTH_LIMIT = 7;

	struct Node
	{
		uint32_t piece[16]; // - K G M R N C P k g m r k c p X
		uint32_t red, black, occupied;
		unsigned long long Counter_hash;
	};

	Node bit_board;
	char piece_name[16] = {'-', 'K', 'G', 'M', 'R', 'N', 'C', 'P', 'k', 'g', 'm', 'r', 'n', 'c', 'p', 'X'};
	int board_to_bit[14] = {7, 6, 5, 4, 3, 2, 1, 14, 13, 12, 11, 10, 9, 8};

	uint32_t file[4] = {0x11111111, 0x22222222, 0x44444444, 0x88888888};
	uint32_t rank[8] = {0x0000000F, 0x000000F0, 0x00000F00, 0x0000F000, 0x000F0000, 0x00F00000, 0x0F000000, 0xF0000000};
	uint32_t pMoves[32] = {18,37,74,132,289,594,1188,2120,4624,9504,19008,33920,73984,152064,304128,542720,1183744,2433024,4866048,8683520,18939904,38928384,77856768,138936320,303038464,622854144,1245708288,2222981120,553648128,1375731712,2751463424,1207959552};
	int magic_index[32] = {31, 0, 1, 5, 2, 16, 27, 6, 3, 14, 17, 19, 28, 11, 7, 21, 30, 4, 15, 26, 13, 18, 10, 20, 29, 25, 12, 9, 24, 8, 23, 22};

	struct Node_Info
	{
		int tMove;
		double tScore;
		int tFlag; // 0 exact 1 lower 2 upper
		int tDepth;
	};

	//X K G M R N C P
	unsigned long long Random_number[32][8][2]; //[position][piece_type][color]
	unsigned long long Random_color[2];
	std::unordered_map<unsigned long, Node_Info> Hash_table;
	int Hash_hit = 0;
	int Chance_cut = 0;

	//Chance search
	double Vmin = 0;
	double Vmax = 3886;

	//History heuristics
	unsigned long long History_table[1024];
	bool cmp(int x, int y){
	  return (History_table[x] < History_table[y]);
	}

	// Utils
	int GetFin(char c);
	int ConvertChessNo(int input);
	int GetPiece_Index(char c);

	// Board
	void initBoardState();
	void generateMove(char move[6]);
	void MakeSimulationMove(int* board, Node& bit_board, int* red_chess_num, int* black_chess_num, int* cover_chess, const int move, const int chess);
	void MakeMove(const char move[6]);
	bool Referee(const int* board, const int Startoint, const int EndPoint, const int color);
	int Expand(const int* board, const int color, int *Result);
	double Evaluate(const int* board);
	double Nega_scout(const int* board, const Node bit_board, int* move, const int red_chess_num, const int black_chess_num, const int* cover_chess, const int color, double alpha, double beta, const int depth, const int remain_depth);
	double ONega_max(const int* board, int* move, const int red_chess_num, const int black_chess_num, const int* cover_chess, const int color, const int depth, const int remain_depth);

	// Display
	void Pirnf_Chessboard();
	void Print_Chessboard(const int *Board);
	void Print_BitBoard(Node& bit_board);

	//Bit
	uint32_t LSB(uint32_t x);
	uint32_t MSB(uint32_t x);
	int GetIndex(uint32_t mask);
	int Bits_Hash(uint32_t x);
	uint32_t CGen(Node bit_board, int src);
	uint32_t CGenCR(uint32_t x);
	uint32_t CGenCL(uint32_t x);
	int Expand_bitboard(Node bit_board, const int color, int *Result);

	//Nega Max
	void MakeMove_Max(int* board, int* red_chess_num, int* black_chess_num, int* cover_chess, const int move, const int chess);
	void MakeMove_Max(int* board, int* red_chess_num, int* black_chess_num, int* cover_chess, const char move[6]);
	double Max_Evaluate(const int* board);
	int Max_GetFin(char c);
	char Max_piecename[14] = {'P', 'C', 'N', 'R', 'M', 'G', 'K', 'p', 'c', 'n', 'r', 'm', 'g', 'k'};
	
};

#endif

