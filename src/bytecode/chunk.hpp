#pragma once
#include "opcode.hpp"
#include "common/value.hpp"
#include <vector>

namespace izi {
    struct Chunk {
        std::vector<uint8_t> code;
        std::vector<Value> constants;

        void write(uint8_t byte) {
            code.push_back(byte);
        }

        size_t addConstant(const Value& value) {
            constants.push_back(value);
            return constants.size() - 1;
        }
        
    };
}
