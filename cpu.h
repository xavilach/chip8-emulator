#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

/* Typedefs */

typedef struct cpu_s cpu_t;

/* Public function declarations */

/**
 * @brief Allocate cpu.
 * 
 * @return Pointer to allocated cpu, or NULL if allocation failed.
 */
cpu_t* cpu_allocate(void);

/**
 * @brief Load program on cpu.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 * @param[in]	program	Program to load.
 * @param[in]	size	Program size.
 */ 
void cpu_load(cpu_t* p_cpu, uint8_t* program, uint16_t size);

/**
 * @brief Run a single cpu cycle.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 */
void cpu_run(cpu_t* p_cpu);

/**
 * @brief Decrement cpu running timers by one tick.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 */
void cpu_tick(cpu_t* p_cpu);

/**
 * @brief Check whether the cpu is halted, waiting for a key press.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 * 
 * @return 1 if cpu halted, else 0.
 */
int cpu_halted(cpu_t* p_cpu);

/**
 * @brief Check whether the cpu modified the graphics.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 * 
 * @return 1 if graphics changed, else 0.
 */
int cpu_graphics_changed(cpu_t* p_cpu);

/**
 * @brief Get pointer to cpu graphics.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 * 
 * @return Pointer to cpu graphics.
 */
uint8_t* cpu_graphics(cpu_t* p_cpu);

/**
 * @brief Press the given key.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 * @param[in]	key		Key to press.
 */
void cpu_press_key(cpu_t* p_cpu, uint8_t key);

/**
 * @brief Release the given key.
 * 
 * @param[in]	p_cpu	Pointer to cpu.
 * @param[in]	key		Key to release.
 */
void cpu_release_key(cpu_t* p_cpu, uint8_t key);

#endif /* CPU_H_ */
