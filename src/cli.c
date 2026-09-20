/* parsing input, display, output */

#ifdef __STDC_ALLOC_LIB__
#define __STDC_WANT_LIB_EXT2__ 1
#else
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>

#include "../include/util.h"
#include "../include/engine.h"
#include "../include/cli.h"
#include "../include/logging.h"

// static enum type_pieces get_pieces_type(uint8_t symb) {
//     symb = GET_STANDART_SYMBOL(symb);
//     switch (symb) {
//         case 'n':   return knight;
//         case 'q':   return queen;
//         case 'k':   return king;
//         case 'r':   return rook;
//         case 'b':   return bishop;
//         case 'p':   return pawn;
//         default :   return empty;
//     }
// }


static int8_t
check_correct_input (char **pos)
{
    if ((pos[0][1] && pos[1][1]) && ((pos[0][1] > '0' && pos[0][1] < '9') &&
									 (pos[1][1] > '0' && pos[1][1] < '9')))
        return 0;
    return ERROR_INPUT_INCORRECT_SYMBOL;
}


static int8_t
get_fig_symbol (enum piece_t *type, enum color_t *color)
{
    // printf(*color == black ? "IT IS BLACK\n" : "IT IS WHITE\n");
    switch(*type)
    {
	case pawn:      return *color == black ? 'p' : 'P';
	case king:      return *color == black ? 'k' : 'K';
	case queen:     return *color == black ? 'q' : 'Q';
	case rook:      return *color == black ? 'r' : 'R';
	case bishop:    return *color == black ? 'b' : 'B';
	case knight:    return *color == black ? 'n' : 'N';
	case empty:     return '.';
	default:
	{
		printf("%s:%d default error\n", __FILE__, __LINE__);
		exit(-1);
	}
    };
};


static void print_board_pve(struct square (*board)[8], enum color_t *player_side) {
	printf("    A B C D E F G H\n");
	for (int8_t i = 0; i < 8; ++i) {
		printf("%d | ", 8 - i);
		for (uint8_t j = 0; j < 8; ++j) {
			printf("%c ", get_fig_symbol(&board[i][j].obj.type,
										 &board[i][j].obj.side));
		}
		printf("| %d\n", 8 - i);
	}
	printf("    A B C D E F G H\n");		
}  


static void 
print_board_pvp (struct square (*board)[8], enum color_t *player_side)
{
    switch (*player_side)
    {
	case white:
	{
		printf("    A B C D E F G H\n");
		for (int8_t i = 0; i < 8; ++i) {
			printf("%d | ", 8 - i);
			for (uint8_t j = 0; j < 8; ++j) {
				printf("%c ", get_fig_symbol(&board[i][j].obj.type,
											 &board[i][j].obj.side));
			}
			printf("| %d\n", 8 - i);
		}
		printf("    A B C D E F G H\n");
		break;
	}
	case black: {
		printf("    H G F E D C B A\n");
		for (int8_t i = 7; i >= 0; --i) {
			printf("%d | ", 8 - i);
			for (int8_t j = 7; j >= 0; --j) {
				printf("%c ", get_fig_symbol(
						   &board[i][j].obj.type,
						   &board[i][j].obj.side));
			}
			printf("| %d\n", 8 - i);
		}
		printf("    H G F E D C B A\n");
		break;
	}
	default: {
		printf("%s:%d Incorrect color error\n", __FILE__, __LINE__);
		exit(-1);
	}
    };
};


static uint8_t
get_pos_value (char *symbol, char *number)
{
    return (GET_STANDART_SYMBOL(*symbol) - 96) + ((57 - *number - 1) * 8);
}


static uint8_t
input_proc (char *input, char **pos)
{
    if (strncmp(input, "--", 2) == 0) {
        input += 2;
        if (strncmp(input, "help", 4) == 0) {
          printf("--CHESS HELP--\n"
                 "You can make your move simple a2:a4 or a2a4\n"
                 "--help\t\tget this help message\n"
                 "--reload\tto reload game\n"
                 "--clear\t\tclear old boards\n\n"
				 "--exit\t\tdon't save and exit\n"                 
				);
			return HELP_CODE;
        }
        else if(strncmp(input, "exit", 4) == 0) {
			return EXIT_CODE;
        }
        else if (strncmp(input, "reload", 6) == 0) {
			printf("\033[2J\033[H");
			fflush(stdout);
			return RELOAD_CODE;
        } else if (strncmp(input, "clear", 5) == 0) {
			printf("\033[2J\033[H");
			fflush(stdout);
			return CLEAR_CODE;
		}
        return INVALID_OPTION;
    } else if (strncmp(input, "O-O", 3) == 0 || (strncmp(input, "o-o", 3) == 0))
	{
		input += 3;
		if (strncmp(input, "-O", 2) == 0 || (strncmp(input, "-o", 2) == 0)) {
			printf("CASTLE_OOO_CODE\n");
			return CASTLE_OOO_CODE;
		}
		printf("CASTLE_OO_CODE\n");
		return CASTLE_OO_CODE;
	}
    else {
        int8_t len_pos = 0;
        for (uint8_t iter = 0; iter < strlen(input); ++iter) {
            if (len_pos == 2) break;
            if ('a' <= GET_STANDART_SYMBOL(input[iter]) && 
				GET_STANDART_SYMBOL(input[iter]) <= 'h') {
                pos[len_pos++] = &input[iter];
            }
        }
        if (len_pos < 2)                return ERROR_INPUT_INCORRECT_LEN;
        if (check_correct_input(pos))   return ERROR_INPUT_INCORRECT_SYMBOL;
    }
    return 0;
}


