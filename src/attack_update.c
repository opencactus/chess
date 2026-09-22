#include "../include/attack_update.h"
#include "../include/util.h"
#include "../include/logging.h"

#include <stdint.h>



uint8_t square_state_upd_by_attacking(struct square *sq,
								   enum color_t *color) {
	if (*color == white)
		sq->w_attack++;
	else
		sq->b_attack++;
	return 0;
}


uint8_t square_state_upd_by_leaving(struct square *sq,
								 enum color_t *color) {
	if (*color == white && sq->w_attack > 0)
		sq->w_attack--;
	else if (*color == black && sq->b_attack > 0) 
		sq->b_attack--;
	return 0;
}


uint8_t
square_is_safe(struct square *sq, enum color_t *side) {
	return ((*side == white && sq->b_attack == 0)
		  || (*side == black && sq->w_attack == 0))
		&& (sq->obj.side != *side);
}


uint8_t
square_is_free(struct square *sq, enum color_t *side) {
	return (sq->obj.side != *side);
}


uint8_t
knight_pos_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
				  uint8_t (*upd_func)(struct square *, enum color_t *))
{
	uint8_t counter = 0;
	if (POS_XP > 1)
    {
		if (POS_YP == 0)
        {
			counter += upd_func(&board[POS_XP - 2][POS_YP + 1], side);
				  
        }
		else if (POS_YP == 7)
        {
			counter += upd_func(&board[POS_XP - 2][POS_YP - 1], side);
        }
		else
        {
			counter += upd_func(&board[POS_XP - 2][POS_YP - 1], side);

			counter += upd_func(&board[POS_XP - 2][POS_YP + 1], side);
        }
    }
	if (POS_XP < 6)
    {
		if (POS_YP == 0)
        {
			counter += upd_func(&board[POS_XP + 2][POS_YP + 1], side);
        }

		else if (POS_YP == 7)
        {
			counter += upd_func(&board[POS_XP + 2][POS_YP - 1], side);
        }
		else
        {
			counter += upd_func(&board[POS_XP + 2][POS_YP - 1], side);

			counter += upd_func(&board[POS_XP + 2][POS_YP + 1], side);
        }
    }
	if (POS_YP > 1)
    {
		if (POS_XP == 0)
        {
			counter += upd_func(&board[POS_XP + 1][POS_YP - 2], side);
        }
		else if (POS_XP == 7)
        {
			counter += upd_func(&board[POS_XP - 1][POS_YP - 2], side);
        }
		else
        {
			counter += upd_func(&board[POS_XP + 1][POS_YP - 2], side);

			counter += upd_func(&board[POS_XP - 1][POS_YP - 2], side);
        }
    }
	if (POS_YP < 6)
    {
		if (POS_XP == 0)
        {
			counter += upd_func(&board[POS_XP + 1][POS_YP + 2], side);
        }
		else if (POS_XP == 7)
        {
			counter += upd_func(&board[POS_XP - 1][POS_YP + 2], side);
        }
		else
        {
			counter += upd_func(&board[POS_XP + 1][POS_YP + 2], side);

			counter += upd_func(&board[POS_XP - 1][POS_YP + 2], side);
        }
    }
	return counter;
}


uint8_t
pawn_pos_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
                uint8_t (*upd_func)(struct square *, enum color_t *)) {
#define PAWN_XLEVEL POS_XP + (*side == black) - (*side == white)
	uint8_t counter = 0;
	if (POS_YP == 0) {
		counter += upd_func(&board[PAWN_XLEVEL][POS_YP + 1], side);
		
	}
	else if (POS_YP == 7) {
		counter += upd_func(&board[PAWN_XLEVEL][POS_YP - 1], side);
		
	} else {
		counter += upd_func(&board[PAWN_XLEVEL][POS_YP + 1], side);
		
		counter += upd_func(&board[PAWN_XLEVEL][POS_YP - 1], side);
		
	}
	return counter;
#undef PAWN_XLEVEL
}



static uint8_t
direct_line_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
				   uint8_t (*upd_func)(struct square *, enum color_t *)) {
	uint8_t counter = 0;
	for (uint8_t j = POS_YP + 1; j < 8; ++j) {
		counter += upd_func(&board[POS_XP][j], side);
		if (board[POS_XP][j].obj.type != empty)
			break;
	}
	for (int8_t j = POS_YP - 1; j >= 0; --j) {
		counter += upd_func(&board[POS_XP][j], side);          
		if (board[POS_XP][j].obj.type != empty)
			break;
	}
	for (uint8_t i = POS_XP + 1; i < 8; ++i) {
		counter += upd_func(&board[i][POS_YP], side);          
		if (board[i][POS_YP].obj.type != empty)
			break;
	}
	for (int8_t i = POS_XP - 1; i >= 0; --i) {
		counter += upd_func(&board[i][POS_YP], side);
		if (board[i][POS_YP].obj.type != empty)
			break;
	}
	return counter;
}


