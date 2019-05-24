#include "cpu.h"
#include "log.h"

#include "SDL2/SDL.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/* Defines */

/* Typedefs */

typedef struct shared_data_s
{
	cpu_t *p_cpu;
	pthread_mutex_t mutex;
	pthread_cond_t key_pressed;
} shared_data_t;

typedef struct rom_s
{
	uint8_t *data;
	size_t size;
} rom_t;

/* Private variables */

static const float draw_frequency = 60.0;  /* Hz */
static const float cpu_frequency = 600.0;  /* Hz */
static const float timer_frequency = 60.0; /* Hz */

static const uint8_t mapped_keys[16] = {
	SDL_SCANCODE_X, // 0
	SDL_SCANCODE_1, // 1
	SDL_SCANCODE_2, // 2
	SDL_SCANCODE_3, // 3
	SDL_SCANCODE_Q, // 4
	SDL_SCANCODE_W, // 5
	SDL_SCANCODE_E, // 6
	SDL_SCANCODE_A, // 7
	SDL_SCANCODE_S, // 8
	SDL_SCANCODE_D, // 9
	SDL_SCANCODE_Z, // 10
	SDL_SCANCODE_C, // 11
	SDL_SCANCODE_4, // 12
	SDL_SCANCODE_R, // 13
	SDL_SCANCODE_F, // 14
	SDL_SCANCODE_V, // 15
};

/* Private function declarations */

static int load_rom(rom_t *p_rom, char *path);
static void *thread_cpu(void *arg);
static void *thread_timers(void *arg);
static void *thread_graphics(void *arg);

/* Public function definitions */

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		ERROR_PRINT("Missing argument.\n");
		return -1;
	}

	rom_t rom;
	if (load_rom(&rom, argv[1]) != 0)
	{
		ERROR_PRINT("load_rom failed.\n");
		return -1;
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		ERROR_PRINT("SDL_Init failed.\n");
		return -1;
	}

	cpu_t *p_cpu = cpu_allocate();

	if (p_cpu)
	{
		cpu_load(p_cpu, rom.data, rom.size);

		shared_data_t shared_data;
		shared_data.p_cpu = p_cpu;
		pthread_mutex_init(&(shared_data.mutex), NULL);
		pthread_cond_init(&(shared_data.key_pressed), NULL);

		pthread_t pth_cpu, pth_graphics, pth_timers;

		(void)pthread_create(&pth_cpu, NULL, thread_cpu, &shared_data);
		(void)pthread_create(&pth_timers, NULL, thread_timers, &shared_data);
		(void)pthread_create(&pth_graphics, NULL, thread_graphics, &shared_data);

		(void)pthread_join(pth_graphics, NULL);

		(void)pthread_cancel(pth_cpu);
		(void)pthread_cancel(pth_timers);
	}
	else
	{
		ERROR_PRINT("cpu_allocate failed.\n");
	}

	SDL_Quit();

	return 0;
}

/* Private function definitions */

static int load_rom(rom_t *p_rom, char *path)
{
	if (!p_rom || !path)
		return -1;

	FILE *file = fopen(path, "rb");

	if (!file)
	{
		ERROR_PRINT_ARGS("fopen failed (%s).\n", path);
		return -1;
	}

	/* Get ROM size. */
	fseek(file, 0, SEEK_END);
	p_rom->size = ftell(file);

	/* Rewind. */
	fseek(file, 0, SEEK_SET);

	p_rom->data = malloc(p_rom->size);
	if (!p_rom->data)
	{
		fclose(file);
		ERROR_PRINT("malloc failed.\n");
		return -1;
	}

	(void)fread(p_rom->data, p_rom->size, 1, file);

	fclose(file);

	return 0;
}

static void *thread_cpu(void *arg)
{
	shared_data_t *data = (shared_data_t *)arg;

	while (1)
	{
		(void)pthread_mutex_lock(&(data->mutex));

		cpu_run(data->p_cpu);

		if (cpu_halted(data->p_cpu))
		{
			/* If CPU is halted, wait for a key pressed. */
			pthread_cond_wait(&(data->key_pressed), &(data->mutex));
		}

		(void)pthread_mutex_unlock(&(data->mutex));

		SDL_Delay((int)(1000.0 / cpu_frequency));
	}
}

static void *thread_timers(void *arg)
{
	shared_data_t *data = (shared_data_t *)arg;

	while (1)
	{
		(void)pthread_mutex_lock(&(data->mutex));
		cpu_tick(data->p_cpu);
		(void)pthread_mutex_unlock(&(data->mutex));

		SDL_Delay((int)(1000.0 / timer_frequency));
	}
}

static void *thread_graphics(void *arg)
{
	shared_data_t *data = (shared_data_t *)arg;

	SDL_Window *window = SDL_CreateWindow("CHIP8-EMULATOR",
										  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320,
										  SDL_WINDOW_SHOWN);
	if (!window)
	{
		ERROR_PRINT("SDL_CreateWindow failed.\n");
		pthread_exit(NULL);
	}

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer)
	{
		ERROR_PRINT("SDL_CreateRenderer failed.\n");
		pthread_exit(NULL);
	}

	/* Clear screen. */
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	int quit = 0;
	while (!quit)
	{
		(void)pthread_mutex_lock(&(data->mutex));

		if (cpu_graphics_changed(data->p_cpu))
		{
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);

			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

			uint8_t *graphics = cpu_graphics(data->p_cpu);

			int line, column;
			for (int line = 0; line < 32; line++)
			{
				for (int column = 0; column < 64; column++)
				{
					if (graphics[column + (line * 64)])
					{
						SDL_Rect rect = {column * 10, line * 10, 10, 10};
						SDL_RenderFillRect(renderer, &rect);
					}
				}
			}

			SDL_RenderPresent(renderer);
		}

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit = 1;
				break;
			}

			const Uint8 *keys = SDL_GetKeyboardState(NULL);

			for (int key = 0; key < 16; key++)
			{
				if (keys[mapped_keys[key]])
				{
					pthread_cond_signal(&(data->key_pressed));
					cpu_press_key(data->p_cpu, key);
				}
				else
				{
					cpu_release_key(data->p_cpu, key);
				}
			}
		}

		(void)pthread_mutex_unlock(&(data->mutex));

		SDL_Delay((int)(1000.0 / draw_frequency));
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	pthread_exit(NULL);
}
