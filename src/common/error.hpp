#pragma once

#include <memory>
#include <string>
#include <vector>
#include <optional>

namespace izi {

// Forward declaration to avoid circular dependency with Value
struct Error;

// Stack frame information for error traces
struct StackFrame {
    std::string functionName;
    std::string fileName;
    int line;
    int column;

    StackFrame(std::string func, std::string file, int ln, int col)
        : functionName(std::move(func)), fileName(std::move(file)), line(ln), column(col) {}
};

// Base Error class for composable, inspectable errors
struct Error {
    std::string message;
    std::string type;
    std::shared_ptr<Error> cause;  // Error chaining
    std::vector<StackFrame> stackTrace;  // Preserved across async boundaries

    // Constructor without cause
    explicit Error(std::string msg, std::string errorType = "Error")
        : message(std::move(msg)), type(std::move(errorType)), cause(nullptr) {}

    // Constructor with cause for error chaining
    Error(std::string msg, std::string errorType, std::shared_ptr<Error> causedBy)
        : message(std::move(msg)), type(std::move(errorType)), cause(std::move(causedBy)) {}

    virtual ~Error() = default;

    // Get full error message including cause chain
    std::string fullMessage() const {
        std::string result = type + ": " + message;
        if (cause) {
            result += "\nCaused by: " + cause->fullMessage();
        }
        return result;
    }

    // Get formatted stack trace
    std::string formatStackTrace() const {
        if (stackTrace.empty()) {
            return "";
        }

        std::string result = "\nStack trace:";
        for (const auto& frame : stackTrace) {
            result += "\n  at " + frame.functionName;
            if (!frame.fileName.empty()) {
                result +=
                    " (" + frame.fileName + ":" + std::to_string(frame.line) + ":" + std::to_string(frame.column) + ")";
            }
        }
        return result;
    }

    // Add a stack frame
    void addStackFrame(const StackFrame& frame) { stackTrace.push_back(frame); }

    // Add a stack frame with parameters
    void addStackFrame(std::string func, std::string file, int line, int col) {
        stackTrace.emplace_back(std::move(func), std::move(file), line, col);
    }
};

// IOError for input/output errors
struct IOError : public Error {
    explicit IOError(std::string msg) : Error(std::move(msg), "IOError") {}

    IOError(std::string msg, std::shared_ptr<Error> causedBy) : Error(std::move(msg), "IOError", std::move(causedBy)) {}
};

// TypeError for type-related errors
struct TypeError : public Error {
    explicit TypeError(std::string msg) : Error(std::move(msg), "TypeError") {}

    TypeError(std::string msg, std::shared_ptr<Error> causedBy)
        : Error(std::move(msg), "TypeError", std::move(causedBy)) {}
};

// ValueError for value-related errors
struct ValueError : public Error {
    explicit ValueError(std::string msg) : Error(std::move(msg), "ValueError") {}

    ValueError(std::string msg, std::shared_ptr<Error> causedBy)
        : Error(std::move(msg), "ValueError", std::move(causedBy)) {}
};

}  // namespace izi
