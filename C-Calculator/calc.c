#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stacks_for_calc.h"

/*Инициализация параметров калькулятора: i/o base, accuracy*/
int init_value(int value, int default_value)
{
	if(value>0) {
		return value;
	}
	else {
		return default_value;
	}
}

/*Перевод числа из одной СС в другую. Запись результата в нужную область памяти*/
void base_conversion(char number[50], int base_of_input, int base_of_output, char (*output)[50])
{
	int int_part, i, j, factor, nonint_flag = 0, sign_switched=0, point_pointer = strlen(number);
	double to_decimal = 0.0, frac_part = 0.0;
	char digit;
	char rev_result[50];

	/*Ищем положение десятичной точки, если она есть*/
	for(i = 0; i < strlen(number); i++) {
		if(number[i]=='.') {
			point_pointer = i;
			break;
		}
	}

	if(base_of_input != 10) {
		/*Отдельно собираем целую и дробную части*/

		/*Целая часть*/
		factor = base_of_input;
		for(i = 0; i < point_pointer; i++) {
			if(isalpha(number[i])) {
				digit = toupper(number[i]);
				digit = digit - 'A' + 10;
			}
			else {
				digit = number[i] - '0';
			}
			to_decimal *= base_of_input;
			to_decimal += digit; 
		}

		/*Дробная часть*/
		/*factor - знаменатель дроби вида 1/base^k*/
		for(i = point_pointer + 1; i < strlen(number); i++) {
			if(isalpha(number[i])) {
				digit = toupper(number[i]);
				digit = digit - 'A' + 10;
			}
			else {
				digit = number[i] - '0';
			}

			to_decimal += digit / (double)factor;
			factor *= base_of_input;
		}	
	}
	else {
		/*Переводить число из 10 в 10 не требуется. Поэтому нужно только учесть знак*/
		to_decimal = atof(number);
        if(to_decimal < 0) {
            to_decimal = -to_decimal;
            sign_switched = 1;
        }
	}	

	i = 0;

	if(base_of_output != 10) {
		int_part = to_decimal;
		frac_part = to_decimal - int_part;

		/*Перевод из 10 в n ЦЕЛОЙ части*/
		while(int_part > 0) {
			if(int_part % base_of_output <= 9) {
				digit = '0' + int_part % base_of_output;
			}
			else {
				digit = 'A' + int_part % base_of_output - 10;
			}
			
			int_part /= base_of_output;
			rev_result[i++] = digit;
		}

		rev_result[i] = 0;
		(*output)[i] = 0;
        if(!sign_switched) {
            for(i = 0; i<strlen(rev_result); i++) {
                (*output)[i] = rev_result[strlen(rev_result)-1-i];
            }
        }
        else {
            for(i = 1; i<strlen(rev_result)+1; i++) {
                (*output)[i] = rev_result[strlen(rev_result)-i];
            }
            (*output)[0] = '-';
        }
		
		if(i==0){
			(*output)[i++] = '0';
		}
		(*output)[i++] = '.';

		/*Перевод из 10 в n ДРОБНОЙ части
		(в данной реализации калькулятора содержащей
		не более 15-ти знаков)*/
			
		for(j=0; j<15; j++) {
			frac_part *= base_of_output;
			int_part = frac_part;
			frac_part -= int_part;
			/*Рассмотрение целой части числа, полученного после умножения на base*/
			if(int_part <= 9) {
				digit = '0' + int_part;
			}
			else {
				digit = 'A' + int_part - 10;
			}
			(*output)[i++] = digit;
		}
		(*output)[i] = 0;
	}
	else {
		/*Переводить число из 10 в 10 не требуется. Нужно только учесть знак*/
		if(!sign_switched) {
			snprintf(*output, 100, "%.15f", to_decimal);
		}
		else {
			snprintf(*output, 100, "%.15f", -to_decimal);
		}
	}
}

