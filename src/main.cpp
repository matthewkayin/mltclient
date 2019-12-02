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
void render_all(std::string in_progress, std::vector<std::string>* chatlog, int scroll_offset);
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

int set_scroll(int scroll_offset, int chatlog_size, int ticks);

// NON-UI FUNCTIONS
void update(std::vector<std::string>* chatlog);
bool attempt_connect(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, Serial* arduino_out);
void send_message(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, std::string* strobe_message, std::string message);
void sysmessage(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, std::string message);
void append_chatlog(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, std::string entry);
void reset_chatlines(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog);

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
    std::vector<std::string> chatlines;
    std::string in_progress = "";
    std::string input = "";
    int cursor_x = 0;
    int cursor_y = separator_point() + 1;
    int scroll_offset = 0;
    MEVENT mouse_event;
    mousemask(BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);

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

    SDL_SetRenderDrawColor(renderer, strobe_r, strobe_g, strobe_b, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    sysmessage(&chatlines, &chatlog, "Welcome to the Modulated Light Transceiver Client!");
    sysmessage(&chatlines, &chatlog, "Type \"/exit\" to exit");
    if(debug){

        sysmessage(&chatlines, &chatlog, "Debug mode is on");
    }

    sysmessage(&chatlines, &chatlog, "Initializing...");
    sysmessage(&chatlines, &chatlog, message);

    bool connected = false;
    Serial arduino_out;
    connected = attempt_connect(&chatlines, &chatlog, &arduino_out);

    render_all(in_progress, &chatlines, scroll_offset);
    move(cursor_y, cursor_x);

    // timing variables
    const unsigned int SECOND = 1000;
    unsigned int TARGET_FPS = 30;
    unsigned int STROBE_TIME = (int)(SECOND / TARGET_FPS);
    unsigned int before_time = SDL_GetTicks(); // returns milliseconds
    unsigned int before_sec = before_time;
    double fps = 0;
    int frames = 0;

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

        int old_chatlines_size = chatlines.size();
        bool chatlog_at_bottom = scroll_offset == (int)chatlines.size() - chatlog_height() || (int)chatlines.size() < chatlog_height();

        int refresh = 0;
        const int ALL = 1;
        const int CHATBOX_ONLY = 2;
        const int TEXTBOX_ONLY = 3;
        int key = getch();
        if(key == ERR){

            // nothing handled yet

        }else if(key == KEY_RESIZE){

            reset_chatlines(&chatlines, &chatlog);
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

        }else if(key == KEY_UP){

            scroll_offset = set_scroll(scroll_offset, chatlines.size(), -1);
            refresh = ALL;

        }else if(key == KEY_DOWN){

            scroll_offset = set_scroll(scroll_offset, chatlines.size(), 1);
            refresh = ALL;

        }else if(key == KEY_MOUSE){

            if(getmouse(&mouse_event) == OK){

                if(mouse_event.bstate & BUTTON4_PRESSED){

                    scroll_offset = set_scroll(scroll_offset, chatlines.size(), -1);
                    refresh = ALL;

                }else if(mouse_event.bstate & BUTTON5_PRESSED){

                    scroll_offset = set_scroll(scroll_offset, chatlines.size(), 1);
                    refresh = ALL;
                }
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

            connected = attempt_connect(&chatlines, &chatlog, &arduino_out);

        }else if(input == "/showfps"){

            sysmessage(&chatlines, &chatlog, "FPS is set to " + std::to_string(TARGET_FPS) + ", last FPS was " + std::to_string(fps));

        }else if(input.find("/setfps ") == 0){

            int index = input.find(" ") + 1;
            TARGET_FPS = std::stoi(input.substr(index, input.length() - index));
            STROBE_TIME = (int)(SECOND / TARGET_FPS);
            sysmessage(&chatlines, &chatlog, "Target FPS is now " + std::to_string(TARGET_FPS) + " and strobe time is " + std::to_string(STROBE_TIME));

        }else if(input == "/setred"){

            strobe_r = 255;
            strobe_g = 0;
            strobe_b = 0;
            SDL_SetRenderDrawColor(renderer, strobe_r, strobe_g, strobe_b, 255);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);

        }else if(input == "/setgreen"){

            strobe_r = 0;
            strobe_g = 255;
            strobe_b = 0;
            SDL_SetRenderDrawColor(renderer, strobe_r, strobe_g, strobe_b, 255);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);

        }else if((int)input.length() > 0 && input.at(0) == '/'){

            sysmessage(&chatlines, &chatlog, "Error! Command not recognized.");

        }else if(input != ""){

            //strobe_message += "101010101010101010101010101010";
            strobe_message += "10101010";
            before_time = SDL_GetTicks();
            before_sec = before_time;
            frames = 0;
            while(true){

                unsigned int after_time = SDL_GetTicks();
                unsigned int elapsed = after_time - before_time;

                if(elapsed >= STROBE_TIME){

                    if(strobe_message == ""){

                        break;
                    }
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

                    SDL_SetRenderDrawColor(renderer, strobe_r, strobe_g, strobe_b, 255);
                    SDL_RenderClear(renderer);
                    SDL_RenderPresent(renderer);

                    strobe_message = strobe_message.substr(1, strobe_message.length() - 1);
                    frames++;
                    before_time = SDL_GetTicks() + (elapsed - STROBE_TIME);
                }
            }
            strobe_r = 0;
            strobe_g = 0;
            strobe_b = 0;
            SDL_SetRenderDrawColor(renderer, strobe_r, strobe_g, strobe_b, 255);
            SDL_RenderClear(renderer);
            SDL_RenderPresent(renderer);
            unsigned int after_time = SDL_GetTicks();
            unsigned int total_elapsed = after_time - before_sec;
            sysmessage(&chatlines, &chatlog, "total milliseconds=" + std::to_string(total_elapsed));
            double seconds = (double)total_elapsed / (double)SECOND;
            fps = frames / seconds;
            sysmessage(&chatlines, &chatlog, "rendered " + std::to_string(frames) + " frames in " + std::to_string(seconds) + " seconds");

            //send_message(&chatlines, &chatlog, &strobe_message, input);
            //sysmessage(&chatlines, &chatlog, "New strobe message is: " + strobe_message);
            /*if(connected){

                send_message(&chatlog, &strobe_message, input);
                before_time = SDL_GetTicks(); // start the strobe timer

            }else{

                sysmessage(&chatlog, "Cannot send message! Device is not connected.");
            }*/
        }

        // clear input buffer
        input = "";

        // check if chatlog has been pushed to and we should scroll down
        if(chatlog_at_bottom && old_chatlines_size < (int)chatlines.size() && (int)chatlines.size() > chatlog_height()){

            scroll_offset = (int)chatlines.size() - chatlog_height();
            refresh = ALL;
        }

        update(&chatlog); // we always call this so that we always update at a regular rate

        // always render last
        if(refresh == ALL){

            render_all(in_progress, &chatlines, scroll_offset);

        }else if(refresh == TEXTBOX_ONLY){

            clear_textbox();
            render_textbox(in_progress);

        }else if(refresh == CHATBOX_ONLY){

            render_chatlog(&chatlines, scroll_offset);
        }
        if(refresh != 0){

            move(cursor_y, cursor_x);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    endwin();

    return 0;
}

