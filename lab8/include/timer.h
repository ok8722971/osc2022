#ifndef TIMER_H
#define TIMER_H


typedef struct {
    void *callback;
    char s[128];
    unsigned long long ticket;
	float start_time;
} timer_event;

void timer_helper(char *s, float startTime);
void remove_front_timer();
unsigned long long get_tick_plus_sec(unsigned int second);
void add_timer(void*, char*,unsigned int);
void set_core_timer_interrupt(unsigned int expired_time);
void set_core_timer_interrupt_by_tick(unsigned long long tick);

#endif
