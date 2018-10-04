#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100+1
#define TRANSITION_SIZE 500+1
#define READING_BUFFER_SIZE 20000+1

/**
* node for TM graph
*/
typedef struct transition {

    char read_char;
    int start_state;
    char write_char;
    int end_state;
    char head_movement;

    struct transition* next_state;          // next state on the list of member i

} transition_t;

/**
* node for the acceptance list
*/
typedef struct state {

    int state_number;

    struct state* next;

} state_t;

/**
* node for the queue
*/
typedef struct queue {

    int current_state;
    char* copy_tape;
    int copy_tape_pointer;
    int move_count;

    struct queue* next;

} queue_t;

/**
* Replaces eol with null in the passed string
*/
void clean_eol(char *str);

/**
* Finds if the passed state is in the acceptance list
* Returns 1 if it is, 0 else
*/
int find_acceptance(state_t* acceptance_head, int current_state);

/**
* Moves tape pointer, returns new value of it
*/
int move_pointer(int tape_pointer, char head_movement, char* *tape);

/**
* Empties memory tape list, sets tape_head = NULL
*/
void empty_tape(char* *tape);

/**
* Duplicates memory tape, returns new head
*/
char* copy_tape(char* original_list);

/**
* Adds a new node to the queue
*/
void enqueue(queue_t* *front, queue_t* *rear, int current_state, char* copy_tape, int copy_tape_pointer, int move_count);

/**
* Returns front element of the queue
*/
queue_t* dequeue(queue_t* *front, queue_t* *rear);

/**
* Cleans the queue
*/
void clean_queue(queue_t* *front, queue_t* *rear);

// DELETE

/**
* Prints out memory tape with pointer highlighted
*/
void print_tape(char* tape, int pointer) {

    int i = 0;

    for (i = 0; tape[i] != '\0'; i++)
        if (i == pointer)
            printf("|%c| ", tape[i]);
        else
            printf("%c ", tape[i]);
    printf("\n");
}

/**
*Prints a transition
*/
void print_transition(transition_t* tr) {

    while (tr) {
        printf("%d %c %c %c %d", tr->start_state, tr->read_char, tr->write_char, tr->head_movement, tr->end_state);
        printf(" || ");
        tr = tr->next_state;
    }
    printf("\n");
}

/**
* Prints out TM transitions graph
*/
void print_TM(transition_t* *transition_list, int size) {

    int i = 0;

    printf("\n");
    for (i = 0; i<size; i++)
        if (transition_list[i])
            print_transition(transition_list[i]);
    printf("-------------\n");
}

/**
* Prints out queue
*/
void print_queue(queue_t* front) {

    queue_t* tmp = front;

    while(tmp) {
        printf("%d", tmp->current_state);
        printf(" | ");
        tmp = tmp->next;
    }
    printf("\n");
}

//DELETE

int main(int argc, char *argv[]) {

    //FILE *file = fopen("Test/IncreasingStuff.txt", "r");

    int max_size = 2000, size = 0;
    transition_t* *transition_list = calloc(max_size, sizeof(transition_t));

    state_t* acceptance_head = NULL;
    queue_t* front = NULL;
    queue_t* rear = NULL;

    state_t* next_state = NULL;
    transition_t* next_tr = NULL;
    int max_transitions = 0;

    int tape_pointer = 0;
    char* tape = NULL;

    queue_t* queue_elem = NULL;
    transition_t* move = NULL;

    char buffer[BUFFER_SIZE] = {"\0"};
    char read_buffer[READING_BUFFER_SIZE] = {"\0"};
    char tr[TRANSITION_SIZE] = {"\0"};

    char ret = '0';

    /** reading "tr" */
    if (fgets(buffer, BUFFER_SIZE, stdin))
        clean_eol(buffer);

    if (strcmp(buffer, "tr") == 0) {
        /** setting transitions */
        if (fgets(tr, TRANSITION_SIZE, stdin))
            clean_eol(tr);

        while (strcmp(tr, "acc") != 0) {        // until acc is found, read a line
            next_tr = calloc(1, sizeof(transition_t));    // allocate a new transition

            next_tr->start_state = atoi(strtok(tr, " "));       // locate first token
            next_tr->read_char = *strtok(NULL, " ");        // call with NULL implies the usage of the previous input
            next_tr->write_char = *strtok(NULL, " ");
            next_tr->head_movement = *strtok(NULL, " ");
            next_tr->end_state = atoi(strtok(NULL, " "));

            if (next_tr->start_state+1 > size)
                size = next_tr->start_state+1;

            if (size > (max_size)) {                        // if start state is bigger than the size
                max_size = 2*max_size;
                transition_list = realloc(transition_list, (max_size) * sizeof(transition_t));      // realloc with the new doubled size
            }

            if (transition_list[next_tr->start_state])                   // if not first state with this number, set as head of the list
                next_tr->next_state = transition_list[next_tr->start_state];
            else
                next_tr->next_state = NULL;

            transition_list[next_tr->start_state] = next_tr;

            if (fgets(tr, TRANSITION_SIZE, stdin))
                clean_eol(tr);
        }
        transition_list = realloc(transition_list, size * sizeof(transition_t));
    }

    /** here, "acc" is already read */
    if (fgets(buffer, BUFFER_SIZE, stdin)) {
        clean_eol(buffer);

        /** setting acceptance states */
        while (strcmp(buffer, "max") != 0) {        // until max is found, read a line
            next_state = calloc(1, sizeof(state_t));
            next_state->state_number = atoi(buffer);

            next_state->next = acceptance_head;       // insertion in the head
            acceptance_head = next_state;

            if (fgets(buffer, BUFFER_SIZE, stdin))
                clean_eol(buffer);
        }
    }

    /** here, "max" is already read */
    if (fgets(buffer, BUFFER_SIZE, stdin)) {
        clean_eol(buffer);

        /** setting max transition */
        max_transitions = atoi(buffer);
    }

    /** reading "run" */
    if (fgets(buffer, BUFFER_SIZE, stdin))
        clean_eol(buffer);

    if (strcmp(buffer, "run") == 0) {

        while (fgets(read_buffer, READING_BUFFER_SIZE, stdin)) {          // until end of stdin is reached, read first string

            clean_eol(read_buffer);

            tape = copy_tape(read_buffer);         // copy the string input into tape memory
            enqueue(&front, &rear, 0, tape, tape_pointer, 0);        // enqueue initial condition

            // perform a BFS
            while (front && ret != '1' && ret != 'U') {             // until there's at least one element in the queue

                queue_elem = dequeue(&front, &rear);         // pull from queue

                if (queue_elem->move_count == max_transitions)     // if reached maximum number of moves, return U
                    ret = 'U';

                else {
                    move = transition_list[queue_elem->current_state];      // if not, get the first transition with the current state taken from the queue

                    while (move && ret != '1') {                                      // iterate over all moves

                        if (move->read_char == queue_elem->copy_tape[queue_elem->copy_tape_pointer]) {     // if it's a possible move

                            if (find_acceptance(acceptance_head, move->end_state) == 1)
                                ret = '1';
                            else {
                                tape = copy_tape(queue_elem->copy_tape);                            // duplicates actual memory tape
                                tape[queue_elem->copy_tape_pointer] = move->write_char;         // set the character in the new memory tape

                                tape_pointer = move_pointer(queue_elem->copy_tape_pointer, move->head_movement, &tape);               // move the pointer

                                enqueue(&front, &rear, move->end_state, tape, tape_pointer, queue_elem->move_count+1);          // enqueue new conditions
                            }
                        }
                        move = move->next_state;
                    }
                    empty_tape(&queue_elem->copy_tape);
                    free(queue_elem);
                }
            }

            fputc(ret, stdout);
            fputc('\n', stdout);
            fflush(stdout);

            ret = '0';
            clean_queue(&front, &rear);
            tape_pointer = 0;
        }
    }

    return 0;
}

