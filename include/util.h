#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define PRINT_DASH printf("==============================================\n");

enum color_t
{ none = 0, black = 1, white = 2 };

enum piece_t

{ empty = 5, pawn = 0, knight = 6, bishop = 2, rook = 4, king = 3, queen = 1 };

enum attack_t
{
    by_white = 1,
    by_black = -1,
    safe_square = 0,
    both_attacked = 8
};

enum game_status
{
	session_active = 	0x01,
	winner_black = 	0x02,
	winner_white = 	0x03,
	end_stalemate = 	0x04
};

enum action_t
{
    attacking,
    leaving
};


struct piece
{
	enum color_t 		side;
	enum piece_t 		type;
};


struct square
{
	uint8_t			w_attack;
    uint8_t 			b_attack;
	enum color_t      	side;
	struct piece 		obj;
 // It needs for optimization of after move alghoritm
};

enum {
    WHITE_OO  = 1 << 0, // 0001
    WHITE_OOO = 1 << 1, // 0010
    BLACK_OO  = 1 << 2, // 0100
    BLACK_OOO = 1 << 3  // 1000
};

struct chess {
	uint8_t			castling_flags;
	uint8_t 			kpos_w;
	uint8_t 			kpos_b;
    uint8_t 			last_move[2];
    enum piece_t 		pawn_transformation;
	enum color_t 		player_side;
	enum game_status	status;
	struct square 		board[8][8];
};

struct square *
create_board(struct square (*board)[8]);

struct square *set_training_board(struct square (*board)[8], char *custom_pos);

enum attack_t get_attack_t(struct square *sq);

uint8_t
find_figure(struct square (*board)[8], enum color_t side, enum piece_t type);

void
print_square_info(struct square *sq);

void
init_engine(struct chess *engine);

#endif // UTIL_H
