#include "cpu.h"

#include "log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Defines */

#define PRINT_INSTR(str_) DEBUG_PRINT("%04x:%02x%02x %s\n", mem_offset(p_cpu, p_cpu->pc), *(p_cpu->pc), *(p_cpu->pc + 1), str_);

#define FONT_CHAR_SIZE (5)
#define FONT_CHAR_COUNT (16)

#define GRAPHICS_COLS (64)
#define GRAPHICS_ROWS (32)
#define GRAPHICS_SIZE (GRAPHICS_COLS * GRAPHICS_ROWS)

#define KEY_COUNT (16)

#define REG_COUNT (16)

#define MEM_SIZE (0x1000)

#define FONT_ADDRESS (0x0000)
#define FONT_SIZE (FONT_CHAR_SIZE * FONT_CHAR_COUNT)

#define ROM_ADDRESS (0x0200)

#define STACK_ADDRESS (0x0FA0)

/* Typedefs */

struct cpu_s
{
	uint8_t memory[MEM_SIZE];
	uint8_t graphics[GRAPHICS_SIZE];

	uint8_t *font;
	uint8_t *pc;
	uint8_t *sp;
	uint8_t *i;

	uint8_t reg_v[REG_COUNT];

	uint8_t timer_delay;
	uint8_t timer_sound;

	uint8_t keys[KEY_COUNT];

	int draw_flag;
	int halted_flag;
};

typedef void (*opcode_handler_t)(cpu_t *p_cpu);

/* Private variables */

static const uint8_t fontset[FONT_SIZE] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

/* Private function declarations */

static void unhandled_opcode_handler(cpu_t *p_cpu);

static void opcode00_handler(cpu_t *p_cpu);
static void opcode01_handler(cpu_t *p_cpu);
static void opcode02_handler(cpu_t *p_cpu);
static void opcode03_handler(cpu_t *p_cpu);
static void opcode04_handler(cpu_t *p_cpu);
static void opcode05_handler(cpu_t *p_cpu);
static void opcode06_handler(cpu_t *p_cpu);
static void opcode07_handler(cpu_t *p_cpu);
static void opcode08_handler(cpu_t *p_cpu);
static void opcode09_handler(cpu_t *p_cpu);
static void opcode10_handler(cpu_t *p_cpu);
static void opcode11_handler(cpu_t *p_cpu);
static void opcode12_handler(cpu_t *p_cpu);
static void opcode13_handler(cpu_t *p_cpu);
static void opcode14_handler(cpu_t *p_cpu);
static void opcode15_handler(cpu_t *p_cpu);

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
	opcode15_handler};

/* Inlined private function definitions */

static inline uint8_t *mem_address(cpu_t *p_cpu, uint16_t offset)
{
	if (offset >= MEM_SIZE)
	{
		ERROR_PRINT("ADDRESS OUT OF BOUND.");
		exit(-1);
	}
	return p_cpu->memory + offset;
}

static inline uint16_t mem_offset(cpu_t *p_cpu, uint8_t *address)
{
	uint16_t offset = (uint16_t)(address - p_cpu->memory);
	if (offset >= MEM_SIZE)
	{
		ERROR_PRINT("ADDRESS OUT OF BOUND.");
		exit(-1);
	}
	return (uint16_t)(address - p_cpu->memory);
}

static inline void stack_push_pc(cpu_t *p_cpu)
{
	p_cpu->sp += sizeof(uint16_t);
	*((uint16_t *)p_cpu->sp) = mem_offset(p_cpu, p_cpu->pc);
}

static inline void stack_pop_pc(cpu_t *p_cpu)
{
	p_cpu->pc = mem_address(p_cpu, *((uint16_t *)p_cpu->sp));
	p_cpu->sp -= sizeof(uint16_t);
}

static inline uint16_t decode_NNN(const cpu_t *p_cpu)
{
	uint16_t nnn = *p_cpu->pc;
	nnn <<= 8;
	nnn |= *(p_cpu->pc + 1);
	nnn &= (uint16_t)0x0FFF;
	return nnn;
}