/**
* Replaces eol with null in the passed string
*/
void clean_eol(char *str) {
    size_t len = strlen(str);
    if (len && str[len-1] == '\n')
        str[len-1] = '\0';
}

/**
* Finds if the passed state is in the acceptance list
* Returns 1 if it is, 0 else
*/
int find_acceptance(state_t* acceptance_head, int current_state) {

    state_t* find = acceptance_head;

    while (find) {
        if (find->state_number == current_state)
            return 1;
        find = find->next;
    }

    return 0;
}

/**
* Moves tape pointer, returns new value of it
*/
int move_pointer(int tape_pointer, char head_movement, char* *tape) {

    size_t len = 0;
    char* tmp = NULL;

    if (head_movement == 'R') {
        len = strlen(*tape);
        if (tape_pointer == len-1) {
            *tape = realloc(*tape, (len+2) * sizeof(char));          // strlen does not count the '\0' which I want to keep
            strcat(*tape, "_\0");
        }
        tape_pointer++;
    }
    else if (head_movement == 'L') {
        if (tape_pointer == 0) {
            len = strlen(*tape);
            tmp = *tape;
            *tape = calloc(len+2, sizeof(char));          // strlen does not count the '\0' which I want to keep
            *tape[0] = '_';
            strcat(*tape, tmp);
            tape_pointer = 1;               // tape pointer needs to be at 0 after --
            free(tmp);
        }
        tape_pointer--;
    }

    return tape_pointer;
}

/**
* Empties memory tape list, sets tape_head = NULL
*/
void empty_tape(char* *tape) {

    free(*tape);
    *tape = NULL;
}

/**
* Duplicates memory tape, returns new head
*/
char* copy_tape(char* original_list) {

    size_t len = strlen(original_list);
    char* list = calloc(len+1, sizeof(char));

    strncpy(list, original_list, len);              // strncpy appends '\0' at the end

    return list;
}

/**
* Adds a new node to the queue
*/
void enqueue(queue_t* *front, queue_t* *rear, int current_state, char* copy_tape, int copy_tape_pointer, int move_count) {

    queue_t* tmp = calloc(1, sizeof(queue_t));
	tmp->current_state = current_state;
	tmp->copy_tape = copy_tape;
	tmp->copy_tape_pointer = copy_tape_pointer;
	tmp->move_count = move_count;

	tmp->next = NULL;

	if(!(*front) && !(*rear))
		*front = *rear = tmp;
    else {
        (*rear)->next = tmp;
        *rear = tmp;
    }
}

/**
* Returns front element of the queue
*/
queue_t* dequeue(queue_t* *front, queue_t* *rear) {

	queue_t* tmp = *front;

	if (*front == *rear)
		*front = *rear = NULL;
	else
		*front = (*front)->next;

	return tmp;
}

/**
* Cleans the queue
*/
void clean_queue(queue_t* *front, queue_t* *rear) {

    queue_t* tmp = NULL;

    while (*front) {
        tmp = dequeue(front, rear);
        empty_tape(&tmp->copy_tape);
        free(tmp);
    }
}
