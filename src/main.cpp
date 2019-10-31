#ifdef _WIN32
    #include <ncurses/ncurses.h>
    #undef KEY_EVENT
#else
    #include <ncurses.h>
#endif
#include "encode.hpp"
#include "serial.hpp"
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>
#include <vector>
#include <ctime>

void render_all(std::string in_progress, int cursor_x, int cursor_y, std::vector<std::string>* chatlog, int scroll_offset);
void render_separator();
void clear_textbox();
void render_textbox(std::string in_progress);
void render_chatlog(std::vector<std::string>* chatlog, int scroll_offset);

// Functions to act as global variables
int textbox_height(); // returns textbox height needed for 140 chars
int separator_point(); // returns row to render line seperator between two windows
int chatlog_height(); // returns chatlog height in number of rows
int chat_segment_width();
std::string current_time();

int get_cursor_index(int cursor_x, int cursor_y);

void update(std::vector<std::string>* chatlog);
void sysmessage(std::vector<std::string>* chatlog, std::string message);

int main(int argc, char* argv[]){

    bool debug = false;

    for(int i = 0; i < argc; i++){

        if(std::strcmp(argv[i], "--debug")){

            debug = true;
        }
    }

    initscr();
    halfdelay(1);
    noecho();
    keypad(stdscr, TRUE);

    std::vector<std::string> chatlog;
    std::string in_progress = "";
    std::string input = "";
    int cursor_x = 0;
    int cursor_y = separator_point() + 1;
    int scroll_offset = 0;

    sysmessage(&chatlog, "Welcome to the Modulated Light Transceiver Client!");
    sysmessage(&chatlog, "Type \"/exit\" to exit");
    sysmessage(&chatlog, "Attempting to find MLT...");

    Serial arduino_out = Serial();
    std::string message = "";
    bool success = arduino_out.open(&message);
    sysmessage(&chatlog, message);

    if(!success && !debug){

        sysmessage(&chatlog, "Program will now exit.");
        render_all(in_progress, cursor_x, cursor_y, &chatlog, scroll_offset);
        cbreak(); // prevent time char input
        getch();

        endwin();
        return 0;
    }

    render_all(in_progress, cursor_x, cursor_y, &chatlog, scroll_offset);

    while(input != "/exit"){

        // handle input here

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
            if(input.at(0) != '/'){

                std::string prefix = "[" + current_time() + "] You: ";
                chatlog.push_back(prefix + input);
            }
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

        if(refresh == ALL){

            render_all(in_progress, cursor_x, cursor_y, &chatlog, scroll_offset);

        }else if(refresh == TEXTBOX_ONLY){

            clear_textbox();
            render_textbox(in_progress);
            move(cursor_y, cursor_x);

        }else if(refresh == CHATBOX_ONLY){

            render_chatlog(&chatlog, scroll_offset);
        }

        update(&chatlog); // we always call this so that we always update at a regular rate
    }

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

    for(unsigned int i = 0; i < chatlog->size(); i++){

        std::string message = chatlog->at(i);
        mvaddstr(i, 0, message.c_str());
    }
}

void update(std::vector<std::string>* chatlog){

    // This function will be at least every tenth of a second
    // Buuut it may be called much more than that, it will be called with each user event
    // So when writing code for it do not write anything that would be performance inhibitive
    // You need to check elapsed time in between certain checks here
}

void sysmessage(std::vector<std::string>* chatlog, std::string message){

    std::string to_push = "--- " + message + " ---";
    chatlog->push_back(to_push);
}
