#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

/* Typedefs */

typedef struct cpu_s cpu_t;

/* Public function declarations */

cpu_t* cpu_allocate(void);
void cpu_load(cpu_t* p_cpu, uint8_t* program, uint16_t size, uint16_t start);

void cpu_run(cpu_t* p_cpu);
void cpu_tick(cpu_t* p_cpu);

int cpu_halted(cpu_t* p_cpu);

int cpu_graphics_changed(cpu_t* p_cpu);
uint8_t* cpu_graphics(cpu_t* p_cpu);

void cpu_press_key(cpu_t* p_cpu, uint8_t key);
void cpu_release_key(cpu_t* p_cpu, uint8_t key);

#endif /* CPU_H_ */
