#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_types.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_image.h>

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "../include/gui.h"
#include "../include/util.h"
#include "../include/engine.h"
#include "../include/logging.h"


int32_t window_size[2];
int32_t board_size[2];


static inline void update_window_size(SDL_Window *window) {
	SDL_GetWindowSize(window, &window_size[0], &window_size[1]);
}


static inline void update_board_size() {
	board_size[0] = window_size[0];
	board_size[1] = window_size[1];
}


static SDL_bool get_square_pos(int32_t *x_cursor, int32_t *y_cursor,
								uint8_t *pos_x, uint8_t *pos_y)
{
	if (*x_cursor < 0 || *x_cursor >= board_size[0]) return SDL_FALSE;
	if (*y_cursor < 0 || *y_cursor >= board_size[1]) return SDL_FALSE;

	*pos_y = *x_cursor / (board_size[0] / 8);
	*pos_x = *y_cursor / (board_size[1] / 8);
	printf("%d %d\n", *pos_x, *pos_y);
	return SDL_TRUE;
}


static inline
SDL_bool rect_under_mcursor(int32_t x_cursor, int32_t y_cursor, SDL_Rect *rect)
{
	return (x_cursor >= rect->x && x_cursor <= (rect->x + rect->w))
			&& (y_cursor >= rect->y && y_cursor <= (rect->y + rect->h));
}


static char *get_path_to_texture(char *file_name) {
	char *rp = get_rp_to_chess_dir();
	char *texturePath;
	asprintf(&texturePath, "%s/%s", rp, file_name);
	free(rp);
	return texturePath;
}

// sq_size - square size
void draw_board(SDL_Renderer *renderer, struct guiChess *global)
{	
	SDL_Rect square;
	square.w = board_size[0]/8;
	square.h = board_size[1]/8;
	for (uint8_t i = 0; i < 8; ++i) { // only if I make move or holding piece?
		square.y = board_size[1]/8 * i;
		for (uint8_t j = 0; j < 8; ++j) {
			square.x = board_size[0]/8 * j;
			if (global->engine->board[i][j].side == white) {
				if (global->guiBoard[i][j].is_highlighted == SDL_TRUE)
					SDL_SetRenderDrawColor(renderer, 160, 216, 183, 255);
				else 
					SDL_SetRenderDrawColor(renderer, 240, 195, 128, 255);
			}
			else {
				if (global->guiBoard[i][j].is_highlighted == SDL_TRUE)
					SDL_SetRenderDrawColor(renderer, 182, 215, 168, 255);
				else
					SDL_SetRenderDrawColor(renderer, 109, 62, 23, 255);
			}
			SDL_RenderFillRect(renderer, &square);
			SDL_RenderCopy(renderer, global->guiBoard[i][j].texture, NULL, &global->guiBoard[i][j].pos);
		}
	}
}


static void draw_menu (SDL_Renderer *renderer,
						struct button *pvp_one_device_button,
						struct button *pvp_local_button,
						struct button *pvp_bot_button,
						struct button *exit_button) {
	SDL_SetRenderDrawColor(renderer, 213, 189, 175, 255);
	SDL_RenderClear(renderer);
		
	SDL_RenderCopy(renderer, exit_button->idleButton, NULL, &exit_button->rect);
	SDL_RenderCopy(renderer, pvp_bot_button->idleButton, NULL, &pvp_bot_button->rect);
	SDL_RenderCopy(renderer, pvp_local_button->idleButton, NULL, &pvp_local_button->rect);
	SDL_RenderCopy(renderer, pvp_one_device_button->idleButton, NULL, &pvp_one_device_button->rect);
}


static SDL_Texture *
get_figure_texture(SDL_Renderer *renderer, struct piece *fig)
{
	char *texturePath = NULL;
	switch (fig->type) {
		case pawn:
			texturePath = get_path_to_texture(fig->side == white ? "pawn.png" : "pawn1.png");
			break;
		case king:
			texturePath = get_path_to_texture(fig->side == white ? "king.png" : "king1.png");
			break;
		case knight:
			texturePath = get_path_to_texture(fig->side == white ? "knight.png" : "knight1.png");
			break;
		case queen:
			texturePath = get_path_to_texture(fig->side == white ? "queen.png" : "queen1.png");
			break;
		case rook:
			texturePath = get_path_to_texture(fig->side == white ? "rook.png" : "rook1.png");
			break;
		case empty:
			return NULL;
		case bishop:
			texturePath = get_path_to_texture(fig->side == white ? "bishop.png" : "bishop1.png");
			break;
	}
	SDL_Texture *buffer = IMG_LoadTexture(renderer, texturePath);
	free(texturePath);

	if (buffer == NULL) {
		printf("SDL_IMG_LoadTexture error: %s\n", SDL_GetError());
		return NULL;
	}
	return buffer;
}


