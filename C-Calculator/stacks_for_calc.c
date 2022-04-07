#include "stacks_for_calc.h"
#include <string.h>

int is_empty(int stack_pointer)
{
    return stack_pointer==-1;
}

void put_in_buffer(char *buffer_pointer, char elem)
{
	*buffer_pointer = elem;
}

char *pop(int *stack_pointer, char stack[][100])
{
    return stack[(*stack_pointer)--];
}

void push(char elem[], int *stack_pointer, char stack[][100])
{
    strcpy(stack[++*stack_pointer], elem);
}


void init(int *pointer, char stack[][100])
{
	int i;
	char aux_stack[25];
	for(i = 0; i < 25; i++) {
		aux_stack[i] = 0;	
	}

	*pointer = -1;
	
	for(i = 0; i < 25; i++) {
		push(aux_stack, pointer, stack);	
	}

	*pointer = -1;
}