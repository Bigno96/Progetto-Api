#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>

#define BUFFER_SIZE 5+1
#define TRANSITION_SIZE 50+1
#define READING_BUFFER_SIZE 100+1

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
    struct transition* right_state;         // next state of next i

} transition_t;

/**
* node for the TM tape
*/
typedef struct tape {

    char character;
    int position;

    struct tape* next;
    struct tape* prev;

} tape_t;

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
    tape_t* copy_tape_head;
    int copy_tape_pointer;
    int move_count;

    struct queue* next;

} queue_t;

transition_t* tm_head = NULL;
tape_t* tape_head = NULL;
state_t* acceptance_head = NULL;
queue_t* front = NULL;
queue_t* rear = NULL;

const char SEPARATOR = '$';

FILE * file = NULL;

/**
* Replaces eol with null in the passed string
*/
void clean_eol(char *str);

/**
* Removes all spaces from a given string
*/
void remove_spaces(char *str);

/**
* Inserts '$' separator between element of passed transition
*/
void insert_separator(char *str);

/**
* Finds the list corresponding to transitions with starting state number = position, starting from 0
* Returns pointer to node searched, null if none
* Pre is modified to point to previous element in the collection of list
*/
transition_t* find_array_element(transition_t* *pre, int position);

/**
* Inserts a new transition into the TM graph
*/
void add_next_transition(transition_t* next);

/**
* Sets down all transitions
*/
void set_transition();

/**
* Sets the acceptance states list
*/
void set_acceptance_states();

/**
* Finds if the passed state is in the acceptance list
* Returns 1 if it is, 0 else
*/
int find_acceptance(int current_state);

/**
* Writes down the input in the memory tape list
*/
void copy_input_to_tape(char buffer[]);

/**
* Moves tape pointer, returns new value of it
*/
int move_pointer(int tape_pointer, char head_movement, tape_t* *head);

/**
* Finds element which has passed position in the memory tape
* Returns NULL if no element is found
*/
tape_t* find_tape_element(tape_t* head, int position);

/**
* Empties memory tape list, sets tape_head = NULL
*/
void empty_tape();

/**
* Duplicates memory tape, returns new head
*/
tape_t* copy_tape(tape_t* original_head);

/**
* Adds a new node to the queue
*/
void enqueue(int current_state, tape_t* copy_tape_head, int copy_tape_pointer, int move_count);

/**
* Returns front element of the queue
*/
queue_t* dequeue();

/**
* Cleans the queue
*/
void clean_queue();

// DELETE

/**
 * Returns the current time in microseconds.
 */
long get_microtime() {
	struct timeval currentTime;

	gettimeofday(&currentTime, NULL);
	return currentTime.tv_sec * (int)1e6 + currentTime.tv_usec;
}

/**
* Prints out memory tape with pointer highlighted
*/
void print_tape(tape_t* head, int pointer) {

    tape_t* tmp = head;

    while(tmp) {
        if (tmp->position == pointer)
            printf("|%s| ", &tmp->character);
        else
            printf("%s ", &tmp->character);
        tmp = tmp->next;
    }
    printf("\n");
}

/**
*Prints a transition
*/
void print_transition(transition_t* tr) {

    printf("%d %c %c %c %d", tr->start_state, tr->read_char, tr->write_char, tr->head_movement, tr->end_state);
    printf(" || ");
}

/**
* Prints out TM transitions graph
*/
void print_TM() {

    transition_t* tmp = tm_head;
    transition_t* next = NULL;

    while(tmp) {
        printf("\n");
        next = tmp;

        while(next) {
            print_transition(next);
            next = next->next_state;
        }

        tmp = tmp->right_state;
        printf("\n");
    }

    printf("-------------\n");
}