void render_all(std::string in_progress, std::vector<std::string>* chatlog, int scroll_offset){

    clear();
    render_separator();
    render_textbox(in_progress);
    render_chatlog(chatlog, scroll_offset);
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
    int lines_to_draw = std::min((int)chatlog->size(), chatlog_height());

    for(int i = 0; i < lines_to_draw; i++){

        std::string message = chatlog->at(i + scroll_offset);
        mvaddstr(i, 0, message.c_str());
    }
}

int set_scroll(int scroll_offset, int chatlog_size, int ticks){

    int new_scroll_offset = scroll_offset + ticks;

    if(new_scroll_offset > chatlog_size - chatlog_height()){

        new_scroll_offset = chatlog_size - chatlog_height();
    }

    if(new_scroll_offset < 0){

        new_scroll_offset = 0;
    }

    return new_scroll_offset;
}

void update(std::vector<std::string>* chatlog){

    // This function will be at least every tenth of a second
    // Buuut it may be called much more than that, it will be called with each user event
    // So when writing code for it do not write anything that would be performance inhibitive
    // You need to check elapsed time in between certain checks here
}

bool attempt_connect(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, Serial* arduino_out){

    sysmessage(chatlines, chatlog, "Attempting to find MLT...");
    *arduino_out = Serial();
    std::string message = "";
    bool success = arduino_out->open(&message);
    sysmessage(chatlines, chatlog, message);
    if(!success){

        sysmessage(chatlines, chatlog, "Ensure your device is connected and type \"/connect\" to try again.");
    }

    return success;
}

