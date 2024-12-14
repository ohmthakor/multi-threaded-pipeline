# multi-threaded-pipeline

## Overview

This project implements a multi-threaded producer-consumer pipeline in C, where threads process input data from stdin and produce formatted output to stdout. It demonstrates the use of threads, mutual exclusion, condition variables, and synchronization mechanisms to handle shared resources effectively.

This program was developed as part of the CS 374 course at OSU.

## Features

### Threaded Processing Pipeline:
- Input Thread: Reads lines of characters from standard input.
- Line Separator Thread: Replaces newline characters with spaces.
- Plus Sign Thread: Replaces every "++" with a caret (^).
- Output Thread: Writes lines of exactly 80 characters to standard output.
  
### Producer-Consumer Model:
Each pair of threads communicates via shared buffers with condition variables.

### Dynamic Line Processing:
- Processes input lines as they are received. 
- Outputs formatted lines immediately when sufficient characters are available.

### Efficient Synchronization: 
No use of sleep() for synchronization; relies entirely on condition variables.

### Compatibility: 
Designed to run on Unix systems with the GCC compiler.


The program processes input from stdin and writes formatted output to stdout. It supports both keyboard and file redirection for input and output.

Examples
Using Keyboard Input:
./line_processor
Type input lines followed by pressing Enter.
Stop processing by typing STOP followed by Enter.
Using File Input and Output Redirection:
./line_processor < input.txt > output.txt
Combined Input and Output Redirection:
./line_processor < input.txt > output.txt
Input Details

Input lines consist of printable ASCII characters (space to tilde).
Newline characters are replaced with spaces.
"++" is replaced with ^.
The program stops processing when a line contains only STOP followed by a newline.
Input lines can be up to 1000 characters, and the program handles up to 49 lines before the stop-processing line.
Output Details

Lines are written to stdout with exactly 80 characters, followed by a newline.
Any remaining characters (less than 80) after the stop-processing line are discarded.
Input Example

```bash
Hello++
World!
This is a test++
STOP
```
Output Example
```bash
Hello^ World! This is a test^
```
Implementation Details

Pipeline Threads:
Input Thread:
Reads input from stdin.
Passes lines to Buffer 1.
Line Separator Thread:
Consumes lines from Buffer 1.
Replaces newline characters with spaces.
Produces lines for Buffer 2.
Plus Sign Thread:
Consumes lines from Buffer 2.
Replaces "++" with ^.
Produces lines for Buffer 3.
Output Thread:
Consumes lines from Buffer 3.
Outputs lines of exactly 80 characters to stdout.
Synchronization:
Each pair of threads communicates through a shared buffer.
Buffers use condition variables to handle full/empty states.
Unbounded Buffers:
Each buffer can store up to 50 lines of 1000 characters.
Technologies Used

C Programming Language: For thread creation and data processing.
POSIX Threads (pthread): To implement multi-threading.
Condition Variables: For thread synchronization.
Mutual Exclusion: Using mutexes to protect shared resources.
Unix System Calls: For I/O operations and thread management.
Learning Outcomes

This project demonstrates:

Practical implementation of the producer-consumer model.
Synchronization of threads using condition variables and mutexes.
Handling concurrent access to shared resources in a multi-threaded environment.
Efficient processing of input-output data streams.
