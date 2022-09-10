extern int buffered_read();
extern void buffer_return(int c);

extern int get_non_whitespace_char();
extern void * parse_label_or_number (int c, int radix);
extern void skip_line();

extern Node * parse_list(char opening_bracket);
extern Node * parse_quote();

/** Set this value first (may be to stdin) */
void read_from(FILE * file);
/** Then call this one, which calls the ones above. */
extern void * parse_value();

