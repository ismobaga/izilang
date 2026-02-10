#pragma once

#include "common/value.hpp"
#include "bytecode/chunk.hpp"
#include <vector>
#include <array>
#include <unordered_map>
#include <string>

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
    
    void setGlobal(const std::string& name, const Value& value);
    
    // void push(Value value);
    // Value pop();
    
private:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, Value> globals;

    CallFrame* currentFrame();
    
    uint8_t readByte();
    uint16_t readShort();

    void push(Value v);
    Value pop();

    static double asNumber(const Value& v);

    template<typename Fn>
    void binaryNumeric(Fn fn) {
        Value b = pop();
        Value a = pop();
        double result = fn(asNumber(a), asNumber(b));
        push(result);
    }
    
};

} // namespace izi