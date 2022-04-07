#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

//will be used in array of structures (cells)
struct conv {
	int number_of_ins; //mul of all previous ...
	int number_of_terms; //for one cell
    int start_pipe;
};

//cmd tree
struct cmd {
    char cmd[100];
    struct cmd *next;
};

//read cmd, do all the necessary things
struct cmd *cmd_tree () {
    int i, j=0, k;
    int spaceflag;
    char buffer[500] = {}, buffercpy[100] = {};

    struct cmd *cur, *tree = malloc(sizeof(struct cmd));
    tree->next = NULL;
    cur = tree;

    gets(buffer);

    //quit shell
    if (!strcmp(buffer, "exit")) {
        kill(getppid(), SIGUSR1);
        _exit(0);
    }

    for (i=0; i<500; i++) {
        if (buffer[i] == ' ') {
            spaceflag = 1;
            continue;
        } else if ((buffer[i] == '+') || (buffer[i] == '|') || (buffer[i] == '<') || (buffer[i] == '>') || (buffer[i] == '\n') || (buffer[i] == '\0') || (buffer[i] == '&')) {
            strcpy(cur->cmd, buffercpy);
            memset(buffercpy, 0, 100);
            cur->next = malloc(sizeof(struct cmd));
            cur->next->next = NULL;
            cur = cur->next;
            if (buffer[i] == '\0') break;

            buffercpy[0] = buffer[i];
            strcpy(cur->cmd, buffercpy);
            memset(buffercpy, 0, 100);
            cur->next = malloc(sizeof(struct cmd));
            cur->next->next = NULL;
            cur = cur->next;

            j=0;
        } else {
            if (spaceflag && buffercpy[0] != '\0') {
                strcpy(cur->cmd, buffercpy);
                memset(buffercpy, 0, 100);
                j=0;
                cur->next = malloc(sizeof(struct cmd));
                cur->next->next = NULL;
                cur = cur->next;
            }
            spaceflag = 0;
            buffercpy[j] = buffer[i];
            j++;
        }
    }

    return tree;
}

//count number of cells
int conv_len(struct cmd *tree)
{
    int number_cells = 1;
    while (tree != NULL) {
        if (tree->cmd[0] == '|') {
            number_cells++;
        }
        tree = tree->next;
    }

    return number_cells;
}

//fill in some struct info
void fill_conv(struct conv *conv_info, struct cmd *tree, int number_nodes)
{
    int i, node = 0;

    for (i=0; i<number_nodes; i++) {
        conv_info[i].number_of_terms = 1;
    }

    while (tree != NULL) {
        if (tree->cmd[0] == '|') {
            node++;
        } else if (tree->cmd[0] == '+') {
            conv_info[node].number_of_terms++;
        }
        tree = tree->next;
    }

	for (i = 0; i < number_nodes; i++) {
		if (i > 0) {
			conv_info[i].number_of_ins = conv_info[i - 1].number_of_ins * conv_info[i].number_of_terms;
            conv_info[i].start_pipe = conv_info[i-1].start_pipe + conv_info[i-1].number_of_ins;
		} else {
			conv_info[0].number_of_ins = conv_info[0].number_of_terms;
            conv_info[0].start_pipe = 0;
        }
	}
}

//yes
int count_number_of_pipes(struct conv *conv_info, int number_nodes)
{
    int n=0, i;
    for (i=0; i<number_nodes; i++) {
        n += conv_info[i].number_of_ins;
    }

    return n;
}

//how many fields will be needed in **argv
int estimate(struct cmd *tree)
{
    int s = 0, redirect = 0;
    while ((tree->cmd[0] != '+') && (tree->cmd[0] != '\0') && (tree->cmd[0] != '|')) {
        if ((tree->cmd[0] == '<') || (tree->cmd[0] == '>')) {
            redirect = 1;
        }
        if (!redirect) {
            s++;
        }
        tree = tree->next;
    }

    return s;
}

