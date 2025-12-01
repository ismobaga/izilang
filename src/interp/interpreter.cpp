
#include "interpreter.hpp"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <string>

#include "ast/expr.hpp"
#include "ast/visitor.hpp"
#include "common/callable.hpp"
#include "common/token.hpp"
#include "common/value.hpp"
#include "interp/native.hpp"
#include "parse/parser.hpp"
#include "parse/lexer.hpp"
namespace izi {

Interpreter::Interpreter() : globals(nullptr), env(&globals) {
registerNativeFunctions(*this);

}

void Interpreter::interpret(const std::vector<StmtPtr>& program) {
    try {
        for (auto& s : program) {
            if (s) {  // Skip null statements from parser errors
                execute(*s);
            }
        }

    } catch (const std::runtime_error& e) {
        std::cerr << "Runtime Error: " << e.what() << '\n';
    }
}

Value Interpreter::evaluate(Expr& expr) {
    return expr.accept(*this);
}
void Interpreter::execute(Stmt& stmt) {
    stmt.accept(*this);
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements, Environment* newEnv) {
    Environment* previous = env;
    env = newEnv;

    try {
        for (const auto& stmt : statements) {
            execute(*stmt);
        }
    } catch (...) {
        env = previous;
        throw;
    }

    env = previous;
}

Value Interpreter::visit(BinaryExpr& expr) {
    Value left = evaluate(*expr.left);
    Value right = evaluate(*expr.right);

    switch (expr.op.type) {
        case TokenType::PLUS:
            if (std::holds_alternative<double>(left) && std::holds_alternative<double>(right)) {
                return std::get<double>(left) + std::get<double>(right);
            }
            if (std::holds_alternative<std::string>(left) && std::holds_alternative<std::string>(right)) {
                return std::get<std::string>(left) + std::get<std::string>(right);
            }
            throw std::runtime_error("Operands must be two numbers or two strings.");

        case TokenType::MINUS:
            return Value{asNumber(left) - asNumber(right)};

        case TokenType::STAR:
            return asNumber(left) * asNumber(right);

        case TokenType::SLASH:
            return asNumber(left) / asNumber(right);

        case TokenType::GREATER:
            return asNumber(left) > asNumber(right);

        case TokenType::GREATER_EQUAL:
            return asNumber(left) >= asNumber(right);

        case TokenType::LESS:
            return asNumber(left) < asNumber(right);

        case TokenType::LESS_EQUAL:
            return asNumber(left) <= asNumber(right);

        case TokenType::EQUAL_EQUAL:
            return left == right;

        case TokenType::BANG_EQUAL:
            return left != right;

        default:
            throw std::runtime_error("Unknown binary operator.");
    }
}

Value Interpreter::visit(UnaryExpr& expr) {
    Value right = evaluate(*expr.right);

    switch (expr.op.type) {
        case TokenType::MINUS:
            return -asNumber(right);

        case TokenType::BANG:
            return !isTruthy(right);

        default:
            throw std::runtime_error("Unknown unary operator.");
    }
}

Value Interpreter::visit(AssignExpr& expr) {
    Value v = evaluate(*expr.value);
    env->assign(expr.name, v);
    return v;
}

Value Interpreter::visit(CallExpr& expr) {
    Value calleVal = evaluate(*expr.callee);
    if (!std::holds_alternative<std::shared_ptr<Callable>>(calleVal))
    {
        throw std::runtime_error("Can only call functions and classes.");
    }
    auto callable = std::get<std::shared_ptr<Callable>>(calleVal);
    // evaluate arguments
    std::vector<Value> arguments;
    for (const auto& argExpr : expr.args) {
        arguments.push_back(evaluate(*argExpr));
    }
    int arity = callable->arity();
    if (arity>=0 && arguments.size() != static_cast<size_t>(arity)) {
        throw std::runtime_error("Expected " + std::to_string(arity) +
                                 " arguments but got " + std::to_string(arguments.size()) + ".");
    }
    return callable->call(*this, arguments);
}

Value Interpreter::visit(LiteralExpr& expr) {
    return expr.value;
}

Value Interpreter::visit(GroupingExpr& expr) {
    return evaluate(*expr.expression);
}

Value Interpreter::visit(VariableExpr& expr) {
    return env->get(expr.name);
}

Value Interpreter::visit(ArrayExpr& expr) {
    auto array = std::make_shared<Array>();
    for (const auto& elementExpr : expr.elements) {
        array->elements.push_back(evaluate(*elementExpr));
    }
    return array;
}
Value Interpreter::visit(MapExpr& expr) {
    auto map = std::make_shared<Map>();
    for (const auto& [key, valueExpr] : expr.entries) {
        map->entries[key] = evaluate(*valueExpr);
    }
    return map;
}
Value Interpreter::visit(IndexExpr& expr) {
    Value collection = evaluate(*expr.collection);
    Value index = evaluate(*expr.index);

    if (std::holds_alternative<std::shared_ptr<Array>>(collection)) {
        auto array = std::get<std::shared_ptr<Array>>(collection);
        size_t idx = static_cast<size_t>(asNumber(index));
        if (idx >= array->elements.size()) {
            throw std::runtime_error("Array index out of bounds.");
        }
        return array->elements[idx];
    } else if (std::holds_alternative<std::shared_ptr<Map>>(collection)) {
        auto map = std::get<std::shared_ptr<Map>>(collection);
        std::string key = std::get<std::string>(index);
        auto it = map->entries.find(key);
        if (it == map->entries.end()) {
            throw std::runtime_error("Key '" + key + "' not found in map.");
        }
        return it->second;
    } else {
        throw std::runtime_error("Indexing is only supported on arrays and maps.");
    }
}

Value Interpreter::visit(SetIndexExpr& expr) {
    Value collection = evaluate(*expr.collection);
    Value index = evaluate(*expr.index);
    Value value = evaluate(*expr.value);

    if (std::holds_alternative<std::shared_ptr<Array>>(collection)) {
        auto array = std::get<std::shared_ptr<Array>>(collection);
        size_t idx = static_cast<size_t>(asNumber(index));
        if (idx <0 ) {
            throw std::runtime_error("Array index cannot be negative.");
        }
        if (idx >= array->elements.size()) {
            throw std::runtime_error("Array index out of bounds.");
        }
        array->elements[idx] = value;
        return value;
    } 
    
    if (std::holds_alternative<std::shared_ptr<Map>>(collection)) {
        auto map = std::get<std::shared_ptr<Map>>(collection);
        if(!std::holds_alternative<std::string>(index)) {
            throw std::runtime_error("Map keys must be strings.");
        }
        std::string key = std::get<std::string>(index);
        map->entries[key] = value;
        return value;
    } 
        throw std::runtime_error("Index assignment is only supported on arrays and maps.");
}

// Statement visitors
void Interpreter::visit(ExprStmt& stmt) {
    evaluate(*stmt.expr);
}

void Interpreter::visit(VarStmt& stmt) {
    Value value = Nil{};
    if (stmt.initializer) {
        value = evaluate(*stmt.initializer);
    }
    env->define(stmt.name, value);
}

void Interpreter::visit(BlockStmt& stmt) {
    Environment blockEnv(env);
    executeBlock(stmt.statements, &blockEnv);
}

void Interpreter::visit(IfStmt& stmt) {
    Value condition = evaluate(*stmt.condition);
    if (isTruthy(condition)) {
        stmt.thenBranch->accept(*this);
    } else if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void Interpreter::visit(WhileStmt& stmt) {
    while (isTruthy(evaluate(*stmt.condition))) {
        stmt.body->accept(*this);
    }
}

void Interpreter::visit(FunctionStmt& stmt) {
    auto fn = std::make_shared<UserFunction>(&stmt, env);
    env->define(stmt.name, fn);
}

void Interpreter::visit(ReturnStmt& stmt) {
    Value v = Nil{};
    if (stmt.value) {
        v = evaluate(*stmt.value);
    }
    throw ReturnSignal{v};
}


void Interpreter::visit(ImportStmt& stmt) {

    std::string modulePath = normalizeModulePath(stmt.module);
    if (importedModules.contains(modulePath) ) {
        // Module already imported
        return;
    }

    std::string source = loadFile(modulePath);
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens));
    auto program = parser.parse();
    interpret(program);

    // Execute the module in a new environment

    importedModules.insert(modulePath);
}
std::string Interpreter::normalizeModulePath(const std::string& path) {
    // Turn "math" into "math.iz"
    if (path.size() >= 3 && path.ends_with(".iz"))
    {
        return path;
    }
    return path + ".iz";
   
}

std::string Interpreter::loadFile(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Could not open file: " + path);
    }   
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
}  // namespace izi