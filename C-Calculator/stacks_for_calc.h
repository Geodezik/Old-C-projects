int is_empty(int stack_pointer);
void put_in_buffer(char *where, char elem);
void push(char elem[], int *stack_pointer, char stack[][100]);
char *pop(int *stack_pointer, char stack[][100]);
void init(int *pointer, char stack[][100]);