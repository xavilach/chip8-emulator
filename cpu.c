#include "cpu.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Defines */

/* Typedefs */

typedef void (*opcode_handler_t)(struct cpu_s* p_cpu);

/* Private function declarations */

static void unhandled_opcode_handler(struct cpu_s* p_cpu);

static void opcode00_handler(struct cpu_s* p_cpu);
static void opcode01_handler(struct cpu_s* p_cpu);
static void opcode02_handler(struct cpu_s* p_cpu);
static void opcode03_handler(struct cpu_s* p_cpu);
static void opcode04_handler(struct cpu_s* p_cpu);
static void opcode05_handler(struct cpu_s* p_cpu);
static void opcode06_handler(struct cpu_s* p_cpu);
static void opcode07_handler(struct cpu_s* p_cpu);
static void opcode08_handler(struct cpu_s* p_cpu);
static void opcode09_handler(struct cpu_s* p_cpu);
static void opcode10_handler(struct cpu_s* p_cpu);
static void opcode11_handler(struct cpu_s* p_cpu);
static void opcode12_handler(struct cpu_s* p_cpu);
static void opcode13_handler(struct cpu_s* p_cpu);
static void opcode14_handler(struct cpu_s* p_cpu);
static void opcode15_handler(struct cpu_s* p_cpu);

static const opcode_handler_t opcode_handlers[16] = {
    opcode00_handler,
    opcode01_handler,
    opcode02_handler,
    opcode03_handler,
    opcode04_handler,
    opcode05_handler,
    opcode06_handler,
    opcode07_handler,
    opcode08_handler,
    opcode09_handler,
    opcode10_handler,
    opcode11_handler,
    opcode12_handler,
    opcode13_handler,
    opcode14_handler,
    opcode15_handler
};

/* Public function definitions */

struct cpu_s* cpu_allocate(void)
{
    struct cpu_s* p_cpu = calloc(1, sizeof(struct cpu_s));
    
    if(p_cpu)
    {
        p_cpu->memory = calloc(4096, sizeof(uint8_t));
        p_cpu->graphics = p_cpu->memory + 0xF00;
        p_cpu->sp = p_cpu->memory + 0xFA0;
        p_cpu->pc = p_cpu->memory + 0x200;
    }
    
    return p_cpu;
}

void cpu_load(struct cpu_s* p_cpu, uint8_t* program, uint16_t size, uint16_t start)
{
    (void) memcpy(p_cpu->memory + start, program, size);
    
    p_cpu->pc = p_cpu->memory + start;
}

void cpu_run(struct cpu_s* p_cpu)
{
    opcode_handlers[(*p_cpu->pc >> 4) & 0x0F](p_cpu);
    
    /* Timers 60Hz */
    if(p_cpu->timer_delay)
        p_cpu->timer_delay--;
        
    if(p_cpu->timer_sound)
        p_cpu->timer_sound--;
}

/* Private function definitions */

static void unhandled_opcode_handler(struct cpu_s* p_cpu)
{
     p_cpu->pc += 2;
}

/* 0NNN	Call		Calls RCA 1802 program at address NNN. Not necessary for most ROMs. */
/* 00E0	Display	disp_clear()	Clears the screen. */
/* 00EE	Flow	return;	Returns from a subroutine. */
static void opcode00_handler(struct cpu_s* p_cpu)
{
    unhandled_opcode_handler(p_cpu);
}

/* 1NNN	Flow	goto NNN;	Jumps to address NNN. */
static void opcode01_handler(struct cpu_s* p_cpu)
{
    /* Set program counter to NNN. */
    uint16_t addr = (((uint16_t) *p_cpu->pc) << 8);
    addr |= (uint16_t) *(p_cpu->pc + 1);
    addr &= (uint16_t) 0x0FFF;
    
    p_cpu->pc = p_cpu->memory + addr;
}

/* 2NNN	Flow	*(0xNNN)()	Calls subroutine at NNN. */
static void opcode02_handler(struct cpu_s* p_cpu)
{
    uint16_t addr = (uint16_t) *p_cpu->pc;
    addr <<= 8;
    addr |= (uint16_t) *(p_cpu->pc + 1);
    addr &= (uint16_t) 0x0FFF;
    
    /* Store current address pointer on stack. */
    *p_cpu->sp = (uint16_t) (p_cpu->pc - p_cpu->memory);
    p_cpu->sp += sizeof(uint16_t);
    
    /* Set program counter to NNN. */
    p_cpu->pc = p_cpu->memory + addr;
}

