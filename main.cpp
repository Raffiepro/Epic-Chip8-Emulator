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

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RenderSetScale(renderer, PIXELSIZE, PIXELSIZE);

    SDL_Event event;

    CPU cpu;
    cpu.Initialise("IBM Logo.ch8");

    while(true)
    {
        if(SDL_PollEvent(&event))
        {
            if(SDL_QUIT==event.type) {break;}
            if(SDL_KEYDOWN==event.type)
            {
                if(SDLK_LEFT==event.key.keysym.sym)
                {std::cout<<"Left";}

                if(SDLK_SPACE==event.key.keysym.sym)
                {
                    cpu.Execute();
                }
            }
            /*if(SDL_KEYUP==event.type)
            {
                std::cout << "Keyup\n";
            }*/
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}