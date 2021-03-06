/* Manipulate the panels of the side window */
/* HEADERS */
#include <ncurses.h>
#include <panel.h>
#include <string.h>
#include <stdlib.h>

#include "anote.h"
#include "config.h"
#include "list.h"
#include "note.h"
#include "tag.h"
#include "cli.h"
#include "utils.h"
#include "sidewin.h"

/* GLOBAL VARIABLES */
char *side_w_header;
int p_width;
int y_offset_sum;
int SIDE_WIN_COLORS;
int side_x_offset = 1;
int expanded = DEFAULT_EXPANDED;
int side_y_offset = HEADER_HEIGHT;
struct d_list *top_tag_index;
struct d_list *sel_tag_index;
struct d_list *circ_tag_list;
struct d_list *panel_list;
WINDOW *side_win;

/* FUNCTION DEFINITIONS */

int /* calculate panel height */
anote_panel_height(Tag t, int full)
{
	if (t == NULL)
		return 0;

	int p_height;
	int t_n_number = tag_get_n_number(t);

	/* no limit for this tag */
	if (full)
		p_height = t_n_number;
	else if (0 <= t_n_number && t_n_number < MAX_NOTES_PER_PANEL)
		p_height = t_n_number;
	else
		p_height = MAX_NOTES_PER_PANEL;

 	/* height + header + borders */
	p_height+=HEADER_HEIGHT + 2;

	return p_height;
}

void
anote_show_panel(PANEL *p)
{
	struct d_list *j;
	WINDOW *p_window;
	int k;
	int limit;
	int p_height;
	int x_offset = 1;
	int y_offset = HEADER_HEIGHT;
	char *name;
	char *text;
	chtype color;

	if (!p)
		return;

	p_height = anote_panel_height((Tag) panel_userptr(p), 0);
	p_window = panel_window(p);
	name = tag_get_name((Tag) panel_userptr(p));

	/* limit of MAX_NOTES_PER_PANEL unless expanded */
	limit = MAX_NOTES_PER_PANEL;
	color = COLOR_PAIR(UNSELECTED_COLORS);

	if (SELECTED_TAG(name)) {
		color = COLOR_PAIR(SELECTED_COLORS);
		if (expanded)
			limit = tag_get_n_number(tag_get(name));
	}

	wattrset(p_window, color);
	box(p_window, 0, 0);
	draw_headers(p_window, p_height, p_width, name, color);

	k = 0;
	j = tag_get_notes(tag_get(name));
	while (j->obj && k < limit) {

		/* truncate the string if its longer than side_win_w-borders characters */
		if (strlen(note_get_text(j->obj)) >= p_width) {
			/* -5 = 2 borders and ... */
			text = substr(note_get_text(j->obj), 0, p_width - 5);
			mvwprintw(p_window, y_offset, x_offset, "%s...", text);
			free(text);
		} else {
			mvwprintw(p_window, y_offset, x_offset, note_get_text(j->obj));
		}


		++y_offset;
		++k;

		CONTINUE_IF(j, j->next);
	}

	if (k < tag_get_n_number((Tag) panel_userptr(p)))
		mvwprintw(p_window, y_offset, x_offset, "+++");
}

void
build_tag_panels(void)
{
	PANEL *p;
	struct d_list *i;

	side_y_offset = HEADER_HEIGHT;

	circ_tag_list = new_list_node_circ();
	panel_list = new_list_node_circ();

	/* first tag is the first of the circular list */
	top_tag_index = circ_tag_list;
	sel_tag_index = top_tag_index;

	i = global_tag_list;
	while (i->obj) {
		if (tag_get_name(i->obj) != d_tag_name) {
			d_list_add_circ(i->obj, &circ_tag_list, tag_get_size());
			p = anote_new_panel(i->obj);
			anote_show_panel(p);
		}

		CONTINUE_IF(i, i->next);
	}

	update_panels();
	doupdate();
}

