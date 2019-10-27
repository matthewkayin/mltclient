#include "encode.hpp"

std::string byte_to_binary(char value){

    int count = 0;
    std::string bitstring = "";

    int quantity = (int)value; // convert input to ASCII decimal
    quantity += 128; // convert signed char integer to unsigned
    int mod;

    // convert ASCII decimal to binary
    while(quantity != 0){

        mod = quantity % 2;
        if(mod == 0){

            quantity = (quantity / 2);
            bitstring = "0" + bitstring;

        }else{

            quantity = ((quantity - 1) / 2);
            bitstring = "1" + bitstring;
        }

        count++;
    }

    // Add extra 0s added need so the result is always an 8 bit string
    if(count < 8){

        int extra = 8 - count;
        for(int i = 0; i < extra; i++){

            bitstring = "0" + bitstring;
        }
    }

    return bitstring;
}

char binary_to_byte(std::string bitstring){

    // First we will convert the bitstring into a decimal number
    int byte_num = 0;

    for(int i = 0; i < 8; i++){

        int index = 7 - i;
        byte_num += pow(2, i) * ((int)(bitstring.at(index)) - (int)'0');
    }

    // subtract 128 because we want a signed char not an unsigned
    byte_num -= 128;

    // finally just do a static cast
    return (char)byte_num;
}

std::string to_segments(std::string message){

    // First compress message as and into a c string
    char out[4096];
    int out_size = smaz_compress(message.c_str(), message.length(), out, sizeof(out));

    // then convert each char of the string into binary and put that in an array of bitstrings
    std::string bitstring = "";
    for(int i = 0; i < out_size; i++){

        bitstring = bitstring + byte_to_binary(out[i]);
    }

    return bitstring;
}

std::string from_segments(std::string bitstring){

    // First turn message into a c string
    char in[4096];
    int in_size = bitstring.length() / 8;

    for(unsigned int i = 0; i < (bitstring.length() / 8); i++){

        in[i] = binary_to_byte(bitstring.substr(i * 8, 8));
    }

    // now use smaz to decmopress
    char out[4096];
    int out_size = smaz_decompress(in, in_size, out, sizeof(out));

    std::string message = "";
    for(int i = 0; i < out_size; i++){

        message += out[i];
    }

    return message;
}
