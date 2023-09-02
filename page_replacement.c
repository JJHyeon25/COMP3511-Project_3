/*
    COMP3511 Fall 2022 
    PA3: Page-Replacement Algorithms

    Your name: Ju Jong Hyeon
    Your ITSC email: jjuab@connect.ust.hk 

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks. 

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Constants
#define UNFILLED_FRAME -1
#define MAX_QUEUE_SIZE 10
#define MAX_FRAMES_AVAILABLE 10 
#define MAX_REFERENCE_STRING 30
#define ALGORITHM_FIFO "FIFO"
#define ALGORITHM_OPT "OPT"
#define ALGORITHM_LRU "LRU"

// Keywords (to be used when parsing the input)
#define KEYWORD_ALGORITHM "algorithm"
#define KEYWORD_FRAMES_AVAILABLE "frames_available"
#define KEYWORD_REFERENCE_STRING_LENGTH "reference_string_length"
#define KEYWORD_REFERENCE_STRING "reference_string"

// Useful string template used in printf()
// We will use diff program to auto-grade the submissions
// Please use the following templates in printf to avoid formatting errors
//
// Example:
//
//   printf(template_total_page_fault, 0)    # Total Page Fault: 0 is printed on the screen
//   printf(template_no_page_fault, 0)       # 0: No Page Fault is printed on the screen

const char template_total_page_fault[] = "Total Page Fault: %d\n";
const char template_no_page_fault[] = "%d: No Page Fault\n";

// Assume that we only need to support 2 types of space characters: 
// " " (space), "\t" (tab)
#define SPACE_CHARS " \t"


// Global variables
char algorithm[10]; 
int reference_string[MAX_REFERENCE_STRING]; 
int reference_string_length; 
int frames_available;
int frames[MAX_FRAMES_AVAILABLE]; 


// Helper function: Check whether the line is a blank line (for input parsing)
int is_blank(char *line) {
    char *ch = line;
    while ( *ch != '\0' ) {
        if ( !isspace(*ch) )
            return 0;
        ch++;
    }
    return 1;
}
// Helper function: Check whether the input line should be skipped
int is_skip(char *line) {
    if ( is_blank(line) )
        return 1;
    char *ch = line ;
    while ( *ch != '\0' ) {
        if ( !isspace(*ch) && *ch == '#')
            return 1;
        ch++;
    }
    return 0;
}
// Helper: parse_tokens function
void parse_tokens(char **argv, char *line, int *numTokens, char *delimiter) {
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

// Helper: parse the input file
void parse_input() {
    FILE *fp = stdin;
    char *line = NULL;
    ssize_t nread;
    size_t len = 0;

    char *two_tokens[2]; // buffer for 2 tokens
    char *reference_string_tokens[MAX_REFERENCE_STRING]; // buffer for the reference string
    int numTokens = 0, n=0, i=0;
    char equal_plus_spaces_delimiters[5] = "";

    strcpy(equal_plus_spaces_delimiters, "=");
    strcat(equal_plus_spaces_delimiters,SPACE_CHARS);

    while ( (nread = getline(&line, &len, fp)) != -1 ) {
        if ( is_skip(line) == 0)  {
            line = strtok(line,"\n");
            
            if (strstr(line, KEYWORD_ALGORITHM)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    strcpy(algorithm, two_tokens[1]);
                }
            } 
            else if (strstr(line, KEYWORD_FRAMES_AVAILABLE)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &frames_available);
                }
            }
            else if (strstr(line, KEYWORD_REFERENCE_STRING_LENGTH)) {
                parse_tokens(two_tokens, line, &numTokens, equal_plus_spaces_delimiters);
                if (numTokens == 2) {
                    sscanf(two_tokens[1], "%d", &reference_string_length);
                }
            } 
            else if (strstr(line, KEYWORD_REFERENCE_STRING)) {

                parse_tokens(two_tokens, line, &numTokens, "=");
                // printf("Debug: %s\n", two_tokens[1]);
                if (numTokens == 2) {
                    parse_tokens(reference_string_tokens, two_tokens[1], &n, SPACE_CHARS );
                    for (i=0; i<n; i++) {
                        sscanf(reference_string_tokens[i], "%d", &reference_string[i]);
                    }
                }
            }
            


        }
    }
}
// Helper: Display the parsed values
void print_parsed_values() {
    int i;
    printf("%s = %s\n", KEYWORD_ALGORITHM, algorithm);
    printf("%s = %d\n", KEYWORD_FRAMES_AVAILABLE, frames_available);
    printf("%s = %d\n", KEYWORD_REFERENCE_STRING_LENGTH, reference_string_length);
    printf("%s = ", KEYWORD_REFERENCE_STRING);
    for (i=0; i<reference_string_length;i++)
        printf("%d ", reference_string[i]);
    printf("\n");

}

// A simple integer queue implementation using a fixed-size array
// Helper functions:
//   queue_init: initialize the queue
//   queue_is_empty: return true if the queue is empty, otherwise false
//   queue_is_full: return true if the queue is full, otherwise false
//   queue_peek: return the current front element of the queue
//   queue_enqueue: insert one item at the end of the queue
//   queue_dequeue: remove one item from the beginning of the queue
//   queue_print: display the queue content, it is useful for debugging
struct Queue {
    int values[MAX_QUEUE_SIZE];
    int front, rear, count;
};
void queue_init(struct Queue* q) {
    q->count = 0;
    q->front = 0;
    q->rear = -1;
}
int queue_is_empty(struct Queue* q) {
    return q->count == 0;
}
int queue_is_full(struct Queue* q) {
    return q->count == MAX_QUEUE_SIZE;
}

int queue_peek(struct Queue* q) {
    return q->values[q->front];
}
void queue_enqueue(struct Queue* q, int new_value) {
    if (!queue_is_full(q)) {
        if ( q->rear == MAX_QUEUE_SIZE -1)
            q->rear = -1;
        q->values[++q->rear] = new_value;
        q->count++;
    }
}
void queue_dequeue(struct Queue* q) {
    q->front++;
    if (q->front == MAX_QUEUE_SIZE)
        q->front = 0;
    q->count--;
}
void queue_print(struct Queue* q) {
    int c = q->count;
    printf("size = %d\n", c);
    int cur = q->front;
    printf("values = ");
    while ( c > 0 ) {
        if ( cur == MAX_QUEUE_SIZE )
            cur = 0;
        printf("%d ", q->values[cur]);
        cur++;
        c--;
    }
    printf("\n");
}

// Helper function:
// This function is useful for printing the fault frames in this format:
// current_frame: f0 f1 ...
//
// For example: the following 4 lines can use this helper function to print 
//
// 7: 7     
// 0: 7 0   
// 1: 7 0 1 
// 2: 2 0 1 
//
// For the non-fault frames, you should use template_no_page_fault (see above)
//
void display_fault_frame(int current_frame) {
    int j;
    printf("%d: ", current_frame);
    for (j=0; j<frames_available; j++) {
        if ( frames[j] != UNFILLED_FRAME )
            printf("%d ", frames[j]);
        else 
            printf("  ");
    }
    printf("\n");
}

int find_index(int start,int size, int ref_frame[],int value) {
	for (int i = start; i < size; ++i) {
		if (ref_frame[i] == value) {
			return i;
		}
	}
	return -1;
}

void algorithm_FIFO() {
   // TODO: Implement the FIFO algorithm here
	struct Queue Q;
	queue_init(&Q);
	// curr: the current value of the reference string
	int curr;
	int frames_left = frames_available;
	int total_page_fault = 0;
	// go through every single reference string
	for (int i = 0; i < reference_string_length; ++i) {
		curr = reference_string[i];
		int curr_index = find_index(0, frames_available - frames_left, frames, curr);
		// case where there is no page fault
		if (curr_index != -1) {
			printf(template_no_page_fault, curr);
		}
		// case where there is page fault, but the frames are not full
		else if (frames_left != 0) {
			queue_enqueue(&Q, curr);
			frames[frames_available - frames_left] = curr;
			--frames_left;
			++total_page_fault;
			display_fault_frame(curr);
		} 
		// case where there is page fault, but the frames are full
		else {
			int remove = queue_peek(&Q);
			int remove_index = find_index(0, frames_available, frames, remove);
			frames[remove_index] = curr;
			queue_dequeue(&Q);
			queue_enqueue(&Q, curr);
			++total_page_fault;
			display_fault_frame(curr);
		}
	}
	printf(template_total_page_fault, total_page_fault);
}

void algorithm_OPT() {
    // TODO: Implement the OPT algorithm here
	// curr: the current value of the reference string
	int curr;
	int frames_left = frames_available;
	int total_page_fault = 0;
	// go through every single reference string
	for (int i = 0; i < reference_string_length; ++i) {
		curr = reference_string[i];
		int curr_index = find_index(0, frames_available - frames_left, frames, curr);
		// case where there is no page fault
		if (curr_index != -1) {
			printf(template_no_page_fault, curr);
		}
		// case where there is page fault, but the frames are not full
		else if (frames_left != 0) {
			frames[frames_available - frames_left] = curr;
			--frames_left;
			++total_page_fault;
			display_fault_frame(curr);
		}
		// case where there is page fault, but the frames are full
		else {
			int index_list[frames_available];
			int far = -1;
			int value = frames[0];
			int index = 0;
			for (int j = 0; j < frames_available; ++j) {
				index_list[j] = find_index(i + 1, reference_string_length, reference_string, frames[j]);
				if (index_list[j] == -1) {
					index_list[j] = reference_string_length;
				}
			}
			for (int j = 0; j < frames_available; ++j) {
				if ((far == reference_string_length) && (index_list[j] == reference_string_length)) {
					if (value > frames[j]) {
						value = frames[j];
						index = j;
					}
				}
				else if ((far != reference_string_length) && (index_list[j] == reference_string_length)) {
					far = reference_string_length;
					value = frames[j];
					index = j;
				}
				else if (index_list[j] > far) {
					far = index_list[j];
					value = frames[j];
					index = j;
				}
			}
			frames[index] = curr;
			++total_page_fault;
			display_fault_frame(curr);
		}
	}
	printf(template_total_page_fault, total_page_fault);
}

void algorithm_LRU() {
    // TODO: Implement the LRU algorithm here
	// curr: the current value of the reference string
	int curr;
	int frames_left = frames_available;
	int total_page_fault = 0;
	int index_list[frames_available];
	// go through every single reference string
	for (int i = 0; i < reference_string_length; ++i) {
		curr = reference_string[i];
		int curr_index = find_index(0, frames_available - frames_left, frames, curr);
		// case where there is no page fault
		if (curr_index != -1) {
			printf(template_no_page_fault, curr);
			index_list[curr_index] = i; ////////////////////////////////////
		}
		// case where there is page fault, but the frames are not full
		else if (frames_left != 0) {
			frames[frames_available - frames_left] = curr;
			index_list[frames_available - frames_left] = i;/////////////////////////////////////////
			--frames_left;
			++total_page_fault;
			display_fault_frame(curr);
		}
		// case where there is page fault, but the frames are full
		else {
			int mindex = i;
			int index;
			for (int j = 0; j < frames_available; ++j) {
				if (index_list[j] < mindex) {
					mindex = index_list[j];
					index = j;
				}
			}
			frames[index] = curr;
			index_list[index] = i;
			++total_page_fault;
			display_fault_frame(curr);
		}
	}
	printf(template_total_page_fault, total_page_fault);
}

void initialize_frames() {
    int i;
    for (i=0; i<frames_available; i++)
        frames[i] = UNFILLED_FRAME;
}



int main() {
    parse_input();
    print_parsed_values();
    initialize_frames();
    if (strcmp(algorithm, ALGORITHM_FIFO) == 0) {
        algorithm_FIFO();
    } 
    else if (strcmp(algorithm, ALGORITHM_OPT) == 0) {
        algorithm_OPT();
    }
    else if (strcmp(algorithm, ALGORITHM_LRU) == 0) {
        algorithm_LRU();
    }    
    return 0;
}