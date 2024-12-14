#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h> // For isatty()

#define MAX_LINES 50            // Maximum number of lines that each buffer can hold
#define MAX_LINE_LENGTH 1000    // Maximum length of each line
#define OUTPUT_LINE_LENGTH 80   // Length of each line in the output

// Buffers for communication between threads
char buffer1[MAX_LINES][MAX_LINE_LENGTH]; // Shared buffer: Input -> Line Separator
char buffer2[MAX_LINES][MAX_LINE_LENGTH]; // Shared buffer: Line Separator -> Plus Sign
char buffer3[MAX_LINES][MAX_LINE_LENGTH]; // Shared buffer: Plus Sign -> Output

// Buffer read and write indices
int read1 = 0, write1 = 0; // Indices for buffer1
int read2 = 0, write2 = 0; // Indices for buffer2
int read3 = 0, write3 = 0; // Indices for buffer3

// Mutexes and condition variables for thread synchronization
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER; // Mutex for buffer1
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; // Mutex for buffer2
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER; // Mutex for buffer3
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;    // Condition variable for buffer1
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;    // Condition variable for buffer2
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;    // Condition variable for buffer3

// Flags to signal thread completion
bool input_done = false;          // Indicates Input Thread is done
bool line_separator_done = false; // Indicates Line Separator Thread is done
bool plus_sign_done = false;      // Indicates Plus Sign Thread is done

// Input Thread: Reads lines from standard input and writes them to buffer1
void *input_thread(void *arg) {
    char line[MAX_LINE_LENGTH];
    bool is_interactive = isatty(fileno(stdin)); // Check if input is from terminal

    if (is_interactive) {
        printf("Enter text (type 'STOP' on a new line to end input):\n");
    }

    while (fgets(line, MAX_LINE_LENGTH, stdin)) {
        // Stop processing if "STOP" is encountered
        if (strcmp(line, "STOP\n") == 0) {
            pthread_mutex_lock(&mutex1);
            input_done = true;                // Signal that input is complete
            pthread_cond_broadcast(&cond1);  // Notify Line Separator Thread
            pthread_mutex_unlock(&mutex1);
            break;
        }
        // Write line to buffer1
        pthread_mutex_lock(&mutex1);
        strcpy(buffer1[write1 % MAX_LINES], line);
        write1++;
        pthread_cond_signal(&cond1); // Notify Line Separator Thread
        pthread_mutex_unlock(&mutex1);
    }

    // Signal input completion if not already signaled
    pthread_mutex_lock(&mutex1);
    input_done = true;
    pthread_cond_broadcast(&cond1);
    pthread_mutex_unlock(&mutex1);

    return NULL;
}

// Line Separator Thread: Replaces newline characters with spaces and writes to buffer2
void *line_separator_thread(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex1);
        // Wait if buffer1 is empty and input is not done
        while (read1 == write1 && !input_done) {
            pthread_cond_wait(&cond1, &mutex1);
        }
        // Exit if input is done and buffer1 is empty
        if (input_done && read1 == write1) {
            pthread_mutex_unlock(&mutex1);
            pthread_mutex_lock(&mutex2);
            line_separator_done = true;      // Signal that line separator is done
            pthread_cond_broadcast(&cond2); // Notify Plus Sign Thread
            pthread_mutex_unlock(&mutex2);
            break;
        }
        // Read a line from buffer1
        char line[MAX_LINE_LENGTH];
        strcpy(line, buffer1[read1 % MAX_LINES]);
        read1++;
        pthread_mutex_unlock(&mutex1);

        // Replace newline with space
        size_t len = strlen(line);
        if (line[len - 1] == '\n') {
            line[len - 1] = ' ';
        }

        // Write processed line to buffer2
        pthread_mutex_lock(&mutex2);
        strcpy(buffer2[write2 % MAX_LINES], line);
        write2++;
        pthread_cond_signal(&cond2); // Notify Plus Sign Thread
        pthread_mutex_unlock(&mutex2);
    }
    return NULL;
}

