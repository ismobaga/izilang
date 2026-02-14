#include "vm.hpp"
#include "bytecode/opcode.hpp"
#include "bytecode/mv_callable.hpp"
#include "bytecode/vm_class.hpp"
#include "interp/izi_class.hpp"
#include <cmath>



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
    bool wasRunning = isRunning;
    isRunning = true;
    
    if (!wasRunning) {
        stack.clear();
        frames.clear();
    }

    size_t startingFrameCount = frames.size();
    CallFrame mainFrame{&entry, entry.code.data(), stack.size()};

    frames.push_back(mainFrame);
    
    // Check for stack overflow
    if (frames.size() > MAX_CALL_FRAMES) {
        isRunning = wasRunning;
        throw std::runtime_error("Stack overflow: Maximum call depth of " + 
                               std::to_string(MAX_CALL_FRAMES) + " exceeded.");
    }

    while (true) {
        try {
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
                    
                    std::shared_ptr<VmCallable> function;
                    if (std::holds_alternative<std::shared_ptr<VmCallable>>(callee)) {
                        function = std::get<std::shared_ptr<VmCallable>>(callee);
                    } else if (std::holds_alternative<std::shared_ptr<VmClass>>(callee)) {
                        function = std::get<std::shared_ptr<VmClass>>(callee);
                    } else {
                        throw std::runtime_error("Can only call VM functions and classes.");
                    }
                    
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
                    
                    // Check if we've returned from the frame we pushed in this run() call
                    if (frames.size() == startingFrameCount) {
                        isRunning = wasRunning;
                        return result;
                    }
                    
                    if (frames.empty()) {
                        isRunning = wasRunning;
                        return result; // Exit the VM
                    }
                    // Clean up the stack
                    stack.resize(finished.stackBase);
                    push(result);
                    break;
                }
                case OpCode::EQUAL: {
                    Value b = pop();
                    Value a = pop();
                    push(a == b);
                    break;
                }
                case OpCode::NOT_EQUAL: {
                    Value b = pop();
                    Value a = pop();
                    push(a != b);
                    break;
                }
                case OpCode::GREATER:
                    binaryComparison([](double a, double b) { return a > b; });
                    break;
                case OpCode::GREATER_EQUAL:
                    binaryComparison([](double a, double b) { return a >= b; });
                    break;
                case OpCode::LESS:
                    binaryComparison([](double a, double b) { return a < b; });
                    break;
                case OpCode::LESS_EQUAL:
                    binaryComparison([](double a, double b) { return a <= b; });
                    break;
                case OpCode::NOT: {
                    Value value = pop();
                    push(!isTruthy(value));
                    break;
                }
                case OpCode::GET_LOCAL: {
                    uint8_t slot = readByte();
                    push(stack[currentFrame()->stackBase + slot]);
                    break;
                }
                case OpCode::SET_LOCAL: {
                    uint8_t slot = readByte();
                    stack[currentFrame()->stackBase + slot] = stack.back();
                    break;
                }
                case OpCode::INDEX: {
                    Value index = pop();
                    Value collection = pop();
                    
                    if (std::holds_alternative<std::shared_ptr<Array>>(collection)) {
                        auto arr = std::get<std::shared_ptr<Array>>(collection);
                        if (!std::holds_alternative<double>(index)) {
                            throw std::runtime_error("Array index must be a number.");
                        }
                        size_t idx = validateArrayIndex(std::get<double>(index));
                        if (idx >= arr->elements.size()) {
                            throw std::runtime_error("Array index out of bounds.");
                        }
                        push(arr->elements[idx]);
                    } else if (std::holds_alternative<std::shared_ptr<Map>>(collection)) {
                        auto map = std::get<std::shared_ptr<Map>>(collection);
                        if (!std::holds_alternative<std::string>(index)) {
                            throw std::runtime_error("Map key must be a string.");
                        }
                        const std::string& key = std::get<std::string>(index);
                        auto it = map->entries.find(key);
                        if (it == map->entries.end()) {
                            push(Nil{});
                        } else {
                            push(it->second);
                        }
                    } else {
                        throw std::runtime_error("Can only index arrays and maps.");
                    }
                    break;
                }
                case OpCode::SET_INDEX: {
                    Value value = pop();
                    Value index = pop();
                    Value collection = pop();
                    
                    if (std::holds_alternative<std::shared_ptr<Array>>(collection)) {
                        auto arr = std::get<std::shared_ptr<Array>>(collection);
                        if (!std::holds_alternative<double>(index)) {
                            throw std::runtime_error("Array index must be a number.");
                        }
                        size_t idx = validateArrayIndex(std::get<double>(index));
                        if (idx >= arr->elements.size()) {
                            throw std::runtime_error("Array index out of bounds.");
                        }
                        arr->elements[idx] = value;
                        push(value);
                    } else if (std::holds_alternative<std::shared_ptr<Map>>(collection)) {
                        auto map = std::get<std::shared_ptr<Map>>(collection);
                        if (!std::holds_alternative<std::string>(index)) {
                            throw std::runtime_error("Map key must be a string.");
                        }
                        const std::string& key = std::get<std::string>(index);
                        map->entries[key] = value;
                        push(value);
                    } else {
                        throw std::runtime_error("Can only index arrays and maps.");
                    }
                    break;
                }
                case OpCode::TRY: {
                    // TRY opcode followed by:
                    // - 2 bytes: offset to catch block (0 if no catch)
                    // - 2 bytes: offset to finally block (0 if no finally)
                    // - 1 byte: name index for catch variable (0 if no catch)
                    uint16_t catchOffset = readShort();
                    uint16_t finallyOffset = readShort();
                    uint8_t catchVarIndex = readByte();
                    
                    ExceptionHandler handler;
                    handler.frameIndex = frames.size() - 1;
                    handler.stackSize = stack.size();
                    
                    // Calculate absolute instruction pointers
                    if (catchOffset > 0) {
                        handler.catchIp = currentFrame()->ip + catchOffset;
                        handler.catchVariable = currentFrame()->chunk->names[catchVarIndex];
                    } else {
                        handler.catchIp = nullptr;
                        handler.catchVariable = "";
                    }
                    
                    if (finallyOffset > 0) {
                        handler.finallyIp = currentFrame()->ip + finallyOffset;
                    } else {
                        handler.finallyIp = nullptr;
                    }
                    
                    exceptionHandlers.push_back(handler);
                    break;
                }
                case OpCode::THROW: {
                    // Pop the exception value from stack and throw it
                    Value exception = pop();
                    throwException(exception);
                    // If throwException returns, it means exception was handled
                    // Continue execution will be at the catch/finally block
                    break;
                }
                case OpCode::END_TRY: {
                    // End of try-catch-finally block
                    // Pop the exception handler from the stack
                    if (!exceptionHandlers.empty()) {
                        exceptionHandlers.pop_back();
                    }
                    break;
                }
                case OpCode::GET_PROPERTY: {
                    uint8_t nameIndex = readByte();
                    const std::string& propertyName = currentFrame()->chunk->names[nameIndex];
                    Value object = pop();
                    
                    if (!std::holds_alternative<std::shared_ptr<Instance>>(object)) {
                        throw std::runtime_error("Only instances have properties.");
                    }
                    
                    auto instance = std::get<std::shared_ptr<Instance>>(object);
                    
                    // Check if it's a field
                    auto fieldIt = instance->fields.find(propertyName);
                    if (fieldIt != instance->fields.end()) {
                        push(fieldIt->second);
                        break;
                    }
                    
                    // Check if it's a method
                    if (std::holds_alternative<std::shared_ptr<VmClass>>(instance->klass)) {
                        auto klass = std::get<std::shared_ptr<VmClass>>(instance->klass);
                        auto method = klass->getMethod(propertyName, instance);
                        if (method) {
                            push(method);
                            break;
                        }
                    }
                    
                    throw std::runtime_error("Undefined property '" + propertyName + "'.");
                }
                case OpCode::SET_PROPERTY: {
                    uint8_t nameIndex = readByte();
                    const std::string& propertyName = currentFrame()->chunk->names[nameIndex];
                    Value value = pop();
                    Value object = pop();
                    
                    if (!std::holds_alternative<std::shared_ptr<Instance>>(object)) {
                        throw std::runtime_error("Only instances have properties.");
                    }
                    
                    auto instance = std::get<std::shared_ptr<Instance>>(object);
                    instance->fields[propertyName] = value;
                    push(value); // Assignment expression returns the value
                    break;
                }
                // ... handle other opcodes ...
                default:
                    throw std::runtime_error("Unknown opcode encountered.");
            }
        } catch (const std::runtime_error& e) {
            // Convert C++ exception to IziLang exception and try to handle it
            Value exception = std::string(e.what());
            
            // Try to handle the exception through the exception handler stack
            if (handleException(exception)) {
                // Exception was handled, continue execution from catch/finally block
                // The handleException already updated the IP
                continue;  // Continue the while loop
            } else {
                // No handler found, report error and exit
                std::cerr << "Uncaught Runtime Error: " << e.what() << '\n';
                isRunning = wasRunning;
                return Nil{};
            }
        }
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

