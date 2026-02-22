#pragma once
#include "opcode.hpp"
#include "common/value.hpp"
#include <vector>

namespace izi {
struct Chunk {
    std::vector<uint8_t> code;
    std::vector<Value> constants;
    std::vector<std::string> names;
    std::vector<int> lines;  // Source line number for each bytecode instruction

    void write(uint8_t byte, int line = 0) {
        code.push_back(byte);
        lines.push_back(line);
    }

    size_t addConstant(const Value& value) {
        constants.push_back(value);
        return constants.size() - 1;
    }

    size_t addName(const std::string& name) {
        names.push_back(name);
        return names.size() - 1;
    }

    // Returns the source line for the bytecode at position offset, or 0 if unavailable.
    int getLine(size_t offset) const {
        if (offset < lines.size()) {
            return lines[offset];
        }
        return 0;
    }
};
}  // namespace izi