void send_message(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, std::string* strobe_message, std::string message){

    char bitstring[4096];
    int bitstring_length = encode(message, bitstring);
    //arduino_out->write(bitstring, bitstring_length);

    for(int i = 0; i < bitstring_length; i++){

        *strobe_message += byte_to_binary(bitstring[i]);
    }

    std::string prefix = "[" + current_time() + "] You: ";
    append_chatlog(chatlines, chatlog, prefix + message);
}

void sysmessage(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, std::string message){

    std::string to_push = "--- " + message + " ---";
    append_chatlog(chatlines, chatlog, to_push);
}

void append_chatlog(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog, std::string entry){

    chatlog->push_back(entry);

    /*while(entry != ""){

        if((int)entry.length() >= COLS){

            chatlines->push_back(entry.substr(0, COLS));
            entry = entry.substr(COLS + 1, entry.length() - COLS);

        }else{

            chatlines->push_back(entry);
            entry = "";
        }
    }*/

    std::string next_line = "";
    size_t pos = 0;
    std::string word;
    int remaining_chars = COLS;
    while(entry != ""){

        pos = entry.find(" ");
        if(pos != std::string::npos){

            word = entry.substr(0, pos);
            entry.erase(0, pos + 1);

        }else{

            word = entry;
            entry = "";
        }

        if((int)word.length() + 1 <= remaining_chars){

            next_line += word + " ";
            remaining_chars -= (int)word.length() + 1;

        }else if((int)word.length() <= remaining_chars){

            next_line += word;
            chatlines->push_back(next_line);
            next_line = "";
            remaining_chars = COLS;

        }else{

            // if the word is super big, keep adding to it in as big of chunks as possible
            if((int)word.length() >= COLS){

                while(word != ""){

                    int chunk_size = std::min(remaining_chars, (int)word.length());
                    next_line += word.substr(0, chunk_size);
                    word = word.substr(chunk_size, word.length() - chunk_size);
                    remaining_chars -= chunk_size;
                    if(remaining_chars == 0){

                        chatlines->push_back(next_line);
                        remaining_chars = COLS;
                        next_line = "";

                    }else{

                        if(word != ""){

                            chatlines->push_back("something is wrong with your algorithm fren");
                        }
                    }
                }

            }else{

                chatlines->push_back(next_line);
                next_line = "";
                next_line += word + " ";
                remaining_chars = COLS - (int)word.length() - 1;
            }
        }
    }

    if(next_line != ""){

        chatlines->push_back(next_line);
    }
}

void reset_chatlines(std::vector<std::string>* chatlines, std::vector<std::string>* chatlog){

    chatlines->clear();
    for(std::vector<std::string>::iterator it = chatlog->begin(); it != chatlog->end(); ++it){

        std::string entry = (*it);
        append_chatlog(chatlines, chatlog, entry);
    }
}
