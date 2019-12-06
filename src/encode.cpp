#include "encode.hpp"

Matrix create_matrix(int rows, int columns){

    Matrix M;
    M.rows = rows;
    M.columns = columns;
    M.values = new int*[rows];
    for(int i = 0; i < rows; i++){

        M.values[i] = new int[columns];
        for(int j = 0; j < columns; j++){

            M.values[i][j] = 0;
        }
    }

    return M;
}

Matrix multiply_matrix(Matrix A, Matrix B){

    if(A.columns != B.rows){

        std::cout << "Cannot multiply! Matrix dimensions do not match" << std::endl;
        return create_matrix(1, 1);
    }

    Matrix C = create_matrix(A.rows, B.columns);
    for(int i = 0; i < A.rows; i++){

        for(int j = 0; j < B.columns; j++){

            int value = 0;
            for(int k = 0; k < A.columns; k++){

                value += A.values[i][k] * B.values[k][j];
            }

            C.values[i][j] = value;
        }
    }

    return C;
}

Matrix mod_matrix(Matrix A, int m){

    Matrix M = create_matrix(A.rows, A.columns);
    for(int i = 0; i < A.rows; i++){

        for(int j = 0; j < A.columns; j++){

            M.values[i][j] = A.values[i][j] % m;
        }
    }

    return M;
}

void print_matrix(Matrix M){

    for(int i = 0; i < M.rows; i++){

        std::cout << "[ ";

        for(int j = 0; j < M.columns; j++){

            std::cout << M.values[i][j] << " ";
        }

        std::cout << "]" << std::endl;
    }
}

Matrix generation_matrix(){

    Matrix G = create_matrix(7, 4);

    G.values[0][0] = 1;
    G.values[0][1] = 1;
    G.values[0][2] = 0;
    G.values[0][3] = 1;

    G.values[1][0] = 1;
    G.values[1][1] = 0;
    G.values[1][2] = 1;
    G.values[1][3] = 1;

    G.values[2][0] = 1;
    G.values[2][1] = 0;
    G.values[2][2] = 0;
    G.values[2][3] = 0;

    G.values[3][0] = 0;
    G.values[3][1] = 1;
    G.values[3][2] = 1;
    G.values[3][3] = 1;

    G.values[4][0] = 0;
    G.values[4][1] = 1;
    G.values[4][2] = 0;
    G.values[4][3] = 0;

    G.values[5][0] = 0;
    G.values[5][1] = 0;
    G.values[5][2] = 1;
    G.values[5][3] = 0;

    G.values[6][0] = 0;
    G.values[6][1] = 0;
    G.values[6][2] = 0;
    G.values[6][3] = 1;

    return G;
}

Matrix parity_matrix(){

    Matrix H = create_matrix(3, 7);

    H.values[0][0] = 1;
    H.values[0][1] = 0;
    H.values[0][2] = 1;
    H.values[0][3] = 0;
    H.values[0][4] = 1;
    H.values[0][5] = 0;
    H.values[0][6] = 1;

    H.values[1][0] = 0;
    H.values[1][1] = 1;
    H.values[1][2] = 1;
    H.values[1][3] = 0;
    H.values[1][4] = 0;
    H.values[1][5] = 1;
    H.values[1][6] = 1;

    H.values[2][0] = 0;
    H.values[2][1] = 0;
    H.values[2][2] = 0;
    H.values[2][3] = 1;
    H.values[2][4] = 1;
    H.values[2][5] = 1;
    H.values[2][6] = 1;

    return H;
}

std::string generate_codeword(std::string data){

    Matrix G = generation_matrix();
    Matrix p = create_matrix(4, 1);
    for(int i = 0; i < 4; i++){

        p.values[i][0] = ((int)data.at(i)) - ((int)'0');
    }

    Matrix x = mod_matrix(multiply_matrix(G, p), 2);

    std::string codeword = "";
    for(int i = 0; i < 7; i++){

        codeword += std::to_string(x.values[i][0]);
    }

    return codeword;
}

std::string parse_codeword(std::string codeword){

    Matrix H = parity_matrix();
    Matrix r = create_matrix(7, 1);
    for(int i = 0; i < 7; i++){

        r.values[i][0] = ((int)codeword.at(i)) - ((int)'0');
    }

    Matrix z = mod_matrix(multiply_matrix(H, r), 2);
    int error_index = (4 * z.values[2][0]) + (2 * z.values[1][0]) + (1 * z.values[0][0]);

    std::string data = "";

    for(int i = 0; i < 7; i++){

        if(i != 0 && i != 1 && i != 3){

            char to_add = codeword.at(i);
            if(i + 1 == error_index){

                if(to_add == '0'){

                    to_add = '1';

                }else{

                    to_add = '0';
                }
            }
            data += to_add;
        }
    }

    return data;
}

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

std::string encode(std::string message){

    // First compress message as and into a c string
    char out_buffer[4096];
    int out_size = smaz_compress(message.c_str(), message.length(), out_buffer, sizeof(out_buffer));

    // then apply hamming codes
    std::string output = "";
    for(int i = 0; i < out_size; i++){

        std::string data = byte_to_binary(out_buffer[i]);
        std::string codeword = generate_codeword(data.substr(0, 4));
        output += codeword;
        codeword = generate_codeword(data.substr(4, 4));
        output += codeword;
    }

    return output;
}

std::string decode(std::string bitstring){

    // First do hamming checking to get the data`
    std::string input = "";
    for(int i = 0; i < bitstring.length() / 7; i++){

        std::string code = bitstring.substr(i * 7, 7);
        std::string data = parse_codeword(code);
        input += data;
    }

    // First turn message into a c string
    char in[4096];
    int in_size = input.length() / 8;

    for(unsigned int i = 0; i < (input.length() / 8); i++){

        in[i] = binary_to_byte(input.substr(i * 8, 8));
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
