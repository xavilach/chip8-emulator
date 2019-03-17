#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

/* Typedefs */

struct cpu_s
{
    uint8_t* memory;
    uint8_t* graphics;
    uint8_t* pc;
    uint8_t* sp;
    uint8_t* i;

    uint8_t reg_v[16];

    uint8_t timer_delay;
    uint8_t timer_sound;
};

/* Private function declarations */

struct cpu_s* cpu_allocate(void);
void cpu_load(struct cpu_s* p_cpu, uint8_t* program, uint16_t size, uint16_t start);
void cpu_run(struct cpu_s* p_cpu);

#endif /* CPU_H_ */