size_t VM::validateArrayIndex(double index) {
    if (index < 0) {
        throw std::runtime_error("Array index must be non-negative.");
    }
    if (index != std::trunc(index)) {
        throw std::runtime_error("Array index must be a whole number.");
    }
    return static_cast<size_t>(index);
}

void VM::throwException(const Value& exception) {
    // Try to handle the exception
    if (!handleException(exception)) {
        // No handler found, convert to C++ exception to be caught by outer catch block
        std::string msg = "Uncaught exception: ";
        if (std::holds_alternative<std::string>(exception)) {
            msg += std::get<std::string>(exception);
        } else if (std::holds_alternative<double>(exception)) {
            msg += std::to_string(std::get<double>(exception));
        } else if (std::holds_alternative<bool>(exception)) {
            msg += std::get<bool>(exception) ? "true" : "false";
        } else {
            msg += "<value>";
        }
        throw std::runtime_error(msg);
    }
}

bool VM::handleException(const Value& exception) {
    // Search for an exception handler in the current call frame
    while (!exceptionHandlers.empty()) {
        ExceptionHandler& handler = exceptionHandlers.back();
        
        // Check if handler is in current frame
        if (handler.frameIndex != frames.size() - 1) {
            // Handler is in a different frame, pop it
            exceptionHandlers.pop_back();
            continue;
        }
        
        // Restore stack to the state when try block was entered
        stack.resize(handler.stackSize);
        
        // If there's a catch block, jump to it
        if (handler.catchIp != nullptr) {
            // Push exception value to stack
            push(exception);
            
            // Set the catch variable as a global
            // NOTE: This is a simplification - ideally catch variables should be local to the catch block
            // However, the current VM implementation uses globals for all variables accessed by name
            globals[handler.catchVariable] = exception;
            
            // Jump to catch block
            currentFrame()->ip = handler.catchIp;
            
            // The END_TRY opcode will clean up the handler
            return true;
        }
        
        // If there's only a finally block (no catch), jump to it
        // NOTE: In a full implementation, the exception should be re-thrown after finally executes
        // For this initial implementation, we execute the finally block and consider the exception handled
        if (handler.finallyIp != nullptr) {
            // Jump to finally block
            currentFrame()->ip = handler.finallyIp;
            return true;
        }
        
        // No catch or finally, pop handler and continue searching
        exceptionHandlers.pop_back();
    }
    
    // No handler found
    return false;
}


} // namespace izi