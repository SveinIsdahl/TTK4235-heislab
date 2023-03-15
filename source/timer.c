#include "timer.h"
time_t startTime; // global variable to store the start time of the timer

// function to check how long is left of the timer
int timer_check() {
    time_t now = time(NULL);
    int elapsed = now - startTime;
    int remaining = 3 - elapsed; // assuming the timer is 60 seconds long
    return remaining;
}

// function to create a new timer, resetting the timer if called again
void timer_set() {
    startTime = time(NULL);
}