/**
* Prints out queue
*/
void print_queue() {

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

    file = fopen("Test/IncreasingStuff.txt", "r");

    int max_transitions = 0;
    char *ptr;

    int move_count = 0;                     // DELETE
    int last_state_queued = 0;              // DELETE

    int tape_pointer = 0;
    tape_t* pointed_element = NULL;
    tape_t* copy_tape_head = NULL;

    queue_t* queue_elem = NULL;
    transition_t* move = NULL;
    transition_t* pre = NULL;

    char buffer[BUFFER_SIZE];
    char read_buffer[READING_BUFFER_SIZE];

    char ret = '0';

    /** reading "tr" */
    if (fgets(buffer, BUFFER_SIZE, file))
        clean_eol(buffer);

    if (strcmp(buffer, "tr") == 0)
        set_transition();

    /** here, already read the string "max" */
    if (fgets(buffer, BUFFER_SIZE, file))
        clean_eol(buffer);

    max_transitions = strtol(buffer, &ptr, 10);

    /** reading "run" */
    if (fgets(buffer, BUFFER_SIZE, file))
        clean_eol(buffer);

    if (strcmp(buffer, "run") == 0) {

        while (feof(file) == 0) {          // until end of file is reached

            if (fgets(read_buffer, READING_BUFFER_SIZE, file))      // read first string
                clean_eol(read_buffer);

            copy_input_to_tape(read_buffer);         // copy the string input into tape memory
            tape_pointer = 0;
            enqueue(0, tape_head, tape_pointer, 0);        // enqueue initial condition

            printf("\n");                   // DELETE
            print_queue();                  // DELETE
            print_tape(tape_head, tape_pointer);                   // DELETE

            // perform a BFS
            while (front) {             // until there's at least one element in the queue

                queue_elem = dequeue();         // pull from queue

                printf("\nDequeue\n");             // DELETE
                move_count = queue_elem->move_count;        // DELETE

                if (queue_elem->move_count == max_transitions) {     // if reached maximum number of moves, return U
                    ret = 'U';
                    break;
                }
                else {
                    move = find_array_element(&pre, queue_elem->current_state);      // if not, get the first transition with the current state taken from the queue

                    while (move) {                                      // iterate over all moves

                        pointed_element = find_tape_element(queue_elem->copy_tape_head, queue_elem->copy_tape_pointer);

                        printf("read char of move: %s\n", &move->read_char);                // DELETE
                        printf("character of tape: %s\n", &pointed_element->character);     // DELETE
                        print_tape(queue_elem->copy_tape_head, queue_elem->copy_tape_pointer);      // DELETE
                        printf("current state: %d\n", queue_elem->current_state);             // DELETE

                        if (strcmp(&move->read_char, "_") == 0 || strcmp(&move->read_char, &pointed_element->character) == 0) {     // if it's a possible move

                            if (find_acceptance(move->end_state) == 1) {
                                ret = '1';
                                break;
                            }

                            copy_tape_head = copy_tape(queue_elem->copy_tape_head);       // duplicates actual memory tape and pointer
                            pointed_element = find_tape_element(copy_tape_head, queue_elem->copy_tape_pointer);
                            pointed_element->character = move->write_char;                  // set the character in the memory tape

                            tape_pointer = move_pointer(queue_elem->copy_tape_pointer, move->head_movement, &copy_tape_head);               // move the pointer

                            printf("next state: %d\n", move->end_state);        // DELETE
                            print_tape(copy_tape_head, tape_pointer);           // DELETE

                            enqueue(move->end_state, copy_tape_head, tape_pointer, queue_elem->move_count+1);
                            printf("Enqueueing\n\n");                       // DELETE

                            last_state_queued = move->end_state;            // DELETE
                        }

                        move = move->next_state;
                    }

                    if (strcmp(&ret, "1") == 0)
                        break;

                    free(queue_elem);
                }
            }

            printf("%c\n", ret);
            fflush(stdout);

            ret = '0';
            clean_queue();
            empty_tape();
            tape_pointer = 0;
        }
    }

    return 0;
}

/**
* Replaces eol with null in the passed string
*/
void clean_eol(char *str) {
    char *c = strchr(str, '\n');                // removes the EOL appended by fgets, if there's any
    if (c)
        *c = '\0';
}

/**
* Removes all spaces from a given string
*/
void remove_spaces(char *str) {

    int count = 0, i = 0;

    for (i = 0; str[i]; i++)
        if (str[i] != ' ')
            str[count++] = str[i];          // counts all non-space chars, puts them near each other

    str[count] = '\0';                      // appends null after all the non-space chars
}

