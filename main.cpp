#define CH8_SDL
#include "emu.hpp"

#include <iostream>
#include <stdio.h>

#include <SDL2/SDL.h>

const int WIDTH=64, HEIGHT=32, PIXELSIZE=10;

SDL_Window *window;
SDL_Renderer *renderer;

int SDL_main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_EVERYTHING);

    window = SDL_CreateWindow("WHAT THE SIGMA", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH*PIXELSIZE, HEIGHT*PIXELSIZE, /*SDL_WINDOW_FULLSCREEN | */SDL_RENDERER_PRESENTVSYNC/* | SDL_WINDOW_ALLOW_HIGHDPI*/);

    if(NULL==window)
    {
        std::cout << "THE WINDOW DIDNT FUCKING OPEN >:((((((((((( ERROR: " << SDL_GetError() << '\n';
        return -1;
    }

    fclose(stdout);

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetScale(renderer, PIXELSIZE, PIXELSIZE);

    SDL_Event event;

    CPU cpu;
    cpu.Initialise(argv[1]);

    size_t delay_time = SDL_GetTicks64();
    size_t sound_time = SDL_GetTicks64();

    size_t now;

    size_t time = SDL_GetTicks64();
    unsigned int delta = 0;
    while(true)
    {
        now = SDL_GetTicks64();
        size_t delay_timer = now - delay_time;
        size_t sound_timer = now - sound_time;
        if(delay_timer>=16 && cpu.delay>0)
        {
            cpu.delay-=1;
            delay_time=now;
        }
        if(sound_timer>=16 && cpu.sound>0)
        {
            cpu.sound-=1;
            sound_time=now;
        }

        delta=now-time;
        if (!(delta > 1000/700.0))
        {
            continue;
        }

        time=now;

        cpu.Execute();

        if(SDL_PollEvent(&event))
        {
            if(SDL_QUIT==event.type) {break;}
            if(SDL_KEYDOWN==event.type)
            {
                for(int k=0;k<16;k++)
                {
                    if(ch8_keys[k] == event.key.keysym.sym) {cpu.keyboard[k]=true;}
                }

                if(SDLK_SPACE==event.key.keysym.sym)
                {
                    cpu.Execute();
                }
            }
            if(SDL_KEYUP==event.type)
            {
                for(int k=0;k<16;k++)
                {
                    if(ch8_keys[k] == event.key.keysym.sym) {cpu.keyboard[k]=false;}
                }
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}