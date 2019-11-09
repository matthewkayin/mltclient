#include <iostream>
#include <SDL2/SDL.h>
#define SDL_MAIN_HANDLED
//compile with g++ mouseTest.cpp -lSDL2

int WinMain(int argc, char* argv[]){

    SDL_Event e;
    e.type = SDL_MOUSEWHEEL;
    while(SDL_PollEvent(&e)){
        if(e.type == SDL_MOUSEWHEEL){
            std::cout << "mouse";
        }
    }
    return 0;
}