static inline uint8_t decode_NN(const cpu_t *p_cpu)
{
	uint8_t nn = *(p_cpu->pc + 1);
	return nn;
}

static inline uint8_t decode_N(const cpu_t *p_cpu)
{
	uint8_t n = *(p_cpu->pc + 1);
	n &= (uint8_t)0x0F;
	return n;
}

static inline uint8_t decode_X(const cpu_t *p_cpu)
{
	uint8_t x = *p_cpu->pc;
	x &= (uint8_t)0x0F;
	return x;
}

static inline uint8_t decode_Y(const cpu_t *p_cpu)
{
	uint8_t y = *(p_cpu->pc + 1);
	y >>= 4;
	y &= (uint8_t)0x0F;
	return y;
}

static inline uint8_t decode_op(const cpu_t *p_cpu)
{
	uint8_t op = *p_cpu->pc;
	op >>= 4;
	op &= (uint8_t)0x0F;
	return op;
}

/* Public function definitions */

cpu_t *cpu_allocate(void)
{
	cpu_t *p_cpu = calloc(1, sizeof(struct cpu_s));

	if (p_cpu)
	{
		p_cpu->sp = mem_address(p_cpu, STACK_ADDRESS);
		p_cpu->pc = mem_address(p_cpu, ROM_ADDRESS);
		p_cpu->font = mem_address(p_cpu, FONT_ADDRESS);
		p_cpu->i = mem_address(p_cpu, 0);
	}

	return p_cpu;
}

void cpu_load(cpu_t *p_cpu, uint8_t *program, uint16_t size)
{
	if (p_cpu && program)
	{
		p_cpu->pc = mem_address(p_cpu, ROM_ADDRESS);

		(void)memset(p_cpu->memory, 0, MEM_SIZE);
		(void)memcpy(p_cpu->pc, program, size);
		(void)memcpy(p_cpu->font, fontset, sizeof(fontset));
	}
}

void cpu_run(cpu_t *p_cpu)
{
	if (p_cpu)
	{
		opcode_handlers[decode_op(p_cpu)](p_cpu);
	}
}

int cpu_halted(cpu_t *p_cpu)
{
	if (p_cpu)
	{
		return p_cpu->halted_flag;
	}
	else
	{
		return 0;
	}
}

void cpu_tick(cpu_t *p_cpu)
{
	if (p_cpu)
	{
		if (p_cpu->timer_delay)
			p_cpu->timer_delay--;

		if (p_cpu->timer_sound)
			p_cpu->timer_sound--;
	}
}

int cpu_graphics_changed(cpu_t *p_cpu)
{
	if (p_cpu)
	{
		int flag = p_cpu->draw_flag;
		p_cpu->draw_flag = 0;
		return flag;
	}
	else
	{
		return 0;
	}
}

uint8_t *cpu_graphics(cpu_t *p_cpu)
{
	if (p_cpu)
	{
		return p_cpu->graphics;
	}
	else
	{
		return NULL;
	}
}

void cpu_press_key(cpu_t *p_cpu, uint8_t key)
{
	if (p_cpu && (key < KEY_COUNT))
	{
		p_cpu->keys[key] = 1;
	}
}

void cpu_release_key(cpu_t *p_cpu, uint8_t key)
{
	if (p_cpu && (key < KEY_COUNT))
	{
		p_cpu->keys[key] = 0;
	}
}

/* Private function definitions */

static void unhandled_opcode_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("UNHANDLED OPCODE");
	exit(-1);
}

/* 0NNN	Call	Calls RCA 1802 program at address NNN. Not necessary for most ROMs. */
/* 00E0	Display	disp_clear()	Clears the screen. */
/* 00EE	Flow	return;	Returns from a subroutine. */
static void opcode00_handler(cpu_t *p_cpu)
{
	switch (*(p_cpu->pc + 1))
	{
	case 0xE0:
		PRINT_INSTR("CLR");

		(void)memset(p_cpu->graphics, 0, GRAPHICS_SIZE);
		p_cpu->pc += 2;
		break;

	case 0xEE:
		PRINT_INSTR("RETURN");

		stack_pop_pc(p_cpu);
		p_cpu->pc += 2;
		break;

	default:
		unhandled_opcode_handler(p_cpu);
		break;
	}
}

