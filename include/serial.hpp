//This class is a wrapper for windows serial commands

#ifndef SERIAL_H
#define SERIAL_H

#include <iostream>
#include <string>
#include <fstream>
#ifdef _WIN32
    #include <windows.h>
#endif

class Serial{

    public:
        Serial();
        void open();
        void close();
        void write(char* data, int no_bytes);
    private:
        std::string location;
        bool opened;
        #ifdef _WIN32
            HANDLE serial_out;
            DCB serial_params;
        #endif
};

#endif
