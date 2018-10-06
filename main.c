#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 100+1
#define TRANSITION_SIZE 500+1
#define READING_BUFFER_SIZE 20000+1

#define INITIAL_GRAPH_SIZE 2000
#define CHUNK_SIZE 500

/**
* node for TM graph
*/
typedef struct transition {

    char read_char;
    int start_state;
    char write_char;
    int end_state;
    char head_movement;

    int acceptance;

    struct transition* next_state;          // next state on the list of member i

} transition_t;

/**
* node for the tape
*/
typedef struct tape {

    int offset;
    char chunk[CHUNK_SIZE];

    struct tape* next;
    struct tape* prev;

} tape_t;

/**
* node for the queue
*/
typedef struct queue {

    int current_state;
    tape_t* copy_tape_head;
    int copy_string_pointer;
    int copy_tape_offset;
    int move_count;

    struct queue* next;

} queue_t;

/**
* Replaces eol with null in the passed string
*/
void clean_eol(char *str);

/**
* Returns max of 2 int
*/
int max(int a, int b);

/**
* Copies given buffer into a memory tape, returns head of the tape
*/
tape_t* copy_buffer_to_tape(char buffer[]);

/**
* Finds char pointed in the memory tape
*/
char find_pointed_char(tape_t* head, int string_pointer, int tape_offset);

/**
* Sets new char in the memory tape
*/
void write_char(tape_t* head, char write_char, int string_pointer, int tape_offset);

/**
* Moves tape pointer, returns new value of it
*/
tape_t* move_pointer(tape_t* head, int* string_pointer, int* tape_offset, char head_movement);

/**
* Empties memory tape list, sets tape_head = NULL
*/
void empty_tape(tape_t* head);

/**
* Duplicates memory tape, returns new head
*/
tape_t* copy_tape(tape_t* original_head);

