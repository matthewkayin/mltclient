#include "serial.hpp"

Serial::Serial(){

    opened = false;
}

void Serial::open(){

    if(opened){

        std::cout << "Error! Already opened serial connection!" << std::endl;
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

        std::cout << "Error! Could not detect serial device!" << std::endl;
        return;

    }else{

        std::cout << "Device detected at " << location << std::endl;
    }

    #ifdef _WIN32
        serial_out = CreateFile(("\\\\.\\" + location).c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if(serial_out == INVALID_HANDLE_VALUE){

            std::cout << "Error opening serial connection" << std::endl;
            return;
        }
        serial_params = {0};
        serial_params.DCBlength = sizeof(serial_params);
        GetCommState(serial_out, &serial_params);
        serial_params.BaudRate = CBR_9600;
        serial_params.ByteSize = 8;
        serial_params.StopBits = ONESTOPBIT;
        serial_params.Parity = NOPARITY;
        SetCommState(serial_out, &serial_params);
    #else
        system(("stty -F " + location + " -hupcl").c_str());
    #endif

    opened = true;
}

void Serial::close(){

    if(!opened){

        std::cout << "Error! Cannot close a connection which isn't open" << std::endl;
        return;
    }

    #ifdef _WIN32
        CloseHandle(serial_out);
    #else
        system(("stty -F " + location + " hupcl").c_str());
    #endif
}

void Serial::write(char* data, int no_bytes){

    #ifdef _WIN32
        DWORD no_bytes_to_write = sizeof(char) * no_bytes;
        DWORD no_bytes_written = 0;
        WriteFile(serial_out, data, no_bytes_to_write, &no_bytes_written, NULL);
    #else
        std::ofstream serial_out;
        serial_out.open(location);

        for(int i = 0; i < no_bytes; i++){

            serial_out << data[i];
        }

        serial_out.close();
    #endif
}