void printf_debug(struct square (*board)[8]) {
	#if 1 == 1
	printf("\nattacked\n");  
	printf("    A B C D E F G H\n");
	for (int8_t i = 0; i < 8; ++i) {
		printf("%d | ", 8 - i);
		for (uint8_t j = 0; j < 8; ++j) {
			enum attack_t attacked = get_attack_t(&board[i][j]);
			if (attacked == 0) {
				printf(". ");
				continue;
			}
			printf("%d ", attacked == -1 ? 2 : attacked);
		}
		printf("| %d\n", 8 - i);
	}
	printf("    A B C D E F G H\n");
	#endif 
	#if 1 == 1
	printf("\nw_attack\n");  
	printf("    A B C D E F G H\n");
	for (int8_t i = 0; i < 8; ++i) {
		printf("%d | ", 8 - i);
		for (uint8_t j = 0; j < 8; ++j) {
			if (board[i][j].w_attack == 0) {
				printf(". ");
				continue;
			}                  
			printf("%d ", board[i][j].w_attack);
		}
		printf("| %d\n", 8 - i);
	}
	printf("    A B C D E F G H\n");
	#endif
	#if 1 == 1
	printf("\nb_attack\n");  
	printf("    A B C D E F G H\n");
	for (int8_t i = 0; i < 8; ++i) {
		printf("%d | ", 8 - i);
		for (uint8_t j = 0; j < 8; ++j) {
			if (board[i][j].b_attack == 0) {
				printf(". ");
				continue;
			}
			printf("%d ", board[i][j].b_attack);
		}
		printf("| %d\n", 8 - i);
	}
	printf("    A B C D E F G H\n");
	#endif
}