/* 3XNN	Cond	if(Vx==NN)	Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block) */
static void opcode03_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t value = *(p_cpu->pc + 1);
    
    /* If VX == NN, skip next instruction. */
    if(value == p_cpu->reg_v[x])
        p_cpu->pc += 2;
    p_cpu->pc += 2;
}

/* 4XNN	Cond	if(Vx!=NN)	Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block) */
static void opcode04_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t value = *(p_cpu->pc + 1);
    
    /* If VX != NN, skip next instruction. */
    if(value != p_cpu->reg_v[x])
        p_cpu->pc += 2;
    p_cpu->pc += 2;
}

/* 5XY0	Cond	if(Vx!=Vy)	Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block) */
static void opcode05_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t y = (*(p_cpu->pc + 1) >> 4) & 0x0F;

    /* If VX == VY, skip next instruction. */
    if(p_cpu->reg_v[y] == p_cpu->reg_v[x])
        p_cpu->pc += 2;
    p_cpu->pc += 2;
}

/* 6XNN	Const	Vx = NN	Sets VX to NN. */
static void opcode06_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t value = *(p_cpu->pc + 1);
    
    p_cpu->reg_v[x] = value;
    p_cpu->pc += 2;
}

/* 7XNN	Const	Vx += NN	Adds NN to VX. (Carry flag is not changed) */
static void opcode07_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t value = *(p_cpu->pc + 1);
    
    p_cpu->reg_v[x] += value;
    p_cpu->pc += 2;
}

/* 8XY0	Assign	Vx=Vy	Sets VX to the value of VY. */
/* 8XY1	BitOp	Vx=Vx|Vy	Sets VX to VX or VY. (Bitwise OR operation) */
/* 8XY2	BitOp	Vx=Vx&Vy	Sets VX to VX and VY. (Bitwise AND operation) */
/* 8XY3	BitOp	Vx=Vx^Vy	Sets VX to VX xor VY. */
/* 8XY4	Math	Vx += Vy	Adds VY to VX. VF is set to 1 when there's a carry, and to 0 when there isn't. */
/* 8XY5	Math	Vx -= Vy	VY is subtracted from VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
/* 8XY6	BitOp	Vx>>=1	Stores the least significant bit of VX in VF and then shifts VX to the right by 1.[2] */
/* 8XY7	Math	Vx=Vy-Vx	Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there isn't. */
/* 8XYE	BitOp	Vx<<=1	Stores the most significant bit of VX in VF and then shifts VX to the left by 1.[3] */
static void opcode08_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t y = (*(p_cpu->pc + 1) >> 4) & 0x0F;
    uint8_t op = *(p_cpu->pc + 1) & 0x0F;
    
    switch(op)
    {
        case (uint8_t) 0x00:
            p_cpu->reg_v[x] = p_cpu->reg_v[y];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x01:
            p_cpu->reg_v[x] |= p_cpu->reg_v[y];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x02:
            p_cpu->reg_v[x] &= p_cpu->reg_v[y];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x03:
            p_cpu->reg_v[x] ^= p_cpu->reg_v[y];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x04:
            {
                uint16_t res = (uint16_t) p_cpu->reg_v[x] + (uint16_t) p_cpu->reg_v[y];
                if(res & 0xFF00)
                    p_cpu->reg_v[15] = 1;
                else
                    p_cpu->reg_v[15] = 0; 
                p_cpu->reg_v[x] += p_cpu->reg_v[y];
                p_cpu->pc += 2;
            }
            break;
        case (uint8_t) 0x05:
            {
                if(p_cpu->reg_v[y] > p_cpu->reg_v[x])
                    p_cpu->reg_v[15] = 0;
                else
                    p_cpu->reg_v[15] = 1;
                p_cpu->reg_v[x] -= p_cpu->reg_v[y];
                p_cpu->pc += 2;
            }
            break;
        case (uint8_t) 0x06:
            p_cpu->reg_v[15] = p_cpu->reg_v[x] & (uint8_t) 0x01;
            p_cpu->reg_v[x] >>= 1;
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x07:
            {
                if(p_cpu->reg_v[x] > p_cpu->reg_v[y])
                    p_cpu->reg_v[15] = 0;
                else
                    p_cpu->reg_v[15] = 1;
                p_cpu->reg_v[x] = p_cpu->reg_v[y] - p_cpu->reg_v[x];
                p_cpu->pc += 2;
            }
        case (uint8_t) 0x0E:
            p_cpu->reg_v[15] = (p_cpu->reg_v[x] >> 7) & (uint8_t) 0x01;
            p_cpu->reg_v[x] <<= 1;
            p_cpu->pc += 2;
            break;
        default:
        unhandled_opcode_handler(p_cpu);
            break;
    }
}