static uint8_t
diagonal_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
                uint8_t (*upd_func)(struct square *, enum color_t *)) {
	DEBUG_MSG("diagonal_update");
	uint8_t counter = 0;
	for (uint8_t j = POS_YP + 1; j < 8 && POS_XP - (j - POS_YP) >= 0; ++j) {
		DEBUG_MSG("iter l1 %d\n", j);
		counter += upd_func(&board[POS_XP - (j - POS_YP)][j], side);
		if (board[POS_XP - (j - POS_YP)][j].obj.type != empty)
			break;
	}
	DEBUG_MSG("loop #2\n");
	for (uint8_t j = POS_YP + 1; j < 8 && POS_XP + (j - POS_YP) < 8; ++j) {
		DEBUG_MSG("iter l3 %d\n", j);
		counter += upd_func(&board[POS_XP + (j - POS_YP)][j], side);
		if (board[POS_XP + (j - POS_YP)][j].obj.type != empty)
			break;
	}
	DEBUG_MSG("loop #3\n");
	for (int8_t j = POS_YP - 1; j >= 0 && POS_XP - (POS_YP - j) >= 0; --j) {
		DEBUG_MSG("iter l2 %d\n", j);
		counter += upd_func(&board[POS_XP - (POS_YP - j)][j], side);
		if (board[POS_XP - (POS_YP - j)][j].obj.type != empty)
			break;
		
	}
	DEBUG_MSG("loop #4\n");
	for (int8_t j = POS_YP - 1; j >= 0 && POS_XP + (POS_YP - j) < 8; --j) {
		DEBUG_MSG("iter l4 %d\n", j);
		counter += upd_func(&board[POS_XP + (POS_YP - j)][j], side);
		if (board[POS_XP + (POS_YP - j)][j].obj.type != empty)
			break;
	}
	return counter;
}


uint8_t
bishop_pos_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
				  uint8_t (*upd_func)(struct square *, enum color_t *)) {
	return diagonal_update(board, side, pos, upd_func);
}


uint8_t
rook_pos_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
                uint8_t (*upd_func)(struct square *, enum color_t *)) {
	return direct_line_update(board, side, pos, upd_func);
}


uint8_t
queen_pos_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
                 uint8_t (*upd_func)(struct square *, enum color_t *)) {
	return	direct_line_update(board, side, pos, upd_func) +
			diagonal_update(board, side, pos, upd_func);
}  


uint8_t
king_pos_update(struct square (*board)[8], enum color_t *side, uint8_t *pos,
                uint8_t (*upd_func)(struct square *, enum color_t *)) {
	uint8_t free_sq = 0;
	if (POS_XP == 0) {
		if (POS_YP < 7) {
			free_sq += upd_func(&board[POS_XP + 1][POS_YP + 1], side);
			free_sq += upd_func(&board[POS_XP][POS_YP + 1], side);
		}
		if (POS_YP > 0) {
			free_sq += upd_func(&board[POS_XP + 1][POS_YP - 1], side);
			free_sq += upd_func(&board[POS_XP][POS_YP - 1], side);
		}      
		free_sq += upd_func(&board[POS_XP + 1][POS_YP], side);
		
	}
	else if (POS_XP == 7) {
		if (POS_YP < 7) {
			free_sq += upd_func(&board[POS_XP - 1][POS_YP + 1], side);
			free_sq += upd_func(&board[POS_XP][POS_YP + 1], side);
		}
		if (POS_YP > 0) {
			free_sq += upd_func(&board[POS_XP - 1][POS_YP - 1], side);
			free_sq += upd_func(&board[POS_XP][POS_YP - 1], side);
		}
		free_sq += upd_func(&board[POS_XP - 1][POS_YP], side);
	} else {
		if (POS_YP < 7) {
			free_sq += upd_func(&board[POS_XP + 1][POS_YP + 1], side);
			free_sq += upd_func(&board[POS_XP][POS_YP + 1], side);						
			free_sq += upd_func(&board[POS_XP - 1][POS_YP + 1], side);
		}
		if (POS_YP > 0) {
			free_sq += upd_func(&board[POS_XP + 1][POS_YP - 1], side);
			free_sq += upd_func(&board[POS_XP - 1][POS_YP - 1], side);
			free_sq += upd_func(&board[POS_XP][POS_YP - 1], side);                        
		}      
		free_sq += upd_func(&board[POS_XP + 1][POS_YP], side);
		free_sq += upd_func(&board[POS_XP - 1][POS_YP], side);
	}

	return free_sq;
}    