uint8_t 
cli_start_pvp_one_device (struct chess *global)
{
	#if DEBUG == 1
    FILE *stream = new_logging(modern_move_logging);
	#endif
    uint8_t print_flag = 1;
    void (*print_board)(struct square (*)[8], enum color_t *) = print_board_pvp;
    for (; global->status == session_active; ) {
        if (print_flag) {
            printf("\033[2J\033[H"); // DEBUG_HERE
            print_board(global->board, &global->player_side);
            printf_debug(global->board);
			/* printf("bits %d\n", global->castling_flags); */
        } else
          print_flag = 1;
        char *user_input = NULL;
        size_t len = 0;
        ssize_t glread = getline(&user_input, &len, stdin);
        if ( glread != -1 && len)
        {
            char **pos = (char **)malloc(sizeof(char *) * 2);
            uint8_t err = input_proc(user_input, pos);
            switch (err)
            {
			case 0: break;
			case HELP_CODE:
			{
				free(user_input);
				free(pos);
				print_flag = 0;                                
				continue;
			}
			case EXIT_CODE:
			{
				free(user_input);
				free(pos);
				fclose(stream);
				return EXIT_CODE;
			}
			case RELOAD_CODE:
			{
				free(user_input);
				free(pos);
				fclose(stream);
				return RELOAD_CODE;
			}
			case CLEAR_CODE: {
				printf("========CLEARED========\n");
				free(user_input);
				free(pos);
				print_flag = 1;
				continue;
			}
			case INVALID_OPTION:
			{
				printf("Incorrect input. Check --help\n");
				free(user_input);
				free(pos);
				print_flag = 0;                                
				continue;
			}
			case ERROR_INPUT_INCORRECT_SYMBOL:
			{
				printf("Incorrect input. Check --help\n");
				free(user_input);
				free(pos);
				print_flag = 0;                                
				continue;
			}
			case ERROR_INPUT_INCORRECT_LEN:
			{
				printf("Incorrect number of pos. Check --help\n");
				free(user_input);
				free(pos);
				print_flag = 0;                                
				continue;
			}
			case CASTLE_OO_CODE:
			{
				if (check_castle_OO(global) == 0) {
					free(user_input);
					free(pos);
					global->last_move[0] = 0;
					global->last_move[1] = 0;
					global->player_side = (global->player_side == white
											? black : white);
					continue;
				}
				else {
					printf("Castle isn't available\n");
					free(user_input);
					free(pos);
					print_flag = 0;
					continue;
				}
			}
			case CASTLE_OOO_CODE:
			{
				if (check_castle_OOO(global) == 0) {
					free(user_input);
					free(pos);
					global->last_move[0] = 0;
					global->last_move[1] = 0;
					global->player_side = (global->player_side == white
											? black : white);
					continue;
				}
				else {
					printf("Castle isn't available\n");
					free(user_input);
					free(pos);
					print_flag = 0;
					continue;
				}
			}
			default:
			{
				perror("Specific case. Check --help\n");
				free(user_input);
				free(pos);
				print_flag = 0;                                
				continue;
			}

            }
            // old pos
            uint8_t opos = get_pos_value(&pos[0][0], &pos[0][1]);
#if RULE_NON_EMPTY_PIECE_MOVED == 1
            if (global->board[OPOS_X][OPOS_Y].obj.type == empty)
            {
                printf("ERROR: EMPTY SQUARE MOVED\n");
                free(user_input); free(pos);
				print_flag = 0;                                
                continue;
            }
#endif
#if RULE_TURN_ORDER == 1
            if (global->board[OPOS_X][OPOS_Y].obj.side
				!= global->player_side)
            {
                printf("ERROR_MOVE_FIGURE_OF_OTHER_SIDE\n");
				print_flag = 0;                                
                free(user_input); free(pos);
                continue;
            }
#endif            
            /* new pos. max value is 64*/
            uint8_t npos = get_pos_value(&pos[1][0], &pos[1][1]);
			printf("npos is %d\n", npos);

            if (check_correct_of_movement(global, &opos, &npos) != 0) {
				fprintf(stderr, "Invalid move\n");
				free(user_input);
				free(pos);
				print_flag = 0;
				continue;

			}
			int8_t log_err = SIMPLE_new_record(stream, &opos, &npos);
			if (log_err)
			{
				free(user_input);
				free(pos);
				fclose(stream);
				return -1;
			}

            if (global->board[OPOS_X][OPOS_Y].obj.type == pawn &&
                ((NPOS_X == 7 &&
                  global->board[OPOS_X][OPOS_Y].obj.side == black) ||
                 (NPOS_X == 0 &&
                  global->board[OPOS_X][OPOS_Y].obj.side == white))) {
				printf("Write new type of a piece\n");
				printf("Queen Knight Rook Bishop\n");
			/* If player missed he got incorrect a piece. Like qishop/bnight */
				int8_t new_piece = getchar();
				int32_t buf_clear;
				while ((buf_clear = getchar()) != '\n' &&
					   buf_clear != EOF) { }

				new_piece = GET_STANDART_SYMBOL(new_piece);
                                
				switch (new_piece) {
				case 'q':
					global->pawn_transformation = queen;
					break;
				case 'k':
					global->pawn_transformation = knight;
					break;
				case 'n':
					global->pawn_transformation = knight;
					break;
				case 'r':
					global->pawn_transformation = rook;
					break;
				case 'b':
					global->pawn_transformation = bishop;
					break;
				default:
					fprintf(stderr, "Incorrect input, try your move again\n");
                    free(user_input);
                    free(pos);
					print_flag = 0;                                
					continue;
				}
			}

			make_new_move(global, &opos, &npos);
			global->last_move[0] = opos;
			global->last_move[1] = npos;
			global->player_side = (global->player_side == white ? black : white);
			free(user_input);
			free(pos);
        }
        else
        {
            free(user_input);
            if (feof(stdin)) {	      
                printf("Get eof, safe exit\n");
                fclose(stream);
				return EXIT_CODE;
            }
            else
            {
                perror("getline");
                fclose(stream);
                return ERROR_INPUT_GETLINE;
            }
        }
    }
    fclose(stream);
	if (print_flag) {
        printf("\033[2J\033[H"); // DEBUG_HERE
        print_board(global->board, &global->player_side);
        /* printf_debug(global->board); */
		/* printf("bits %d\n", global->castling_flags); */
    } else
		print_flag = 1;

	printf("===========GAME IS FINISHED===========\n");
	switch (global->status) {
		case session_active:
			printf("Incorrect game finished status\n");
			return GAME_STATUS_SESSION_ACTIVE;
		case end_stalemate:
			printf("===========STALEMATE===========\n");
			return GAME_STATUS_END_STALEMATE;
		case winner_black: {
			printf("===========BLACK WIN===========\n");
			return GAME_STATUS_END_BLACK_WIN;
		}
		case winner_white: {
			printf("===========WHITE WIN===========\n");
			return GAME_STATUS_END_WHITE_WIN;
			}
	}
    return EXIT_CODE;
}

uint8_t (*cli_start_menu())(struct chess *global) {
	printf(
		" ________________________\n"
		"|Choose game mode\t|\n"
		"|1. pvp on one device\t|\n"
		"|2. pvp in local network|\n"
		"|3. player vs bot\t|\n"
		" ________________________\n"
	);
	int32_t in = getchar();
	int32_t pass_err_symb;
	while ((pass_err_symb = getchar()) != '\n' && pass_err_symb != EOF) {}
	new_debug_record("cli %c mode\n", in);
	switch (in) {
		case ('1'):
			return cli_start_pvp_one_device;
		case ('2'):
		case ('3'):
		case (EOF):
		default:
			printf("Incorrect input");
			return NULL;
	}
}