PANEL * /* create panel and insert it on list */
anote_new_panel(Tag t)
{
	PANEL *p = NULL;
	WINDOW *p_window;
	int p_height;

	/* if expanded load tag with full height */
	if (SELECTED_TAG(tag_get_name(t)) && expanded)
		p_height = anote_panel_height(t, 1);
	else
		p_height = anote_panel_height(t, 0);

	p_window = derwin(side_win, p_height, p_width, side_y_offset, side_x_offset);
	if (p_window) {
		p = new_panel(p_window);
		set_panel_userptr(p, t); /* panel_userptr point to its tag */
		d_list_add_circ(p, &panel_list, sizeof(*p));

		/* on big side window mode, list tags side by side */
		if (curr_lay_size == BIG_SW) {
			if (side_x_offset == 1) {
				side_x_offset = side_win_w/2;
				y_offset_sum = p_height;
			} else {
				side_x_offset = 1;
				y_offset_sum = (p_height > y_offset_sum) ? p_height : y_offset_sum;
				side_y_offset+=y_offset_sum;
			}

		} else {
			side_y_offset+=p_height;
		}
	}


	return p;
}

PANEL * /* returns the panel containing t */
anote_search_panel(Tag t)
{
	Tag t_aux;
	PANEL *p = NULL;
	struct d_list *i; /* traverse tag_list   */
	char *name = tag_get_name(t);;

	i = panel_list;
	do {
		t_aux = (Tag) panel_userptr(i->obj);

		if (strcmp(name, tag_get_name(t_aux)) == 0)
			break;

		i = i->next;
	} while (i != panel_list);

	/* found it */
	if (strcmp(name, tag_get_name(t_aux)) == 0)
		p = i->obj;

	return p;
}

/* repositions every panel setting the top_tag_index to
 * the first position available and working from there.
 */
void
scroll_panels(void)
{
	side_y_offset = HEADER_HEIGHT;
	struct d_list *i;

	i = top_tag_index;
	do {

		anote_new_panel(i->obj);

		i = i->next;
	} while (side_y_offset < side_win_h && i != top_tag_index);

	update_panels();
	doupdate();
}

void
delete_panels(void)
{
	struct d_list *i;

	i = panel_list;
	do {

		del_panel((PANEL *) i->obj);
		delwin(panel_window((PANEL *) i->obj));

		i = i->next;
	} while (i != panel_list);

	delete_list_circ(&panel_list);

	panel_list = new_list_node_circ();
}

void
reload_side_win(void)
{
	struct d_list *i;
	PANEL *p;

	werase(side_win);
	draw_headers(side_win, side_win_h, side_win_w, side_w_header, COLOR_PAIR(SIDE_WIN_COLORS));

	i = top_tag_index;
	do {
		p = anote_search_panel(i->obj);

		werase(panel_window(p));
		anote_show_panel(p);

		i = i->next;
	} while (i != top_tag_index);
	scroll_panels();
}

void
color_side_win(void)
{
	draw_headers(side_win, side_win_h, side_win_w, side_w_header, COLOR_PAIR(SIDE_WIN_COLORS));
	show_win(side_win, COLOR_PAIR(SIDE_WIN_COLORS));
}

void /* tag manipulations */
side_win_actions(int c)
{
	switch(c)
	{
		case A_CR:      /* FALLTHROUGH */
		case A_NEWLINE:
			CLEAR_WINDOW(side_win);
			load_displayed_tag(tag_get_name(sel_tag_index->obj));
			sel_note_i = d_tag_notes;
			delete_panels();
			delete_list_circ(&circ_tag_list);
			build_tag_panels();
			cur_win = main_win;
			MAIN_WIN_COLORS = SELECTED_COLORS;
			SIDE_WIN_COLORS = UNSELECTED_COLORS;
			reload_main_win();
			reload_side_win();
			break;

		case 'j':      /* FALLTHROUGH */
		case KEY_DOWN:
			CLEAR_WINDOW(side_win);
			top_tag_index = top_tag_index->next;
			sel_tag_index = top_tag_index;
			delete_panels();
			scroll_panels();
			reload_side_win();
			break;

		case 'k':      /* FALLTHROUGH */
		case KEY_UP:
			CLEAR_WINDOW(side_win);
			top_tag_index = d_list_prev(top_tag_index->obj, &circ_tag_list);
			sel_tag_index = top_tag_index;
			delete_panels();
			scroll_panels();
			reload_side_win();
			break;

		case A_TAB:
			cur_win = main_win;
			MAIN_WIN_COLORS = SELECTED_COLORS;
			SIDE_WIN_COLORS = UNSELECTED_COLORS;
			color_main_win();
			color_side_win();
			break;

		default:
			break;
	}

}
