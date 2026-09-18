#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/util.h"
#include "../include/engine.h"
#include "../include/cli.h"
#include "../include/logging.h"
#include "../include/gui.h"


void write_help_message() {
	printf(
		"Usage: bvchess [MODE]\n\n"
		"[MODE] devided at 2 types: gui and cli\n"
		"It allows run chess menu in your category\n"
		"If you don't chose one of this chess's will launch gui menu\n"
	);
}

uint8_t (*argv_proccessing(int32_t argc, char *argv[]))(struct chess *) {
	if (strncmp(argv[1], "gui", 3) == 0) return gui_start_menu;
	else if (strncmp(argv[1], "cli", 3) == 0) return cli_start_menu();
	else {
		new_debug_record("argv_proccessing error. exit");
		return NULL;
	}
}


enum color_t *user_side;
int32_t
main (int32_t argc, char *argv[]) {

    new_debug_record("New main start");
	
	uint8_t (*game_mode)(struct chess *);
	if (argc == 1) {
		write_help_message();
		game_mode = gui_start_menu;
	}
	else {
		game_mode = argv_proccessing(argc, argv);
	}

	if (game_mode == NULL) {
		printf("Incorrect input.\n");
		write_help_message();
		close_debug_stream();
		return 0;
	}
    for (;;)
    {
        struct chess *global;
        global = (struct chess *)(malloc(sizeof(struct chess)));
        uint8_t exit_code;
        if (global)
        {
			// Board init
			global->player_side = white;
            global->pawn_transformation = empty;
            global->last_move[0] = 0;
            global->last_move[1] = 0;
			global->status = session_active;
			global->castling_flags = 0b1111;
			
            user_side = &global->player_side;
            create_board(global->board);
			
            /* set_training_board(global->board, */
            /*         "DEEEKEEE" */
            /*         "EEEEEEEE" */
            /*         "EEEEnEEE" */
            /*         "EbEEdEEE" */
            /*         "EEEEEEEE" */
            /*         "EEEEEEEE" */
            /*         "EEEEEEEE" */
            /*         "EEEEkEEE"); */

			// It makes because if I change board to specific
			// always kpos will be correct
			global->kpos_b = find_figure(global->board, black, king);
			global->kpos_w = find_figure(global->board, white, king);
			
			if (global->kpos_b == 255 || global->kpos_w == 255) {
				printf("Missing King white or black side\n");
				printf("You need to check kings were added to board\n");
				sleep(3);
				exit(0);
			}
			init_attacking_board(global->board);

			
			exit_code = game_mode(global);
			/* exit_code = cli_start_pvp_one_device(global); */
        }
        free(global);
		
        switch (exit_code) {
            case EXIT_CODE:
                break;
            case RELOAD_CODE:
                printf("========RELOADED=======\n");
                DEBUG_MSG("reloaded cli game");
                continue;
			case GAME_STATUS_END_BLACK_WIN:
			case GAME_STATUS_END_WHITE_WIN:
			case GAME_STATUS_END_STALEMATE:
			case CORRECT_GUI_EXIT_CODE:
				printf("After 0 sec game will be closed\n");
				fflush(stdout);
				#if DEBUG == 1
        		close_debug_stream();
				#endif
				/* sleep(1); */
				return 0;
			case GAME_STATUS_SESSION_ACTIVE:
            default: {
				fflush(stdout);
              	close_debug_stream();
              	return -1;
            }
        }
		
		#if DEBUG == 1
        close_debug_stream();
		#endif
		
        break;
    }
    return 0;
}
