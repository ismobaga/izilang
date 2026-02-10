#include "vm.hpp"
#include "bytecode/opcode.hpp"
#include "bytecode/mv_callable.hpp"



namespace izi {

VM::VM() : stack(), frames() {
    stack.reserve(STACK_MAX);
    frames.reserve(64); // Arbitrary limit on call frames
}

void VM::setGlobal(const std::string& name, const Value& value) {
    globals[name] = value;
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
            OpCode op = static_cast<OpCode>( readByte());
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
                case OpCode::GET_GLOBAL: {
                    uint8_t nameIndex = readByte();
                    const std::string& name = currentFrame()->chunk->names[nameIndex];
                    auto it = globals.find(name);
                    if (it == globals.end()) {
                        throw std::runtime_error("Undefined variable '" + name + "'.");
                    }
                    push(it->second);
                    break;
                }
                case OpCode::SET_GLOBAL: {
                    uint8_t nameIndex = readByte();
                    const std::string& name = currentFrame()->chunk->names[nameIndex];
                    Value value = stack.back(); // Peek at the value
                    globals[name] = value;
                    break;
                }
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
                    if (!std::holds_alternative<std::shared_ptr<VmCallable>>(callee)) {
                        throw std::runtime_error("Can only call VM functions.");
                    }
                    auto function = std::get<std::shared_ptr<VmCallable>>(callee);
                    int ar = function->arity();
                    if (ar >= 0 && argCount != ar) {
                        throw std::runtime_error("Expected " + std::to_string(function->arity()) +
                                                 " arguments but got " + std::to_string(argCount) + ".");
                    }
                    std::vector<Value> args;
                    args.reserve(argCount);
                    for (size_t i = 0; i < argCount; ++i) {
                        args.push_back(stack[stack.size() - argCount + i]);
                    }

                    // Pop arguments and callee
                    for (size_t i = 0; i < argCount + 1; ++i) {
                        pop();
                    }
                    Value result = function->call(*this, args);
                    push(result);
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

void VM::push(Value v) {
    stack.push_back(std::move(v));
}

Value VM::pop() {
    if (stack.empty()) {
        throw std::runtime_error("Stack underflow.");
    }
    Value value = std::move(stack.back());
    stack.pop_back();
    return value;
}

double VM::asNumber(const Value& v) {
    if (!std::holds_alternative<double>(v)) {
        throw std::runtime_error("Expected number.");
    }
    return std::get<double>(v);
}

} // namespace izi