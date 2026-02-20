#pragma once

#include <cstdint>

namespace izi {

enum class OpCode : uint8_t {
    // Constants
    CONSTANT,  // Load a constant
    NIL,  // Push nil
    TRUE,  // Push true
    FALSE,  // Push false

    // Arithmetic operations
    ADD,  // Addition
    SUBTRACT,  // Subtraction
    MULTIPLY,  // Multiplication
    DIVIDE,  // Division
    MODULO,  // Modulo
    NEGATE,  // Unary negation

    // Comparison operations
    EQUAL,  // Equality comparison
    NOT_EQUAL,  // Inequality comparison
    GREATER,  // Greater than
    GREATER_EQUAL,  // Greater than or equal
    LESS,  // Less than
    LESS_EQUAL,  // Less than or equal

    // Logical operations
    NOT,  // Logical NOT

    // Variables
    GET_GLOBAL,  // Get global variable
    SET_GLOBAL,  // Set global variable
    GET_LOCAL,  // Get local variable
    SET_LOCAL,  // Set local variable

    INDEX,  // Indexing operation
    SET_INDEX,  // Set value at index

    // Control flow
    JUMP,  // Unconditional jump
    JUMP_IF_FALSE,  // Conditional jump
    LOOP,  // Loop back

    // Functions
    CALL,  // Call function
    RETURN,  // Return from function

    // Stack manipulation
    POP,  // Pop top of stack
    PRINT,  // Print statement

    // Exception handling
    TRY,  // Set up exception handler (followed by catch offset and finally offset)
    THROW,  // Throw an exception
    END_TRY,  // End try-catch-finally block

    // Class support (v0.3)
    GET_PROPERTY,  // Get a property from an instance (followed by name index)
    SET_PROPERTY,  // Set a property on an instance (followed by name index)
    GET_SUPER_METHOD,  // Get a method from superclass and bind to this
};

}  // namespace izi