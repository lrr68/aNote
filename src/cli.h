#ifndef _CLI_H
#define _CLI_H

#define HEADER_HEIGHT 3

#define CLEAR_WINDOW(x)     \
	wmove(x, HEADER_HEIGHT, 1); \
	wclrtobot(x);

/* ANOTE KEYS */
typedef enum {
	A_TAB = 9,
	A_NEWLINE = 10,
	A_CR = 13,
} A_Keys;

typedef enum {
	ALIGN_LEFT = 0,
	ALIGN_CENTER,
	NOTE_ONLY,
	NOTE_COMP,
	NOTE_PRIO,
	NOTE_COMP_PRIO,
} DisplayModes;

/* COLOR PAIRS */
typedef enum {
	SELECTED_COLORS = 1,
	UNSELECTED_COLORS,
	HIGHLIGHT_COLORS,
	DEFAULT_COLORS,
	MENU_COLORS_FG,
	MENU_COLORS_BG,
} AnoteColors;


/* FUNCTION PROTOTYPES */
void start_anote_cli(void);
void load_displayed_tag(char *tag_name);
void draw_headers(WINDOW *window, int height, int width, char *label, chtype color);
void print_align_center(WINDOW *win, int start_y, int start_x, int width, char *string, chtype color);
void reload_main_win(void);

/* GLOBAL VARIABLES */
extern WINDOW *cur_win;
extern WINDOW *main_win;
extern Tag displayed_tag;
extern int d_tag_n_number;
extern char *d_tag_name;
extern struct d_list *d_tag_notes;

extern struct d_list *panel_list;
extern int MAIN_WIN_COLORS;
extern int main_win_h;
extern int main_win_w;
extern int side_win_h;
extern int side_win_w;
extern int footer_h;
extern int footer_w;
extern int max_row;
extern int max_col;

#endif
