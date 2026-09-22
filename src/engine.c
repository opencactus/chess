#include "../include/util.h"
#include "../include/engine.h"
#include "../include/logging.h"
#include "../include/attack_update.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>


static struct piece *
check_between_direct_line (struct square (*board)[8], uint8_t *opos,
						   uint8_t *npos)
{
    if (NPOS_XP == OPOS_XP) {
        uint8_t min = (OPOS_YP < NPOS_YP) ? OPOS_YP : NPOS_YP;
        uint8_t max = (OPOS_YP > NPOS_YP) ? OPOS_YP : NPOS_YP;
        for (uint8_t between = min + 1; between < max; ++between)
            if (board[OPOS_XP][between].obj.type != empty)
                return &board[OPOS_XP][between].obj;
    }
    else {
        uint8_t min = (OPOS_XP < NPOS_XP) ? OPOS_XP : NPOS_XP;
        uint8_t max = (OPOS_XP > NPOS_XP) ? OPOS_XP : NPOS_XP;
        for (uint8_t between = min + 1; between < max; ++between)
        {
            if (board[between][OPOS_YP].obj.type != empty)
                return &board[between][OPOS_YP].obj;
        }
    }
    return NULL;
}


// Why Do I use "struct square *" here? TODO
static struct square *
check_between_diagonal (struct square (*board)[8], uint8_t *opos,
                        uint8_t *npos)
{
    uint8_t min = (OPOS_XP < NPOS_XP) ? *opos - 1 : *npos - 1;
    uint8_t max = (OPOS_XP > NPOS_XP) ? *opos - 1 : *npos - 1;
    if (min % 8 < max % 8)
    {
        for (uint8_t diag = min / 8 + 1; diag < max / 8; ++diag)
        {
            if (board[diag][min % 8 + (diag - (min / 8))].obj.type
				!= empty)
                return &board[diag][min % 8 + (diag - (min / 8))];
        }
    }
    else
    {
        for (uint8_t diag = min / 8 + 1; diag < max / 8; ++diag)
        {
            if (board[diag][min % 8 - (diag - (min / 8))].obj.type
				!= empty)
                return &board[diag][min % 8 - (diag - (min / 8))];
        }
    }
    return NULL;
}


static uint8_t
check_hidden_king_attack (	struct square (*board)[8], uint8_t *kpos_w,
							uint8_t *kpos_b, uint8_t *opos) {
	uint8_t kpos = board[OPOS_XP][OPOS_YP].obj.side == white
		? *kpos_w 
		: *kpos_b;
	uint8_t nepos = 0;
    if (OPOS_XP == KPOS_X) { // horizontal
        struct piece *non_empty = check_between_direct_line(
			board, opos, &kpos);
        if (non_empty == NULL) {
            if (OPOS_YP > KPOS_Y) {
                for (uint8_t horiz = OPOS_YP + 1; horiz < 8; ++horiz) {
                    if (board[OPOS_XP][horiz].obj.type != empty) {
                        non_empty = &board[OPOS_XP][horiz].obj;
						nepos = OPOS_XP * 8 + horiz + 1;
                        break;
                    }
                }
            }
            else {
                for (int8_t horiz = OPOS_YP - 1; horiz >= 0; --horiz) {
                    if (board[OPOS_XP][horiz].obj.type != empty) {
                        non_empty = &board[OPOS_XP][horiz].obj;
						nepos = OPOS_XP * 8 + horiz + 1;
                        break;
                    }
                }
            }
        }
        if (non_empty == NULL
						|| non_empty->side == board[OPOS_XP][OPOS_YP].obj.side)
            return 0;
        if (non_empty->type == rook || non_empty->type == queen)
            return nepos;
        return 0;
    }

    if (OPOS_YP == KPOS_Y) { // vertical
        struct piece *non_empty = check_between_direct_line(
			board, opos, &kpos);
        if (non_empty == NULL) {
            if (OPOS_XP < KPOS_X) {
                for (int8_t vert = OPOS_XP - 1; vert >= 0; --vert) {
                    if (board[vert][OPOS_YP].obj.type != empty) {
                        non_empty = &board[vert][OPOS_YP].obj;
						nepos = vert * 8 + OPOS_YP + 1;
                        break;
                    }
                }
            }
            else {
                for (uint8_t vert = OPOS_XP + 1; vert < 8; ++vert) {
                    if (board[vert][OPOS_YP].obj.type != empty) {
                        non_empty = &board[vert][OPOS_YP].obj;
						nepos = vert * 8 + OPOS_YP + 1;
                        break;
                    }
                }
            }
        }
        if (non_empty == NULL
				|| non_empty->side == board[OPOS_XP][OPOS_YP].obj.side)
            return 0;
        if (non_empty->type == rook || non_empty->type == queen)
            return nepos;
        return 0;
    }
    // DIAGONAL
    if (abs(OPOS_XP - KPOS_X) == abs(OPOS_YP - KPOS_Y)) {
        struct square *non_empty = check_between_diagonal(
			board, opos, &kpos);
	
        if (non_empty == NULL) {
            if (OPOS_XP < KPOS_X) {
                if (OPOS_YP < KPOS_Y) {
                    DEBUG_MSG("diagonal loop #1");
                    for (int8_t diag = OPOS_XP - 1;
                         diag >= 0 && OPOS_YP - (OPOS_XP - diag) >= 0; --diag) {
                        /* DEBUG_MSG("%d", board[diag][OPOS_YP - (OPOS_XP - diag)].obj.type); */
                        if (board[diag][OPOS_YP - (OPOS_XP - diag)].obj.type
																	!= empty) {
							non_empty = &board[diag][OPOS_YP - (OPOS_XP - diag)];
							nepos = diag * 8 + (OPOS_YP - (OPOS_XP - diag)) + 1;
							break;
                        }
                    }
                }
                else {
                    DEBUG_MSG("diagonal loop #2");
                    for (int8_t diag = OPOS_XP - 1;
                         diag >= 0 && OPOS_YP + (OPOS_XP - diag) < 8; --diag)
						if (board[diag][OPOS_YP + (OPOS_XP - diag)]
							.obj.type != empty) {
							non_empty = &board[diag][OPOS_YP + (OPOS_XP - diag)];
							nepos = diag * 8 + (OPOS_YP + (OPOS_XP - diag)) + 1;
							break;
                        }
                }
            }
            else {
                DEBUG_MSG("diagonal loop #3");
                if (OPOS_YP < KPOS_Y) {
					for (int8_t diag = OPOS_XP + 1;
						 diag < 8 && OPOS_YP - (diag - OPOS_XP) >= 0; diag++)
						if (board[diag][OPOS_YP - (diag - OPOS_XP)]
							.obj.type != empty) {
							non_empty = &board[diag][OPOS_YP - (diag - OPOS_XP)];
							nepos = diag * 8 + (OPOS_YP - (diag - OPOS_XP)) + 1;
							break;
                        }
                }
                else {
                    DEBUG_MSG("diagonal loop #4");
                    for (int8_t diag = OPOS_XP + 1;
                         diag < 8 && OPOS_YP + (diag - OPOS_XP) < 8; diag++)
						if (board[diag][OPOS_YP + (diag - OPOS_XP)]
							.obj.type != empty) {
							non_empty = &board[diag][OPOS_YP + (diag - OPOS_XP)];
							nepos = diag * 8 + (OPOS_YP + (diag - OPOS_XP)) + 1;
							break;
					}
                }
            }
        }
		if (non_empty == NULL
				|| non_empty->obj.side == board[OPOS_XP][OPOS_YP].obj.side)
				return 0;
        if (non_empty->obj.type == bishop || non_empty->obj.type == queen)
            return nepos;
        return 0;
    }
    return 0;
}


