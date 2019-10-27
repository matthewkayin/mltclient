#ifndef ENCODE_H
#define ENCODE_H

#include "smaz.hpp"
#include <string>
#include <iostream>
#include <cmath>

std::string byte_to_binary(char value);
char binary_to_byte(std::string bitstring);
std::string encode(std::string message);
std::string decode(std::string bitstring);

#endif
