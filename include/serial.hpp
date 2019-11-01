//This class is a wrapper for windows serial commands

#ifndef SERIAL_H
#define SERIAL_H

#include <iostream>
#include <string>
#include <fstream>

class Serial{

    public:
        Serial();
        bool open(std::string* message);
        void close();
        void write(char* data, int no_bytes);
    private:
        std::string location;
        bool opened;
};

#endif