/* 1NNN	Flow	goto NNN;	Jumps to address NNN. */
static void opcode01_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("JUMP");

	p_cpu->pc = mem_address(p_cpu, decode_NNN(p_cpu));
}

/* 2NNN	Flow	*(0xNNN)()	Calls subroutine at NNN. */
static void opcode02_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("CALL");

	stack_push_pc(p_cpu);
	p_cpu->pc = mem_address(p_cpu, decode_NNN(p_cpu));
}

/* 3XNN	Cond	if(Vx==NN)	Skips the next instruction if VX equals NN. (Usually the next instruction is a jump to skip a code block) */
static void opcode03_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("if(Vx==NN)");

	uint8_t x = decode_X(p_cpu);
	uint8_t nn = decode_NN(p_cpu);

	/* If VX == NN, skip next instruction. */
	if (nn == p_cpu->reg_v[x])
	{
		p_cpu->pc += 4;
	}
	else
	{
		p_cpu->pc += 2;
	}
}

/* 4XNN	Cond	if(Vx!=NN)	Skips the next instruction if VX doesn't equal NN. (Usually the next instruction is a jump to skip a code block) */
static void opcode04_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("if(Vx!=NN)");

	uint8_t x = decode_X(p_cpu);
	uint8_t nn = decode_NN(p_cpu);

	/* If VX != NN, skip next instruction. */
	if (nn != p_cpu->reg_v[x])
	{
		p_cpu->pc += 4;
	}
	else
	{
		p_cpu->pc += 2;
	}
}

/* 5XY0	Cond	if(Vx!=Vy)	Skips the next instruction if VX equals VY. (Usually the next instruction is a jump to skip a code block) */
static void opcode05_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("if(Vx!=Vy)");

	uint8_t x = decode_X(p_cpu);
	uint8_t y = decode_Y(p_cpu);

	/* If VX == VY, skip next instruction. */
	if (p_cpu->reg_v[x] == p_cpu->reg_v[y])
	{
		p_cpu->pc += 4;
	}
	else
	{
		p_cpu->pc += 2;
	}
}

/* 6XNN	Const	Vx = NN	Sets VX to NN. */
static void opcode06_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("Vx=NN");

	uint8_t x = decode_X(p_cpu);
	uint8_t nn = decode_NN(p_cpu);

	p_cpu->reg_v[x] = nn;
	p_cpu->pc += 2;
}

/* 7XNN	Const	Vx += NN	Adds NN to VX. (Carry flag is not changed) */
static void opcode07_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("Vx+=NN");

	uint8_t x = decode_X(p_cpu);
	uint8_t nn = decode_NN(p_cpu);

	p_cpu->reg_v[x] += nn;
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
static void opcode08_handler(cpu_t *p_cpu)
{
	uint8_t x = decode_X(p_cpu);
	uint8_t y = decode_Y(p_cpu);
	uint8_t n = decode_N(p_cpu);

	switch (n)
	{
	case (uint8_t)0x00:
		PRINT_INSTR("Vx=Vy");

		p_cpu->reg_v[x] = p_cpu->reg_v[y];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x01:
		PRINT_INSTR("Vx|=Vy");

		p_cpu->reg_v[x] |= p_cpu->reg_v[y];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x02:
		PRINT_INSTR("Vx&=Vy");

		p_cpu->reg_v[x] &= p_cpu->reg_v[y];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x03:
		PRINT_INSTR("Vx^=Vy");

		p_cpu->reg_v[x] ^= p_cpu->reg_v[y];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x04:
	{
		PRINT_INSTR("Vx+=Vy");

		uint16_t sum = p_cpu->reg_v[x] + p_cpu->reg_v[y];
		if (sum & (uint16_t)0xFF00)
		{
			p_cpu->reg_v[0xF] = 1;
		}
		else
		{
			p_cpu->reg_v[0xF] = 0;
		}

		p_cpu->reg_v[x] = (uint8_t)sum;
		p_cpu->pc += 2;
	}
	break;
	case (uint8_t)0x05:
	{
		PRINT_INSTR("Vx-=Vy");

		if (p_cpu->reg_v[y] > p_cpu->reg_v[x])
		{
			p_cpu->reg_v[15] = 0;
		}
		else
		{
			p_cpu->reg_v[15] = 1;
		}

		p_cpu->reg_v[x] -= p_cpu->reg_v[y];
		p_cpu->pc += 2;
	}
	break;
	case (uint8_t)0x06:
		PRINT_INSTR("Vx>>=1");

		p_cpu->reg_v[0xF] = p_cpu->reg_v[x] & (uint8_t)0x01;
		p_cpu->reg_v[x] >>= 1;
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x07:
	{
		PRINT_INSTR("Vx=Vy-Vx");

		if (p_cpu->reg_v[x] > p_cpu->reg_v[y])
		{
			p_cpu->reg_v[15] = 0;
		}
		else
		{
			p_cpu->reg_v[15] = 1;
		}

		p_cpu->reg_v[x] = p_cpu->reg_v[y] - p_cpu->reg_v[x];
		p_cpu->pc += 2;
	}
	break;
	case (uint8_t)0x0E:
		PRINT_INSTR("Vx<<=1");

		p_cpu->reg_v[0xF] = (p_cpu->reg_v[x] >> 7) & (uint8_t)0x01;
		p_cpu->reg_v[x] <<= 1;
		p_cpu->pc += 2;
		break;
	default:
		unhandled_opcode_handler(p_cpu);
		break;
	}
}

