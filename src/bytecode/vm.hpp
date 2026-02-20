#pragma once

#include "common/value.hpp"
#include "bytecode/chunk.hpp"
#include <vector>
#include <array>
#include <unordered_map>
#include <string>

namespace izi {

constexpr size_t STACK_MAX = 256;
constexpr size_t MAX_CALL_FRAMES = 256;  // Maximum call depth for stack overflow protection

struct CallFrame {
    const Chunk* chunk;
    const uint8_t* ip;  // Instruction pointer
    size_t stackBase;  // Start index in the VM stack for this call frame
};

// Exception handler for try-catch-finally blocks
struct ExceptionHandler {
    size_t frameIndex;  // Index of the call frame where this handler was set up
    const uint8_t* catchIp;  // Instruction pointer to catch block (nullptr if no catch)
    const uint8_t* finallyIp;  // Instruction pointer to finally block (nullptr if no finally)
    size_t stackSize;  // Stack size when handler was set up
    std::string catchVariable;  // Variable name to bind exception to in catch block
};

class VM {
   public:
    VM();

    Value run(const Chunk& chunk);

    void setGlobal(const std::string& name, const Value& value);

    // Runtime safety limits
    size_t getCallDepth() const { return frames.size(); }
    size_t getStackSize() const { return stack.size(); }

    // Get all global variables (for REPL :vars command)
    const std::unordered_map<std::string, Value>& getGlobals() const { return globals; }

    // void push(Value value);
    // Value pop();

   private:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
    std::unordered_map<std::string, Value> globals;
    std::vector<ExceptionHandler> exceptionHandlers;  // Stack of exception handlers
    bool isRunning = false;

    CallFrame* currentFrame();

    uint8_t readByte();
    uint16_t readShort();

    void push(Value v);
    Value pop();

    // Exception handling helpers
    void throwException(const Value& exception);
    bool handleException(const Value& exception);

    static double asNumber(const Value& v);
    static size_t validateArrayIndex(double index);

    template <typename Fn>
    void binaryNumeric(Fn fn) {
        Value b = pop();
        Value a = pop();
        double result = fn(asNumber(a), asNumber(b));
        push(result);
    }

    template <typename Fn>
    void binaryComparison(Fn fn) {
        Value b = pop();
        Value a = pop();
        bool result = fn(asNumber(a), asNumber(b));
        push(result);
    }
};

}  // namespace izi