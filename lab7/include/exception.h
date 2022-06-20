#ifndef EXCEPTION_H
#define EXCEPTION_H

#define INTR_STK_SIZE 4096
#define INTR_STK_TOP_IDX (INTR_STK_SIZE - 16) // sp need 16bytes alignment

struct trapframe {
    uint64_t x[31]; // general register from x0 ~ x30
    uint64_t sp_el0;
    uint64_t elr_el1;
    uint64_t spsr_el1;
};

void lock();
void unlock();
int get_lock();

#endif
