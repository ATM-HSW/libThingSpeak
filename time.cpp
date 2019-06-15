#include "time.h"

static unsigned long one_millisecond_counter = 0;
static void one_millisecond_callback(void) {
    one_millisecond_counter++;
}

unsigned long millis(void) {
    static Ticker milli_timer;
    static bool attach_flag = true;
    if (attach_flag) {
        attach_flag = false;
        milli_timer.attach_us(&one_millisecond_callback, 1000);
    }
    return one_millisecond_counter;
}

void delay(int t)
{
    wait_ms(t);
}