/**
* Adds a new node to the queue
*/
void enqueue(queue_t* *front, queue_t* *rear, int current_state, tape_t* copy_tape_head, int copy_string_pointer, int copy_tape_offset, int move_count);

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
void print_tape(tape_t* head, int string_pointer, int tape_offset) {

    tape_t* curr = head;
    int i = 0;

    while (curr) {
        if (curr->offset == tape_offset)
            for (i = 0; i < CHUNK_SIZE; i++)
                if (i == string_pointer)
                    printf("|%c|", curr->chunk[i]);
                else
                    printf("%c", curr->chunk[i]);
        else
            printf("%s", curr->chunk);
        curr = curr->next;
    }
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
        if (transition_list[i]->start_state != -1)
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

    //FILE *file = fopen("Test/FancyLoops.txt", "r");

    int max_size = INITIAL_GRAPH_SIZE, size = 0;
    int i = 0;
    transition_t* *transition_list = calloc(max_size, sizeof(transition_t*));

    queue_t* front = NULL;
    queue_t* rear = NULL;

    transition_t* next_tr = NULL;
    int max_transitions = 0;

    int string_pointer = 0, tape_offset = 0;
    tape_t* tape_head = NULL;
    char pointed_char = '\0';

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

            next_tr->acceptance = 0;

            if (max(next_tr->start_state+1, next_tr->end_state+1) > size)
                size = max(next_tr->start_state+1, next_tr->end_state+1);

            if (size > (max_size)) {                        // if start state is bigger than the size
                max_size = 2*max_size;
                transition_list = realloc(transition_list, (max_size) * sizeof(transition_t*));      // realloc with the new doubled size
            }

            if (transition_list[next_tr->start_state])                   // if not first state with this number, set as head of the list
                next_tr->next_state = transition_list[next_tr->start_state];
            else
                next_tr->next_state = NULL;

            transition_list[next_tr->start_state] = next_tr;

            if (fgets(tr, TRANSITION_SIZE, stdin))
                clean_eol(tr);
        }
        transition_list = realloc(transition_list, size * sizeof(transition_t*));
    }

    /** here, "acc" is already read */
    if (fgets(buffer, BUFFER_SIZE, stdin)) {
        clean_eol(buffer);

        /** setting acceptance states */
        while (strcmp(buffer, "max") != 0) {        // until max is found, read a line
            next_tr = calloc(1, sizeof(transition_t));
            next_tr->start_state = -1;          // sentinel value, no transitions for this state
            next_tr->next_state = NULL;           // pointer safety
            next_tr->acceptance = 1;
            transition_list[atoi(buffer)] = next_tr;

            if (fgets(buffer, BUFFER_SIZE, stdin))
                clean_eol(buffer);
        }
    }

    /** filling array spaces of state with no transition and not final */
    for (i = 0; i < size; i++)
        if (!transition_list[i]) {
            next_tr = calloc(1, sizeof(transition_t));
            next_tr->start_state = -1;      // sentinel value, no transitions for this state
            next_tr->next_state = NULL;           // pointer safety
            next_tr->acceptance = 0;
            transition_list[i] = next_tr;
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

            tape_head = copy_buffer_to_tape(read_buffer);         // copy the string input into tape memory
            enqueue(&front, &rear, 0, tape_head, string_pointer, tape_offset, 0);        // enqueue initial condition

            // perform a BFS
            while (front && ret != '1' && ret != 'U') {             // until there's at least one element in the queue

                queue_elem = dequeue(&front, &rear);         // pull from queue
                pointed_char = find_pointed_char(queue_elem->copy_tape_head, queue_elem->copy_string_pointer, queue_elem->copy_tape_offset);

                /*printf("Dequeue\n");                                            // DELETE
                printf("Current state = %d\n", queue_elem->current_state);        // DELETE
                print_tape(queue_elem->copy_tape_head, queue_elem->copy_string_pointer, queue_elem->copy_tape_offset);      // DELETE
                printf("Pointed char in the tape = %c\n", pointed_char); */         // DELETE

                if (queue_elem->move_count == max_transitions)     // if reached maximum number of moves, return U
                    ret = 'U';

                else {
                    move = transition_list[queue_elem->current_state];      // if not, get the first transition with the current state taken from the queue

                    while (move && ret != '1') {                                      // iterate over all moves

                        //printf("Char in the move = %c\n", move->read_char);         // DELETE

                        if (move->read_char == pointed_char) {     // if it's a possible move

                            if (transition_list[move->end_state]->acceptance == 1)
                                ret = '1';
                            else {
                                tape_head = copy_tape(queue_elem->copy_tape_head);                            // duplicates actual memory tape

                                string_pointer = queue_elem->copy_string_pointer;
                                tape_offset = queue_elem->copy_tape_offset;
                                write_char(tape_head, move->write_char, string_pointer, tape_offset);         // set the character in the new memory tape

                                tape_head = move_pointer(tape_head, &string_pointer, &tape_offset, move->head_movement);               // move the pointer

                                enqueue(&front, &rear, move->end_state, tape_head, string_pointer, tape_offset, queue_elem->move_count+1);          // enqueue new conditions

                                /*printf("\nEnqueue\n");              // DELETE
                                printf("Next state %d\n", move->end_state);             // DELETE
                                print_tape(tape_head, string_pointer, tape_offset);             // DELETE
                                printf("\n");*/
                            }
                        }
                        move = move->next_state;
                    }
                    empty_tape(queue_elem->copy_tape_head);
                    free(queue_elem);
                }
            }

            fputc(ret, stdout);
            fputc('\n', stdout);
            fflush(stdout);

            ret = '0';
            clean_queue(&front, &rear);
            string_pointer = 0;
            tape_offset = 0;
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
* Returns max of 2 int
*/
int max(int a, int b) {
    if (a >= b)
        return a;
    return b;
}

/**
* Copies given buffer into a memory tape, returns head of the tape
*/
tape_t* copy_buffer_to_tape(char buffer[]) {

    tape_t* new_head = calloc(1, sizeof(tape_t));
    tape_t* prev = NULL;
    size_t len = strlen(buffer);

    int i = 0;
    int size = len/CHUNK_SIZE;
    int mod = len%CHUNK_SIZE;
    if (mod != 0)
        size++;

    new_head->offset = 0;
    strncpy(new_head->chunk, buffer, CHUNK_SIZE);
    new_head->prev = NULL;

    prev = new_head;
    for (i = 1; i < size; i++) {
        prev->next = calloc(1, sizeof(tape_t));
        prev->next->offset = i;
        strncpy(prev->next->chunk, &buffer[CHUNK_SIZE*i], CHUNK_SIZE);

        prev->next->prev = prev;
        prev = prev->next;
    }
    prev->next = NULL;

    if (mod != 0)
        for (i = mod; i < CHUNK_SIZE; i++)
            prev->chunk[i] = '_';

    return new_head;
}

/**
* Finds char pointed in the memory tape
*/
char find_pointed_char(tape_t* head, int string_pointer, int tape_offset) {

    tape_t* curr = head;

    while (curr) {
        if (curr->offset == tape_offset)
            return curr->chunk[string_pointer];
        curr = curr->next;
    }

    return '\0';
}

/**
* Sets new char in the memory tape
*/
void write_char(tape_t* head, char write_char, int string_pointer, int tape_offset) {

    tape_t* curr = head;

    while (curr) {
        if (curr->offset == tape_offset)
            curr->chunk[string_pointer] = write_char;
        curr = curr->next;
    }
}

/**
* Moves tape pointer and tape_offset, returns head of tape
*/
tape_t* move_pointer(tape_t* head, int* string_pointer, int* tape_offset, char head_movement) {

    tape_t* tmp = head;
    int i = 0;

    if (head_movement == 'R') {

        if (*string_pointer%CHUNK_SIZE == CHUNK_SIZE-1) {         // if it's the last char in the chunk

            while (tmp && tmp->offset != *tape_offset)          // find correct offset element in the tape
                tmp = tmp->next;

            if (!tmp->next) {                                   // if moving to the next offset brings to a not initialized element
                tmp->next = calloc(1, sizeof(tape_t));
                tmp->next->offset = ++(*tape_offset);
                for (i = 0; i < CHUNK_SIZE; i++)                // sets all the new chunk element to '_'
                    tmp->next->chunk[i] = '_';

                tmp->next->next = NULL;
                tmp->next->prev = tmp;
            }
            else
                (*tape_offset)++;

            *string_pointer = 0;
        }
        else
            (*string_pointer)++;
    }
    else if (head_movement == 'L') {

        if (*string_pointer == 0) {                             // if it's the first char in the chunk

            while (tmp && tmp->offset != *tape_offset)          // find correct offset element in the tape
                tmp = tmp->next;

            if (!tmp->prev) {                                   // if moving to the next offset brings to a not initialized element
                tmp->prev = calloc(1, sizeof(tape_t));
                tmp->prev->offset = --(*tape_offset);
                for (i = 0; i < CHUNK_SIZE; i++)                // sets all the new chunk element to '_'
                    tmp->prev->chunk[i] = '_';

                tmp->prev->prev = NULL;
                tmp->prev->next = tmp;

                head = tmp->prev;                               // changes new head
            }
            else
                (*tape_offset)--;

            *string_pointer = CHUNK_SIZE-1;
        }
        else
            (*string_pointer)--;
    }

    return head;
}

/**
* Empties memory tape list, sets tape_head = NULL
*/
void empty_tape(tape_t* head) {

    tape_t* curr = head;
    tape_t* next = NULL;

    while (curr) {
        next = curr->next;
        free(curr);
        curr = next;
    }
    head = NULL;
}

/**
* Duplicates memory tape, returns new head
*/
tape_t* copy_tape(tape_t* original_head) {

    tape_t* curr = NULL;
    tape_t* prev = NULL;

    if (!original_head)
        return NULL;

    tape_t* new_head = calloc(1, sizeof(tape_t));           // starts with head
    new_head->offset = original_head->offset;
    strncpy(new_head->chunk, original_head->chunk, CHUNK_SIZE);

    new_head->prev = NULL;
    prev = new_head;
    curr = original_head->next;

    while(curr) {                                          // until original list is finished
        prev->next = calloc(1, sizeof(tape_t));
        prev->next->offset = curr->offset;
        strncpy(prev->next->chunk, curr->chunk, CHUNK_SIZE);

        prev->next->prev = prev;

        prev = prev->next;
        curr = curr->next;
    }

    prev->next = NULL;

    return new_head;
}

/**
* Adds a new node to the queue
*/
void enqueue(queue_t* *front, queue_t* *rear, int current_state, tape_t* copy_tape_head, int copy_string_pointer, int copy_tape_offset, int move_count) {

    queue_t* tmp = calloc(1, sizeof(queue_t));
	tmp->current_state = current_state;
	tmp->copy_tape_head = copy_tape_head;
	tmp->copy_string_pointer = copy_string_pointer;
	tmp->copy_tape_offset = copy_tape_offset;
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
        empty_tape(tmp->copy_tape_head);
        free(tmp);
    }
}
