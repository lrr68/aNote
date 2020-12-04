/* TYPES */
struct note {
	int priority; /* 0:max priority */
	char *text;
	char *tag;
};

/* FUNCTION PROTOTYPES */
struct note *new_note(char *text);
void edit_note_pri(int n_pri, struct note *n);
void edit_note_tag(char *n_tag, struct note *n);
void edit_note_text(char *n_text, struct note *n);
