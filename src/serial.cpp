#include "serial.hpp"

Serial::Serial(){

    opened = false;
}

bool Serial::open(std::string* message){

    if(opened){

        *message = "Error! Already opened serial connection!";
        return false;
    }

    std::string base_location_name = "";
    #ifdef _WIN32
        base_location_name = "COM";
    #else
        base_location_name = "/dev/ttyACM";
    #endif

    int attempts = 0;

    while(location == "" && attempts < 5){

        attempts++;
        for(int i = 0; i < 256; i++){

            std::string check_name = base_location_name + std::to_string(i);
            std::ifstream serial_check(check_name.c_str());
            if(serial_check.good()){

                location = check_name;
            }
        }
    }

    if(location == ""){

        *message = "Error! Could not detect serial device!";
        return false;

    }else{

        *message = "Success! Device detected at " + location;
    }

    #ifdef _WIN32
    #else
        system(("stty -F " + location + " -hupcl").c_str());
    #endif

    opened = true;
    return true;
}

void Serial::close(){

    if(!opened){

        std::cout << "Error! Cannot close a connection which isn't open" << std::endl;
        return;
    }

    #ifdef _WIN32
    #else
        system(("stty -F " + location + " hupcl").c_str());
    #endif
}

void Serial::write(char* data, int no_bytes){

    std::ofstream serial_out;
    serial_out.open(location);

    for(int i = 0; i < no_bytes; i++){

        serial_out << data[i];
    }

    serial_out.close();
}