/* 9XY0	Cond	if(Vx==Vy)	Skips the next instruction if VX doesn't equal VY. (Usually the next instruction is a jump to skip a code block) */
static void opcode09_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("if(Vx==Vy)");

	uint8_t x = decode_X(p_cpu);
	uint8_t y = decode_Y(p_cpu);

	/* If VX != VY, skip next instruction. */
	if (p_cpu->reg_v[x] != p_cpu->reg_v[y])
	{
		p_cpu->pc += 4;
	}
	else
	{
		p_cpu->pc += 2;
	}
}

/* ANNN	MEM	I=NNN	Sets I to the address NNN. */
static void opcode10_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("I=NNN");

	uint16_t nnn = decode_NNN(p_cpu);

	p_cpu->i = mem_address(p_cpu, nnn);
	p_cpu->pc += 2;
}

/* BNNN	Flow	PC=V0+NNN	Jumps to the address NNN plus V0. */
static void opcode11_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("PC=V0+NNN");

	uint8_t v0 = p_cpu->reg_v[0];
	uint16_t nnn = decode_NNN(p_cpu);

	p_cpu->pc = mem_address(p_cpu, v0 + nnn);
}

/* CXNN	Rand	Vx=rand()&NN	Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN. */
static void opcode12_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("Vx=rand()&NN");

	uint8_t x = decode_X(p_cpu);
	uint8_t nn = decode_NN(p_cpu);

	p_cpu->reg_v[x] = ((uint8_t)(rand() % 256)) & nn;
	p_cpu->pc += 2;
}

/* DXYN	Disp	draw(Vx,Vy,N)	Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels. 
    Each row of 8 pixels is read as bit-coded starting from memory location I; I value doesn’t change after the execution of this instruction. 
    As described above, VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that doesn’t happen. */
static void opcode13_handler(cpu_t *p_cpu)
{
	PRINT_INSTR("draw(Vx,Vy,N)");

	uint8_t x = decode_X(p_cpu);
	uint8_t y = decode_Y(p_cpu);
	uint8_t n = decode_N(p_cpu);

	p_cpu->reg_v[0xF] = 0;

	int line, column;
	for (line = 0; line < (int)n; line++)
	{

		uint8_t sprite = *(p_cpu->i + line);

		for (column = 0; column < 8; column++)
		{

			uint8_t X = (p_cpu->reg_v[x] + column) % GRAPHICS_COLS;
			uint8_t Y = (p_cpu->reg_v[y] + line) % GRAPHICS_ROWS;

			uint8_t *pixel_screen = p_cpu->graphics + X + (Y * GRAPHICS_COLS);

			uint8_t sprite_bit = sprite >> (7 - column);
			sprite_bit &= (uint8_t)0x01;

			if (sprite_bit && *pixel_screen)
			{
				p_cpu->reg_v[0xF] = 1;
			}

			*pixel_screen = *pixel_screen ^ sprite_bit;
		}
	}

	p_cpu->draw_flag = 1;
	p_cpu->pc += 2;
}