/*Чтение числа целиком при обнаружении цифры или буквы в записи выражения*/
void read_found_operand(int *where, char buffer[100], char (*tmp_operand)[100])
{
	int i, j;
	i = j = 0;
	/*Проход по операнду, побочный сдвиг указателя*/
	while(1){
		if((isalnum(buffer[*where])) || (buffer[*where] == '.')) {
			(*tmp_operand)[i] = buffer[*where];
			i++;
			(*where)++;
		}
		else {
			for(j = i; j < strlen(tmp_operand); j++) {
				(*tmp_operand)[j] = 0;
			}
			break;
		}
	}
}


/*----------------*/
/* MAIN ALGORITHM */
/*----------------*/
int main()
{
	int base_of_input = 10, base_of_output = 10, accuracy = 6, uflag = 1, point_index, original_expression_length, i, j;
	char expression[100], input[100];
	double res;

	/*Буферы и стеки*/
	char buffer[50][100]; 
	int buffer_pointer = -1;
	init(&buffer_pointer, buffer);

	char arithm_buffer[50][100];
	int arithm_pointer = -1;
	init(&arithm_pointer, arithm_buffer);

	char *op1, *op2, operand_buffer[50];

	char pushing_buffer[50], *popping_buffer;
	/*Большинство элементов никогда не изменятся*/
	for(i=0; i<50; i++) {
		pushing_buffer[i] = 0;
	}

	char tmp_operand[50], tmp_operand_base_10[50];

	printf("Welcome to C Calculator!\n");
	while(1) {
		printf("Enter C to open calculator, Q to quit, I for some info: ");
		gets(input);
		if((input[0]=='Q') || (input[0]=='C')) {
			break;
		}
		else if(input[0]=='I') {
			printf("____________________________________________________________\n                         C Calculator\nAuthor: Sorokin Oleg, CMC MSU, gr. 216\nRules:\n1. Your expression can't be longer than 100 characters\n2. Operands can't be longer than 50 characters\n3. Fractional parts can't be longer than 15 characters\n4. Bases supported: 2-36\n5. Operations supported: +, - (binary and unary), *, /, (, )\n6. The calculator will be closed after giving an answer\n____________________________________________________________\n\n");
		}
		else {
			printf("Unknown command. Try again\n");
		}
	}
	if(input[0]=='C') {
		/*Ввод всех данных, интерфейс*/
		printf("\nEnter input (your expression's) base: ");
		gets(input);
		base_of_input = init_value(atoi(input), 10);
		printf("Enter output base: ");
		gets(input);
		base_of_output = init_value(atoi(input), 10);
		printf("Enter needed accuracy: ");
		gets(input);
		accuracy = init_value(atoi(input), 6);
		printf("Enter correct math expression: ");
		gets(expression);
		original_expression_length = strlen(expression);

		/*Внешние "мысленные скобки", необходимые для перевода в ПОЛИЗ*/
		put_in_buffer(&pushing_buffer, '(');
		push(pushing_buffer, &buffer_pointer, buffer);
		expression[strlen(expression)] = ')';

		/*Построение обратной польской нотации*/
		for(i = 0; i <= original_expression_length; i++) {
			if(isalnum(expression[i])) {
				read_found_operand(&i, expression, &tmp_operand);
				base_conversion(tmp_operand, base_of_input, 10, &tmp_operand_base_10);
				push(tmp_operand_base_10, &arithm_pointer, arithm_buffer);
				uflag = 0;
			} 

			switch(expression[i]) {
				/*В стек*/
				case '(':
					put_in_buffer(&pushing_buffer, '(');
					push(pushing_buffer, &buffer_pointer, buffer);
					uflag = 1;
					break;
				/*Вынуть всё до (*/
				case ')':
					while(!is_empty(buffer_pointer)) {
						popping_buffer = pop(&buffer_pointer, buffer);
						if(popping_buffer[0] == '(') {
							break;
						}
						push(popping_buffer, &arithm_pointer, arithm_buffer);
						uflag = 0;
					}
					break;
				/*Вынуть все операции старшие или равные по приоритету.
				Продолжать до ( или конца стека*/
				case '*':
				case '/':
					if(!is_empty(buffer_pointer)) {
						do {
							popping_buffer = pop(&buffer_pointer, buffer);
							if((popping_buffer[0] == '(') || (popping_buffer[0] == '+') || (popping_buffer[0] == '-')) {
								push(popping_buffer, &buffer_pointer, buffer);
								break;
							}
							push(popping_buffer, &arithm_pointer, arithm_buffer);
						} while(buffer_pointer != -1);
					}
					put_in_buffer(&pushing_buffer, expression[i]);
					push(pushing_buffer, &buffer_pointer, buffer);
					uflag = 0;
					break;
				/*Вынуть все операции старшие или равные по приоритету.
				Продолжать до ( или конца стека*/
				case '+':
				case '-':
					if(uflag) {
						put_in_buffer(&pushing_buffer, '0');
						push(pushing_buffer, &arithm_pointer, arithm_buffer);
					}
					if(!is_empty(buffer_pointer)) {
						do {
							popping_buffer = pop(&buffer_pointer, buffer);
							if(popping_buffer[0] == '(') {
								push(popping_buffer, &buffer_pointer, buffer);
								break;
							}
							push(popping_buffer, &arithm_pointer, arithm_buffer);
						} while(buffer_pointer != -1);
					}
					put_in_buffer(&pushing_buffer, expression[i]);;
					push(pushing_buffer, &buffer_pointer, buffer);
					uflag = 0;
					break;
				/*Пробел*/
				default:
					continue;
			}
		}

		/*На этом этапе имеем построенную обратную польскую
		нотацию, хранящуюся в арифметическом стеке.
		Теперь реализуем алгоритм Дейкстры для данных из
		арифметического стека.*/
		
		/*Очистка вспомогательного буфера для алгоритма Дейкстры*/
		init(&buffer_pointer, buffer);

		for(i = 0; i < 50; i++) {
			if(arithm_buffer[i][0] == 0) {
				break;
			}
			if(isalnum(arithm_buffer[i][0])) {
				push(arithm_buffer[i], &buffer_pointer, buffer);	
			}
			/*Применение операции к операндам из арифметического стека*/
			switch(arithm_buffer[i][0]) {
				case '+':
					op2 = pop(&buffer_pointer, buffer);
					op1 = pop(&buffer_pointer, buffer);
					snprintf(operand_buffer, 100, "%.15f", atof(op1)+atof(op2));
					push(operand_buffer, &buffer_pointer, buffer);
					break;
				case '-':
					op2 = pop(&buffer_pointer, buffer);
					op1 = pop(&buffer_pointer, buffer);
					snprintf(operand_buffer, 100, "%.15f", atof(op1)-atof(op2));
					push(operand_buffer, &buffer_pointer, buffer);
					break;
				case '/':
					op2 = pop(&buffer_pointer, buffer);
					op1 = pop(&buffer_pointer, buffer);
					snprintf(operand_buffer, 100, "%.15f", atof(op1)/atof(op2));
					push(operand_buffer, &buffer_pointer, buffer);
					break;
				case '*':
					op2 = pop(&buffer_pointer, buffer);
					op1 = pop(&buffer_pointer, buffer);
					snprintf(operand_buffer, 100, "%.15f", atof(op1)*atof(op2));
					push(operand_buffer, &buffer_pointer, buffer);
					break;
			}
		}
		/*Завершение алгоритма Дейкстры*/
		res = atof(pop(&buffer_pointer, buffer));
		if(res >= 0) {
			snprintf(tmp_operand_base_10, 100, "%.15f", res);
		}
		else {
			printf("-");
			res = -res;
			snprintf(tmp_operand_base_10, 100, "%.15f", res);
		}

		/*Перевод ответа в требуемую СС*/
		base_conversion(tmp_operand_base_10, 10, base_of_output, &tmp_operand);

		/*Печать ответа с требуемой точностью*/
		/*Поиск точки и печать целой части*/
		for(i = 0; i < strlen(tmp_operand); i++) {
			printf("%c", tmp_operand[i]);
			if(tmp_operand[i]=='.') {
				point_index = i;
				break;
			}
		}
		for(i = point_index+1; i < point_index+1+accuracy; i++) {
			printf("%c", tmp_operand[i]);
			if(i == point_index+accuracy) {
				printf("\n");
			}
		}
	}
	return 0;
}
