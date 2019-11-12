#ifdef _WIN32
    #include <ncurses/ncurses.h>
    #define SDL_MAIN_HANDLED
#else
    #include <ncurses.h>
#endif
#include <SDL2/SDL.h>
#include "encode.hpp"
#include "serial.hpp"
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <ctime>

// RENDERING FUNCTIONS
void render_all(std::string in_progress, int cursor_x, int cursor_y, std::vector<std::string>* chatlog, int scroll_offset);
void render_separator();
void clear_textbox();
void render_textbox(std::string in_progress);
void render_chatlog(std::vector<std::string>* chatlog, int scroll_offset);

// FUNCTIONS THAT ACT LIKE GLOBAL VARIABLES
int textbox_height(); // returns textbox height needed for 140 chars
int separator_point(); // returns what row to render line seperator between two windows
int chatlog_height(); // returns chatlog height in number of rows
std::string current_time(); // returns current time formatted in HH:MM (military time)
int get_cursor_index(int cursor_x, int cursor_y); // gets the index of the string that the cursor is at

// NON-UI FUNCTIONS
void update(std::vector<std::string>* chatlog);
bool attempt_connect(std::vector<std::string>* chatlog, Serial* arduino_out);
void send_message(std::vector<std::string>* chatlog, std::string* strobe_message, std::string message);
void sysmessage(std::vector<std::string>* chatlog, std::string message);