static int32_t
check_pawn_move (struct chess *global, uint8_t *opos, uint8_t *npos)
{
    if ((NPOS_XP - OPOS_XP >= 0 && global->board[OPOS_XP][OPOS_YP].obj.side
		 == white)
        || (OPOS_XP - NPOS_XP >= 0 && global->board[OPOS_XP][OPOS_YP].obj.side
            == black))
    {
        DEBUG_MSG("ERROR_PAWN_ILLEGAL_DIRECTION");
        return ERROR_PAWN_ILLEGAL_DIRECTION;
    }
    else if (abs(OPOS_XP - NPOS_XP) > 2)
    {
        DEBUG_MSG("ERROR_PAWN_LEN_MORE_THEN_2");
        return ERROR_PAWN_LEN_MORE_THEN_2;
    }
    else if (OPOS_YP != NPOS_YP) // attacking
    {
        if (abs(OPOS_YP - NPOS_YP) > 1 || abs(OPOS_XP - NPOS_XP) > 1)
        {
            DEBUG_MSG("ERROR_PAWN_ILLEGAL_DIAGONAL_ATTACK");
            return ERROR_PAWN_ILLEGAL_DIAGONAL_ATTACK;
        }
        if (global->board[NPOS_XP][NPOS_YP].obj.type == empty)
        {
            if (global->board[(global->last_move[1] - 1) / 8][(global->last_move[1] - 1) % 8]
				.obj.type == pawn)
            {
                if (abs(((global->last_move[1] - 1) / 8) - ((global->last_move[0] - 1) / 8))
					== 2)
					if (NPOS_YP == ((global->last_move[1] - 1) % 8)) {
						pawn_pos_update(global->board, &global->board[(global->last_move[1] - 1) / 8][(global->last_move[1] - 1) % 8]
										.obj.side, &global->last_move[1], SQ_UPD_LEAVE);
						global->board[(global->last_move[1] - 1) / 8][(global->last_move[1] - 1) % 8]
							.obj.type = empty;
						global->board[(global->last_move[1] - 1) / 8][(global->last_move[1] - 1) % 8]
							.obj.side = none;
						return 0;
					}                    
            }
            DEBUG_MSG("ERROR_PAWN_ATTACK_EMPTY_SQUARE");
            return ERROR_PAWN_ATTACK_EMPTY_SQUARE;
        }
    }
    else
    {
        if (abs(OPOS_XP - NPOS_XP) == 2)
        {
            if ((OPOS_XP == 1 && global->board[OPOS_XP][OPOS_YP].obj.side
				 == black) 
                || (OPOS_XP == 6 && global->board[OPOS_XP][OPOS_YP].obj.side
                    == white))
            {
                new_debug_record("a pawn move in 2 square is correct");
                return 0;
            }
            else
            {
                new_debug_record("ERROR_PAWN_MOVE_2_NOT_IN_START");
                return ERROR_PAWN_MOVE_2_NOT_IN_START;
            }
        }
        if (global->board[NPOS_XP][NPOS_YP].obj.type != empty)
        {
            new_debug_record("ERROR_PAWN_MEET_BARRIER");
            return ERROR_PAWN_MEET_BARRIER;
        }
    }
    return 0;
}


static int32_t
check_bishop_move (struct square (*board)[8], uint8_t *opos, uint8_t *npos)
{
    if (abs(OPOS_XP - NPOS_XP) != abs(OPOS_YP - NPOS_YP))
    {
        new_debug_record("Incorrect bishop move");
        return ERROR_BISHOP_INCORRECT_MOVE;
    }
    if (check_between_diagonal(board, opos, npos))
    {
        new_debug_record("A piece between diagonal move bishop");
        return ERROR_BISHOP_MOVE_THROUGHOUT_A_PIECE;
    }
    return 0;
}