void
return_piece_back ( SDL_Renderer *renderer,
					struct guiChess *global,
					struct active_figure *active,
					struct active_figure *oldPos)
{
	active->gui->is_highlighted = oldPos->gui->is_highlighted;
	active->gui->pos = oldPos->gui->pos;
	active->gui->texture = oldPos->gui->texture;
	
	active->obj->side = oldPos->obj->side;
	active->obj->type = oldPos->obj->type;

	active->gui->is_highlighted = SDL_FALSE;
	active->gui = NULL;
	active->obj = NULL;
	printf("=========================\n");
	printf("End section\n");
									
	/* free(oldPos->gui); */
	/* free(oldPos->obj); */
	/* oldPos->gui = NULL; */
	// Idk it's too much each iter but maybe I'll get glitches without it. Testing
	/* oldPos->obj = NULL; */
							
	SDL_RenderClear(renderer);
	draw_board(renderer, global);
	SDL_RenderPresent(renderer);
}



static uint8_t
gui_start_pvp_one_device(struct chess *engine, SDL_Window *window,
						SDL_Renderer *renderer)
{
	// TODO: switch-case of chess mode (local/one_device/vs_bot)
	/* SDL_RenderClear(renderer); */
	uint8_t is_running = 1;
	update_window_size(window);
	board_size[0] = 1000;
	board_size[1] = 1000;	
	struct guiChess global;
	global.engine = engine;
	
	for (uint8_t i = 0; i < 8; ++i) {
		for (uint8_t j = 0; j < 8; ++j) {
			global.guiBoard[i][j].is_highlighted = SDL_FALSE;
			global.guiBoard[i][j].texture = get_figure_texture(renderer,
												&engine->board[i][j].obj);
			global.guiBoard[i][j].pos.w = board_size[0]/8;
			global.guiBoard[i][j].pos.h = board_size[1]/8;
			global.guiBoard[i][j].pos.x = j * board_size[0]/8;
			// BUG: If I change window resolution I get glitches
			global.guiBoard[i][j].pos.y = i * board_size[1]/8;
		}
	}


	SDL_RenderClear(renderer);
	draw_board(renderer, &global);
	SDL_RenderPresent(renderer);
	SDL_bool is_mouse_holding = SDL_FALSE;
	struct active_figure oldPos;
	struct active_figure active;
	active.gui = NULL;
	active.obj = NULL;

	oldPos.gui = NULL;
	oldPos.obj = NULL;	
	oldPos.gui = (struct guiPiece *)(malloc(sizeof(struct guiPiece)));
	oldPos.obj = (struct piece *)(malloc(sizeof(struct piece)));
	while (global.engine->status == session_active) {
		SDL_Event event;
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_q:
						case SDLK_ESCAPE:

							if (oldPos.obj != NULL && oldPos.gui != NULL) {
								free(oldPos.gui);
								free(oldPos.obj);
							}
							return 1;
						case SDLK_r:
							if (oldPos.gui != NULL) {
								free(oldPos.gui);
								free(oldPos.obj);
							}
							return 2;
						case SDLK_k:
							printf("%d status \n", global.engine->status);
							break;
					}
					break;
				case SDL_WINDOWEVENT:
					update_window_size(window);
					update_board_size();
					
					for (uint8_t i = 0; i < 8; ++i)
						for (uint8_t j = 0; j < 8; ++j) {
						global.guiBoard[i][j].pos.x = board_size[0]/8 * j;
						global.guiBoard[i][j].pos.y = board_size[1]/8 * i;
						
						global.guiBoard[i][j].pos.w = board_size[0]/8;
						global.guiBoard[i][j].pos.h = board_size[1]/8;
					}
					SDL_RenderClear(renderer);
					draw_board(renderer, &global);
					SDL_RenderPresent(renderer);
					break;
				case SDL_MOUSEMOTION:
					switch (event.button.button) {
						case 1: {// Left button
							if (!is_mouse_holding || active.gui == NULL) break;
							active.gui->pos.x = event.motion.x - window_size[0] / 8 / 2;
							active.gui->pos.y = event.motion.y - window_size[1] / 8 / 2;
							
							SDL_RenderClear(renderer);
							draw_board(renderer, &global);
							SDL_RenderCopy(renderer, active.gui->texture, NULL, &active.gui->pos);
							SDL_RenderPresent(renderer);
						}
						default: break;
					}
					break;
				case SDL_MOUSEBUTTONDOWN: {
					printf("SDL_MOUSEBUTTONDOWN\n");
					uint8_t i = 0;
					uint8_t j = 0;
					if (get_square_pos(&event.button.x, &event.button.y, &i, &j)
						 == SDL_FALSE ) break;
					is_mouse_holding = SDL_TRUE;
					if (global.engine->board[i][j].obj.type != empty &&
						global.engine->board[i][j].obj.side == global.engine->player_side &&
							rect_under_mcursor(event.button.x, event.button.y,
							&global.guiBoard[i][j].pos)) {
						
						oldPos.gui->is_highlighted = global.guiBoard[i][j].is_highlighted;
						oldPos.gui->pos = global.guiBoard[i][j].pos;
						oldPos.gui->texture = global.guiBoard[i][j].texture;

						oldPos.obj->side = global.engine->board[i][j].obj.side;
						oldPos.obj->type = global.engine->board[i][j].obj.type;
						oldPos.pos = i * 8 + j + 1;

						active.gui = &global.guiBoard[i][j];
						active.obj = &global.engine->board[i][j].obj;

						global.guiBoard[i][j].is_highlighted = SDL_TRUE;
					}
					/* struct piece *obj = find_obj( */
					break;
				}
				case SDL_MOUSEBUTTONUP: {
					printf("SDL_MOUSEBUTTONUP\n");
					uint8_t i = 0;
					uint8_t j = 0;
					if (active.obj == NULL)
						break;
					if (get_square_pos(&event.button.x, &event.button.y, &i, &j)
						 == SDL_FALSE ) break;
					active.pos = i * 8 + j + 1;
					switch (event.button.button) {
						case 1: {
							printf("ELEMENT_BELOW\n");
							printf("oldPos.pos %d\n", oldPos.pos);
							if (
							event.button.x < 0 || event.button.x > board_size[0]
						|| event.button.y < 0 || event.button.y > board_size[1]){
								return_piece_back(	renderer, &global,
												&active, &oldPos);
								break;
							}
							// CHECK CASTLE_OOO 5b 61w
							else if (global.engine->board[(oldPos.pos - 1) / 8][(oldPos.pos - 1) % 8].obj.type == king) {
								if ((oldPos.pos == 5
									 && active.pos >= 1 && active.pos <= 3)
									|| (oldPos.pos == 61
									 && active.pos >= 57 && active.pos <= 59)) {
									 if (check_castle_OOO(global.engine) == 0) {

									global.engine->last_move[0] = 0;
									global.engine->last_move[1] = 0;
									global.guiBoard[(oldPos.pos - 1) / 8][(oldPos.pos - 1) % 8].is_highlighted = SDL_FALSE;

#define KING_ROW oldPos.obj->side == white ? 7 : 0 // row
#define KING_COLUMN  2 // always index 2
								
									global.guiBoard[KING_ROW][KING_COLUMN].texture = active.gui->texture;
									active.gui->texture = NULL;
									active.gui->pos = oldPos.gui->pos;
									active.gui = NULL;
									active.obj = NULL;

									global.guiBoard[KING_ROW][KING_COLUMN+1].texture = global.guiBoard[KING_ROW][KING_COLUMN-2].texture;
									global.guiBoard[KING_ROW][KING_COLUMN-2].texture = NULL;
									/* global */
									SDL_RenderClear(renderer);
									draw_board(renderer, &global);
									SDL_RenderPresent(renderer);
									global.engine->player_side = (global.engine->player_side == white
												? black : white);
									}
									else
										return_piece_back(	renderer, &global,
															&active, &oldPos);
									break;

								}
								// CHECK CASTLE_OO 5b 61w
								else if (global.engine->board[(oldPos.pos - 1) / 8][(oldPos.pos - 1) % 8].obj.type == king) {
								if ((oldPos.pos == 5
									 && (active.pos == 7 ||active.pos == 8))
									|| (oldPos.pos == 61
									 && (active.pos == 63 || active.pos == 64)))
									 {
									 if (check_castle_OO(global.engine) == 0) {

									global.engine->last_move[0] = 0;
									global.engine->last_move[1] = 0;
									global.guiBoard[(oldPos.pos - 1) / 8][(oldPos.pos - 1) % 8].is_highlighted = SDL_FALSE;
#undef KING_ROW
#undef KING_COLUMN
#define KING_ROW oldPos.obj->side == white ? 7 : 0 // row
#define KING_COLUMN 6  // always index 6
								
									global.guiBoard[KING_ROW][KING_COLUMN].texture = active.gui->texture;
									active.gui->texture = NULL;
									active.gui->pos = oldPos.gui->pos;
									active.gui = NULL;
									active.obj = NULL;

									global.guiBoard[KING_ROW][KING_COLUMN-1].texture = global.guiBoard[KING_ROW][KING_COLUMN+1].texture;
									global.guiBoard[KING_ROW][KING_COLUMN+1].texture = NULL;
									/* global */
									SDL_RenderClear(renderer);
									draw_board(renderer, &global);
									SDL_RenderPresent(renderer);
									global.engine->player_side = (global.engine->player_side == white
												? black : white);
									}
									else
										return_piece_back(	renderer, &global,
															&active, &oldPos);
									break;
									}
								}
							}
#undef KING_ROW
#undef KING_COLUMN							
							if (engine->board[i][j].obj.side == active.obj->side) {
								return_piece_back(	renderer, &global,
												&active, &oldPos);
								break;
							}
							if (check_correct_of_movement(
								global.engine, &oldPos.pos, &active.pos) == 0) {
								printf("check correct of movement is correct\n");
								make_new_move(global.engine, &oldPos.pos, &active.pos);
								global.engine->last_move[0] = oldPos.pos;
								global.engine->last_move[1] = active.pos;
								global.engine->player_side = (global.engine->player_side == white ? black : white);
								global.guiBoard[(oldPos.pos - 1) / 8][(oldPos.pos - 1) % 8].is_highlighted = SDL_FALSE;
								
								global.guiBoard[i][j].texture = active.gui->texture;
								active.gui->texture = NULL;
								active.gui->pos = oldPos.gui->pos;
								active.gui = NULL;
								active.obj = NULL;

								SDL_RenderClear(renderer);
								draw_board(renderer, &global);
								SDL_RenderPresent(renderer);
							}
							else {
								return_piece_back(	renderer, &global,
													&active, &oldPos);
								break;
							}
							break;
						}
					}
					active.obj = NULL;
					active.gui = NULL;
					printf("end case\n");
					is_mouse_holding = SDL_FALSE;
					break;
				}
				case SDL_QUIT:
					is_running = 0;
					break;
				default: break;
			}
		}
		/* SDL_RenderPresent(renderer); ???*/
		SDL_Delay(25);
	}
	free(oldPos.gui);
	free(oldPos.obj);
	return 0;
}