//prep for execvp
char **find_argv(int cell, int i, struct cmd *tree, struct conv *conv_info)
{
    char **argv;
    int j = i % conv_info[cell].number_of_terms, cellcount = 0, termcount = 0;
    int redirect;

    while ((tree->cmd[0] != '\0') && (cellcount != cell)) {
        if (tree->cmd[0] == '|') {
            cellcount++;
        }
        tree = tree->next;
    }

    while (j != termcount) {
        if ((tree->cmd[0] == '+')) {
            termcount++;
        }
        tree = tree->next;
    }

    j = 0;

    argv = calloc(1 + estimate(tree), sizeof(char *));

    while ((tree->cmd[0] != '+') && (tree->cmd[0] != '\0') && (tree->cmd[0] != '|')) {
        if ((tree->cmd[0] == '<') || ((tree->cmd[0] == '>'))){
            redirect = 1;
        }
        if (!redirect) {
            argv[j] = malloc(100 * sizeof(char));
            strcpy(argv[j], tree->cmd);
            j++;
        }
        tree = tree->next;
    }

    argv[j] = NULL;

    return argv;
}

//Previous opipes to new ipipes for further work
void pass_to_cell(int m, int ipipes[][2], int opipes[][2], struct conv *conv_info, int max)
{
	int i, j, k, sbuf;
	char buf[1];

    for (j = conv_info[m-1].start_pipe; j < conv_info[m-1].start_pipe + conv_info[m-1].number_of_ins; j++) {
        if (!fork()) {
            while ((sbuf = read(opipes[j][0], buf, 1)) > 0) {
                /*Overflow's impossible*/
                for (i = conv_info[m].start_pipe + (j - conv_info[m-1].start_pipe)*conv_info[m].number_of_terms; i < conv_info[m].start_pipe + (1 + j - conv_info[m-1].start_pipe)*conv_info[m].number_of_terms; i++) {
                    write(ipipes[i][1], buf, 1);
                }
            }
            _exit(0);
        }
        wait(NULL);
    }
}

// cell1 < a.txt ...
int check_input_redirect(struct cmd *tree)
{
    int fd = -1;
    char buf[1];
    while (tree != NULL) {
        if (tree->cmd[0] == '<') {
            tree = tree->next;
            fd = open(tree->cmd, O_RDONLY);
            break;
        }
        tree = tree->next;
    }

    return fd;
}

//mirror ipipes->opipes. opipes[i] = f(ipipes[i])
//m is the cell
void run_cell(int m, int ipipes[][2], int opipes[][2], struct conv *conv_info, struct cmd *tree, int max)
{
    int i, j;
    int sbuf;
    int fd = -1;
    char buf[1];
    if (m == 0) {
        fd = check_input_redirect(tree);
    }

    /*Running cell multiple times (if needed)*/
    for (i=0; i < conv_info[m].number_of_ins; i++) {
            char **argv = find_argv(m, i, tree, conv_info);
        if (!fork()) {
            if (m > 0) {
                dup2(ipipes[conv_info[m].start_pipe + i][0], 0);
                close(ipipes[conv_info[m].start_pipe + i][0]);
            }
            if (fd != -1) {
                lseek(fd, 0, SEEK_SET);
                dup2(fd, 0);
                close(fd);
            }
            dup2(opipes[conv_info[m].start_pipe + i][1], 1);
            close(opipes[conv_info[m].start_pipe + i][1]);

            execvp(argv[0], argv);
            _exit(1);
        }
        wait(NULL);
    }
    close(fd);
}