/* 9XY0	Cond	if(Vx==Vy)	Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block) */
static void opcode09_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t y = (*(p_cpu->pc + 1) >> 4) & 0x0F;

    /* If VX != VY, skip next instruction. */
    if(p_cpu->reg_v[y] != p_cpu->reg_v[x])
        p_cpu->pc += 2;
    p_cpu->pc += 2;
}

/* ANNN	MEM	I = NNN	Sets I to the address NNN. */
static void opcode10_handler(struct cpu_s* p_cpu)
{
    uint16_t addr = (((uint16_t) *p_cpu->pc) << 8);
    addr |= (uint16_t) *(p_cpu->pc + 1);
    addr &= (uint16_t) 0x0FFF;
    
    p_cpu->i = p_cpu->memory + addr;
    p_cpu->pc += 2;
}

/* BNNN	Flow	PC=V0+NNN	Jumps to the address NNN plus V0. */
static void opcode11_handler(struct cpu_s* p_cpu)
{
    uint16_t addr = (((uint16_t) *p_cpu->pc) << 8);
    addr |= (uint16_t) *(p_cpu->pc + 1);
    addr &= (uint16_t) 0x0FFF;

    p_cpu->pc = p_cpu->memory + p_cpu->reg_v[0] + addr;
}

/* CXNN	Rand	Vx=rand()&NN	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. */
static void opcode12_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t value = *(p_cpu->pc + 1);
    
    p_cpu->reg_v[x] = ((uint8_t) (rand() % 256)) & value;

    p_cpu->pc += 2;
}

/* DXYN	Disp	draw(Vx,Vy,N)	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
    Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. 
    As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen. */
static void opcode13_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    uint8_t y = (*(p_cpu->pc + 1) >> 4) & 0x0F;
    uint8_t n = *(p_cpu->pc + 1) & 0x0F;
    int line, column;
    
    p_cpu->reg_v[15] = 0;

    for(line = 0; line < (int) n; line ++)
    {
        uint8_t pixel = *(p_cpu->i + line);
        
        for(column = 0; column < 8; column ++)
        {
            if(pixel & (0x80 >> column))
            {
                p_cpu->graphics[x + column + ((y + line) * 64)] &= 0x01;
                if(p_cpu->graphics[x + column + ((y + line) * 64)])
                    p_cpu->reg_v[15] = 1;
                p_cpu->graphics[x + column + ((y + line) * 64)] ^= 0x01;
            }
        }
    }
    
    p_cpu->pc += 2;
}

/*
EX9E	KeyOp	if(key()==Vx)	Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
EXA1	KeyOp	if(key()!=Vx)	Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)*/
static void opcode14_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    
    (void) x;
    
    switch(*(p_cpu->pc + 1))
    {
        case (uint8_t) 0x9E:
        case (uint8_t) 0xA1:
        default:
        unhandled_opcode_handler(p_cpu);
            break;
    }
}

/*
FX07	Timer	Vx = get_delay()	Sets VX to the value of the delay timer.
FX0A	KeyOp	Vx = get_key()	A key press is awaited, and then stored in VX. (Blocking Operation. All instruction halted until next key event)
FX15	Timer	delay_timer(Vx)	Sets the delay timer to VX.
FX18	Sound	sound_timer(Vx)	Sets the sound timer to VX.
FX1E	MEM	I +=Vx	Adds VX to I.[4]
FX29	MEM	I=sprite_addr[Vx]	Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
FX33	BCD	set_BCD(Vx);
*(I+0)=BCD(3);

*(I+1)=BCD(2);

*(I+2)=BCD(1);

Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
FX55	MEM	reg_dump(Vx,&I)	Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
FX65	MEM	reg_load(Vx,&I)	Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
*/
static void opcode15_handler(struct cpu_s* p_cpu)
{
    uint8_t x = *p_cpu->pc & 0x0F;
    
    switch(*(p_cpu->pc + 1))
    {
        case (uint8_t) 0x07:
            p_cpu->reg_v[x] = p_cpu->timer_delay;
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x15:
            p_cpu->timer_delay = p_cpu->reg_v[x];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x18:
            p_cpu->timer_sound = p_cpu->reg_v[x];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x1E:
            p_cpu->i += p_cpu->reg_v[x];
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x55:
            (void) memcpy(p_cpu->i, p_cpu->reg_v, (x + 1) * sizeof(uint8_t));
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x65:
            (void) memcpy(p_cpu->reg_v, p_cpu->i, (x + 1) * sizeof(uint8_t));
            p_cpu->pc += 2;
            break;
        case (uint8_t) 0x0A:
        case (uint8_t) 0x29:
        case (uint8_t) 0x33:
        default:
        unhandled_opcode_handler(p_cpu);
            break;
    }
}
