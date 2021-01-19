#include "float.h"
#include "MyAI.h"

#define DEPTH_LIMIT 3
#define max(a, b) ((a > b)? a : b)
#define min(a, b) ((a < b)? a : b)



MyAI::MyAI(void){}

MyAI::~MyAI(void){}

bool MyAI::protocol_version(const char* data[], char* response){
	strcpy(response, "1.0.0");
  return 0;
}

bool MyAI::name(const char* data[], char* response){
	strcpy(response, "Negascout");
	return 0;
}

bool MyAI::version(const char* data[], char* response){
	strcpy(response, "1.0.0");
	return 0;
}

bool MyAI::known_command(const char* data[], char* response){
  for(int i = 0; i < COMMAND_NUM; i++){
		if(!strcmp(data[0], commands_name[i])){
			strcpy(response, "true");
			return 0;
		}
	}
	strcpy(response, "false");
	return 0;
}

bool MyAI::list_commands(const char* data[], char* response){
  for(int i = 0; i < COMMAND_NUM; i++){
		strcat(response, commands_name[i]);
		if(i < COMMAND_NUM - 1){
			strcat(response, "\n");
		}
	}
	return 0;
}

bool MyAI::quit(const char* data[], char* response){
  fprintf(stderr, "Bye\n"); 
	return 0;
}

