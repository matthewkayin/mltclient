#include "encode.hpp"
#include <ncurses.h>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <ctime>

void render_all(std::string in_progress, int cursor_x, int cursor_y, std::vector<std::string>* chatlog, int scroll_offset);
void render_separator();
void render_textbox(std::string in_progress);

// Functions to act as global variables
int textbox_height(); // returns textbox height needed for 140 chars
int separator_point(); // returns row to render line seperator between two windows
int chatlog_height(); // returns chatlog height in number of rows
int chat_segment_width();
std::string current_time();

int get_cursor_index(int cursor_x, int cursor_y);

void render_chatlog(std::vector<std::string>* chatlog, int scroll_offset);
void append_chatlog(std::vector<std::string>* chatlog, std::string message);

int main(){

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    clear();
    render_separator();

    std::vector<std::string> chatlog;
    std::string in_progress = "";
    std::string input = "";
    int cursor_x = 0;
    int cursor_y = separator_point() + 1;
    int scroll_offset = 0;

    while(input != "/exit"){

        // handle input here

        bool render = false;
        int key = getch();
        if(key == KEY_RESIZE){

            render = true;

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
            render = true;

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
                render = true;
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
                render = true;
            }
        }

        if(render){

            render_all(in_progress, cursor_x, cursor_y, &chatlog, scroll_offset);
        }
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
