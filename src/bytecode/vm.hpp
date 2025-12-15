#pragma once

#include "common/value.hpp"
#include "bytecode/chunk.hpp"
#include <vector>
#include <array>

namespace izi {

constexpr size_t STACK_MAX = 256;

struct CallFrame {
    const Chunk* chunk;
    const uint8_t* ip; // Instruction pointer
    size_t stackBase;  // Start index in the VM stack for this call frame

};

class VM {
public:
    VM();
    
    
    Value run(const Chunk& chunk);
    
    void push(Value value);
    Value pop();
    
private:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;

    CallFrame* currentFrame();
    
    uint8_t readByte();
    uint16_t readShort();

    void push(Value v);
    Value pop();

    static double asNumber(const Value& v);

    template<typename Fn>
    void binaryNumeric(Fn fn);
    
};

} // namespace izi