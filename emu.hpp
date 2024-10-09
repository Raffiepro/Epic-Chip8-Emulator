#pragma once

#include <fstream>
#include <stack>
#include <stdio.h>
#include <string.h>
#include <random>
#include <time.h>

#include <SDL2/SDL.h>

extern SDL_Window *window;
extern SDL_Renderer *renderer;

using u8 = unsigned char;
using u16 = unsigned short;

u8 ch8_font[] = {
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
    0xF0, 0x80, 0xF0, 0x80, 0x80 // F
};

SDL_KeyCode ch8_keys[] = {
    SDLK_1, SDLK_2, SDLK_3, SDLK_4,
    SDLK_q, SDLK_w, SDLK_e, SDLK_r,
    SDLK_a, SDLK_s, SDLK_d, SDLK_f,
    SDLK_z, SDLK_x, SDLK_c, SDLK_v
};

union CH8R_Color
{
    u8 color;
    struct
    {
        u8 b : 1;
        u8 g : 1;
        u8 r : 1;
    };
};

struct CPU
{
    u16 PC=0; //Program counter
    u16 I=0; //Index register
    std::stack<u16> stk; //Stack
    u8 delay=0; //Delay timer
    u8 sound=0; //Sound timer
    u8 V0,V1,V2,V3,V4,V5,V6,V7,V8,V9,VA,VB,VC,VD,VE,VF; //Variable registers
    
    u8 memory[4096];

    bool screen[2048];

    bool keyboard[16];

    CH8R_Color col;

    void load_rom(const char* filename)
    {
        std::ifstream infile(filename, std::ios::binary);

        // Get length of file
        infile.seekg(0, std::ios::end);
        size_t length = infile.tellg();
        infile.seekg(0, std::ios::beg);

        // Avoid overflow
        if (length > (4096 - 0x200))
        {
            length = 4096 - 0x200;
        }

        // Read file
        infile.read((char*)memory+0x200, length);
    }