bool MyAI::boardsize(const char* data[], char* response){
  fprintf(stderr, "BoardSize: %s x %s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::reset_board(const char* data[], char* response){
	this->Red_Time = -1; // unknown
	this->Black_Time = -1; // unknown
	this->initBoardState();
	return 0;
}

bool MyAI::num_repetition(const char* data[], char* response){
  return 0;
}

bool MyAI::num_moves_to_draw(const char* data[], char* response){
  return 0;
}

bool MyAI::move(const char* data[], char* response){
  char move[6];
	sprintf(move, "%s-%s", data[0], data[1]);
	this->MakeMove(move);
	int temp = 16;
	this->MakeMove_Max(this->Max_Board, &temp, &temp, this->Max_CoverChess, move);
	return 0;
}

bool MyAI::flip(const char* data[], char* response){
  char move[6];
	sprintf(move, "%s(%s)", data[0], data[1]);
	this->MakeMove(move);
	int temp = 16;
	this->MakeMove_Max(this->Max_Board, &temp, &temp, this->Max_CoverChess, move);
	return 0;
}

bool MyAI::genmove(const char* data[], char* response){
	// set color
	if(!strcmp(data[0], "red")){
		this->Color = RED;
	}else if(!strcmp(data[0], "black")){
		this->Color = BLACK;
	}else{
		this->Color = 2;
	}
	// genmove
  	char move[6];
	this->generateMove(move);
	sprintf(response, "%c%c %c%c", move[0], move[1], move[3], move[4]);
	return 0;
}

bool MyAI::game_over(const char* data[], char* response){
  fprintf(stderr, "Game Result: %s\n", data[0]); 
	return 0;
}

bool MyAI::ready(const char* data[], char* response){
  return 0;
}

bool MyAI::time_settings(const char* data[], char* response){
  return 0;
}

bool MyAI::time_left(const char* data[], char* response){
  if(!strcmp(data[0], "red")){
		sscanf(data[1], "%d", &(this->Red_Time));
	}else{
		sscanf(data[1], "%d", &(this->Black_Time));
	}
	fprintf(stderr, "Time Left(%s): %s\n", data[0], data[1]); 
	return 0;
}

bool MyAI::showboard(const char* data[], char* response){
  Pirnf_Chessboard();
	return 0;
}


// *********************** AI FUNCTION *********************** //

int MyAI::GetFin(char c)
{
	static const char skind[]={'-','K','G','M','R','N','C','P','k','g','m','r','n','c','p','X'};
	for(int f=0;f<16;f++)if(c==skind[f])return f;
	return -1;
}

void MyAI::initBoardState()
{	
	//int iPieceCount[14] = {5, 2, 2, 2, 2, 2, 1, 5, 2, 2, 2, 2, 2, 1};
	int iPieceCount[16] = {0, 1, 2, 2, 2, 2, 2, 5, 1, 2, 2, 2, 2, 2, 5, 0};
	memcpy(this->CoverChess,iPieceCount,sizeof(int)*16);
	Red_Chess_Num = 16; Black_Chess_Num = 16;

	//convert to my format
	for(int i = 0; i < 32; i++)
		this->Board[i] = CHESS_COVER;
	this->Pirnf_Chessboard();

	memset(&bit_board, 0, sizeof(bit_board));
	bit_board.piece[15] = 0xffffffff;
	bit_board.occupied = 0xffffffff;

	std::random_device rd;
 	std::default_random_engine generator(rd());
 	std::uniform_int_distribution<unsigned long long> distribution(0,0xFFFFFFFFFFFFFFFF);

 	bit_board.Counter_hash = 0;
 	for(int i = 0; i < 32; i++){
 		for(int j = 1; j < 8; j++){
 			Random_number[i][j][0] = distribution(generator);
 			Random_number[i][j][1] = distribution(generator);
 		}
 		Random_number[i][0][0] =  Random_number[i][0][1] = distribution(generator);
 		bit_board.Counter_hash = (bit_board.Counter_hash^Random_number[i][0][0]);
 	}
 	Hash_table.clear();

	//Nega Max
	int PieceCount[14] = {5, 2, 2, 2, 2, 2, 1, 5, 2, 2, 2, 2, 2, 1};
	memcpy(this->Max_CoverChess,PieceCount,sizeof(int)*14);
	int Index = 0;
	for(int i=0;i<8;i++)
	{
		for(int j=0;j<4;j++)
		{
			this->Max_Board[Index] = -1;
			Index++;
		}
	}

}

void MyAI::generateMove(char move[6])
{
	/* generateMove Call by reference: change src,dst */
	this->node = 0;
	this->onode = 0;
	int startPoint = 0;
	int EndPoint = 0;

	int temp_best_move = 0;
	double temp = ONega_max(this->Max_Board, &temp_best_move, this->Red_Chess_Num, this->Black_Chess_Num, this->Max_CoverChess, this->Color, 0, DEPTH_LIMIT);
	int temp_startPoint = temp_best_move/100;
	int temp_EndPoint   = temp_best_move%100;

	int best_move = 0;
	double t = Nega_scout(this->Board, this-> bit_board, &best_move, this->Red_Chess_Num, this->Black_Chess_Num, this->CoverChess, this->Color, -DBL_MAX, DBL_MAX, 0, DEPTH_LIMIT);
	//fprintf(stderr,"Expand = %d\n\n", Expand_bitboard(this-> bit_board, this->Color, Result));
	//fprintf(stderr, "red chess num = %d black chess num = %d\n",Red_Chess_Num, Black_Chess_Num);
	startPoint = best_move/100;
	EndPoint   = best_move%100;
	sprintf(move, "%c%c-%c%c",'d'-(startPoint%4),'8'-(startPoint/4),'d'-(EndPoint%4),'8'-(EndPoint/4));

	printf("Nega max score  = %f Nega scout score = %f\n",temp, t);
	printf("Nega max %c%c-%c%c\n",'a'+(temp_startPoint%4),'1'+(7-temp_startPoint/4),'a'+(temp_EndPoint%4),'1'+(7-temp_EndPoint/4));
	printf("Nega scout %c%c-%c%c\n",'d'-(startPoint%4),'8'-(startPoint/4),'d'-(EndPoint%4),'8'-(EndPoint/4));

	if (temp != t)
		fprintf(stderr, "DIFFERENT SCORE Nega max score  = %f Nega scout score = %f\n",temp, t);

	printf("My result: \n--------------------------\n");
	printf("Nega max: %lf (node: %d) onode %d\n", t, this->node, this->onode);
	printf("(%d) -> (%d)\n",startPoint,EndPoint);
	printf("<%c> -> <%c>\n",piece_name[Board[(startPoint/4)*4+((3-startPoint%4)%4)]],piece_name[Board[(EndPoint/4)*4+((3-EndPoint%4)%4)]]);
	printf("move:%s\n",move);
	printf("--------------------------\n");
	//this->Pirnf_Chessboard();
	printf("Evaluate %f Max_Evaluate %f\n", Evaluate(Board), Max_Evaluate(Max_Board));
	printf("HASH COUNT = %d Hash table size = %lu\n", Hash_hit , Hash_table.size());
	Hash_hit = 0;
}

void MyAI::MakeSimulationMove(int* board,  Node& bit_board, int* red_chess_num, int* black_chess_num, int* cover_chess, const int move, const int chess){
	int src = move/100, dst = move%100;
	uint32_t from = 1 << src;
	uint32_t to = 1 << dst;
	uint32_t src_dst = from^to;
	//change into board array value
	int r = src/4; int dr = dst/4;
	int c = (3 - src%4) % 4; int dc = (3 - dst%4) % 4;

	//board
	int bsrc = r*4+c;
	int bdst = dr*4+dc;

	if(src == dst){ 
		board[bsrc] = chess;
		cover_chess[chess]--;

		bit_board.piece[15] ^= from;
		bit_board.piece[chess] ^= from;
		if(chess <= 7){
			bit_board.red ^= from;
			bit_board.Counter_hash ^= Random_number[src][0][RED] ^ Random_number[src][chess][RED];
		}
		else{
			bit_board.black ^= from;
			bit_board.Counter_hash ^= Random_number[src][0][BLACK] ^ Random_number[src][chess-7][BLACK];
		}
	}else { 
		int move_piece = board[bsrc];
		bit_board.piece[move_piece] ^= src_dst;
		if(move_piece <= 7 && move_piece >= 1){
			bit_board.red ^= src_dst;
			bit_board.Counter_hash ^= Random_number[src][move_piece][RED] ^ Random_number[dst][move_piece][RED];
		}
		else if (move_piece >= 8 && move_piece <= 14){
			bit_board.black ^= src_dst;
			bit_board.Counter_hash ^= Random_number[src][move_piece-7][BLACK] ^ Random_number[dst][move_piece-7][BLACK];
		}

		int dst_piece = board[bdst];
		if(dst_piece != CHESS_EMPTY){
			if(dst_piece <= 7 && dst_piece >= 1){
				(*red_chess_num)--;
				bit_board.red ^= to;
				bit_board.Counter_hash ^= Random_number[dst][dst_piece][RED];
			}else if(dst_piece >= 8 && dst_piece <= 14){
				(*black_chess_num)--;
				bit_board.black ^= to;
				bit_board.Counter_hash ^= Random_number[dst][dst_piece-7][BLACK];
			}
			bit_board.piece[dst_piece] ^= to;
			bit_board.occupied ^= from;
			bit_board.piece[0] ^= from;

		}
		else{
			bit_board.occupied ^= src_dst;
			bit_board.piece[0] ^= src_dst;
		}

		board[bdst] = board[bsrc];
		board[bsrc] = CHESS_EMPTY;
	}

	//printf("src %d dst %d %c\n", src, dst, piece_name[chess]);
	//Print_BitBoard(bit_board);
}

void MyAI::MakeMove(const char move[6])
{ 
	int src, dst;
	src = ('8'-move[1])*4+(move[0]-'a');

	//bit board
	int b_src = ('8'-move[1])*4+('d'-move[0]);

	uint32_t from = 1 << b_src;
	if(move[2]=='('){ 
		Board[src] = GetFin(move[3]);
		CoverChess[GetFin(move[3])]--;

		bit_board.piece[15] ^= from;
		int piece_type = GetFin(move[3]);
		bit_board.piece[piece_type] ^= from;

		if(piece_type <= 7){
			bit_board.red ^= from;
			bit_board.Counter_hash ^= Random_number[b_src][0][RED] ^ Random_number[b_src][piece_type][RED];
		}
		else {
			bit_board.black ^= from;
			bit_board.Counter_hash ^= Random_number[b_src][0][BLACK] ^ Random_number[b_src][piece_type-7][BLACK];
		}

	}else { 
		dst = ('8'-move[4])*4+(move[3]-'a');
		int b_dst = ('8'-move[4])*4+('d'-move[3]);
		printf("# call move(): move : %d-%d \n",b_src,b_dst); 
		//
		uint32_t to = 1 << b_dst;
		uint32_t src_dst = from ^ to;
		int move_piece = Board[src];
		//printf("board[src] %d move_piece %d\n", board[src], move_piece);
		
		bit_board.piece[move_piece] ^= src_dst;
		if(move_piece <= 7 && move_piece >= 1){
			bit_board.red ^= src_dst;
			bit_board.Counter_hash ^= Random_number[b_src][move_piece][RED] ^ Random_number[b_dst][move_piece][RED];
		}
		else if (move_piece >= 8 && move_piece <= 14){
			bit_board.black ^= src_dst;
			bit_board.Counter_hash ^= Random_number[b_src][move_piece-7][BLACK] ^ Random_number[b_dst][move_piece-7][BLACK];
		}
		
		int dst_piece = Board[dst];
		printf(">>>>> move_piece %d dst_piece %d\n", move_piece, dst_piece);
		if(dst_piece != CHESS_EMPTY){ //capture
			if(dst_piece <= 7 && dst_piece >= 1){
				Red_Chess_Num--;
				bit_board.red ^= to;
				bit_board.Counter_hash ^= Random_number[b_dst][dst_piece][RED];
			}else if(dst_piece >= 8 && dst_piece <= 14){
				Black_Chess_Num--;
				bit_board.black ^= to;
				bit_board.Counter_hash ^= Random_number[b_dst][dst_piece-7][BLACK];
			}
			bit_board.piece[dst_piece] ^= to;
			bit_board.occupied ^= from;
			bit_board.piece[0] ^= from;

		}
		else{ //move only
			bit_board.occupied ^= src_dst;
			bit_board.piece[0] ^= src_dst;
		}

		Board[dst] = Board[src];
		Board[src] = 0;

	}
	Pirnf_Chessboard();
	//Print_BitBoard(bit_board);
	/* init time */
}

int MyAI::Expand_bitboard(Node bit_board, const int color, int *Result){
	int start = color == RED ? 1 : 8;
	int ResultCount = 0;
	for(int i = start; i < start + 7; i++){
		uint32_t p = bit_board.piece[i];
		while(p){
			uint32_t mask = LSB(p);
			p ^= mask;
			int src = GetIndex(mask);
			uint32_t dest;
			switch (i){
				case 1:
					dest = pMoves[src] & (bit_board.black^bit_board.piece[14]);
					break;
				case 2:
					dest = pMoves[src] & (bit_board.black^bit_board.piece[8]);
					break;
				case 3:
					dest = pMoves[src] & (bit_board.black^bit_board.piece[8]^bit_board.piece[9]);
					break;
				case 4:
					dest = pMoves[src] & (bit_board.piece[11] | bit_board.piece[12] | bit_board.piece[13] | bit_board.piece[14]);
					break;
				case 5:
					dest = pMoves[src] & (bit_board.piece[12] | bit_board.piece[13] | bit_board.piece[14]);
					break;
				case 6:
					dest = CGen(bit_board, src) & bit_board.black;
					break;
				case 7:
					dest = pMoves[src] & (bit_board.piece[14] | bit_board.piece[8]);
					break;
				case 8:
					dest = pMoves[src] & (bit_board.red^bit_board.piece[7]);
					break;
				case 9:
					dest = pMoves[src] & (bit_board.red^bit_board.piece[1]);
					break;
				case 10:
					dest = pMoves[src] & (bit_board.red^bit_board.piece[1]^bit_board.piece[2]);
					break;
				case 11:
					dest = pMoves[src] & (bit_board.piece[4] | bit_board.piece[5] | bit_board.piece[6] | bit_board.piece[7]);
					break;
				case 12:
					dest = pMoves[src] & (bit_board.piece[5] | bit_board.piece[6] | bit_board.piece[7]);
					break;
				case 13:
					dest = CGen(bit_board, src) & bit_board.red;
					break;
				case 14:
					dest = pMoves[src] & (bit_board.piece[1] | bit_board.piece[7]);
					break;
				default:
					dest = 0;
			}
			while(dest){
				uint32_t mask2 = LSB(dest);
				dest ^= mask2;
				int result = GetIndex(mask2);
				Result[ResultCount] = src*100+result;
				ResultCount++;
				//printf("possible moves %d %d\n",src, result);
			}
			
		}
	}
	for(int i = start; i < start + 7; i++){
		uint32_t p = bit_board.piece[i];
		while(p){
			uint32_t mask = LSB(p);
			p ^= mask;
			int src = GetIndex(mask);
			uint32_t dest = pMoves[src] & bit_board.piece[0];
			while(dest){
				uint32_t mask2 = LSB(dest);
				dest ^= mask2;
				int result = GetIndex(mask2);
				Result[ResultCount] = src*100+result;
				ResultCount++;
				//printf("possible moves %d %d\n",src, result);
			}
		}
	}
	return ResultCount;
}

uint32_t MyAI::LSB(uint32_t temp){
	uint64_t x = temp;
	x = x & (-x);
	uint32_t y = x & 0xffffffff;
	return y;
}

uint32_t MyAI::MSB(uint32_t temp){
	uint64_t x = temp;
	x |= x >> 32;
	x |= x >> 16;
	x |= x >> 8;
	x |= x >> 4;
	x |= x >> 2;
	x |= x >> 1;
	x = (x >> 1) + 1;
	temp = x & 0xffffffff;
	return temp;
}

int MyAI::GetIndex(uint32_t mask){
	return magic_index[Bits_Hash(mask)];
}

int MyAI::Bits_Hash(uint32_t x){
	return (x * 0x08ED2BE6) >> 27;
}

uint32_t MyAI::CGen(Node bit_board, int src){
	//handle horizontal (row)
	int row = src/4;
	int column = src%4;
	uint32_t dest = 0;
	uint32_t x = ((rank[row]&bit_board.occupied)^(1<<src)) >> (4*row);
	if (column == 0){
		dest = CGenCL(x);
	}
	else if (column == 1){
		dest = ((x&12) == 12) ? 8 : 0;
	}
	else if (column == 2){
		dest = ((x&3) == 3) ? 1 : 0;
	}
	else{
		dest = CGenCR(x);
	}

	uint32_t dest2 = 0;
	x = ((file[column]&bit_board.occupied)^(1<<src)) >> (column);
	if (row == 0){
		dest2 = CGenCL(x);
	}
	else if (row == 1){
		dest2 = CGenCL(x&0x11111100);
	}
	else if (row == 2){
		if((x & 0x00000011) == 0x00000011)
			dest2 |= 1;
		dest2 |= CGenCL(x&0x11111000);
	}
	else if (row == 3){
		dest2 = CGenCR(x&0x00000111);
		dest2 |= CGenCL(x&0x11110000);
	}
	else if (row == 4){
		dest2 = CGenCR(x&0x00001111);
		dest2 |= CGenCL(x&0x11100000);
	}
	else if (row == 5){
		dest2 = CGenCR(x&0x00011111);
		dest2 |= CGenCL(x&0x11000000);
	}
	else if (row == 6){
		dest2 = CGenCR(x&0x00111111);
	}
	else{
		dest2 = CGenCR(x);
	}
	uint32_t result = dest << (4*row);
	result |= (dest2 << column);
	//fprintf(stderr, "%%%%%%%% result %u dest %u dest2 %u\n", result, dest, dest2);
	return result;
}

uint32_t MyAI::CGenCL(uint32_t x){
	if(x){
		uint32_t mask = LSB(x);
		return ((x ^= mask) ? LSB(x) : 0);
	}
	else{
		return 0;
	}
}

uint32_t MyAI::CGenCR(uint32_t x){
	if(x){
		uint32_t mask = MSB(x);
		return ((x^=mask) ? MSB(x) : 0);
	}
	else
		return 0;

}

// always use my point of view, so use this->Color
double MyAI::Evaluate(const int* board){
	// total score
	double score = 1943; // 1*5+180*2+6*2+18*2+90*2+270+810*1
	// static material values
	// cover and empty are both zero
	static const double values[16] = {0, 810, 270, 90, 18, 6, 180, 1, 810, 270, 90, 18, 6, 180, 1, 0};
	for(int i = 0; i < 32; i++){
		if(!(board[i] == CHESS_EMPTY || board[i] == CHESS_COVER)){
			if((board[i] - 1)/ 7 == this->Color){
				score += values[board[i]];
			}else{
				score -= values[board[i]];
			}
		}
	}
	return score;
}

double MyAI::Nega_scout(const int* board, const Node bit_board, int* move, const int red_chess_num, const int black_chess_num, const int* cover_chess, const int color, double alpha, double beta, const int depth, const int remain_depth){
	if(remain_depth == 0){ // reach limit of depth
		this->node++;
		return Evaluate(board) * (2*((depth&1)^1)-1); // odd: *-1, even: *1
	}else if(red_chess_num == 0 || black_chess_num == 0){ // terminal node (no chess type)
		this->node++;
		return Evaluate(board) * (2*((depth&1)^1)-1);
	}
	//Transposition
	double old_alpha = alpha;
	unsigned long hash_index = (bit_board.Counter_hash << 32) >> 32;
	Node_Info searched_node = {0, 0, -1, -1};
	if (Hash_table.count(hash_index) > 0){
		Hash_hit++;
		searched_node = Hash_table[hash_index];
		if(searched_node.tDepth > remain_depth){
			if (searched_node.tFlag == 0){
				return searched_node.tScore;
			}
			else if(searched_node.tFlag == 1){
				alpha = max(alpha, searched_node.tScore);
			}
			else{
				beta = min(alpha, searched_node.tScore);
			}
			if (alpha >= beta)
				return searched_node.tScore;
		}
	}

	int new_board[32], new_cover[16], new_move, new_red, new_black;
	Node new_bitboard;
	double m = -DBL_MAX;

	if (searched_node.tDepth >= 0){
		new_red = red_chess_num, new_black = black_chess_num;
		memcpy(new_board, board, sizeof(int)*32);
		memcpy(new_cover, cover_chess, sizeof(int)*16);
		new_bitboard = bit_board;

		MakeSimulationMove(new_board, new_bitboard, &new_red, &new_black, new_cover, searched_node.tMove, CHESS_COVER); 
		m = -Nega_scout(new_board, new_bitboard, &new_move, new_red, new_black, new_cover, color^1, -beta, -alpha, depth+1, remain_depth-1);
		*move = searched_node.tMove;

		if (m >= beta){
			Node_Info new_searched_node;
			if(m <= old_alpha){
				new_searched_node.tFlag = 2;
			}
			else if(m >= beta){
				new_searched_node.tFlag = 1;
			}
			else{
				new_searched_node.tFlag = 0;
			}
			new_searched_node.tMove = searched_node.tMove;
			new_searched_node.tScore = m;
			new_searched_node.tDepth = remain_depth;
			Hash_table[hash_index] = new_searched_node;
		}


	}
	int Result[2048];
	// Moves[] = {move} U {flip}, Chess[] = {remain chess}
	int Chess[2048];
	int flip_count = 0, remain_count = 0, remain_total = 0;
	int move_count = 0;

	// move
	//move_count = Expand(board, color, Result);
	move_count = Expand_bitboard(bit_board, color, Result);
	// flip
	for(int j = 1; j < 15; j++){ // find remain chess
		if(cover_chess[j] > 0){
			Chess[remain_count] = j;
			remain_count++;
			remain_total += cover_chess[j];
		}
	}
	uint32_t covered_chess = bit_board.piece[15];

	while(covered_chess){
		uint32_t mask = LSB(covered_chess);
		covered_chess ^= mask;
		int result = GetIndex(mask);
		Result[move_count + flip_count] = result*100+result;
		flip_count++;
	}
	//printf("move count %d flip count %d\n", move_count, flip_count); 
	//fflush(stdout);


	if(move_count + flip_count == 0){ // terminal node (no move type)
		this->node++;
		return Evaluate(board) * (2*((depth&1)^1)-1);
	}else{
		//double m = -DBL_MAX;
		double n = beta;
		// int new_board[32], new_cover[16], new_move, new_red, new_black;
		// Node new_bitboard;
		// search deeper
		for(int i = 0; i < move_count; i++){ // move
			if (Result[i] == searched_node.tMove)
				continue;
			new_red = red_chess_num, new_black = black_chess_num;
			memcpy(new_board, board, sizeof(int)*32);
			memcpy(new_cover, cover_chess, sizeof(int)*16);
			new_bitboard = bit_board;
			
			MakeSimulationMove(new_board, new_bitboard, &new_red, &new_black, new_cover, Result[i], CHESS_COVER); // -1: NULL
			double t = -Nega_scout(new_board, new_bitboard, &new_move, new_red, new_black, new_cover, color^1, -n, -max(alpha, m), depth+1, remain_depth-1);
			if(t > m){
				*move = Result[i];
				if (n == beta || remain_depth < 3 || t >= beta){
					m = t;
				}
				else{
					m = -Nega_scout(new_board, new_bitboard, &new_move, new_red, new_black, new_cover, color^1, -beta, -t, depth+1, remain_depth-1);
				} 
			}
			if(m >= beta){
				Node_Info new_searched_node;
				if(m <= old_alpha){
					new_searched_node.tFlag = 2;
				}
				else if(m >= beta){
					new_searched_node.tFlag = 1;
				}
				else{
					new_searched_node.tFlag = 0;
				}
				new_searched_node.tMove = *move;
				new_searched_node.tScore = m;
				new_searched_node.tDepth = remain_depth;
				Hash_table[hash_index] = new_searched_node;
				return m;
			}
			n = max(alpha, m) + 1;
		}
		for(int i = move_count; i < move_count + flip_count; i++){ // flip
			double total = 0;
			for(int k = 0; k < remain_count; k++){
				new_red = red_chess_num, new_black = black_chess_num;
				memcpy(new_board, board, sizeof(int)*32);
				memcpy(new_cover, cover_chess, sizeof(int)*16);
				new_bitboard = bit_board;
				//Print_BitBoard(new_bitboard);
				MakeSimulationMove(new_board, new_bitboard, &new_red, &new_black, new_cover, Result[i], Chess[k]);
				//Print_BitBoard(new_bitboard);
				double t = -Nega_scout(new_board, new_bitboard, &new_move, new_red, new_black, new_cover, color^1, -DBL_MAX, DBL_MAX, depth+1, remain_depth-1);
				//printf("t = %f Chess[k] = %c\n", t, piece_name[Chess[k]]);
				total += cover_chess[Chess[k]] * t;
			}

			double E_score = (total / remain_total); // calculate the expect value of flip
			if(E_score > m){ 
				m = E_score;
				*move = Result[i];
			}
			if(m >= beta){
				Node_Info new_searched_node;
				if(m <= old_alpha){
					new_searched_node.tFlag = 2;
				}
				else if(m >= beta){
					new_searched_node.tFlag = 1;
				}
				else{
					new_searched_node.tFlag = 0;
				}
				new_searched_node.tMove = *move;
				new_searched_node.tScore = m;
				new_searched_node.tDepth = remain_depth;
				Hash_table[hash_index] = new_searched_node;
				return m;
			}
		}
		Node_Info new_searched_node;
		if(m <= old_alpha){
			new_searched_node.tFlag = 2;
		}
		else if(m >= beta){
			new_searched_node.tFlag = 1;
		}
		else{
			new_searched_node.tFlag = 0;
		}
		new_searched_node.tMove = *move;
		new_searched_node.tScore = m;
		new_searched_node.tDepth = remain_depth;
		Hash_table[hash_index] = new_searched_node;
		return m;
	}
}

void MyAI::Print_BitBoard(Node& bit_board)
{
	printf("***********************************\n");
	for(int i = 0; i < 8; i++){
		printf("<%d>", i);
		for(int j = 3; j >= 0; j--){
			int now = i*4+j;
			for(int k = 0; k < 16; k++){
				if(bit_board.piece[k] & (1 << now)){
					printf("%5c", piece_name[k]);
				}
			}
		}	
		printf("\n");	
	}
	printf("\n***********************************\n");
	printf("covered_chess!!!!!!\n");
	for(int i = 0; i < 8; i++){
		printf("<%d>", i);
		for(int j = 3; j >= 0; j--){
			int now = i*4+j;
			if(bit_board.piece[15] & (1 << now)){
				printf("%5c", 'X');
			}
			else
				printf("%5c", '-');
		}	
		printf("\n");	
	}
	printf("Red!!!!!!\n");
	for(int i = 0; i < 8; i++){
		printf("<%d>", i);
		for(int j = 3; j >= 0; j--){
			int now = i*4+j;
			if(bit_board.red & (1 << now)){
				printf("%5c", 'R');
			}
			else
				printf("%5c", '-');
		}	
		printf("\n");	
	}
	printf("Black!!!!!!\n");
	for(int i = 0; i < 8; i++){
		printf("<%d>", i);
		for(int j = 3; j >= 0; j--){
			int now = i*4+j;
			if(bit_board.black & (1 << now)){
				printf("%5c", 'B');
			}
			else
				printf("%5c", '-');
		}	
		printf("\n");	
	}
	printf("@@@@ccupied\n");
	for(int i = 0; i < 8; i++){
		printf("<%d>", i);
		for(int j = 3; j >= 0; j--){
			int now = i*4+j;
			if(bit_board.occupied & (1 << now)){
				printf("%5c", 'O');
			}
			else
				printf("%5c", '-');
		}	
		printf("\n");	
	}
}

//Display chess board
void MyAI::Pirnf_Chessboard()
{	
	char myColor[10]="";
	if(Color == -99)
		strcpy(myColor,"Unknown");
	else if(this->Color == RED)
		strcpy(myColor,"Red");
	else
		strcpy(myColor,"Black");
	printf("------------%s-------------\n",myColor);
	printf("<8> ");
	for(int i=0;i<32;i++)
	{
		if(i != 0 && i % 4 == 0)
		{
			printf("\n<%d> ",8-(i/4));
		}

		printf("%5c", piece_name[this->Board[i]]);
	}
	printf("\n\n     ");
	for(int i=0;i<4;i++)
	{
		printf(" <%c> ",'a'+i);
	}
	printf("RED %d BLACK %d\n", Red_Chess_Num, Black_Chess_Num );
	printf("\n\n");

}

void MyAI::Print_Chessboard(const int *Board)
{	
	printf("------------check-------------\n");
	printf("<8> ");
	for(int i=0;i<32;i++)
	{
		if(i != 0 && i % 4 == 0)
		{
			printf("\n<%d> ",8-(i/4));
		}

		printf("%5c", piece_name[Board[i]]);
	}
	printf("\n\n     ");
	for(int i=0;i<4;i++)
	{
		printf(" <%c> ",'a'+i);
	}
	printf("\n\n");
}

//Nega Max
int MyAI::ConvertChessNo(int input)
{
	switch(input)
	{
	case 0:
		return -2;
		break;
	case 8:
		return -1;
		break;
	case 1:
		return 6;
		break;
	case 2:
		return 5;
		break;
	case 3:
		return 4;
		break;
	case 4:
		return 3;
		break;
	case 5:
		return 2;
		break;
	case 6:
		return 1;
		break;
	case 7:
		return 0;
		break;
	case 9:
		return 13;
		break;
	case 10:
		return 12;
		break;
	case 11:
		return 11;
		break;
	case 12:
		return 10;
		break;
	case 13:
		return 9;
		break;
	case 14:
		return 8;
		break;
	case 15:
		return 7;
		break;
	}
	return -1;
}

double MyAI::ONega_max(const int* board, int* move, const int red_chess_num, const int black_chess_num, const int* cover_chess, const int color, const int depth, const int remain_depth){
	if(remain_depth == 0){ // reach limit of depth
		this->onode++;
		return Max_Evaluate(board) * (2*((depth&1)^1)-1); // odd: *-1, even: *1
	}else if(red_chess_num == 0 || black_chess_num == 0){ // terminal node (no chess type)
		this->onode++;
		return Max_Evaluate(board) * (2*((depth&1)^1)-1);
	}

	int Result[1024];
	// Moves[] = {move} U {flip}, Chess[] = {remain chess}
	int Moves[2048], Chess[2048];
	int flip_count = 0, remain_count = 0, remain_total = 0;
	int move_count = 0;

	// move
	move_count = Expand(board, color, Result);
	memcpy(Moves, Result, sizeof(int)*move_count);
	// flip
	for(int j = 0; j < 14; j++){ // find remain chess
		if(cover_chess[j] > 0){
			Chess[remain_count] = j;
			remain_count++;
			remain_total += cover_chess[j];
		}
	}
	for(int i = 0; i < 32; i++){ // find cover
		if(board[i] == -1){
			Moves[move_count + flip_count] = i*100+i;
			flip_count++;
		}
	}
	//printf("nega max move count %d flip count %d\n", move_count, flip_count);
	if(move_count + flip_count == 0){ // terminal node (no move type)
		this->onode++;
		return Max_Evaluate(board) * (2*((depth&1)^1)-1);
	}else{
		double m = -DBL_MAX;
		int new_board[32], new_cover[14], new_move, new_red, new_black;
		// search deeper
		for(int i = 0; i < move_count; i++){ // move
			new_red = red_chess_num, new_black = black_chess_num;
			memcpy(new_board, board, sizeof(int)*32);
			memcpy(new_cover, cover_chess, sizeof(int)*14);
			
			MakeMove_Max(new_board, &new_red, &new_black, new_cover, Moves[i], -1); // -1: NULL
			double t = -ONega_max(new_board, &new_move, new_red, new_black, new_cover, color^1, depth+1, remain_depth-1);
			if(t > m){ 
				m = t;
				*move = Moves[i];
			}
		}
		for(int i = move_count; i < move_count + flip_count; i++){ // flip
			double total = 0;
			for(int k = 0; k < remain_count; k++){
				new_red = red_chess_num, new_black = black_chess_num;
				memcpy(new_board, board, sizeof(int)*32);
				memcpy(new_cover, cover_chess, sizeof(int)*14);
				
				MakeMove_Max(new_board, &new_red, &new_black, new_cover, Moves[i], Chess[k]);
				double t = -ONega_max(new_board, &new_move, new_red, new_black, new_cover, color^1, depth+1, remain_depth-1);
				//printf("Nega Max t = %f Chess[k] = %c\n", t, Max_piecename[Chess[k]]);
				total += cover_chess[Chess[k]] * t;
			}

			double E_score = (total / remain_total); // calculate the expect value of flip
			if(E_score > m){ 
				m = E_score;
				*move = Moves[i];
			}
		}
		return m;
	}
}

void MyAI::MakeMove_Max(int* board, int* red_chess_num, int* black_chess_num, int* cover_chess, const int move, const int chess){
	int src = move/100, dst = move%100;
	if(src == dst){ 
		board[src] = chess;
		cover_chess[chess]--;
	}else { 
		if(board[dst] != -2){
			if(board[dst] / 7 == 0){
				(*red_chess_num)--;
			}else{
				(*black_chess_num)--;
			}
		}
		board[dst] = board[src];
		board[src] = -2;
	}
}

void MyAI::MakeMove_Max(int* board, int* red_chess_num, int* black_chess_num, int* cover_chess, const char move[6])
{ 
	int src, dst;
	src = ('8'-move[1])*4+(move[0]-'a');
	if(move[2]=='('){ 
		printf("# call flip(): flip(%d,%d) = %d\n",src, src, Max_GetFin(move[3])); 
		board[src] = ConvertChessNo(Max_GetFin(move[3]));
		cover_chess[ConvertChessNo(Max_GetFin(move[3]))]--;
		//Pirnf_Chessboard();
	}else { 
		dst = ('8'-move[4])*4+(move[3]-'a');
		printf("# call move(): move : %d-%d \n",src,dst); 
		if(board[dst] != -2){
			if(board[dst] / 7 == 0){
				(*red_chess_num)--;
			}else{
				(*black_chess_num)--;
			}
		}
		board[dst] = board[src];
		board[src] = -2;
		//Pirnf_Chessboard();
	}
	/* init time */
}

int MyAI::Expand(const int* board, const int color,int *Result)
{
	int ResultCount = 0;
	for(int i=0;i<32;i++)
	{
		if(board[i] >= 0 && board[i]/7 == color)
		{
			//Cannon 
			if(board[i] % 7 == 1)
			{
				int row = i/4;
				int col = i%4;
				//jump horizontal?? <-
				for(int rowCount=row*4;rowCount<(row+1)*4;rowCount++)
				{
					if(Referee(board,i,rowCount,color))
					{
						Result[ResultCount] = i*100+rowCount;
						ResultCount++;
					}
				}
				for(int colCount=col; colCount<32;colCount += 4)
				{
				
					if(Referee(board,i,colCount,color))
					{
						Result[ResultCount] = i*100+colCount;
						ResultCount++;
					}
				}
			}
			else
			{
				int Move[4] = {i-4,i+1,i+4,i-1};
				for(int k=0; k<4;k++)
				{
					
					if(Move[k] >= 0 && Move[k] < 32 && Referee(board,i,Move[k],color))
					{
						Result[ResultCount] = i*100+Move[k];
						ResultCount++;
					}
				}
			}
		
		};
	}
	return ResultCount;
}


// Referee
bool MyAI::Referee(const int* chess, const int from_location_no, const int to_location_no, const int UserId)
{
	int MessageNo = 0;
	bool IsCurrent = true;
	int from_chess_no = chess[from_location_no];
	int to_chess_no = chess[to_location_no];
	int from_row = from_location_no / 4;
	int to_row = to_location_no / 4;
	int from_col = from_location_no % 4;
	int to_col = to_location_no % 4;

	if(from_chess_no < 0 || ( to_chess_no < 0 && to_chess_no != -2) )
	{  
		MessageNo = 1;
		//strcat(Message,"**no chess can move**");
		//strcat(Message,"**can't move darkchess**");
		IsCurrent = false;
	}
	else if (from_chess_no >= 0 && from_chess_no / 7 != UserId)
	{
		MessageNo = 2;
		//strcat(Message,"**not my chess**");
		IsCurrent = false;
	}
	else if((from_chess_no / 7 == to_chess_no / 7) && to_chess_no >= 0)
	{
		MessageNo = 3;
		//strcat(Message,"**can't eat my self**");
		IsCurrent = false;
	}
	//check attack
	else if(to_chess_no == -2 && abs(from_row-to_row) + abs(from_col-to_col)  == 1)//legal move
	{
		IsCurrent = true;
	}	
	else if(from_chess_no % 7 == 1 ) //judge cannon
	{
		int row_gap = from_row-to_row;
		int col_gap = from_col-to_col;
		int between_Count = 0;
		//slant
		if(from_row-to_row == 0 || from_col-to_col == 0)
		{
			//row
			if(row_gap == 0) 
			{
				for(int i=1;i<abs(col_gap);i++)
				{
					int between_chess;
					if(col_gap>0)
						between_chess = chess[from_location_no-i] ;
					else
						between_chess = chess[from_location_no+i] ;
					if(between_chess != -2)
						between_Count++;
				}
			}
			//column
			else
			{
				for(int i=1;i<abs(row_gap);i++)
				{
					int between_chess;
					if(row_gap > 0)
						between_chess = chess[from_location_no-4*i] ;
					else
						between_chess = chess[from_location_no+4*i] ;
					if(between_chess != -2)
						between_Count++;
				}
				
			}
			
			if(between_Count != 1 )
			{
				MessageNo = 4;
				//strcat(Message,"**gun can't eat opp without between one piece**");
				IsCurrent = false;
			}
			else if(to_chess_no == -2)
			{
				MessageNo = 5;
				//strcat(Message,"**gun can't eat opp without between one piece**");
				IsCurrent = false;
			}
		}
		//slide
		else
		{
			MessageNo = 6;
			//strcat(Message,"**cant slide**");
			IsCurrent = false;
		}
	}
	else // non gun
	{
		//judge pawn or king

		//distance
		if( abs(from_row-to_row) + abs(from_col-to_col)  > 1)
		{
			MessageNo = 7;
			//strcat(Message,"**cant eat**");
			IsCurrent = false;
		}
		//judge pawn
		else if(from_chess_no % 7 == 0)
		{
			if(to_chess_no % 7 != 0 && to_chess_no % 7 != 6)
			{
				MessageNo = 8;
				//strcat(Message,"**pawn only eat pawn and king**");
				IsCurrent = false;
			}
		}
		//judge king
		else if(from_chess_no % 7 == 6 && to_chess_no % 7 == 0)
		{
			MessageNo = 9;
			//strcat(Message,"**king can't eat pawn**");
			IsCurrent = false;
		}
		else if(from_chess_no % 7 < to_chess_no% 7)
		{
			MessageNo = 10;
			//strcat(Message,"**cant eat**");
			IsCurrent = false;
		}
	}
	return IsCurrent;
}

double MyAI::Max_Evaluate(const int* board){
	// total score
	double score = 1943; // 1*5+180*2+6*2+18*2+90*2+270*2+810*1
	// static material values
	// cover and empty are both zero
	static const double values[14] = {1,180,6,18,90,270,810,1,180,6,18,90,270,810};
	for(int i = 0; i < 32; i++){
		if(!(board[i] == -2 || board[i] == -1)){
			if(board[i] / 7 == this->Color){
				score += values[board[i]];
			}else{
				score -= values[board[i]];
			}
		}
	}

	return score;
}

int MyAI::Max_GetFin(char c)
{
	static const char skind[]={'-','K','G','M','R','N','C','P','X','k','g','m','r','n','c','p'};
	for(int f=0;f<16;f++)if(c==skind[f])return f;
	return -1;
}


