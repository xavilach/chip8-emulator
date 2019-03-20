#include "cpu.h"

#include "SDL2/SDL.h"

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
	if (argc < 1)
		return -1;

	FILE *fptr = fopen(argv[1], "rb");

	if (fptr == NULL) {
		return -1;
	}

	fseek(fptr, 0, SEEK_END);
	size_t size = ftell(fptr);
	fseek(fptr, 0, SEEK_SET);

	char* buffer = malloc(size);
	if (buffer == NULL) {
		return -1;
	}

	fread(buffer, size, 1, fptr);

	fclose(fptr);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		return -1;
	}

	SDL_Window *window = SDL_CreateWindow("CHIP8-EMULATOR",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 320,
			SDL_WINDOW_SHOWN);
	if (window == NULL) {
		return -1;
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (renderer == NULL) {
		return -1;
	}

	struct cpu_s* p_cpu = cpu_allocate();

	if (p_cpu) {
		cpu_load(p_cpu, buffer, size, 0x200);

		while(1) {
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			SDL_Event event;
			while( SDL_PollEvent( &event ) ){
                
                switch( event.type ){
                    case SDL_QUIT:
                        return 0;
                        
                    default:
                        break;
                }
                
                if (keys[SDL_SCANCODE_0]){
					cpu_press_key(p_cpu, 0);
				} else {
					cpu_release_key(p_cpu, 0);
				}
                if (keys[SDL_SCANCODE_1]){
					cpu_press_key(p_cpu, 1);
				} else {
					cpu_release_key(p_cpu, 1);
				}
                if (keys[SDL_SCANCODE_2]){
					cpu_press_key(p_cpu, 2);
				} else {
					cpu_release_key(p_cpu, 2);
				}
                if (keys[SDL_SCANCODE_3]){
					cpu_press_key(p_cpu, 3);
				} else {
					cpu_release_key(p_cpu, 3);
				}
                if (keys[SDL_SCANCODE_4]){
					cpu_press_key(p_cpu, 4);
				} else {
					cpu_release_key(p_cpu, 4);
				}
                if (keys[SDL_SCANCODE_5]){
					cpu_press_key(p_cpu, 5);
				} else {
					cpu_release_key(p_cpu, 5);
				}
                if (keys[SDL_SCANCODE_6]){
					cpu_press_key(p_cpu, 6);
				} else {
					cpu_release_key(p_cpu, 6);
				}
                if (keys[SDL_SCANCODE_7]){
					cpu_press_key(p_cpu, 7);
				} else {
					cpu_release_key(p_cpu, 7);
				}
                if (keys[SDL_SCANCODE_8]){
					cpu_press_key(p_cpu, 8);
				} else {
					cpu_release_key(p_cpu, 8);
				}
                if (keys[SDL_SCANCODE_9]){
					cpu_press_key(p_cpu, 9);
				} else {
					cpu_release_key(p_cpu, 9);
				}
                if (keys[SDL_SCANCODE_A]){
					cpu_press_key(p_cpu, 10);
				} else {
					cpu_release_key(p_cpu, 10);
				}
                if (keys[SDL_SCANCODE_B]){
					cpu_press_key(p_cpu, 11);
				} else {
					cpu_release_key(p_cpu, 11);
				}
                if (keys[SDL_SCANCODE_C]){
					cpu_press_key(p_cpu, 12);
				} else {
					cpu_release_key(p_cpu, 12);
				}
                if (keys[SDL_SCANCODE_D]){
					cpu_press_key(p_cpu, 13);
				} else {
					cpu_release_key(p_cpu, 13);
				}
                if (keys[SDL_SCANCODE_E]){
					cpu_press_key(p_cpu, 14);
				} else {
					cpu_release_key(p_cpu, 14);
				}
                if (keys[SDL_SCANCODE_F]){
					cpu_press_key(p_cpu, 15);
				} else {
					cpu_release_key(p_cpu, 15);
				}
            }
			
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);
			cpu_run(p_cpu);

			cpu_tick(p_cpu);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);
			
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			
			uint8_t* graphics = cpu_graphics(p_cpu);
			
			int line, column;
			for(int line=0; line<32; line++)
			{
				for(int column=0; column<64; column++)
				{
					if(graphics[column/8 + (line * 8)] & (0x80 >> column % 8))
					{
						SDL_Rect rect = {column * 10, line * 10, 10, 10};
						SDL_RenderFillRect(renderer, &rect); 
					}
				}
			}
						
			SDL_RenderPresent(renderer);

			SDL_Delay(1000/60);
		}
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
