#include "catch.hpp"
#include "common/error.hpp"
#include "common/value.hpp"
#include <memory>

using namespace izi;

TEST_CASE("Error: Basic error creation", "[error]") {
    SECTION("Create base Error with message") {
        auto err = std::make_shared<Error>("Something went wrong");
        REQUIRE(err->message == "Something went wrong");
        REQUIRE(err->type == "Error");
        REQUIRE(err->cause == nullptr);
        REQUIRE(err->stackTrace.empty());
    }

    SECTION("Error full message without cause") {
        auto err = std::make_shared<Error>("Test error", "CustomError");
        REQUIRE(err->fullMessage() == "CustomError: Test error");
    }
}

TEST_CASE("Error: Named error types", "[error]") {
    SECTION("Create IOError") {
        auto err = std::make_shared<IOError>("file not found");
        REQUIRE(err->message == "file not found");
        REQUIRE(err->type == "IOError");
        REQUIRE(err->cause == nullptr);
    }

    SECTION("Create TypeError") {
        auto err = std::make_shared<TypeError>("expected number, got string");
        REQUIRE(err->message == "expected number, got string");
        REQUIRE(err->type == "TypeError");
        REQUIRE(err->cause == nullptr);
    }

    SECTION("Create ValueError") {
        auto err = std::make_shared<ValueError>("invalid value: -1");
        REQUIRE(err->message == "invalid value: -1");
        REQUIRE(err->type == "ValueError");
        REQUIRE(err->cause == nullptr);
    }
}

TEST_CASE("Error: Error chaining with cause", "[error]") {
    SECTION("Chain two errors") {
        auto rootCause = std::make_shared<IOError>("connection timeout");
        auto topError = std::make_shared<ValueError>("failed to fetch data", rootCause);

        REQUIRE(topError->cause != nullptr);
        REQUIRE(topError->cause->message == "connection timeout");
        REQUIRE(topError->cause->type == "IOError");
    }

    SECTION("Full message with cause chain") {
        auto rootCause = std::make_shared<IOError>("disk full");
        auto midError = std::make_shared<Error>("write failed", "WriteError", rootCause);
        auto topError = std::make_shared<ValueError>("save failed", midError);

        std::string fullMsg = topError->fullMessage();
        REQUIRE(fullMsg.find("ValueError: save failed") != std::string::npos);
        REQUIRE(fullMsg.find("WriteError: write failed") != std::string::npos);
        REQUIRE(fullMsg.find("IOError: disk full") != std::string::npos);
    }

    SECTION("Error chaining preserves cause") {
        auto cause1 = std::make_shared<IOError>("network error");
        auto cause2 = std::make_shared<TypeError>("type mismatch", cause1);
        auto finalError = std::make_shared<ValueError>("validation failed", cause2);

        REQUIRE(finalError->cause != nullptr);
        REQUIRE(finalError->cause->cause != nullptr);
        REQUIRE(finalError->cause->cause->type == "IOError");
    }
}

TEST_CASE("Error: Stack traces", "[error]") {
    SECTION("Add stack frames to error") {
        auto err = std::make_shared<Error>("runtime error");

        err->addStackFrame("main", "main.izi", 10, 5);
        err->addStackFrame("helper", "utils.izi", 25, 12);

        REQUIRE(err->stackTrace.size() == 2);
        REQUIRE(err->stackTrace[0].functionName == "main");
        REQUIRE(err->stackTrace[0].fileName == "main.izi");
        REQUIRE(err->stackTrace[0].line == 10);
        REQUIRE(err->stackTrace[0].column == 5);

        REQUIRE(err->stackTrace[1].functionName == "helper");
        REQUIRE(err->stackTrace[1].fileName == "utils.izi");
        REQUIRE(err->stackTrace[1].line == 25);
        REQUIRE(err->stackTrace[1].column == 12);
    }

    SECTION("Format stack trace") {
        auto err = std::make_shared<IOError>("file not found");
        err->addStackFrame("readFile", "io.izi", 42, 8);
        err->addStackFrame("processData", "main.izi", 15, 3);

        std::string trace = err->formatStackTrace();
        REQUIRE(trace.find("Stack trace:") != std::string::npos);
        REQUIRE(trace.find("at readFile") != std::string::npos);
        REQUIRE(trace.find("io.izi:42:8") != std::string::npos);
        REQUIRE(trace.find("at processData") != std::string::npos);
        REQUIRE(trace.find("main.izi:15:3") != std::string::npos);
    }

    SECTION("Empty stack trace returns empty string") {
        auto err = std::make_shared<Error>("no trace");
        REQUIRE(err->formatStackTrace() == "");
    }

    SECTION("Add stack frame using StackFrame object") {
        auto err = std::make_shared<TypeError>("type error");
        StackFrame frame("validate", "validator.izi", 100, 20);
        err->addStackFrame(frame);

        REQUIRE(err->stackTrace.size() == 1);
        REQUIRE(err->stackTrace[0].functionName == "validate");
    }
}

TEST_CASE("Error: Integration with Value variant", "[error][value]") {
    SECTION("Error as Value") {
        auto err = std::make_shared<IOError>("test error");
        Value v = err;

        REQUIRE(std::holds_alternative<std::shared_ptr<Error>>(v));
        auto extractedErr = std::get<std::shared_ptr<Error>>(v);
        REQUIRE(extractedErr->message == "test error");
        REQUIRE(extractedErr->type == "IOError");
    }

    SECTION("getTypeName returns 'error' for Error values") {
        auto err = std::make_shared<ValueError>("value error");
        Value v = err;

        REQUIRE(getTypeName(v) == "error");
    }

    SECTION("isTruthy returns true for Error values") {
        auto err = std::make_shared<TypeError>("type error");
        Value v = err;

        REQUIRE(isTruthy(v) == true);
    }

    SECTION("valueToString formats error properly") {
        auto rootCause = std::make_shared<IOError>("disk error");
        auto err = std::make_shared<ValueError>("invalid input", rootCause);
        Value v = err;

        std::string str = valueToString(v);
        REQUIRE(str.find("ValueError: invalid input") != std::string::npos);
        REQUIRE(str.find("IOError: disk error") != std::string::npos);
    }
}

TEST_CASE("Error: Stack traces preserved across error chain", "[error]") {
    SECTION("Multiple errors with different stack traces") {
        auto cause = std::make_shared<IOError>("network timeout");
        cause->addStackFrame("connect", "network.izi", 50, 10);

        auto topError = std::make_shared<ValueError>("operation failed", cause);
        topError->addStackFrame("performOperation", "main.izi", 20, 5);

        // Each error maintains its own stack trace
        REQUIRE(cause->stackTrace.size() == 1);
        REQUIRE(topError->stackTrace.size() == 1);
        REQUIRE(cause->stackTrace[0].functionName == "connect");
        REQUIRE(topError->stackTrace[0].functionName == "performOperation");
    }
}

TEST_CASE("Error: Type system completeness", "[error]") {
    SECTION("All error types derive from Error") {
        auto ioErr = std::make_shared<IOError>("io");
        auto typeErr = std::make_shared<TypeError>("type");
        auto valErr = std::make_shared<ValueError>("value");

        // All should be convertible to base Error pointer
        std::shared_ptr<Error> base1 = ioErr;
        std::shared_ptr<Error> base2 = typeErr;
        std::shared_ptr<Error> base3 = valErr;

        REQUIRE(base1->type == "IOError");
        REQUIRE(base2->type == "TypeError");
        REQUIRE(base3->type == "ValueError");
    }
}