/**
* Inserts '$' separator between element of passed transition
*/
void insert_separator(char *str) {

    int pointer = 0, count = 0;
    char tmp[TRANSITION_SIZE] = "\0";

    strncpy(tmp, str, TRANSITION_SIZE);

    while (tmp[pointer] <= 57 && tmp[pointer] >= 48)      // if char is a number
         str[count++] = tmp[pointer++];

    str[count++] = SEPARATOR;

    while (tmp[pointer] < 48 || tmp[pointer] > 57) {        // add separator
        str[count++] = tmp[pointer++];
        str[count++] = SEPARATOR;
    }

    while (tmp[pointer] <= 57 && tmp[pointer] >= 48)      // if char is a number
         str[count++] = tmp[pointer++];

     str[count++] = SEPARATOR;
}

/**
* Finds the list corresponding to transitions with starting state number = position, starting from 0
* Returns pointer to node searched, null if none
* Pre is modified to point to previous element in the collection of list
*/
transition_t* find_array_element(transition_t* *pre, int position) {

    transition_t* ret = tm_head;           // return is pointed to head at first

    while (ret && ret->start_state != position) {
        *pre = ret;                      // pre is set to previous value
        ret = ret->right_state;         // ret is set to the next state
    }

    return ret;
}

/**
* Inserts a new transition into the TM graph
*/
void add_next_transition(transition_t* next) {

    transition_t* pre = NULL;
    transition_t* found_state = find_array_element(&pre, next->start_state);       // find state position based on start_state

    if (!found_state && !pre)       // if no elements, next is the head
        tm_head = next;
    else if (found_state && !pre) {    // if first position, set him has next state
        next->next_state = found_state;
        next->right_state = found_state->right_state;
        found_state->right_state = NULL;
        tm_head = next;
    }
    else if (!found_state && pre) {      // if no state, but pre exists, next is set as right element of pre
        pre->right_state = next;
    }
    else {                                   // if a state is found, is set as next state of found state
        pre->right_state = next;
        next->next_state = found_state;
        next->right_state = found_state->right_state;
        found_state->right_state = NULL;
    }
}

/**
* Sets down all transitions
*/
void set_transition() {

    char tr[TRANSITION_SIZE] = "\0";
    char *ptr;
    char *token[5];
    transition_t* next;
    int i = 0;

    if (fgets(tr, TRANSITION_SIZE, file))
        clean_eol(tr);

    while (strcmp(tr, "acc") != 0) {        // until acc is found, read a line
        next = (transition_t*) malloc(sizeof(transition_t));    // allocate a new transition

        remove_spaces(tr);
        insert_separator(tr);

        token[i] = strtok(tr, &SEPARATOR);       // locate first token
        while (token[i])                            // locate all other token
            token[++i] = strtok(NULL, &SEPARATOR);     // call with NULL implies the usage of the previous input
        i = 0;

        next->start_state = strtol(token[0], &ptr, 10);
        next->read_char = *token[1];
        next->write_char = *token[2];
        next->head_movement = *token[3];
        next->end_state = strtol(token[4], &ptr, 10);

        next->next_state = NULL;
        next->right_state = NULL;

        add_next_transition(next);     // set transition in the list

        if (fgets(tr, TRANSITION_SIZE, file))
            clean_eol(tr);
    }

    // here, "acc" is read
    set_acceptance_states();
}

/**
* Sets the acceptance states list
*/
void set_acceptance_states() {

    char acc[BUFFER_SIZE] = "\0";
    char *ptr;
    state_t* next;

    if (fgets(acc, BUFFER_SIZE, file))
        clean_eol(acc);

    while (strcmp(acc, "max") != 0) {        // until max is found, read a line
        next = (state_t*) malloc(sizeof(state_t));
        next->state_number = strtol(acc, &ptr, 10);

        next->next = acceptance_head;       // insertion in the head
        acceptance_head = next;

        if (fgets(acc, BUFFER_SIZE, file))
            clean_eol(acc);
    }
}

