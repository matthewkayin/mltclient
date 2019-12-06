#ifndef ENCODE_H
#define ENCODE_H

#include "smaz.hpp"
#include <string>
#include <iostream>
#include <cmath>
#include <iostream>

struct Matrix{

    int rows;
    int columns;
    int** values;
};

Matrix create_matrix(int rows, int columns);
Matrix multiply_matrix(Matrix A, Matrix B);
Matrix mod_matrix(Matrix A, int m);
void print_matrix(Matrix M);

Matrix generation_matrix();
Matrix parity_matrix();
std::string generate_codeword(std::string data);
std::string parse_codeword(std::string codeword);

std::string byte_to_binary(char value);
char binary_to_byte(std::string bitstring);
std::string encode(std::string message); // takes string into bitstring for arduino output
std::string decode(std::string bitstring); // takes bitstirng into string for chatlog display

#endif