// ... > a.txt or >> a.txt
int check_output_redirect(struct cmd *tree)
{
    int fd = -1;
    while (tree != NULL) {
        if (tree->cmd[0] == '>') {
            if (tree->next->next->cmd[0] == '>') {
                fd = open(tree->next->next->next->cmd, O_WRONLY | O_APPEND | O_CREAT, 0777);
            } else {
                fd = open(tree->next->cmd, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            }
            break;
        }
        tree = tree->next;
    }

    return fd;
}

//cat answer
void collector(int number_of_pipes, int number_of_cells, struct conv *conv_info, struct cmd *tree, int opipes[][2])
{
    char buf[1];
    int i, sbuf;
    int fd = check_output_redirect(tree);

    if (fd == -1) {
        for (i = number_of_pipes - conv_info[number_of_cells - 1].number_of_ins; i < number_of_pipes; i++) {
            while ((sbuf = read(opipes[i][0], buf, 1)) > 0) {
                printf("%s", buf);
            }
        }
    } else {
        for (i = number_of_pipes - conv_info[number_of_cells - 1].number_of_ins; i < number_of_pipes; i++) {
            while ((sbuf = read(opipes[i][0], buf, 1)) > 0) {
                write(fd, buf, 1);
            }
        }
        close(fd);
    }
}

//cmd life cycle
void life(int number_of_cells, int number_of_pipes, struct conv *conv_info, struct cmd *tree)
{
    int i, j;
    int ipipes[number_of_pipes][2], opipes[number_of_pipes][2];

    for (i=0; i<number_of_pipes; i++) {
        pipe(ipipes[i]);
        pipe(opipes[i]);
    }
    for (i=0; i<number_of_cells; i++) {
        if (i != 0) {
            for(j=conv_info[i-1].start_pipe; j<conv_info[i-1].start_pipe + conv_info[i-1].number_of_ins; j++) {
                close(opipes[j][1]);
            }
        }
        if (!fork()) {
            if (i != 0) {
                pass_to_cell(i, ipipes, opipes, conv_info, number_of_pipes);
            }
            _exit(0);
        }

        for(j=conv_info[i].start_pipe; j<conv_info[i].start_pipe + conv_info[i].number_of_ins; j++) {
            close(ipipes[j][1]);
        }

        if(!fork()) {
            run_cell(i, ipipes, opipes, conv_info, tree, number_of_pipes);
            _exit(0);
        }
    }

    while(wait(NULL) != -1);

    for (i = number_of_pipes - conv_info[number_of_cells - 1].number_of_ins; i < number_of_pipes; i++) {
        close(opipes[i][1]);
    }

    collector(number_of_pipes, number_of_cells, conv_info, tree, opipes);

    //close ALL pipes for sure
    for (i=0; i<number_of_pipes; i++) {
        close(ipipes[i][0]);
        close(ipipes[i][1]);
        close(opipes[i][0]);
        close(opipes[i][1]);
    }
}

//(Conflicting names)
//if daemon? (if & found)
int daemon_(struct cmd *tree)
{
    int fd, d = 0;
    while (tree != NULL) {
        if (tree->cmd[0] == '&') {
            //printf("DAEMON!!!\n");
            //fflush(stdout);
            // using /dev/null to disconnect from stdio
            fd = open("/dev/null", O_RDONLY);
            dup2(fd, 0);
            close(fd);
            fd = open("/dev/null", O_WRONLY);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
            //Own process group
            setpgrp();
            d = 1;
            break;
        }
        tree = tree->next;
    }

    return d;
}

void free_tree(struct cmd *tree)
{
    if (tree != NULL) {
        free_tree(tree->next);
    }
    free(tree);
}

//exit line => SIGUSR1 => exit(0)
void exit_shell(int s)
{
	wait(NULL);
	exit(0);
}

int main()
{
    struct cmd *tree;
    int number_of_cells;
    struct conv *conv_info;
    int number_of_pipes, pid;
	signal(SIGUSR1, exit_shell);

    //main
    while (1) {
        if (!fork()) {
            printf("\n$ ");
            //getlines
            tree = cmd_tree();
            //&
            if (daemon_(tree)) {
                if (fork()) {
                    _exit(0);
                }
            }
            number_of_cells = conv_len(tree);
            conv_info = calloc(number_of_cells, sizeof(struct conv));
            fill_conv(conv_info, tree, number_of_cells);
            number_of_pipes = count_number_of_pipes(conv_info, number_of_cells);
            
            life(number_of_cells, number_of_pipes, conv_info, tree);

            while (wait(NULL) != -1);

            free_tree(tree);
            _exit(0);
        }
        wait(NULL);
    }

    return 0;
}