int main(int argc, char* argv[]){

    bool debug = false;

    for(int i = 0; i < argc; i++){

        if(std::strcmp(argv[i], "--debug") == 0){

            debug = true;
        }
    }

    // init ncurses
    initscr();
    //halfdelay(1);
    nodelay(stdscr, TRUE);
    noecho();
    keypad(stdscr, TRUE);

    std::vector<std::string> chatlog;
    std::string in_progress = "";
    std::string input = "";
    int cursor_x = 0;
    int cursor_y = separator_point() + 1;
    int scroll_offset = 0;

    // init sdl
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    std::string message;
    std::string strobe_message = "";
    bool is_fullscreen = false;
    bool sdl_close = false;
    int strobe_r = 0;
    int strobe_b = 0;
    int strobe_g = 0;

    bool sdl_success = !(SDL_Init(SDL_INIT_VIDEO) < 0);
    window = SDL_CreateWindow("Strobe Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    sdl_success = sdl_success && window && renderer;

    if(sdl_success){

        message = "Strobe window initialized successfully";

    }else{

        message = "Error initializing strobe window, SDL Error: " + std::string(SDL_GetError());
    }

    sysmessage(&chatlog, "Welcome to the Modulated Light Transceiver Client!");
    sysmessage(&chatlog, "Type \"/exit\" to exit");
    if(debug){

        sysmessage(&chatlog, "Debug mode is on");
    }

    sysmessage(&chatlog, "Initializing...");
    sysmessage(&chatlog, message);

    bool connected = false;
    Serial arduino_out;
    connected = attempt_connect(&chatlog, &arduino_out);

    render_all(in_progress, cursor_x, cursor_y, &chatlog, scroll_offset);

    // timing variables
    const unsigned int STROBE_TIME = 100; // something is wrong with the timing
    unsigned int before_time = SDL_GetTicks(); // returns milliseconds

    while(input != "/exit" && !sdl_close){

        SDL_Event e;

        while(SDL_PollEvent(&e) != 0){

            if(e.type == SDL_QUIT){

                sdl_close = true;

            }else if(e.type == SDL_MOUSEBUTTONDOWN){

                if(e.button.button == SDL_BUTTON_LEFT){

                    if(is_fullscreen){

                        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "1");
                        SDL_SetWindowFullscreen(window, 0);
                        is_fullscreen = false;

                    }else{

                        SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                        is_fullscreen = true;
                    }
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, strobe_r, strobe_g, strobe_b, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);

        int refresh = 0;
        const int ALL = 1;
        const int CHATBOX_ONLY = 2;
        const int TEXTBOX_ONLY = 3;
        int key = getch();
        if(key == ERR){

            // nothing handled yet

        }else if(key == KEY_RESIZE){

            refresh = ALL;

        }else if(key == 10){

            // RETURN INPUT
            input = in_progress;
            in_progress = "";
            clear();
            render_separator();
            cursor_x = 0;
            cursor_y = separator_point() + 1;
            refresh = ALL;

        }else if(key == 8 || key == 127){

            // BACKSPACE INPUT
            int cursor_index = get_cursor_index(cursor_x, cursor_y);
            if(cursor_index != 0){

                in_progress.erase(cursor_index - 1, 1);
                cursor_x--;
                if(cursor_x < 0){

                    cursor_x = COLS - 1;
                    cursor_y--;
                }
                refresh = TEXTBOX_ONLY;
            }

        }else if(key == KEY_LEFT){

            int cursor_index = get_cursor_index(cursor_x, cursor_y);
            if(cursor_index != 0){

                cursor_x--;
                if(cursor_x < 0){

                    cursor_x = COLS - 1;
                    cursor_y--;
                }
                move(cursor_y, cursor_x);
            }

        }else if(key == KEY_RIGHT){

            unsigned int cursor_index = get_cursor_index(cursor_x, cursor_y);
            if(cursor_index != in_progress.length()){

                cursor_x++;
                if(cursor_x >= COLS){

                    cursor_x = 0;
                    cursor_y++;
                }
                move(cursor_y, cursor_x);
            }

        }else{

            if(in_progress.length() < 140){

                int cursor_index = get_cursor_index(cursor_x, cursor_y);
                in_progress.insert(cursor_index, 1, (char)key);
                cursor_x++;
                if(cursor_x >= COLS){

                    cursor_x = 0;
                    cursor_y++;
                }
                refresh = TEXTBOX_ONLY;
            }
        }

        // handle input here
        if(input == "/exit"){

            break;

        }else if(input == "/connect"){

            connected = attempt_connect(&chatlog, &arduino_out);

        }else if(input != ""){

            send_message(&chatlog, &strobe_message, input);
            sysmessage(&chatlog, "New strobe message is: " + strobe_message);
            /*if(connected){

                send_message(&chatlog, &strobe_message, input);
                before_time = SDL_GetTicks(); // start the strobe timer

            }else{

                sysmessage(&chatlog, "Cannot send message! Device is not connected.");
            }*/
        }

        // clear input buffer
        input = "";

        unsigned int elapsed = SDL_GetTicks() - before_time;
        if(elapsed >= STROBE_TIME){

            if(strobe_message != ""){

                char next = strobe_message.at(0);
                if(next == '1'){

                    strobe_r = 0;
                    strobe_g = 255;
                    strobe_b = 0;

                }else if(next == '0'){

                    strobe_r = 255;
                    strobe_g = 0;
                    strobe_b = 0;
                }

                strobe_message = strobe_message.substr(1, strobe_message.length() - 1);
                if(strobe_message == ""){

                    strobe_r = 0;
                    strobe_g = 0;
                    strobe_b = 0;
                }
            }

            before_time = SDL_GetTicks() + (elapsed - STROBE_TIME);
        }
        update(&chatlog); // we always call this so that we always update at a regular rate

        // always render last
        if(refresh == ALL){

            render_all(in_progress, cursor_x, cursor_y, &chatlog, scroll_offset);

        }else if(refresh == TEXTBOX_ONLY){

            clear_textbox();
            render_textbox(in_progress);
            move(cursor_y, cursor_x);

        }else if(refresh == CHATBOX_ONLY){

            render_chatlog(&chatlog, scroll_offset);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    endwin();
    return 0;
}

void render_all(std::string in_progress, int cursor_x, int cursor_y, std::vector<std::string>* chatlog, int scroll_offset){

    clear();
    render_separator();
    render_textbox(in_progress);
    render_chatlog(chatlog, scroll_offset);
    move(cursor_y, cursor_x);
}

void render_separator(){

    int row = separator_point();
    char dash = '_'; // using em-dash for a clean line
    for(int i = 0; i < COLS; i++){

        mvaddch(row, i, dash);
    }
}

void clear_textbox(){

    int cx = 0;
    int cy = separator_point() + 1;
    for(int i = 0; i < 140; i++){

        mvaddch(cy, cx, ' ');

        cx++;
        if(cx >= COLS){

            cx = 0;
            cy++;
        }
    }
}

void render_textbox(std::string in_progress){

    int cx = 0;
    int cy = separator_point() + 1;
    for(unsigned int i = 0; i < in_progress.length(); i++){

        char to_put = in_progress.at(i);
        mvaddch(cy, cx, to_put);

        cx++;
        if(cx >= COLS){

            cx = 0;
            cy++;
        }
    }
}

// NOTE these three are heavily reliant on each other, be wary of changing them

int textbox_height(){

    return ceil(140.0 / COLS);
}

int separator_point(){

    return LINES - textbox_height() - 1;
}

int chatlog_height(){

    return separator_point();
}

std::string current_time(){

    time_t t = time(NULL);
    tm* now = localtime(&t);
    std::string hour = std::to_string(now->tm_hour);
    std::string minute = std::to_string(now->tm_min);
    if(hour.length() == 1){

        hour = "0" + hour;
    }
    if(minute.length() == 1){

        minute = "0" + minute;
    }

    return hour + ":" + minute;
}

int get_cursor_index(int cursor_x, int cursor_y){

    int y_offset = cursor_y - (separator_point() + 1);
    return (y_offset * COLS) + cursor_x;
}

void render_chatlog(std::vector<std::string>* chatlog, int scroll_offset){

    // TODO incorporate scroll offset
    unsigned int i = 0;
    if (chatlog->size() > 25){
        for (int i = 25; i < chatlog->size(); i++){
            std::string message = chatlog->at(i);
            mvaddstr((i%24), 0, message.c_str());
        }
    }

    else{
        for(unsigned int i = 0; i < chatlog->size(); i++){
            std::string message = chatlog->at(i);
            mvaddstr(i, 0, message.c_str());
        }
    }
}

void update(std::vector<std::string>* chatlog){

    // This function will be at least every tenth of a second
    // Buuut it may be called much more than that, it will be called with each user event
    // So when writing code for it do not write anything that would be performance inhibitive
    // You need to check elapsed time in between certain checks here
}

bool attempt_connect(std::vector<std::string>* chatlog, Serial* arduino_out){

    sysmessage(chatlog, "Attempting to find MLT...");
    *arduino_out = Serial();
    std::string message = "";
    bool success = arduino_out->open(&message);
    sysmessage(chatlog, message);
    if(!success){

        sysmessage(chatlog, "Ensure your device is connected and type \"/connect\" to try again.");
    }

    return success;
}

void send_message(std::vector<std::string>* chatlog, std::string* strobe_message, std::string message){

    char bitstring[4096];
    int bitstring_length = encode(message, bitstring);
    //arduino_out->write(bitstring, bitstring_length);

    for(int i = 0; i < bitstring_length; i++){

        *strobe_message += byte_to_binary(bitstring[i]);
    }

    std::string prefix = "[" + current_time() + "] You: ";
    chatlog->push_back(prefix + message);
}

void sysmessage(std::vector<std::string>* chatlog, std::string message){

    std::string to_push = "--- " + message + " ---";
    chatlog->push_back(to_push);
}
