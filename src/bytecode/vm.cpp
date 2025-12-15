#include "vm.hpp"
#include "bytecode/opcode.hpp"



namespace izi {

VM::VM() : stack(), frames() {
    stack.reserve(STACK_MAX);
    frames.reserve(64); // Arbitrary limit on call frames
}

CallFrame* VM::currentFrame() {
    return &frames.back();  
}

uint8_t VM::readByte() {
    CallFrame* frame = currentFrame();
    return *frame->ip++;
}

uint16_t VM::readShort() {
    CallFrame* frame = currentFrame();
    uint16_t high = *frame->ip++;
    uint16_t low = *frame->ip++;
    return (high << 8) | low;

}

Value VM::run(const Chunk& entry) {
    stack.clear();
    frames.clear();

    CallFrame mainFrame{&entry, entry.code.data(), 0};

    frames.push_back(mainFrame);

    try {
        while (true) {
            OpCode op = static_cast<OpCode>() readByte());
            switch (static_cast<OpCode>(op)) {
                case OpCode::CONSTANT: {
                    uint8_t index = readByte();
                    const Value& constant = currentFrame()->chunk->constants[index];
                    push(constant);
                    break;
                }
                case OpCode::NIL:
                    push(Nil{});
                    break;
                case OpCode::TRUE:
                    push(true);
                    break;
                case OpCode::FALSE:
                    push(false);
                    break;
                case OpCode::POP:
                    pop();
                    break;
                case OpCode::PRINT: {
                    Value value = pop();
                    printValue(value);
                    std::cout << '\n';
                    break;
                }
                case OpCode::ADD:
                    binaryNumeric([](double a, double b) { return a + b; });
                    break;
                case OpCode::SUBTRACT:
                    binaryNumeric([](double a, double b) { return a - b; });
                    break;
                case OpCode::MULTIPLY:
                    binaryNumeric([](double a, double b) { return a * b; });
                    break;
                case OpCode::DIVIDE:
                    binaryNumeric([](double a, double b) { return a / b; });
                    break;
                case OpCode::NEGATE: {
                    Value value = pop();
                    push(-asNumber(value));
                    break;
                }
                case OpCode::JUMP: {
                    uint16_t offset = readShort();
                    currentFrame()->ip += offset;
                    break;
                }
                case OpCode::JUMP_IF_FALSE: {
                    uint16_t offset = readShort();
                    Value condition = stack.back();
                    if (!isTruthy(condition)) {
                        currentFrame()->ip += offset;
                    }
                    break;
                }
                case OpCode::LOOP: {
                    uint16_t offset = readShort();
                    currentFrame()->ip -= offset;
                    break;
                }  
                case OpCode::CALL: {
                    uint8_t argCount = readByte();
                    Value callee = stack[stack.size() - 1 - argCount];
                    if (!std::holds_alternative<std::shared_ptr<Callable>>(callee)) {
                        throw std::runtime_error("Can only call functions and classes.");
                    }
                    auto function = std::get<std::shared_ptr<Callable>>(callee);
                    int ar = function->arity();
                    if (ar>=0 && argCount != ar) {
                        throw std::runtime_error("Expected " + std::to_string(function->arity()) +
                                                 " arguments but got " + std::to_string(argCount) + ".");
                    }
                    if(auto nativeFn = std::dynamic_pointer_cast<NativeFunction>(function)) {
                        std::vector<Value> args;
                        args.reserve(argCount);
                        for (size_t i = 0; i < argCount; ++i) {
                            args.push_back(stack[stack.size() - argCount + i]);
                        }

                        // Pop arguments and callee
                        for (size_t i = 0; i < argCount + 1; ++i) {
                            pop();
                        }
                        Value result = nativeFn->call(*this, args);
                        push(result);
                    }else if(auto fn = std::dynamic_pointer_cast<BytecodeFunction>(function)) {
                        CallFrame frame{&fn->chunk, fn->chunk.code.data(), stack.size() - argCount -1};
                        frames.push_back(frame);
                    } 
                    else {
                        throw std::runtime_error("Only native functions are supported in this VM version.");
                    }
                    break;
                }
                case OpCode::RETURN: {
                    Value result = pop();
                    CallFrame finished = frames.back();
                    frames.pop_back();
                    if (frames.empty()) {
                        return result; // Exit the VM
                    }
                    // Clean up the stack
                    stack.resize(finished.stackBase);
                    push(result);
                    break;
                }
                // ... handle other opcodes ...
                default:
                    throw std::runtime_error("Unknown opcode encountered.");
            }
        }
    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << '\n';
        return Nil{};
    }


}