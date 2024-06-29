#pragma once

#include <fstream>
#include <stack>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

using u8 = unsigned char;
using u16 = unsigned short;

u8 font[] = {0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
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
0xF0, 0x80, 0xF0, 0x80, 0x80};  // F

struct CPU
{
    u16 PC=0; //Program counter
    u16 I=0; //Index register
    std::stack<u16> stack; //Stack
    u8 delay=0; //Delay timer
    u8 sound=0; //Sound timer
    u8 V0,V1,V2,V3,V4,V5,V6,V7,V8,V9,VA,VB,VC,VD,VE,VF; //Variable registers
    u8 memory[4096];
    bool screen[2048];

    void load_rom(const char* filename)
    {
        std::ifstream infile(filename, std::ios::binary);

        // Get length of file
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);

        // Don't overflow the buffer!
        if (length > (4096 - 0x200))
        {
            length = 4096 - 0x200;
        }

        // Read file
        infile.read((char*)memory+0x200, length);
    }

    void Initialise(const char* rom_name)
    {
        strcpy((char*)memory, (char*)font);
        load_rom(rom_name);
        PC=0x200;
    }

    u16 Fetch()
    {
        u16 instruction;
                    
        instruction = memory[PC];

        instruction = instruction<<8;

        instruction |= memory[PC+1];

        return instruction;
    }

    u8 *getVarReg(u8& var)
    {
        return (&V0)+var;
    }

    void Decode(u16 opcode)
    {
        const u16 FIRST_NIBBLE_MASK = 0xF000;
        const u16 SECOND_NIBBLE_MASK = 0x0F00;
        const u16 THIRD_NIBBLE_MASK = 0x00F0;
        const u16 FOURTH_NIBBLE_MASK = 0x000F;
        
        const u16 SECOND_BYTE_MASK = 0x00FF;
        
        const u16 DATA_MASK = 0x0FFF; //Mask to get last(rightmost) 12 bits

        switch(FIRST_NIBBLE_MASK & opcode)
        {
            case 0x0000:
                switch(opcode & FOURTH_NIBBLE_MASK)
                {
                    case 0x0000:
                    {
                        SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
                        SDL_RenderClear(renderer);
                        memset((void*)screen, 0, 2048);

                        SDL_RenderPresent(renderer);
                        printf("CLS\n");
                        break;
                    }

                    case 0x000E:
                        printf("RET\n");
                    break;
                }
            break;

            case 0x1000:
            {
                PC = (opcode & DATA_MASK) - 2;// - 0x202; //-202 used if not emulating old ram locations
                printf("JMP %04X\n", opcode & DATA_MASK);
                break;
            }

            case 0x3000:
            {
                u8 Vx = (opcode&SECOND_NIBBLE_MASK)>>8;
                if (*getVarReg(Vx) == opcode & SECOND_BYTE_MASK)
                {
                    PC+=2;
                }

                printf("Skip next instruction if V%01X == %02X\n", (opcode&SECOND_NIBBLE_MASK)>>8, opcode & SECOND_BYTE_MASK);
                break;
            }

            case 0x6000:
            {
                u8 Vx = (opcode&SECOND_NIBBLE_MASK)>>8;
                *getVarReg(Vx) = opcode & SECOND_BYTE_MASK;

                printf("Set register V%01X to %02X\n", (opcode&SECOND_NIBBLE_MASK)>>8, opcode & SECOND_BYTE_MASK);
                break;
            }

            case 0x7000:
            {
                u8 Vx = (opcode&SECOND_NIBBLE_MASK)>>8;
                *getVarReg(Vx) += opcode & SECOND_BYTE_MASK;
                printf("ADD %01X %02X\n", (opcode&SECOND_NIBBLE_MASK)>>8, opcode & SECOND_BYTE_MASK);
                break;
            }

            case 0xA000:
            {
                I = opcode & DATA_MASK;// - 0x200; //-200 used if not emulating old ram locations
                printf("I to %04X\n", opcode & DATA_MASK);
                break;
            }

            case 0xD000:
            {
                u8 x_pos = (opcode&SECOND_NIBBLE_MASK)>>8;
                u8 y_pos = (opcode&THIRD_NIBBLE_MASK)>>4;
                u8 bytes = opcode&FOURTH_NIBBLE_MASK;
                x_pos = *getVarReg(x_pos);
                y_pos = *getVarReg(y_pos);

                printf("DRW X: %d Y: %d Bytes: %d\n", x_pos, y_pos, bytes);

                for(u8 y=0;y<bytes;y++)
                {
                    printf("Y: %02X\n", memory[I+y]);
                    for(u8 x=0;x<8;x++)
                    {
                        bool bit = (memory[I+y] & ( 1 << 7-x ));
                        
                        if(!bit) continue;

                        int screen_pos = ((int)y+y_pos)*64+(x+x_pos);
                        screen[screen_pos] ^= 1;
                        
                        if(screen[screen_pos])
                        {
                            SDL_SetRenderDrawColor(renderer, 255,255,255, 255);
                            SDL_RenderDrawPoint(renderer, x+x_pos, y+y_pos);
                        }
                        else
                        {
                            SDL_SetRenderDrawColor(renderer, 255,255,255, 255);
                            SDL_RenderDrawPoint(renderer, x+x_pos, y+y_pos);
                        }
                    }
                }

                /*SDL_Rect rect;
                rect.x = x_pos;
                rect.y = y_pos;
                rect.w = 8;
                rect.h = bytes;
                SDL_RenderDrawRect(renderer, &rect);*/

                SDL_RenderPresent(renderer);
                break;
            }

            default:
            printf("Unknown instruction: %04X\n", opcode);break;
        }
    }

    void Execute()
    {
        printf("EPIC: %04X PC: %04X\n", Fetch(), PC);

        Decode(Fetch());
        PC+=2;
    }

    void test()
    {
        printf("V5: %02X\n", V5);

        Decode(0x6550);

        printf("V5: %02X\n", V5);

        Decode(0x7550);

        printf("V5: %02X\n", V5);

        printf("I: %03X\n", I);

        Decode(0xA500);

        printf("I: %03X\n", I);
    }

    void print_mem()
    {
        for(u16 i;i<4096;i++)
        {
            printf("%X\n", memory[i]);
        }
    }
};