// Plus Sign Thread: Replaces "++" with "^" and writes to buffer3
void *plus_sign_thread(void *arg) {
    while (true) {
        pthread_mutex_lock(&mutex2);
        // Wait if buffer2 is empty and line separator is not done
        while (read2 == write2 && !line_separator_done) {
            pthread_cond_wait(&cond2, &mutex2);
        }
        // Exit if line separator is done and buffer2 is empty
        if (line_separator_done && read2 == write2) {
            pthread_mutex_unlock(&mutex2);
            pthread_mutex_lock(&mutex3);
            plus_sign_done = true;           // Signal that plus sign processing is done
            pthread_cond_broadcast(&cond3); // Notify Output Thread
            pthread_mutex_unlock(&mutex3);
            break;
        }
        // Read a line from buffer2
        char line[MAX_LINE_LENGTH];
        strcpy(line, buffer2[read2 % MAX_LINES]);
        read2++;
        pthread_mutex_unlock(&mutex2);

        // Replace "++" with "^"
        char result[MAX_LINE_LENGTH] = "";
        int i = 0;
        while (line[i] != '\0') {
            if (line[i] == '+' && line[i + 1] == '+') {
                strcat(result, "^");
                i += 2; // Skip over both '+' signs
            } else {
                strncat(result, &line[i], 1);
                i++;
            }
        }

        // Write processed line to buffer3
        pthread_mutex_lock(&mutex3);
        strcpy(buffer3[write3 % MAX_LINES], result);
        write3++;
        pthread_cond_signal(&cond3); // Notify Output Thread
        pthread_mutex_unlock(&mutex3);
    }
    return NULL;
}

// Output Thread: Writes 80-character lines to standard output
void *output_thread(void *arg) {
    char output[OUTPUT_LINE_LENGTH + 1] = ""; // Buffer for 80-character output lines
    int output_index = 0;

    while (true) {
        pthread_mutex_lock(&mutex3);
        
        // Wait if buffer3 is empty and plus sign processing is not done
        while (read3 == write3 && !plus_sign_done) {
            pthread_cond_wait(&cond3, &mutex3);
        }
        
        // Exit if plus sign processing is done and buffer3 is empty
        if (plus_sign_done && read3 == write3) {
            pthread_mutex_unlock(&mutex3);
            break;
        }

        // Read a line from buffer3
        char line[MAX_LINE_LENGTH];
        strcpy(line, buffer3[read3 % MAX_LINES]);
        read3++;
        pthread_cond_signal(&cond3); // Signal other threads waiting on buffer3
        pthread_mutex_unlock(&mutex3);

        // Accumulate characters into the output buffer
        int i;
        for (i = 0; line[i] != '\0'; i++) {
            output[output_index++] = line[i];

            // When output buffer reaches 80 characters, print it
            if (output_index == OUTPUT_LINE_LENGTH) {
                output[OUTPUT_LINE_LENGTH] = '\0';
                printf("%s\n", output); // Print the complete 80-character line
                fflush(stdout);
                output_index = 0; // Reset buffer index
            }
        }
    }

    // Discard any remaining characters if they form an incomplete line
    if (output_index > 0) {
        // Only print if at least one character has been accumulated
        output[output_index] = '\0';
        printf("%s\n", output);
        fflush(stdout);
        output_index = 0; // Reset buffer index
    }

    return NULL;
}

// Creates and joins all threads
int main() {
    pthread_t t1, t2, t3, t4;

    // Create threads
    pthread_create(&t1, NULL, input_thread, NULL);
    pthread_create(&t2, NULL, line_separator_thread, NULL);
    pthread_create(&t3, NULL, plus_sign_thread, NULL);
    pthread_create(&t4, NULL, output_thread, NULL);

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);

    return 0;
}
