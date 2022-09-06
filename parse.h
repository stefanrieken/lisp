extern int buffered_read();
extern void buffer_return(int c);

extern int get_non_whitespace_char();
extern void * parse_label_or_number (int c, int radix);
extern void skip_line();

extern void * parse_value();
extern Node * parse_list();
extern Node * parse_quote();

