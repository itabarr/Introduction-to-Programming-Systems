#ifndef BASIC_COMMANDS_H
#define BASIC_COMMANDS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_COMMANDS 1024

// Sleep for x milliseconds
void msleep(int x);

// Increment a counter in a file with the specified name
void increment(int x);

// Decrement a counter in a file with the specified name
void decrement(int x);

// Run a string of commands in order
void run_job(void *commands_str);

#endif