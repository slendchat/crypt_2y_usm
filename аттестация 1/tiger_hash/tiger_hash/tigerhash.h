#pragma once

#ifndef TIGERHASH_TIGERHASH_H
#define TIGERHASH_TIGERHASH_H

#include <sstream>
#include <iostream>
#include <stdint.h>
#include "sboxes.h"
#include "outputs.h"

namespace tiger {
    // 64 bits words for intermediate hash values
    uint64_t _a;
    uint64_t _b;
    uint64_t _c;

    // Buf values
    uint64_t aa;
    uint64_t bb;
    uint64_t cc;

    // Global array for storing block for hash computation
    uint64_t _x[8];

    // Save intermediate hash in buf values
    void save_abc() {
        aa = _a;
        bb = _b;
        cc = _c;
        info("Saving intermediate values: \n\t_a=%s, \n\t_b=%s, \n\t_c=%s\n",
            std::bitset<64>(_a).to_string().c_str(),
            std::bitset<64>(_b).to_string().c_str(),
            std::bitset<64>(_c).to_string().c_str());
    }

    // Function for computing one round of hash
    void round(uint64_t& a, uint64_t& b, uint64_t& c, unsigned int index, char mul) {
        // Convert c 64-bit value to array of bytes
        std::string c_str = std::bitset<64>(c).to_string();
        unsigned char c_arr[8]{ 0 };
        for (std::size_t i = 0, j = 7; i < c_str.length(); i += 8, --j) {
            c_arr[j] = (unsigned char)std::bitset<8>(c_str.substr(i, 8)).to_ulong();
            info("c_arr[%d] = \n\t%s\n", j, std::bitset<64>(c_arr[j]).to_string().c_str());
        }

        info("Before round: \n\ta=%s, \n\tb=%s, \n\tc=%s\n",
            std::bitset<64>(a).to_string().c_str(),
            std::bitset<64>(b).to_string().c_str(),
            std::bitset<64>(c).to_string().c_str());

        c ^= _x[index];
        a -= sboxes::t1[c_arr[0]] ^ sboxes::t2[c_arr[2]] ^ sboxes::t3[c_arr[4]] ^ sboxes::t4[c_arr[6]];
        b -= sboxes::t4[c_arr[1]] ^ sboxes::t3[c_arr[3]] ^ sboxes::t2[c_arr[5]] ^ sboxes::t1[c_arr[7]];
        b *= mul;

        info("After round: \n\ta=%s, \n\tb=%s, \n\tc=%s, \n\tmul=%d, \n\tindex=%u\n",
            std::bitset<64>(a).to_string().c_str(),
            std::bitset<64>(b).to_string().c_str(),
            std::bitset<64>(c).to_string().c_str(),
            mul, index);
    }

    // Function for initiating round computation
    void pass(uint64_t& a, uint64_t& b, uint64_t& c, char mul) {
        round(a, b, c, 0, mul);
        round(b, c, a, 1, mul);
        round(c, a, b, 2, mul);
        round(a, b, c, 3, mul);
        round(b, c, a, 4, mul);
        round(c, a, b, 5, mul);
        round(a, b, c, 6, mul);
        round(b, c, a, 7, mul);
    }

    // Key schedule for modifying _x array
    void key_schedule() {
      for (int i = 0; i < 8; i++){
        info("Before key schedule: _x[%d]=\n\t%s\n",
          i,
          std::bitset<64>(_x[i]).to_string().c_str());
      }

        _x[0] -= _x[7] ^ 0xA5A5A5A5A5A5A5A5;
        _x[1] ^= _x[0];
        _x[2] += _x[1];
        _x[3] -= _x[2] ^ ((~_x[1]) << 19);
        _x[4] ^= _x[3];
        _x[5] += _x[4];
        _x[6] -= _x[5] ^ ((~_x[4]) >> 23);
        _x[7] ^= _x[6];
        _x[0] += _x[7];
        _x[1] -= _x[0] ^ ((~_x[7]) << 19);
        _x[2] ^= _x[1];
        _x[3] += _x[2];
        _x[4] -= _x[3] ^ ((~_x[2]) >> 23);
        _x[5] ^= _x[4];
        _x[6] += _x[5];
        _x[7] -= _x[6] ^ 0x0123456789ABCDEF;

        for (int i = 0; i < 8; i++) {
          info("After key schedule: _x[%d]=\n\t%s\n",
            i,
            std::bitset<64>(_x[i]).to_string().c_str());
        }

    }
    

    // Feedforward step
    void feedforward() {
        info("Before feedforward: _a=%s, _b=%s, _c=%s\n",
            std::bitset<64>(_a).to_string().c_str(),
            std::bitset<64>(_b).to_string().c_str(),
            std::bitset<64>(_c).to_string().c_str());

        _a ^= aa;
        _b -= bb;
        _c += cc;

        info("After feedforward: _a=%s, _b=%s, _c=%s\n",
            std::bitset<64>(_a).to_string().c_str(),
            std::bitset<64>(_b).to_string().c_str(),
            std::bitset<64>(_c).to_string().c_str());
    }

    // Convert 64-bit value to hex string
    std::string to_hex(uint64_t value) {
        std::stringstream stream;
        stream << std::hex << value;
        std::string result(stream.str());

        while (result.length() != 16) {
            result = '0' + result;
        }

        return result;
    }

    // Compute Tiger hash from input string
    std::string hash(std::string value) {
        // Basic hash initialization
        _a = 0x0123456789ABCDEF;
        _b = 0xFEDCBA9876543210;
        _c = 0xF096A5B4C3B2E187;

        okay("Initialized hash values: _a=%s, _b=%s, _c=%s\n",
            std::bitset<64>(_a).to_string().c_str(),
            std::bitset<64>(_b).to_string().c_str(),
            std::bitset<64>(_c).to_string().c_str());

        // Basic buffer initialization
        aa = 0;
        bb = 0;
        cc = 0;

        // Pad input message to 512-bit blocks
        while (value.length() % 64 != 0) {
            value = char(0) + value;
        }

        // Process each 512-bit block
        for (std::size_t k = 0; k < value.length(); k += 64) {
            std::string buf_str = value.substr(k, 64);
            
            // Divide the 512-bit block into eight 64-bit blocks
            for (std::size_t i = 0, j = 7; i < buf_str.length(); i += 8, --j) {
                std::string bit_str;
                std::string str_word = buf_str.substr(i, 8);
                for (int n = 0; n < 8; ++n) {
                    bit_str += std::bitset<8>(str_word[n]).to_string();
                }
                _x[j] = std::bitset<64>(bit_str).to_ullong();
            }

            info("Processing block %zu: _x[0]=%s, _x[7]=%s\n", k / 64,
                std::bitset<64>(_x[0]).to_string().c_str(),
                std::bitset<64>(_x[7]).to_string().c_str());

            // Compute intermediate hash values for the block
            info("round [%d]\n", k % 64);
            save_abc();
            pass(_a, _b, _c, 5);
            key_schedule();
            pass(_c, _a, _b, 7);
            key_schedule();
            pass(_b, _c, _a, 9);
            feedforward();
        }

        // Return the final hash result in hexadecimal form
        return to_hex(_a) + to_hex(_b) + to_hex(_c);
    }
}

#endif //TIGERHASH_TIGERHASH_H