    void Initialise(const char* rom_name)
    {
        srand(time(0));
        strcpy((char*)memory, (char*)ch8_font);
        memset((void*)screen, 0, 2048);
        memset((void*)keyboard, 0, 16);
        col.color=0x5;

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

    u8 *getVarReg(u8 var)
    {
        return (&V0)+var;
    }

    //These next two functions are to make it easier to implement other graphics frameworks
    void CLS()
    {
        SDL_SetRenderDrawColor(renderer, 0,0,0, 255);
        SDL_RenderClear(renderer);
        memset((void*)screen, 0, 2048);

        SDL_RenderPresent(renderer);
    }

    void DrawPoint(u8 x, u8 y, bool state)
    {
        SDL_SetRenderDrawColor(renderer, col.r*state*255,col.g*state*255,col.b*state*255, 255);
        SDL_RenderDrawPoint(renderer, x, y);
    }

    void updateGraphics()
    {
        SDL_RenderPresent(renderer);
    }

    void Decode(u16 opcode)
    {
        const u16 FIRST_NIBBLE_MASK = 0xF000;
        const u16 SECOND_NIBBLE_MASK = 0x0F00;
        const u16 THIRD_NIBBLE_MASK = 0x00F0;
        const u16 FOURTH_NIBBLE_MASK = 0x000F;
        
        const u16 SECOND_BYTE_MASK = 0x00FF;
        
        const u16 DATA_MASK = 0x0FFF; //Mask to get last(rightmost) 12 bits

        u8 Vx = (opcode&SECOND_NIBBLE_MASK)>>8;
        u8 Vy = (opcode&THIRD_NIBBLE_MASK)>>4;
        u8 *Vx_reg=getVarReg((opcode&SECOND_NIBBLE_MASK)>>8);
        u8 *Vy_reg=getVarReg((opcode&THIRD_NIBBLE_MASK)>>4);

        switch(FIRST_NIBBLE_MASK & opcode)
        {
            case 0x0000:
                switch(opcode & FOURTH_NIBBLE_MASK)
                {
                    case 0x0000:
                    {
                        CLS();
                        #ifdef DBGINSTRUCTIONS
                        printf("CLS\n");
                        #endif
                        break;
                    }

                    case 0x000E:
                        PC = stk.top();
                        stk.pop();
                        #ifdef DBGINSTRUCTIONS
                        printf("RET\n");
                        #endif
                    break;
                    
                    default: printf("-----Unknown instruction: %04X-----\n", opcode);break;
                }
            break;

            case 0x1000:
            {
                PC = (opcode & DATA_MASK) - 2;// - 0x202; //-202 used if not emulating old ram locations
                #ifdef DBGINSTRUCTIONS
                printf("JMP %04X\n", opcode & DATA_MASK);
                #endif
                break;
            }

            case 0x2000:
            {
                stk.push(PC);
                PC = (opcode & DATA_MASK) - 2;// - 0x202; //-202 used if not emulating old ram locations
                #ifdef DBGINSTRUCTIONS
                printf("CALL %04X\n", opcode & DATA_MASK);
                #endif
                break;
            }

            case 0x3000:
            {
                u8 check=opcode & SECOND_BYTE_MASK;
                if (*Vx_reg == check)
                {
                    PC+=2;
                }

                #ifdef DBGINSTRUCTIONS
                printf("Skip next instruction if V%01X(%02X) == %02X\n", Vx, *Vx_reg, opcode & SECOND_BYTE_MASK);
                #endif
                break;
            }

            case 0x4000:
            {
                u8 check=opcode & SECOND_BYTE_MASK;
                if (*Vx_reg != check)
                {
                    PC+=2;
                }

                #ifdef DBGINSTRUCTIONS
                printf("Skip next instruction if V%01X(%02X) != %02X\n", Vx, *Vx_reg, opcode & SECOND_BYTE_MASK);
                #endif
                break;
            }

            case 0x6000:
            {
                *Vx_reg = opcode & SECOND_BYTE_MASK;
                #ifdef DBGINSTRUCTIONS
                printf("Set register V%01X to %02X\n", Vx, opcode & SECOND_BYTE_MASK);
                #endif
                break;
            }

            case 0x7000:
            {
                *Vx_reg += opcode & SECOND_BYTE_MASK;
                #ifdef DBGINSTRUCTIONS
                printf("ADD %01X %02X\n", Vx, opcode & SECOND_BYTE_MASK);
                #endif
                break;
            }

            case 0x8000:
            {
                switch(opcode & FOURTH_NIBBLE_MASK)
                {
                    case 0x0000:
                    {
                        *Vx_reg = *Vy_reg;
                        #ifdef DBGINSTRUCTIONS
                        printf("LD V%01X, V%01X\n",Vx,Vy);
                        #endif
                        break;
                    }
                    case 0x0002:
                    {
                        *Vx_reg &= *Vy_reg;
                        #ifdef DBGINSTRUCTIONS
                        printf("AND V%01X, V%01X\n",Vx,Vy);
                        #endif
                        break;
                    }
                    case 0x0004:
                    {
                        u16 add = *Vx_reg; add+=*Vy_reg;
                        *Vx_reg = add;
                        VF = add>255;
                        #ifdef DBGINSTRUCTIONS
                        printf("ADD V%01X, V%01X\n",Vx,Vy);
                        #endif
                        break;
                    }
                    case 0x0005:
                    {
                        if(*Vx_reg>*Vy_reg) {VF=1;} else {VF=0;}
                        *Vx_reg -= *Vy_reg;
                        #ifdef DBGINSTRUCTIONS
                        printf("SUB V%01X, V%01X\n",Vx,Vy);
                        #endif
                        break;
                    }
                    case 0x000C:
                    {
                        col.color = Vy;
                        #ifdef DBGINSTRUCTIONS
                        printf("LD COL, %01X\n",Vy);
                        #endif
                        break;
                    }
                    case 0x000D:
                    {
                        col.color += Vy;
                        col.color %= 8;
                        #ifdef DBGINSTRUCTIONS
                        printf("ADD COL, %01X\n",Vy);
                        #endif
                        break;
                    }

                    default:
                    {
                        printf("-----Unknown instruction: %04X-----\n", opcode);
                        break;
                    }
                }
                break;
            }

            case 0xA000:
            {
                I = opcode & DATA_MASK;// - 0x200; //-200 used if not emulating old ram locations
                #ifdef DBGINSTRUCTIONS
                printf("I to %04X\n", opcode & DATA_MASK);
                #endif
                break;
            }

            case 0xC000:
            {
                *Vx_reg = (rand()%256) & (opcode&SECOND_BYTE_MASK);

                #ifdef DBGINSTRUCTIONS
                printf("RND V%01X, %02X\n", Vx, opcode & SECOND_BYTE_MASK);
                #endif
                break;
            }

            case 0xD000:
            {
                u8 x_pos = *Vx_reg;
                u8 y_pos = *Vy_reg;
                u8 bytes = opcode&FOURTH_NIBBLE_MASK;
                VF=0;
                
                for(u8 y=0;y<bytes;y++)
                {
                    //#ifdef dbg; printf("Y: %02X\n", memory[I+y]);
                    for(u8 x=0;x<8;x++)
                    {
                        bool bit = (memory[I+y] & ( 1 << 7-x ));
                        
                        if(!bit) continue;

                        u8 pixelX = (x+x_pos)%64;
                        u8 pixelY = (y+y_pos)%32;

                        int screen_pos = pixelY*64+pixelX;
                        screen[screen_pos] ^= 1;
                        
                        if(screen[screen_pos])
                        {
                            DrawPoint(pixelX, pixelY, true);
                        }
                        else
                        {
                            DrawPoint(pixelX, pixelY, false);
                            VF=1;
                        }
                    }
                }

                updateGraphics();
                #ifdef DBGINSTRUCTIONS
                printf("DRW X: %d Y: %d Bytes: %d\n", x_pos, y_pos, bytes);
                #endif
                break;
            }

            case 0xE000:
            {
                switch(opcode & SECOND_BYTE_MASK)
                {
                    case 0x009E:
                    {
                        if(keyboard[*Vx_reg]) {PC+=2;}// printf("KEY PRESSED SKIPPING\n");}
                        #ifdef DBGINSTRUCTIONS
                        printf("SKP V%01X(%02d)\n",Vx,*Vx_reg);
                        #endif
                        break;
                    }
                    case 0x00A1:
                    {
                        if(!keyboard[*Vx_reg]) {PC+=2;}// printf("KEY NOT PRESSED SKIPPING\n");}
                        #ifdef DBGINSTRUCTIONS
                        printf("SKNP V%01X(%02d)\n",Vx,*Vx_reg);
                        #endif
                        break;
                    }

                    default:
                    {
                        printf("-----Unknown instruction: %04X-----\n", opcode);
                        break;
                    }
                }
                break;
            }

            case 0xF000:
            {
                switch(opcode & SECOND_BYTE_MASK)
                {
                    case 0x0007:
                    {
                        *Vx_reg=delay;
                        #ifdef DBGINSTRUCTIONS
                        printf("LD V%01X, DT(%03d)\n",Vx,delay);
                        #endif
                        break;
                    }
                    case 0x0015:
                    {
                        delay=*Vx_reg;
                        #ifdef DBGINSTRUCTIONS
                        printf("LD DT(%03d), V%01X\n",delay,Vx);
                        #endif
                        break;
                    }
                    case 0x0018:
                    {
                        sound=*Vx_reg;
                        #ifdef DBGINSTRUCTIONS
                        printf("Set sound timer = V%01X\n",Vx);
                        #endif
                        break;
                    }
                    case 0x0029:
                    {
                        I = *Vx_reg * 5;
                        #ifdef DBGINSTRUCTIONS
                        printf("LD HEX sprite %01X\n",Vx);
                        #endif
                        break;
                    }
                    case 0x0033:
                    {
                        printf("PLOPPY\n");
                        u8 Vx = *Vx_reg;

                        memory[I] = Vx/100;
                        memory[I+1] = (Vx/10)%10;
                        memory[I+2] = Vx - Vx/100*100 - (Vx/10)%10*10;

                        #ifdef DBGINSTRUCTIONS
                        printf("LD B, %02d %02d %02d\n", Vx/100, (Vx/10)%10, Vx - Vx/100*100 - (Vx/10)%10*10);
                        #endif
                        break;
                    }
                    case 0x0065:
                    {
                        for(u8 v;v<=Vx;v++)
                        {
                            *getVarReg(v)=memory[I+v];
                        }
                        
                        #ifdef DBGINSTRUCTIONS
                        printf("LD V%01X, [I]\n", Vx);
                        #endif
                        break;
                    }
                    case 0x000A:
                    {
                        I = *Vx_reg * 5;
                        #ifdef DBGINSTRUCTIONS
                        printf("LD HEX sprite %01X\n",Vx);
                        #endif
                        break;
                    }

                    default:
                    {
                        printf("-----Unknown instruction: %04X-----\n", opcode);
                        break;
                    }
                }
                break;
            }

            default:
            {
                printf("-----Unknown instruction: %04X-----\n", opcode);
                break;
            }
        }
    }

    void Execute()
    {
        //printf("OPCODE: %04X PC: %04X\n", Fetch(), PC);
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
            #ifdef DBGINSTRUCTIONS
            printf("%X\n", memory[i]);
            #endif
        }
    }
};