/**
* Finds if the passed state is in the acceptance list
* Returns 1 if it is, 0 else
*/
int find_acceptance(int current_state) {

    state_t* find = acceptance_head;

    while (find) {
        if (find->state_number == current_state)
            return 1;
        find = find->next;
    }

    return 0;
}

/**
* Writes down the input in the memory tape list
*/
void copy_input_to_tape(char buffer[]) {

    int i = 0;
    tape_t* prev = NULL;
    tape_t* tmp = NULL;

    while (buffer[i] != '\0') {
        printf("%c", buffer[i]);        // DELETE
        fflush(stdout);                 // DELETE

        tmp = (tape_t*) malloc(sizeof(tape_t));
        tmp->character = buffer[i];
        tmp->position = i;

        tmp->next = NULL;
        tmp->prev = prev;

        if (!tape_head)
            tape_head = tmp;
        else
            prev->next = tmp;

        prev = tmp;
        i++;
    }
}

/**
* Finds element which has passed position in the memory tape
* Returns NULL if no element is found
*/
tape_t* find_tape_element(tape_t* head, int position) {

    tape_t* ret = head;           // return is pointed to head at first

    while (ret) {
        if (ret->position == position)
            return ret;
        ret = ret->next;
    }

    return ret;
}

/**
* Moves tape pointer, returns new value of it
*/
int move_pointer(int tape_pointer, char head_movement, tape_t* *head) {

    char upper_case = toupper(head_movement);
    tape_t* tmp = NULL;
    tape_t* found = find_tape_element(*head, tape_pointer);

    if (strcmp(&upper_case, "R") == 0) {
        if (!found->next) {
            tmp = (tape_t*) malloc(sizeof(tape_t));
            tmp->character = '\0';
            tmp->position = tape_pointer+1;

            tmp->next = NULL;
            tmp->prev = found;
            found->next = tmp;
        }
        tape_pointer++;
    }
    else if (strcmp(&upper_case, "L") == 0) {
        if (!found->prev) {
            tmp = (tape_t*) malloc(sizeof(tape_t));
            tmp->character = '\0';
            tmp->position = tape_pointer-1;

            tmp->prev = NULL;
            tmp->next = found;
            found->prev = tmp;
            *head = tmp;
        }
        tape_pointer--;
    }

    return tape_pointer;
}

/**
* Empties memory tape list, sets tape_head = NULL
*/
void empty_tape() {

    tape_t* current = tape_head;
    tape_t* next = NULL;

    while (current) {
        next = current->next;
        free(current);
        current = next;
    }

    tape_head = NULL;
}

/**
* Duplicates memory tape, returns new head
*/
tape_t* copy_tape(tape_t* original_head) {

    tape_t* current = original_head;
    tape_t* tmp = NULL;
    tape_t* prev = NULL;
    tape_t* head_copy = NULL;

    while (current) {
        tmp = (tape_t*) malloc(sizeof(tape_t));
        tmp->character = current->character;
        tmp->position = current->position;

        tmp->next = NULL;
        tmp->prev = prev;

        if (!head_copy)
            head_copy = tmp;
        else
            prev->next = tmp;

        prev = tmp;
        current = current->next;
    }

    return head_copy;
}

/**
* Adds a new node to the queue
*/
void enqueue(int current_state, tape_t* copy_tape_head, int copy_tape_pointer, int move_count) {

    queue_t* tmp = (queue_t*) malloc(sizeof(queue_t));
	tmp->current_state = current_state;
	tmp->copy_tape_head = copy_tape_head;
	tmp->copy_tape_pointer = copy_tape_pointer;
	tmp->move_count = move_count;

	tmp->next = NULL;

	if(!front && !rear)
		front = rear = tmp;
    else {
        rear->next = tmp;
        rear = tmp;
    }
}

/**
* Returns front element of the queue
*/
queue_t* dequeue() {

	queue_t* tmp = front;

	if (front == rear)
		front = rear = NULL;
	else
		front = front->next;

	return(tmp);
}

/**
* Cleans the queue
*/
void clean_queue() {

    while (front)
        free(dequeue());
}