static struct button create_button(int32_t x, int32_t y,
				SDL_Renderer *renderer, char *file_name) {
	struct button bt;
	bt.isHighlighted = SDL_FALSE;
	bt.rect.x = x;
	bt.rect.y = y;
	bt.rect.w = BUTTON_MENU_WIDTH;
	bt.rect.h = BUTTON_MENU_HEIGHT;

	char *texturePath = get_path_to_texture(file_name);
	bt.idleButton = IMG_LoadTexture(renderer, texturePath);
	free(texturePath);

	if (bt.idleButton == NULL) {
		printf("SDL_IMG_LoadTexture error: %s\n", SDL_GetError());
	}
	/* bt.hgButton = IMG_LoadTexture(renderer, path_to_hg); */

	return bt;
}


static void load_board(SDL_Window *window) {
	
}


uint8_t gui_start_menu(struct chess *engine) {

	new_debug_record("gui_init\n");
	
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
	    printf("SDL_Init error: %s\n", SDL_GetError());
    	return ERROR_GUI_WINDOW_INIT;
	}
	
	SDL_Window *window = SDL_CreateWindow(
    	"bvchess",
	    SDL_WINDOWPOS_CENTERED,
    	SDL_WINDOWPOS_CENTERED,
	    INITIAL_WINDOW_WIDTH,
    	INITIAL_WINDOW_HEIGHT,
	    SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_HIDDEN | SDL_WINDOW_BORDERLESS
	);

	if (window == NULL) {
    	printf("SDL_CreateWindow error: %s\n", SDL_GetError());
	    SDL_Quit();
    	return ERROR_GUI_WINDOW_CREATION;
	}
	new_debug_record("gui was inited\n");
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
					SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
					
	if (!renderer) {
    	printf("SDL_Renderer error: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
	    SDL_Quit();
		return ERROR_GUI_RENDERER_CREATION;
	}

	SDL_ShowWindow(window);

	SDL_SetWindowMinimumSize(window, 400, 400);
	
	struct button pvp_one_device_button = create_button(
			INITIAL_WINDOW_WIDTH / 2 - 100,
			INITIAL_WINDOW_HEIGHT / 2 - DISTANCE_BETWEEN_MENU_BUTTONS,
			renderer,
			"pvp_1d.png");
	struct button pvp_local_button = create_button(
			INITIAL_WINDOW_WIDTH / 2 - 100,
			INITIAL_WINDOW_HEIGHT / 2,
			renderer,
			"pvp_2p.png");
	struct button pvp_bot_button = create_button(
			INITIAL_WINDOW_WIDTH /  2 - 100,
			INITIAL_WINDOW_HEIGHT / 2 + DISTANCE_BETWEEN_MENU_BUTTONS,
			renderer,
			"pvp_ai.png");
	struct button exit_button = create_button(
			INITIAL_WINDOW_WIDTH / 2 - 100,
			INITIAL_WINDOW_HEIGHT / 2 + DISTANCE_BETWEEN_MENU_BUTTONS * 2,
			renderer,
			"exit.png");

	SDL_RenderSetLogicalSize(renderer, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
	
	SDL_RenderClear(renderer);
	draw_menu(renderer,
		&pvp_one_device_button, &pvp_local_button,
		&pvp_bot_button, &exit_button);
	SDL_RenderPresent(renderer);
	
	uint8_t is_running = 1;
	while (is_running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_WINDOWEVENT: {
					printf("Window size is %d %d\n", window_size[0], window_size[1]);
					printf("Board size is %d %d\n", board_size[0], board_size[1]);			
					update_window_size(window);
					update_board_size();					
					SDL_RenderClear(renderer);
					draw_menu(renderer,
						&pvp_one_device_button, &pvp_local_button,
						&pvp_bot_button, &exit_button);
					SDL_RenderPresent(renderer);
					break;
				}
				case SDL_MOUSEBUTTONUP:
			// When I press at a piece when it just moved under cursor 
					if (rect_under_mcursor(event.button.x, event.button.y,
										&exit_button.rect)) {
						printf("Exit button. Session will be ended soon\n");
						is_running = 0;
					}
					else if (rect_under_mcursor(event.button.x, event.button.y,
										&pvp_one_device_button.rect)) {
						++is_running; // HARD-CODE crutch!!!
						do { // it's crutch CHANGE !!!
							init_engine(engine);
							            set_training_board(engine->board,
						                    TRAINING_BOARD);
							init_attacking_board(engine->board);
							engine->kpos_b = find_figure(engine->board, black, king);
							engine->kpos_w = find_figure(engine->board, white, king);
							is_running = gui_start_pvp_one_device
													(engine, window, renderer);
							} while (is_running == 2);
						// here win status
							printf("===========GAME IS FINISHED===========\n");
							switch (engine->status) {
								case session_active: {
									break;
									}
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
							SDL_Delay(5000);
						}
						SDL_RenderClear(renderer);
						draw_menu(renderer,
							&pvp_one_device_button, &pvp_local_button,
							&pvp_bot_button, &exit_button);
						SDL_RenderPresent(renderer);
						}
					if (is_running == 0)
						goto quit;
					draw_menu(renderer,
						&pvp_one_device_button, &pvp_local_button,
						&pvp_bot_button, &exit_button);
						SDL_RenderPresent(renderer);
					break;
				case SDL_KEYDOWN:
					switch(event.key.keysym.sym)
						case SDLK_q:
						case SDLK_ESCAPE:
							is_running = 0;
						default:
							break;
					break; 
				case SDL_QUIT: {
					is_running = 0;
					break;
				}
			}
		}
		SDL_Delay(25);
	}
	// TODO: think how to realize more optimaze option to calling the function
	// and in the case of exit to menu return to menu_loop without next call menu
quit:
	SDL_DestroyWindow(window);
	SDL_Quit();
	return CORRECT_GUI_EXIT_CODE;
}