static int32_t
check_knight_move (uint8_t *opos, uint8_t *npos)
{

    if (((abs(OPOS_XP - NPOS_XP) == 1 && abs(OPOS_YP - NPOS_YP) == 2)
		 || ((abs(OPOS_XP - NPOS_XP) == 2 && abs(OPOS_YP - NPOS_YP) == 1))))
        return 0;
    return ERROR_KNIGHT_INCORRECT_MOVE;
}


static int32_t
check_rook_move (struct square (*board)[8], uint8_t *opos, uint8_t *npos)
{
    if (OPOS_XP == NPOS_XP)
    {
        if (check_between_direct_line(board, opos, npos) != NULL)
        {
            new_debug_record("XP ERROR_BETWEEN_OPOS_NPOS_PIECES");
            return ERROR_BETWEEN_OPOS_NPOS_PIECES;
        }
        else
            return 0;
    }
    else if (OPOS_YP == NPOS_YP)
    {
        if (check_between_direct_line(board, opos, npos) != NULL)
        {
            new_debug_record("YP ERROR_BETWEEN_OPOS_NPOS_PIECES");
            return ERROR_BETWEEN_OPOS_NPOS_PIECES;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        new_debug_record("ERROR_ROOK_INCORRECT_MOVE");
        return ERROR_ROOK_INCORRECT_MOVE;
    }
}


static int32_t
check_queen_move (struct square (*board)[8], uint8_t *opos, uint8_t *npos)
{
    if (OPOS_XP == NPOS_XP)
    {
        if (check_between_direct_line(board, opos, npos) != NULL)
        {
            new_debug_record("XP ERROR_BETWEEN_OPOS_NPOS_PIECES");
            return ERROR_BETWEEN_OPOS_NPOS_PIECES;
        }
        else
        {
            return 0;
        }
    }
    else if (OPOS_YP == NPOS_YP)
    {
        if (check_between_direct_line(board, opos, npos) != NULL)
        {
            new_debug_record("YP ERROR_BETWEEN_OPOS_NPOS_PIECES");
            return ERROR_BETWEEN_OPOS_NPOS_PIECES;
        }
        else
        {
            return 0;
        }
    }
    else if (abs(OPOS_XP - NPOS_XP) == abs(OPOS_YP - NPOS_YP))
    {
        if (check_between_diagonal(board, opos, npos) != NULL)
        {
            new_debug_record("A piece between diagonal move queen");
            return ERROR_QUEEN_MOVE_THROUGHOUT_A_PIECE;
        }
    }
    else
    {
        new_debug_record("ERROR_QUEEN_INCORRECT_MOVE");
        return ERROR_QUEEN_INCORRECT_MOVE;
    }
    return 0; 
}


static int32_t
check_king_move (struct square (*board)[8], uint8_t *opos, uint8_t *npos)
{
    // Bad realization. Code repeat. Need to rewrite in the future
    switch (get_attack_t(&board[NPOS_XP][NPOS_YP]))
    {
	case by_white:
	{
		if (board[OPOS_XP][OPOS_YP].obj.side == black)
		{
			new_debug_record("ERROR_KING_MOVE_TO_ATTACKED_SQUARE");
			return ERROR_KING_MOVE_TO_ATTACKED_SQUARE; 
		}
		break;
	}
	case by_black:
	{
		if (board[OPOS_XP][OPOS_YP].obj.side == white)
		{
			new_debug_record("ERROR_KING_MOVE_TO_ATTACKED_SQUARE");
			return ERROR_KING_MOVE_TO_ATTACKED_SQUARE; 
		}
		break;
	}
	case both_attacked:
	{
		new_debug_record("ERROR_KING_MOVE_TO_ATTACKED_SQUARE");
		return ERROR_KING_MOVE_TO_ATTACKED_SQUARE; 
	}
	default:
		break;
    }
    if (abs(OPOS_XP - NPOS_XP) > 1 || abs(NPOS_YP - OPOS_YP) > 1)
    {
        new_debug_record("ERROR_KING_LEN_MORE_THEN_1");
        return ERROR_KING_LEN_MORE_THEN_1;
    }
    return 0;
}


static uint8_t clean_the_piece_attack(struct square (*board)[8], uint8_t *pos) {

	switch (board[POS_XP][POS_YP].obj.type) {
	case pawn:
		return pawn_pos_update(board, &board[POS_XP][POS_YP].obj.side,
							   pos, SQ_UPD_LEAVE);
	case knight:
		return knight_pos_update(board, &board[POS_XP][POS_YP].obj.side,
								 pos, SQ_UPD_LEAVE);
	case king: 
		return king_pos_update(board, &board[POS_XP][POS_YP].obj.side,
							   pos, SQ_UPD_LEAVE);
	case queen:
		return queen_pos_update(board, &board[POS_XP][POS_YP].obj.side,
								pos, SQ_UPD_LEAVE);
	case rook:
		return rook_pos_update(board, &board[POS_XP][POS_YP].obj.side,
							   pos, SQ_UPD_LEAVE);
	case bishop:
		return bishop_pos_update(board, &board[POS_XP][POS_YP].obj.side,
								 pos, SQ_UPD_LEAVE);
	case empty: { return 1; }
	}
	/* if (non_empty && (non_empty->type == queen || non_empty->type ==
	 * rook)) */
	return 1;
}


// TODO: end this function using kngiht_npos_update as example
static uint8_t npos_update(struct square (*board)[8], enum color_t side,
                           enum piece_t type, uint8_t *npos) {
	if (side == none)
		return ERROR_INPUT_ABSENT_PIECES;
  
    switch (type) {
	    case pawn:
			return pawn_pos_update(board, &side, npos, SQ_UPD_ATTCK);
		case knight:
			return knight_pos_update(board, &side, npos, SQ_UPD_ATTCK);
		case king:
			return king_pos_update(board, &side, npos, SQ_UPD_ATTCK);
		case queen:
			return queen_pos_update(board, &side, npos, SQ_UPD_ATTCK);
		case rook: 
			return rook_pos_update(board, &side, npos, SQ_UPD_ATTCK);
		case bishop:
			return bishop_pos_update(board, &side, npos, SQ_UPD_ATTCK);
	}
	return ERROR_INPUT_ABSENT_PIECES;
}


static uint8_t *check_hidden_attack(struct square (*board)[8], uint8_t *pos,
                                    uint8_t (*upd_func)(struct square *,
                                                              enum color_t *))
{
	DEBUG_MSG("check_hidden_attack open");
	struct piece *non_empty = NULL;

	if (POS_YP != 0) {
		for (uint8_t j = POS_YP + 1; j < 8; ++j)
			if (board[POS_XP][j].obj.type != empty) {
				non_empty = &board[POS_XP][j].obj;
				break;
			}
		if (non_empty &&
			(non_empty->type == rook || non_empty->type == queen)) {
			DEBUG_MSG("loop #1");
			for (int8_t j = POS_YP - 1; j >= 0; --j) {
				upd_func(
					&board[POS_XP][j], &non_empty->side);
				if (board[POS_XP][j].obj.type != empty) break;
			}
		}
		non_empty = NULL;
                
		for (uint8_t j = POS_YP + 1; j < 8 && POS_XP - (j - POS_YP) >= 0; ++j)
			if (board[POS_XP - (j - POS_YP)][j].obj.type != empty) {
				non_empty = &board[POS_XP - (j - POS_YP)][j].obj;
				break;
			}
		if (non_empty &&
			(non_empty->type == bishop || non_empty->type == queen)) {
			for (int8_t j = POS_YP - 1;
				 j >= 0 && POS_XP + (POS_YP - j) < 8; --j) {
				 upd_func(
					&board[POS_XP + (POS_YP - j)][j], &non_empty->side);
				if (board[POS_XP + (POS_YP - j)][j].obj.type != empty) break;
			}
		}
		non_empty = NULL;

		for (uint8_t j = POS_YP + 1; j < 8 && POS_XP + (j - POS_YP) < 8; ++j)
			if (board[POS_XP + (j - POS_YP)][j].obj.type != empty) {
				non_empty = &board[POS_XP + (j - POS_YP)][j].obj;
				break;
			}
		if (non_empty &&
			(non_empty->type == bishop || non_empty->type == queen)) {
			for (int8_t j = POS_YP - 1;
				 j >= 0 && POS_XP - (POS_YP - j) >= 0; --j) {
				upd_func(
					&board[POS_XP - (POS_YP - j)][j], &non_empty->side);
				if (board[POS_XP - (POS_YP - j)][j].obj.type != empty) break;
			}
		}
	}
	non_empty = NULL;     
        
	if (POS_YP != 7) {
		for (int8_t j = POS_YP - 1; j >= 0; --j)
			if (board[POS_XP][j].obj.type != empty) {
				non_empty = &board[POS_XP][j].obj;
				break;
			}           
		if (non_empty && (non_empty->type == rook ||
						  non_empty->type == queen)) {
			DEBUG_MSG("loop #2");
			for (int8_t j = POS_YP + 1; j < 8; ++j) {
				upd_func(
					&board[POS_XP][j], &non_empty->side);
				if (board[POS_XP][j].obj.type != empty) break;
			}
		}
		non_empty = NULL;

		for (int8_t j = POS_YP - 1; j >= 0 && POS_XP - (POS_YP - j) >= 0; --j)
			if (board[POS_XP - (POS_YP - j)][j].obj.type != empty) {
				non_empty = &board[POS_XP - (POS_YP - j)][j].obj;
				break;
			}
		if (non_empty &&
			(non_empty->type == bishop || non_empty->type == queen)) {
			for (uint8_t j = POS_YP + 1;
				 j < 8 && POS_XP + (j - POS_YP) < 8; ++j) {
				upd_func(
					&board[POS_XP + (j - POS_YP)][j], &non_empty->side);
				if (board[POS_XP + (j - POS_YP)][j].obj.type != empty) break;
			}
		}
		non_empty = NULL;

		for (int8_t j = POS_YP - 1; j >= 0 && POS_XP + (POS_YP - j) < 8; --j)
			if (board[POS_XP + (POS_YP - j)][j].obj.type != empty) {
				non_empty = &board[POS_XP + (POS_YP - j)][j].obj;
				break;
			}
		if (non_empty &&
			(non_empty->type == bishop || non_empty->type == queen)) {
			for (uint8_t j = POS_YP + 1;
				 j < 8 && POS_XP - (j - POS_YP) >= 0; ++j) {
				upd_func(
					&board[POS_XP - (j - POS_YP)][j], &non_empty->side);
				if (board[POS_XP - (j - POS_YP)][j].obj.type != empty) break;
			}
		}
	}
	non_empty = NULL;        
	if (POS_XP != 0) {
		for (uint8_t i = POS_XP + 1; i < 8; ++i)
			if (board[i][POS_YP].obj.type != empty) {
				non_empty = &board[i][POS_YP].obj;
				break;
			}           
		if (non_empty && (non_empty->type == rook ||
						  non_empty->type == queen)) {
			DEBUG_MSG("loop #3");
			for (int8_t i = POS_XP - 1; i >= 0; --i) {
				upd_func(&board[i][POS_YP], &non_empty->side);
				if (board[i][POS_YP].obj.type != empty) break;
			}
		}
	}
	non_empty = NULL;
	if (POS_XP != 7) {
		for (int8_t i = POS_XP - 1; i >= 0; --i)
			if (board[i][POS_YP].obj.type != empty) {
				non_empty = &board[i][POS_YP].obj;
				/* print_square_info(&board[i][POS_YP]); */
				break;
			}
		if (non_empty && (non_empty->type == rook ||
						  non_empty->type == queen)) {
			DEBUG_MSG("loop #4");
			for (int8_t i = POS_XP + 1; i < 8; ++i) {
				upd_func(&board[i][POS_YP], &non_empty->side);
				if (board[i][POS_YP].obj.type != empty) break;
			}
		}
	}
	non_empty = NULL;
	return 0;
}


static uint8_t count_free_squares(struct square (*board)[8], enum color_t *side,
                           		enum piece_t type, uint8_t *npos) {
	if (*side == none) {
		return ERROR_INPUT_ABSENT_PIECES;
	}
	printf("621 engine debug\n");
  
    switch (type) {
    case pawn: {
#define PAWN_XLEVEL (NPOS_XP + (*side == black) - (*side == white))
	uint8_t counter = 0;
	if (NPOS_YP == 0) {
		counter += (board[PAWN_XLEVEL][NPOS_YP + 1].obj.side != *side
					&& board[PAWN_XLEVEL][NPOS_YP + 1].obj.type != empty);
	}
	else if (NPOS_YP == 7) {
		counter += (board[PAWN_XLEVEL][NPOS_YP - 1].obj.side != *side
					&& board[PAWN_XLEVEL][NPOS_YP - 1].obj.type != empty);
		
	} else {
		counter += (board[PAWN_XLEVEL][NPOS_YP + 1].obj.side != *side
					&& board[PAWN_XLEVEL][NPOS_YP + 1].obj.type != empty);
		counter += (board[PAWN_XLEVEL][NPOS_YP - 1].obj.side != *side
					&& board[PAWN_XLEVEL][NPOS_YP - 1].obj.type != empty);
	}
	counter += board[PAWN_XLEVEL][NPOS_YP].obj.type == empty;
	return counter;
#undef PAWN_XLEVEL
	}
	case knight:
		return knight_pos_update(board, side, npos, square_is_free);
	case king:
		return king_pos_update(board, side, npos, square_is_safe);
	case queen:
		return queen_pos_update(board, side, npos, square_is_free);
	case rook: 
		return rook_pos_update(board, side, npos, square_is_free);
	case bishop:
		return bishop_pos_update(board, side, npos, square_is_free);
    }
	return ERROR_INPUT_ABSENT_PIECES;
}


void
init_attacking_board(struct square (*board)[8]) {
	for (uint8_t i = 0; i < 8; ++i) {
		for (uint8_t j = 0; j < 8; ++j) {
			uint8_t pos = i*8 + j + 1;
			npos_update(board, board[i][j].obj.side, board[i][j].obj.type, &pos);
		}
	}
}


static int32_t
is_this_movement_correct(struct chess *global, uint8_t *opos, uint8_t *npos) {
    switch (global->board[OPOS_XP][OPOS_YP].obj.type) {
	case pawn:
	{
		return check_pawn_move(global, opos, npos);
	}
	case knight:
	{
		return check_knight_move(opos, npos);
	}
	case king:
	{
		return check_king_move(global->board, opos, npos);
	}
	case queen:
	{
		return check_queen_move(global->board, opos, npos);
	}
	case rook:
	{
		return check_rook_move(global->board, opos, npos);
	}
	case bishop:
	{
		return check_bishop_move(global->board, opos, npos);
	}
	case empty: { return ERROR_INPUT_ABSENT_PIECES; }
    }
}




#define APOS_XP ((*attacking_piece - 1) / 8)
#define APOS_YP ((*attacking_piece - 1) % 8)
#define MAX_NUM_FREE_SQUARES 6
uint8_t *find_common_line(
    struct square (*board)[8],
    uint8_t *kpos,
    uint8_t *attacking_piece,
	uint8_t free_common_line_pos[MAX_NUM_FREE_SQUARES])
{
	if (board[(*attacking_piece - 1) / 8][(*attacking_piece - 1) % 8].obj.type
		== pawn
	||  board[(*attacking_piece - 1) / 8][(*attacking_piece - 1) % 8].obj.type
		== knight)
		return NULL;

	uint8_t count = 0;
    if (KPOS_XP == APOS_XP) {
        uint8_t min = (KPOS_YP < APOS_YP) ? KPOS_YP : APOS_YP;
        uint8_t max = (KPOS_YP > APOS_YP) ? KPOS_YP : APOS_YP;
        for (uint8_t between = min + 1; between < max; ++between)
            if (board[KPOS_XP][between].obj.type == empty) {
                free_common_line_pos[count++] = (KPOS_XP * 8 + between + 1);
			}
		return free_common_line_pos;
    }
    else if (KPOS_YP == APOS_YP) {
        uint8_t min = (KPOS_XP < APOS_XP) ? KPOS_XP : APOS_XP;
        uint8_t max = (KPOS_XP > APOS_XP) ? KPOS_XP : APOS_XP;
        for (uint8_t between = min + 1; between < max; ++between) {
            if (board[between][KPOS_YP].obj.type == empty) {
                free_common_line_pos[count++] = (between * 8 + KPOS_YP + 1);
			}
		return free_common_line_pos;
        }
    }
	
    uint8_t min = (KPOS_XP < APOS_XP) ? *kpos - 1 : *attacking_piece - 1;
    uint8_t max = (KPOS_XP > APOS_XP) ? *kpos - 1 : *attacking_piece - 1;
    if (min % 8 < max % 8)
    {
        for (uint8_t diag = min / 8 + 1; diag < max / 8; ++diag)
        {
            if (board[diag][min % 8 + (diag - (min / 8))].obj.type
				== empty)
				free_common_line_pos[count++] = (diag * 8 + (min % 8 + (diag - (min / 8))) + 1);
				
        }
		return free_common_line_pos;
    }
    else
    {
        for (uint8_t diag = min / 8 + 1; diag < max / 8; ++diag)
        {
            if (board[diag][min % 8 - (diag - (min / 8))].obj.type
				== empty)
				free_common_line_pos[count++] = (diag * 8 + (min % 8 - (diag - (min / 8))));
        }
		return free_common_line_pos;
    }
    return NULL;
#undef APOS_XP
#undef APOS_YP
}


uint8_t
check_on_checkmate_position(struct chess *global, enum color_t side,
							uint8_t *attacking_piece) {
	uint8_t *kpos = side == white ? &global->kpos_w : &global->kpos_b;
	if ((side == black && global->board[KPOS_XP][KPOS_YP].w_attack == 0)
		|| (side == white && global->board[KPOS_XP][KPOS_YP].b_attack == 0))
		return 0;
	if (count_free_squares(global->board, &side, king, kpos) != 0)
		return 0;

	uint8_t free_common_line_pos[MAX_NUM_FREE_SQUARES] = {0};
	
	find_common_line(global->board, kpos, attacking_piece, free_common_line_pos);

	for (uint8_t i = 0; i < 8; ++i)
		for (uint8_t j = 0; j < 8; ++j) {
		uint8_t pos = i * 8 + j + 1;
		if (global->board[i][j].obj.side != side) continue;
		if (check_hidden_king_attack(global->board,
							&global->kpos_w, &global->kpos_b, &pos) != 0)
			continue;
		if (is_this_movement_correct(global, &pos, attacking_piece) == 0)
			return 0;
	/* 6 because maximum number of free squares in one line is 6 */
		if (free_common_line_pos[0] == 0) continue;
		for (uint8_t count = 0; count < MAX_NUM_FREE_SQUARES
								&& free_common_line_pos[count] != 0; ++count) {
			if (is_this_movement_correct
					(global, &pos, &free_common_line_pos[count]) == 0)
					return 0;
		}
	}
	return 255;
#undef MAX_NUM_FREE_SQUARES
}


uint8_t
check_on_stalemate_position(struct chess *global, enum color_t side)
{
	uint8_t *kpos = side == white ? &global->kpos_w : &global->kpos_b;

	if (count_free_squares(global->board, &side, king, kpos) != 0)
		return 0;
		
	for (uint8_t i = 0; i < 8; ++i) {
		for (uint8_t j = 0; j < 8; ++j) {
			if (global->board[i][j].obj.side != side) continue;
			uint8_t pos = i * 8 + j + 1;
			uint8_t attacking_piece = 0;
			if ((attacking_piece = check_hidden_king_attack(global->board,
							&global->kpos_w, &global->kpos_b, &pos)) != 0) {
				if (is_this_movement_correct(global, &pos, &attacking_piece)
																		== 0) {
					return 0;
				}
			}
			else if (count_free_squares(global->board, &side,
									global->board[i][j].obj.type, &pos) != 0) {
				return 0;
			}
		}
	}
	if ((global->board[KPOS_XP][KPOS_YP].b_attack > 0 && side == white) ||
		(global->board[KPOS_XP][KPOS_YP].b_attack > 0 && side == white)) {
		return 0;
	}
	return 255;
}


uint8_t
make_new_move(struct chess *global, uint8_t *opos, uint8_t *npos) {
	/*
	* Here I clean enemy attack because this new move is legit and
	* engine sure what this move happened at 100%
	* (only if next cond would work correctly)
	*/
	if (global->board[NPOS_XP][NPOS_YP].obj.type != empty)
		clean_the_piece_attack(global->board, npos);

	// clean old attack moved figure
	clean_the_piece_attack(global->board, opos);
	// assignment new side
	global->board[NPOS_XP][NPOS_YP].obj.side =
									global->board[OPOS_XP][OPOS_YP].obj.side;

	/*
	* This needs for pawn_transformation. CLI/GUI interface updates this variable
	* with selected a new figure
	*/
	if (global->pawn_transformation != empty) {

		global->board[OPOS_XP][OPOS_YP].obj.type = empty;
		global->board[OPOS_XP][OPOS_YP].obj.side = none;

		// It needs if pawn bit enemy figure at last horizontal and change diagonal
		// So if behind was rook it attack all vertical or for other cases
		check_hidden_attack(global->board, opos, square_state_upd_by_attacking);

		// just npos update and clean pawn_transformation
		global->board[NPOS_XP][NPOS_YP].obj.type = global->pawn_transformation;
		global->pawn_transformation = empty;
		npos_update(global->board, global->board[NPOS_XP][NPOS_YP].obj.side,
					global->board[NPOS_XP][NPOS_YP].obj.type, npos);
	} else {
		/* It made for memory optimization (Don't create new buffer variable */
		global->pawn_transformation = global->board[OPOS_XP][OPOS_YP].obj.type;

		// remove old pos figure
		global->board[OPOS_XP][OPOS_YP].obj.type = empty;
		global->board[OPOS_XP][OPOS_YP].obj.side = none;
		// prevention like above
		check_hidden_attack(global->board, opos, square_state_upd_by_attacking);

		global->board[NPOS_XP][NPOS_YP].obj.type = global->pawn_transformation;
		global->pawn_transformation = empty;
		npos_update(global->board, global->board[NPOS_XP][NPOS_YP].obj.side,
					global->board[NPOS_XP][NPOS_YP].obj.type, npos);
	}
	check_hidden_attack(global->board, npos, square_state_upd_by_leaving);
	if (global->board[NPOS_XP][NPOS_YP].obj.type == king) {
		if (global->board[NPOS_XP][NPOS_YP].obj.side == white) {
			global->kpos_w = *npos;
			global->castling_flags &= 0b1100;
			}
		else {
			global->kpos_b = *npos;
			global->castling_flags &= 0b0011;
		}
	}
	if (global->board[NPOS_XP][NPOS_YP].obj.type == rook) {
		if (global->board[NPOS_XP][NPOS_YP].obj.side == white) {
			if (OPOS_XP == 7 && OPOS_YP == 7)
				global->castling_flags &= (WHITE_OO ^ 0b1111);
			else if (OPOS_XP == 7 && OPOS_YP == 0)
				global->castling_flags &= (WHITE_OOO ^ 0b1111);
			}
		else {
			if (OPOS_XP == 0 && OPOS_YP == 7)
				global->castling_flags &= (BLACK_OO ^ 0b1111);
			else if (OPOS_XP == 0 && OPOS_YP == 0)
				global->castling_flags &= (BLACK_OOO ^ 0b1111);
		}
	}
	if (check_on_stalemate_position(global, global->board[NPOS_XP][NPOS_YP]
								.obj.side == white ? black : white) != 0)
		global->status = end_stalemate;
	else if (check_on_checkmate_position(global, global->board[NPOS_XP][NPOS_YP]
								.obj.side == white ? black : white, npos) != 0)
		global->status = global->board[NPOS_XP][NPOS_YP].obj.side
										== white ? winner_white : winner_black;
	return 0;
}


static uint8_t
check_on_living_shield(struct chess *global, uint8_t *opos, uint8_t *npos) {
	struct chess buffer = *global;
	
	make_new_move(&buffer, opos, npos);
	
	return (buffer.board[(buffer.kpos_b - 1) / 8][(buffer.kpos_b - 1) % 8]
		.w_attack > 0 && buffer.board[NPOS_XP][NPOS_YP].obj.side == black)
		 || (buffer.board[(buffer.kpos_w - 1) / 8][(buffer.kpos_w - 1) % 8]
		.b_attack > 0 && buffer.board[NPOS_XP][NPOS_YP].obj.side == white);
}



int32_t
check_correct_of_movement (struct chess *global, uint8_t *opos, uint8_t *npos) {
	/*
	*	It's just check the move correctly.
	*	It can't return end of the game and it'd working only if
	*	old position wasn't final
	*
	*	Like I moved rook to check position. It's not means what after this func
	*	I get result of check complete board for winning. Just check legit move.
	*/
	if (global->board[(global->kpos_b - 1) / 8][(global->kpos_b - 1) % 8]
		.w_attack > 0) {
		if (global->board[OPOS_XP][OPOS_YP].obj.type == king &&
              global->board[NPOS_XP][NPOS_YP].w_attack > 0) {
			  printf("It's danger square! 1\n");
			  DEBUG_MSG("ERROR_KING_MOVE_TO_ATTACKED_SQUARE\n");
			  return ERROR_KING_MOVE_TO_ATTACKED_SQUARE;
			}
		else if (check_on_living_shield(global, opos, npos)) {
			  DEBUG_MSG("ERROR_USELESS_MOVE_DURING_KING_ATTACK\n");
			  printf("Your king underattack! 1\n");                  
			  return ERROR_USELESS_MOVE_DURING_KING_ATTACK;
			}
	} else if (global->board[(global->kpos_w - 1) / 8][(global->kpos_w - 1) % 8]
			   .b_attack > 0) {
		if (global->board[OPOS_XP][OPOS_YP].obj.type == king &&
			global->board[NPOS_XP][NPOS_YP].b_attack > 0) {
			DEBUG_MSG("ERROR_KING_MOVE_TO_ATTACKED_SQUARE\n");
			printf("It's danger square! 2\n");
			return ERROR_KING_MOVE_TO_ATTACKED_SQUARE;
		}
		else if (check_on_living_shield(global, opos, npos)) {
			DEBUG_MSG("ERROR_USELESS_MOVE_DURING_KING_ATTACK\n");
			printf("Your king underattack! 2\n");                  
			return ERROR_USELESS_MOVE_DURING_KING_ATTACK;
		}
	}

	// It's not minor move (in 1 square limit)
	// Maybe it's overhead because I literally added same checks in gui and cli
	// for optimization. But for ai I need to leave it here. I need to realize.
	if (*opos == *npos)
    {
        printf("ERROR_MINOR_MOVE\n");
        return ERROR_MINOR_MOVE;
    }
	// It's not friendly fire
    if (global->board[NPOS_XP][NPOS_YP].obj.side
		== global->board[OPOS_XP][OPOS_YP].obj.side)
    {
        printf("ERROR_INPUT_FRIENDLY_ATTACK\n");
        return ERROR_INPUT_FRIENDLY_ATTACK;
    }
	
	// This condition checks what if before a piece was shield for king
	// the new position will be safely for king again
    if ((get_attack_t(&global->board[OPOS_XP][OPOS_YP])
			== both_attacked
		|| get_attack_t(&global->board[OPOS_XP][OPOS_YP])
			== (global->board[OPOS_XP][OPOS_YP].obj.side
				== white ? by_black : by_white))
		&& check_hidden_king_attack(global->board, &global->kpos_w,
									&global->kpos_b, opos))
    {
		if (check_on_living_shield(global, opos, npos) != 0){
			printf("check_hidden_king_attack\n");
        	return -1;
		}
    }
    // return result of checking specifically type moved figure
    return is_this_movement_correct(global, opos, npos);
}

static void
castle_update(struct chess *global, uint8_t *kpos, uint8_t *rpos, uint8_t type)
{
	clean_the_piece_attack(global->board, rpos);
	clean_the_piece_attack(global->board, kpos);
	/* type means OO or OOO castling. OO is 2, OOO is 3 */
	/* Not safely, but works. Only certain type argument */
	uint8_t nkpos;
	uint8_t nrpos;
	if (type == 2) {
		nkpos = *kpos + 2;
		nrpos = *rpos - 2;
	} else {
		nkpos = *kpos - 2;
		nrpos = *rpos + 3;
	}
	global->board[(*kpos - 1) / 8][(*kpos - 1) % 8].obj.type = empty;
	global->board[(*kpos - 1) / 8][(*kpos - 1) % 8].obj.side = none;
	global->board[(*rpos - 1) / 8][(*rpos - 1) % 8].obj.type = empty;
	global->board[(*rpos - 1) / 8][(*rpos - 1) % 8].obj.side = none;

	global->board[(nkpos - 1) / 8][(nkpos - 1) % 8].obj.type = king;
	global->board[(nkpos - 1) / 8][(nkpos - 1) % 8].obj.side =
															global->player_side;
	npos_update(global->board, global->player_side, king, &nkpos);
	global->board[(nrpos - 1) / 8][(nrpos - 1) % 8].obj.type = rook;
	global->board[(nrpos - 1) / 8][(nrpos - 1) % 8].obj.side =
															global->player_side;
	npos_update(global->board, global->player_side, rook, &nrpos);

	if (check_on_stalemate_position(global, global->player_side
										== white ? black : white) != 0)
		global->status = end_stalemate;
	else if (check_on_checkmate_position(global, global->player_side
										== white ? black : white, &nrpos) != 0)
		global->status = global->player_side
										== white ? winner_white : winner_black;
}


#define BOARD global->board
uint8_t
check_castle_OO(struct chess *global) {
	switch (global->player_side) {
	case white: {
		if ((global->castling_flags & WHITE_OO) == 0)
			return ERROR_CASTLE_ISNT_AVAILABLE;
		if ( BOARD[7][5].obj.type != empty || BOARD[7][6].obj.type != empty
		  || BOARD[7][5].b_attack > 0      || BOARD[7][6].b_attack > 0)
			return ERROR_CASTLE_ROOK_IS_BLOCKED;
		if (BOARD[7][4].b_attack > 0)
			return ERROR_KING_UNDER_ATTACK;
		uint8_t rpos = 64;
		/* It doesn't work. Need to fix (castle update)*/
		castle_update(global, &global->kpos_w, &rpos, 2);
		global->castling_flags &= 0b1100;
		break;
		}
	case black: {
		if ((global->castling_flags & BLACK_OO) == 0)
			return ERROR_CASTLE_ISNT_AVAILABLE;
		if ((BOARD[0][5].obj.type != empty || BOARD[0][6].obj.type != empty
		  || BOARD[0][5].w_attack > 0      || BOARD[0][6].w_attack > 0))
			return ERROR_CASTLE_ROOK_IS_BLOCKED;
		if (BOARD[0][4].w_attack > 0)
			return ERROR_KING_UNDER_ATTACK;
		uint8_t rpos = 8;
		castle_update(global, &global->kpos_b, &rpos, 2);
		global->castling_flags &= 0b0011;
		break;
		}
	case none:
		return ERROR_IMPOSSIBLE_RESULT;
	}
	return 0;
}


uint8_t
check_castle_OOO(struct chess *global) {
	printf("OOO\n");
	switch (global->player_side) {
	case white: {
			if ((global->castling_flags & WHITE_OOO) == 0)
				return ERROR_CASTLE_ISNT_AVAILABLE;
			if ( BOARD[7][1].obj.type != empty || BOARD[7][1].b_attack > 0
			  || BOARD[7][2].obj.type != empty || BOARD[7][2].b_attack > 0
			  || BOARD[7][3].obj.type != empty || BOARD[7][3].b_attack > 0)
				return ERROR_CASTLE_ROOK_IS_BLOCKED;
			if (BOARD[7][4].b_attack > 0)
				return ERROR_KING_UNDER_ATTACK;
			uint8_t rpos = 57;
			castle_update(global, &global->kpos_w, &rpos, 3);
			global->castling_flags &= 0b1100;
			break;
		}
	case black: {
			if ((global->castling_flags & BLACK_OOO) == 0)
				return ERROR_CASTLE_ISNT_AVAILABLE;
			if ( BOARD[0][1].obj.type != empty || BOARD[0][1].w_attack > 0
			  || BOARD[0][2].obj.type != empty || BOARD[0][2].w_attack > 0
			  || BOARD[0][3].obj.type != empty || BOARD[0][3].w_attack > 0)
				return ERROR_CASTLE_ROOK_IS_BLOCKED;
			printf("qqweqweqwe\n");
			if (BOARD[0][4].w_attack > 0)
				return ERROR_KING_UNDER_ATTACK;
			uint8_t rpos = 1;
			castle_update(global, &global->kpos_b, &rpos, 3);
			global->castling_flags &= 0b0011;
		}
	}
	return 0;
}
#undef BOARD