/*
EX9E	KeyOp	if(key()==Vx)	Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
EXA1	KeyOp	if(key()!=Vx)	Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)*/
static void opcode14_handler(cpu_t *p_cpu)
{
	uint8_t x = decode_X(p_cpu);
	uint8_t nn = decode_NN(p_cpu);

	switch (nn)
	{
	case (uint8_t)0x9E:
	{
		PRINT_INSTR("if(key()==Vx)");

		uint8_t key = p_cpu->reg_v[x];

		if ((key < KEY_COUNT) && (p_cpu->keys[key]))
		{
			p_cpu->pc += 4;
		}
		else
		{
			p_cpu->pc += 2;
		}
	}
	break;
	case (uint8_t)0xA1:
	{
		PRINT_INSTR("if(key()!=Vx)");

		uint8_t key = p_cpu->reg_v[x];

		if ((key < KEY_COUNT) && (!p_cpu->keys[key]))
		{
			p_cpu->pc += 4;
		}
		else
		{
			p_cpu->pc += 2;
		}
	}
	break;
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

Stores the binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. 
* (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
FX55	MEM	reg_dump(Vx,&I)	Stores V0 to VX (including VX) in memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
FX65	MEM	reg_load(Vx,&I)	Fills V0 to VX (including VX) with values from memory starting at address I. The offset from I is increased by 1 for each value written, but I itself is left unmodified.
*/
static void opcode15_handler(cpu_t *p_cpu)
{
	uint8_t x = decode_X(p_cpu);

	switch (decode_NN(p_cpu))
	{
	case (uint8_t)0x07:
		PRINT_INSTR("Vx=get_delay()");

		p_cpu->reg_v[x] = p_cpu->timer_delay;
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x0A:
		PRINT_INSTR("Vx=get_key()");

		p_cpu->halted_flag = 1;
		for (uint8_t i = 0; i < KEY_COUNT; i++)
		{
			if (p_cpu->keys[i])
			{
				p_cpu->halted_flag = 0;
				p_cpu->reg_v[x] = i;
				p_cpu->pc += 2;
				break;
			}
		}
		break;
	case (uint8_t)0x15:
		PRINT_INSTR("delay_timer(Vx)");

		p_cpu->timer_delay = p_cpu->reg_v[x];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x18:
		PRINT_INSTR("sound_timer(Vx)");

		p_cpu->timer_sound = p_cpu->reg_v[x];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x1E:
		PRINT_INSTR("I+=Vx");

		p_cpu->i += p_cpu->reg_v[x];
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x29:
		PRINT_INSTR("I=sprite_addr[Vx]");

		p_cpu->i = p_cpu->font + (p_cpu->reg_v[x] * FONT_CHAR_SIZE);
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x33:
	{
		PRINT_INSTR("BCD(Vx)");

		uint8_t vx = p_cpu->reg_v[x];

		*(p_cpu->i + 2) = vx % 10;
		vx /= 10;
		*(p_cpu->i + 1) = vx % 10;
		vx /= 10;
		*(p_cpu->i + 0) = vx % 10;

		p_cpu->pc += 2;
	}
	break;
	case (uint8_t)0x55:
		PRINT_INSTR("reg_dump(Vx,&I)");

		(void)memcpy(p_cpu->i, p_cpu->reg_v, (x + 1) * sizeof(uint8_t));
		p_cpu->pc += 2;
		break;
	case (uint8_t)0x65:
		PRINT_INSTR("reg_load(Vx,&I)");

		(void)memcpy(p_cpu->reg_v, p_cpu->i, (x + 1) * sizeof(uint8_t));
		p_cpu->pc += 2;
		break;
	default:
		unhandled_opcode_handler(p_cpu);
		break;
	}
}
