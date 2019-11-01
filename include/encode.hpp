#ifndef ENCODE_H
#define ENCODE_H

#include "smaz.hpp"
#include <string>
#include <iostream>
#include <cmath>

std::string byte_to_binary(char value);
char binary_to_byte(std::string bitstring);
int encode(std::string message, char* out); // takes string into bitstring for arduino output
std::string decode(std::string bitstring); // takes bitstirng into string for chatlog display

#endif
