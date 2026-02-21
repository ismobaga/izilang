#include "interpreter.hpp"

#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>

#include "ast/expr.hpp"
#include "ast/pattern.hpp"
#include "ast/visitor.hpp"
#include "common/callable.hpp"
#include "common/token.hpp"
#include "common/value.hpp"
#include "common/module_path.hpp"
#include "interp/native.hpp"
#include "interp/native_modules.hpp"
#include "interp/izi_class.hpp"
#include "parse/parser.hpp"
#include "parse/lexer.hpp"
namespace izi {

// Constructor needs to be defined to call registerNativeFunctions
Interpreter::Interpreter(std::string_view source)
    : source_(source), globals(arena_.create()), env(globals) {
    registerNativeFunctions(*this);
}

void Interpreter::interpret(const std::vector<StmtPtr>& program) {
    for (auto& s : program) {
        if (s) {  // Skip null statements from parser errors
            execute(*s);
        }
    }
}

Value Interpreter::evaluate(Expr& expr) {
    return expr.accept(*this);
}
void Interpreter::execute(Stmt& stmt) {
    if (debugHook_ && stmt.line > 0) {
        debugHook_->onStatement(stmt.line, currentFile);
    }
    stmt.accept(*this);
}

double Interpreter::toNumber(const Value& v, const Token& token) {
    if (!std::holds_alternative<double>(v)) {
        throw RuntimeError(token, "Expected number, got " + getTypeName(v) + ".");
    }
    return std::get<double>(v);
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
    // Handle short-circuit operators separately
    if (expr.op.type == TokenType::OR) {
        Value left = evaluate(*expr.left);
        if (isTruthy(left)) {
            return left;  // Short-circuit: return left if truthy
        }
        return evaluate(*expr.right);  // Only evaluate right if left is falsy
    }

    if (expr.op.type == TokenType::AND) {
        Value left = evaluate(*expr.left);
        if (!isTruthy(left)) {
            return left;  // Short-circuit: return left if falsy
        }
        return evaluate(*expr.right);  // Only evaluate right if left is truthy
    }

    // For all other operators, evaluate both operands
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
            throw RuntimeError(expr.op, "Cannot add " + getTypeName(left) + " and " + getTypeName(right) +
                                            ". Operands must be two numbers or two strings.");

        case TokenType::MINUS:
            return Value{toNumber(left, expr.op) - toNumber(right, expr.op)};

        case TokenType::STAR:
            return toNumber(left, expr.op) * toNumber(right, expr.op);

        case TokenType::SLASH:
            return toNumber(left, expr.op) / toNumber(right, expr.op);

        case TokenType::PERCENT: {
            double l = toNumber(left, expr.op);
            double r = toNumber(right, expr.op);
            if (r == 0.0) {
                throw RuntimeError(expr.op, "Division by zero in modulo operation.");
            }
            return std::fmod(l, r);
        }

        case TokenType::GREATER:
            return toNumber(left, expr.op) > toNumber(right, expr.op);

        case TokenType::GREATER_EQUAL:
            return toNumber(left, expr.op) >= toNumber(right, expr.op);

        case TokenType::LESS:
            return toNumber(left, expr.op) < toNumber(right, expr.op);

        case TokenType::LESS_EQUAL:
            return toNumber(left, expr.op) <= toNumber(right, expr.op);

        case TokenType::EQUAL_EQUAL:
            return left == right;

        case TokenType::BANG_EQUAL:
            return left != right;

        default:
            throw RuntimeError(expr.op, "Unknown binary operator.");
    }
}

Value Interpreter::visit(UnaryExpr& expr) {
    Value right = evaluate(*expr.right);

    switch (expr.op.type) {
        case TokenType::MINUS:
            return -toNumber(right, expr.op);

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
    if (!std::holds_alternative<std::shared_ptr<Callable>>(calleVal)) {
        throw std::runtime_error("Can only call functions and classes.");
    }
    auto callable = std::get<std::shared_ptr<Callable>>(calleVal);
    // evaluate arguments
    std::vector<Value> arguments;
    for (const auto& argExpr : expr.args) {
        arguments.push_back(evaluate(*argExpr));
    }
    int arity = callable->arity();
    if (arity >= 0 && arguments.size() != static_cast<size_t>(arity)) {
        throw std::runtime_error("Expected " + std::to_string(arity) + " arguments but got " +
                                 std::to_string(arguments.size()) + ".");
    }
    return callable->call(*this, arguments);
}

Value Interpreter::visit(LiteralExpr& expr) {
    return expr.value;
}

Value Interpreter::visit(GroupingExpr& expr) {
    return evaluate(*expr.expression);
}

Value Interpreter::visit(ConditionalExpr& expr) {
    Value condition = evaluate(*expr.condition);
    if (isTruthy(condition)) {
        return evaluate(*expr.thenBranch);
    } else {
        return evaluate(*expr.elseBranch);
    }
}

Value Interpreter::visit(VariableExpr& expr) {
    return env->get(expr.name);
}

Value Interpreter::visit(ArrayExpr& expr) {
    auto array = std::make_shared<Array>();
    for (const auto& elementExpr : expr.elements) {
        // Check if this element is a spread expression
        if (auto* spreadExpr = dynamic_cast<SpreadExpr*>(elementExpr.get())) {
            Value spreadValue = evaluate(*spreadExpr->argument);
            // If it's an array, spread its elements
            if (std::holds_alternative<std::shared_ptr<Array>>(spreadValue)) {
                auto spreadArray = std::get<std::shared_ptr<Array>>(spreadValue);
                for (const auto& elem : spreadArray->elements) {
                    array->elements.push_back(elem);
                }
            } else {
                throw std::runtime_error("Spread operator can only be used with arrays in array context.");
            }
        } else {
            array->elements.push_back(evaluate(*elementExpr));
        }
    }
    return array;
}
Value Interpreter::visit(MapExpr& expr) {
    auto map = std::make_shared<Map>();
    for (const auto& [key, valueExpr] : expr.entries) {
        // Check if this is a spread expression (indicated by empty key)
        if (key.empty()) {
            if (auto* spreadExpr = dynamic_cast<SpreadExpr*>(valueExpr.get())) {
                Value spreadValue = evaluate(*spreadExpr->argument);
                // If it's a map, spread its entries
                if (std::holds_alternative<std::shared_ptr<Map>>(spreadValue)) {
                    auto spreadMap = std::get<std::shared_ptr<Map>>(spreadValue);
                    for (const auto& [k, v] : spreadMap->entries) {
                        map->entries[k] = v;
                    }
                } else {
                    throw std::runtime_error("Spread operator can only be used with maps in map context.");
                }
            }
        } else {
            map->entries[key] = evaluate(*valueExpr);
        }
    }
    return map;
}

Value Interpreter::visit(SpreadExpr& expr) {
    // SpreadExpr should only be evaluated in the context of array or map literals
    // If we get here, it means spread was used outside of those contexts
    throw std::runtime_error("Spread operator can only be used inside array or map literals.");
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
        if (idx < 0) {
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
        if (!std::holds_alternative<std::string>(index)) {
            throw std::runtime_error("Map keys must be strings.");
        }
        std::string key = std::get<std::string>(index);
        map->entries[key] = value;
        return value;
    }
    throw std::runtime_error("Index assignment is only supported on arrays and maps.");
}

Value Interpreter::visit(FunctionExpr& expr) {
    // Create a UserFunction that directly references the FunctionExpr
    // The FunctionExpr is part of the AST and lives for the duration of the program
    // so this pointer will remain valid
    auto func = std::make_shared<UserFunction>(&expr, env);
    return func;
}

Value Interpreter::visit(MatchExpr& expr) {
    // Evaluate the value to match against
    Value matchValue = evaluate(*expr.value);

    // Try each case in order
    for (size_t i = 0; i < expr.cases.size(); ++i) {
        auto& matchCase = expr.cases[i];

        bool matched = false;
        Value boundValue = Nil{};
        std::string varName;

        // Check if pattern matches
        if (auto* wildcard = dynamic_cast<WildcardPattern*>(matchCase.pattern.get())) {
            // Wildcard always matches
            matched = true;
        } else if (auto* literal = dynamic_cast<LiteralPattern*>(matchCase.pattern.get())) {
            // Literal pattern: check for equality
            matched = (matchValue == literal->value);
        } else if (auto* variable = dynamic_cast<VariablePattern*>(matchCase.pattern.get())) {
            // Variable pattern: always matches and binds the value
            matched = true;
            boundValue = matchValue;
            varName = variable->name;
        }

        // If pattern matched, check guard condition if present
        if (matched && matchCase.guard) {
            // Create a new environment for guard evaluation
            auto guardEnv = arena_.create(env);

            // If variable pattern, bind the variable in guard scope
            if (!varName.empty()) {
                guardEnv->define(varName, boundValue);
            }

            // Evaluate guard in the new environment
            auto previousEnv = env;
            env = guardEnv;
            Value guardResult = evaluate(*matchCase.guard);
            env = previousEnv;

            // Check if guard evaluates to truthy value
            if (!isTruthy(guardResult)) {
                matched = false;
            }
        }

        // If everything matched, evaluate and return the result
        if (matched) {
            // Create a new environment for result evaluation
            auto resultEnv = arena_.create(env);

            // If variable pattern, bind the variable in result scope
            if (!varName.empty()) {
                resultEnv->define(varName, boundValue);
            }

            // Evaluate result in the new environment
            auto previousEnv = env;
            env = resultEnv;
            Value result = evaluate(*matchCase.result);
            env = previousEnv;

            return result;
        }
    }

    // No pattern matched
    throw std::runtime_error("No matching pattern found in match expression");
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

    // Check if this is a destructuring declaration
    if (stmt.pattern != nullptr) {
        // Array destructuring
        if (auto* arrayPattern = dynamic_cast<ArrayPattern*>(stmt.pattern.get())) {
            if (!std::holds_alternative<std::shared_ptr<Array>>(value)) {
                throw std::runtime_error("Array destructuring requires an array value.");
            }
            auto array = std::get<std::shared_ptr<Array>>(value);

            // Bind each variable in the pattern to corresponding array element
            for (size_t i = 0; i < arrayPattern->elements.size(); ++i) {
                if (auto* varPattern = dynamic_cast<VariablePattern*>(arrayPattern->elements[i].get())) {
                    Value elemValue = Nil{};
                    if (i < array->elements.size()) {
                        elemValue = array->elements[i];
                    }
                    env->define(varPattern->name, elemValue);
                }
            }
            return;
        }

        // Map destructuring
        if (auto* mapPattern = dynamic_cast<MapPattern*>(stmt.pattern.get())) {
            if (!std::holds_alternative<std::shared_ptr<Map>>(value)) {
                throw std::runtime_error("Map destructuring requires a map value.");
            }
            auto map = std::get<std::shared_ptr<Map>>(value);

            // Bind each variable in the pattern to corresponding map entry
            for (const auto& key : mapPattern->keys) {
                Value mapValue = Nil{};
                auto it = map->entries.find(key);
                if (it != map->entries.end()) {
                    mapValue = it->second;
                }
                env->define(key, mapValue);
            }
            return;
        }
    }

    // Simple variable declaration
    env->define(stmt.name, value);
}

void Interpreter::visit(BlockStmt& stmt) {
    auto blockEnv = arena_.create(env);
    executeBlock(stmt.statements, blockEnv);
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
        try {
            stmt.body->accept(*this);
        } catch (const BreakSignal&) {
            break;  // Exit the loop
        } catch (const ContinueSignal&) {
            continue;  // Continue to next iteration
        }
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
    std::string modulePath = stmt.module;

    // Check if this is a native module
    if (isNativeModule(modulePath)) {
        // Handle native module import
        Value moduleValue = getNativeModule(modulePath, *this);
        auto moduleMap = std::get<std::shared_ptr<Map>>(moduleValue);

        if (stmt.isWildcard) {
            // import * as name from "math"
            env->define(stmt.wildcardAlias, moduleValue);
        } else if (!stmt.namedImports.empty()) {
            // import { sqrt, pi } from "math" or import { default as x } from "mod"
            for (size_t i = 0; i < stmt.namedImports.size(); ++i) {
                const auto& name = stmt.namedImports[i];
                const auto& alias = (!stmt.namedAliases.empty() && !stmt.namedAliases[i].empty())
                                        ? stmt.namedAliases[i]
                                        : name;
                auto it = moduleMap->entries.find(name);
                if (it == moduleMap->entries.end()) {
                    throw std::runtime_error("Module '" + modulePath + "' does not export '" + name + "'");
                }
                env->define(alias, it->second);
            }
        } else {
            // import "math" - bind as module object
            env->define(modulePath, moduleValue);
        }

        // Mark as imported to avoid re-importing
        importedModules.insert(modulePath);
        return;
    }

    // Handle file-based modules with relative path resolution
    modulePath = ModulePath::resolveImport(stmt.module, currentFile);

    // Canonicalize the path for proper deduplication and cycle detection
    std::string canonicalPath = ModulePath::canonicalize(modulePath);

    if (stmt.isWildcard || !stmt.namedImports.empty()) {
        // Wildcard or named import: load in isolated scope and collect exports
        auto exports = loadModuleWithExports(modulePath, currentFile);

        if (stmt.isWildcard) {
            // import * as alias from "./module" -> create namespace Map
            auto map = std::make_shared<Map>();
            map->entries = exports;
            env->define(stmt.wildcardAlias, Value(map));
        } else {
            // import { a, b } from "./module" -> validate and bind named exports
            for (size_t i = 0; i < stmt.namedImports.size(); ++i) {
                const auto& name = stmt.namedImports[i];
                const auto& alias = (!stmt.namedAliases.empty() && !stmt.namedAliases[i].empty())
                                        ? stmt.namedAliases[i]
                                        : name;
                auto it = exports.find(name);
                if (it == exports.end()) {
                    throw std::runtime_error("Module '" + stmt.module + "' does not export '" + name + "'");
                }
                env->define(alias, it->second);
            }
        }
        return;
    }

    // Simple import: run in current (global) scope for backward compatibility

    // Check if module is already imported (avoid re-importing)
    if (importedModules.contains(canonicalPath)) {
        return;
    }

    // Check for circular imports
    auto it = std::find(importStack.begin(), importStack.end(), canonicalPath);
    if (it != importStack.end()) {
        // Build circular import chain message
        std::string chain;
        for (size_t i = std::distance(importStack.begin(), it); i < importStack.size(); ++i) {
            if (!chain.empty()) chain += " -> ";
            chain += importStack[i];
        }
        chain += " -> " + canonicalPath;
        throw std::runtime_error("Circular import detected: " + chain);
    }

    // Load and parse the module, keeping AST alive in the program cache
    std::string source = loadFile(modulePath);
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens));
    auto& cachedProgram = moduleProgramCache_[canonicalPath];
    cachedProgram = parser.parse();

    // Save current file and push to import stack
    std::string previousFile = currentFile;
    currentFile = canonicalPath;
    importStack.push_back(canonicalPath);

    try {
        // Execute the module in current global scope
        interpret(cachedProgram);

        // Pop from import stack and mark as imported
        importStack.pop_back();
        importedModules.insert(canonicalPath);

        // Restore previous file
        currentFile = previousFile;
    } catch (...) {
        // On error, restore state and rethrow; keep the cached program alive
        importStack.pop_back();
        currentFile = previousFile;
        throw;
    }
}

std::unordered_map<std::string, Value> Interpreter::loadModuleWithExports(const std::string& modulePath,
                                                                           const std::string& fromFile) {
    std::string resolvedPath = ModulePath::resolveImport(modulePath, fromFile);
    std::string canonicalPath = ModulePath::canonicalize(resolvedPath);

    // Return cached exports if already loaded in isolated mode
    auto cacheIt = moduleExportCache_.find(canonicalPath);
    if (cacheIt != moduleExportCache_.end()) {
        return cacheIt->second;
    }

    // Check for circular imports
    auto stackIt = std::find(importStack.begin(), importStack.end(), canonicalPath);
    if (stackIt != importStack.end()) {
        std::string chain;
        for (size_t i = std::distance(importStack.begin(), stackIt); i < importStack.size(); ++i) {
            if (!chain.empty()) chain += " -> ";
            chain += importStack[i];
        }
        chain += " -> " + canonicalPath;
        throw std::runtime_error("Circular import detected: " + chain);
    }

    // Load and parse, keeping AST alive in the program cache
    std::string source = loadFile(resolvedPath);
    Lexer lexer(source);
    auto tokens = lexer.scanTokens();
    Parser parser(std::move(tokens));
    auto& cachedProgram = moduleProgramCache_[canonicalPath];
    cachedProgram = parser.parse();

    // Execute in isolated module scope
    auto moduleEnv = arena_.create(globals);
    std::unordered_map<std::string, Value> exports;

    auto prevEnv = env;
    auto prevFile = currentFile;
    auto* prevModuleExports = currentModuleExports_;

    env = moduleEnv;
    currentFile = canonicalPath;
    currentModuleExports_ = &exports;
    importStack.push_back(canonicalPath);

    try {
        interpret(cachedProgram);
        importStack.pop_back();
        importedModules.insert(canonicalPath);
        moduleExportCache_[canonicalPath] = exports;
    } catch (...) {
        importStack.pop_back();
        env = prevEnv;
        currentFile = prevFile;
        currentModuleExports_ = prevModuleExports;
        // Keep the cached program alive to avoid dangling pointers in any
        // UserFunction objects that may have been created before the error.
        throw;
    }

    env = prevEnv;
    currentFile = prevFile;
    currentModuleExports_ = prevModuleExports;

    return exports;
}

std::string Interpreter::normalizeModulePath(const std::string& path) {
    // Turn "math" into "math.iz"
    if (path.size() >= 3 && path.ends_with(".iz")) {
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

void Interpreter::visit(ExportStmt& stmt) {
    if (stmt.isDefault) {
        Value value;
        if (stmt.declaration) {
            // export default fn/var — execute declaration, then capture its value
            execute(*stmt.declaration);
            if (auto* fnStmt = dynamic_cast<FunctionStmt*>(stmt.declaration.get())) {
                value = env->get(fnStmt->name);
            } else if (auto* varStmt = dynamic_cast<VarStmt*>(stmt.declaration.get())) {
                value = env->get(varStmt->name);
            }
        } else if (stmt.defaultExpr) {
            // export default <expr>
            value = evaluate(*stmt.defaultExpr);
        }
        if (currentModuleExports_) {
            (*currentModuleExports_)["default"] = value;
        }
        return;
    }

    // Regular export: execute the declaration
    execute(*stmt.declaration);

    // If we're in a module context, record the exported name
    if (currentModuleExports_) {
        if (auto* varStmt = dynamic_cast<VarStmt*>(stmt.declaration.get())) {
            (*currentModuleExports_)[varStmt->name] = env->get(varStmt->name);
        } else if (auto* fnStmt = dynamic_cast<FunctionStmt*>(stmt.declaration.get())) {
            (*currentModuleExports_)[fnStmt->name] = env->get(fnStmt->name);
        } else if (auto* classStmt = dynamic_cast<ClassStmt*>(stmt.declaration.get())) {
            (*currentModuleExports_)[classStmt->name] = env->get(classStmt->name);
        }
    }
}

void Interpreter::visit(ReExportStmt& stmt) {
    // Re-export only affects the current module's exports
    if (!currentModuleExports_) {
        // Re-export at top level is a no-op (no module context)
        return;
    }

    // Load the source module exports
    auto exports = loadModuleWithExports(stmt.module, currentFile);

    if (stmt.isWildcard) {
        // export * from "module" — re-export all
        for (auto& [name, value] : exports) {
            (*currentModuleExports_)[name] = value;
        }
    } else {
        // export { a, b } from "module" — re-export named
        for (const auto& name : stmt.names) {
            auto it = exports.find(name);
            if (it == exports.end()) {
                throw std::runtime_error("Module '" + stmt.module + "' does not export '" + name + "'");
            }
            (*currentModuleExports_)[name] = it->second;
        }
    }
}

void Interpreter::visit(BreakStmt& /*stmt*/) {
    throw BreakSignal{};
}

void Interpreter::visit(ContinueStmt& /*stmt*/) {
    throw ContinueSignal{};
}

void Interpreter::visit(TryStmt& stmt) {
    bool exceptionCaught = false;
    Value caughtException;
    Token exceptionToken(TokenType::ERROR, "", 0, 0);

    // Execute try block
    try {
        execute(*stmt.tryBlock);
    } catch (const ThrowSignal& e) {
        exceptionCaught = true;
        caughtException = e.exception;
        exceptionToken = e.token;

        // Execute catch block if present
        if (stmt.catchBlock != nullptr) {
            // Extract statements from BlockStmt
            auto* blockPtr = dynamic_cast<BlockStmt*>(stmt.catchBlock.get());
            if (blockPtr) {
                // Create new environment for catch block with exception variable
                auto catchEnv = arena_.create(env);

                // Bind exception to catch variable
                if (!stmt.catchVariable.empty()) {
                    catchEnv->define(stmt.catchVariable, caughtException);
                }

                executeBlock(blockPtr->statements, catchEnv);
                exceptionCaught = false;  // Exception was handled
            }
        }
    }

    // Execute finally block if present (always executes)
    if (stmt.finallyBlock != nullptr) {
        execute(*stmt.finallyBlock);
    }

    // Re-throw if exception wasn't caught
    if (exceptionCaught) {
        throw ThrowSignal(caughtException, exceptionToken);
    }
}

void Interpreter::visit(ThrowStmt& stmt) {
    Value exceptionValue = evaluate(*stmt.value);
    throw ThrowSignal(exceptionValue, stmt.keyword);
}

// v0.3: Class support
void Interpreter::visit(ClassStmt& stmt) {
    // Get superclass if it exists
    std::shared_ptr<IziClass> superclass = nullptr;
    if (!stmt.superclass.empty()) {
        Value superValue = env->get(stmt.superclass);
        if (!std::holds_alternative<std::shared_ptr<Callable>>(superValue)) {
            throw RuntimeError(Token(TokenType::IDENTIFIER, stmt.superclass, 0, 0), "Superclass must be a class.");
        }
        auto superCallable = std::get<std::shared_ptr<Callable>>(superValue);
        superclass = std::dynamic_pointer_cast<IziClass>(superCallable);
        if (!superclass) {
            throw RuntimeError(Token(TokenType::IDENTIFIER, stmt.superclass, 0, 0), "Superclass must be a class.");
        }
    }

    // Evaluate field defaults
    std::unordered_map<std::string, Value> fieldDefaults;
    std::vector<std::string> fieldNames;

    for (const auto& field : stmt.fields) {
        fieldNames.push_back(field->name);
        if (field->initializer) {
            fieldDefaults[field->name] = evaluate(*field->initializer);
        }
    }

    // Create method callables
    // If we have a superclass, we need to define 'super' in the method's environment
    std::unordered_map<std::string, Value> methods;
    for (const auto& method : stmt.methods) {
        Environment* methodEnv = env;

        // If there's a superclass, create a new environment with 'super' defined
        if (superclass) {
            methodEnv = arena_.create(env);
            methodEnv->define("super", superclass);
        }

        auto userFunc = std::make_shared<UserFunction>(method.get(), methodEnv);
        methods[method->name] = userFunc;
    }

    // Create the class
    auto klass = std::make_shared<IziClass>(stmt.name, superclass, std::move(fieldNames), std::move(fieldDefaults),
                                            std::move(methods));

    // Define the class in the current environment
    env->define(stmt.name, klass);
}

// v0.3: Property access
Value Interpreter::visit(PropertyExpr& expr) {
    Value object = expr.object->accept(*this);

    // Handle instance property access
    if (std::holds_alternative<std::shared_ptr<Instance>>(object)) {
        auto instance = std::get<std::shared_ptr<Instance>>(object);

        // Check if it's a field
        auto fieldIt = instance->fields.find(expr.property);
        if (fieldIt != instance->fields.end()) {
            return fieldIt->second;
        }

        // Check if it's a method
        Value method = Nil{};
        if (std::holds_alternative<std::shared_ptr<IziClass>>(instance->klass)) {
            auto klass = std::get<std::shared_ptr<IziClass>>(instance->klass);
            method = klass->getMethod(expr.property, instance);
        } else {
            throw RuntimeError(Token(TokenType::DOT, expr.property, 0, 0),
                               "Cannot access method from VM class in interpreter mode");
        }
        if (!std::holds_alternative<Nil>(method)) {
            return method;
        }

        throw RuntimeError(Token(TokenType::DOT, expr.property, 0, 0), "Undefined property '" + expr.property + "'.");
    }

    // Handle map property access (backward compatibility)
    if (std::holds_alternative<std::shared_ptr<Map>>(object)) {
        auto map = std::get<std::shared_ptr<Map>>(object);
        auto it = map->entries.find(expr.property);
        if (it != map->entries.end()) {
            return it->second;
        }
        throw RuntimeError(Token(TokenType::DOT, expr.property, 0, 0), "Property '" + expr.property + "' not found.");
    }

    throw RuntimeError(Token(TokenType::DOT, expr.property, 0, 0), "Only instances and maps support property access.");
}

// v0.3: Property assignment
Value Interpreter::visit(SetPropertyExpr& expr) {
    Value object = expr.object->accept(*this);
    Value value = expr.value->accept(*this);

    // Handle instance property assignment
    if (std::holds_alternative<std::shared_ptr<Instance>>(object)) {
        auto instance = std::get<std::shared_ptr<Instance>>(object);
        instance->fields[expr.property] = value;
        return value;
    }

    // Handle map property assignment (backward compatibility)
    if (std::holds_alternative<std::shared_ptr<Map>>(object)) {
        auto map = std::get<std::shared_ptr<Map>>(object);
        map->entries[expr.property] = value;
        return value;
    }

    throw RuntimeError(Token(TokenType::DOT, expr.property, 0, 0),
                       "Only instances and maps support property assignment.");
}

// v0.3: This expression
Value Interpreter::visit(ThisExpr& expr) {
    try {
        return env->get("this");
    } catch (const std::runtime_error& e) {
        throw RuntimeError(Token(TokenType::THIS, "this", 0, 0), "Cannot use 'this' outside of a class method.");
    }
}

// v0.3: Super expression
Value Interpreter::visit(SuperExpr& expr) {
    // Get the superclass from the environment
    std::shared_ptr<IziClass> superclass;
    try {
        Value superValue = env->get("super");
        if (!std::holds_alternative<std::shared_ptr<Callable>>(superValue)) {
            throw RuntimeError(Token(TokenType::SUPER, "super", 0, 0), "Invalid superclass reference.");
        }
        auto superCallable = std::get<std::shared_ptr<Callable>>(superValue);
        superclass = std::dynamic_pointer_cast<IziClass>(superCallable);
        if (!superclass) {
            throw RuntimeError(Token(TokenType::SUPER, "super", 0, 0), "Invalid superclass reference.");
        }
    } catch (const std::runtime_error& e) {
        throw RuntimeError(Token(TokenType::SUPER, "super", 0, 0), "Cannot use 'super' outside of a subclass method.");
    }

    // Get 'this' to bind the method to
    std::shared_ptr<Instance> instance;
    try {
        Value thisValue = env->get("this");
        if (!std::holds_alternative<std::shared_ptr<Instance>>(thisValue)) {
            throw RuntimeError(Token(TokenType::SUPER, "super", 0, 0), "Cannot use 'super' without a valid instance.");
        }
        instance = std::get<std::shared_ptr<Instance>>(thisValue);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(Token(TokenType::SUPER, "super", 0, 0), "Cannot use 'super' outside of a class method.");
    }

    // Get the method from the superclass
    Value method = superclass->getMethod(expr.method, instance);
    if (std::holds_alternative<Nil>(method)) {
        throw RuntimeError(Token(TokenType::SUPER, "super", 0, 0),
                           "Undefined method '" + expr.method + "' in superclass.");
    }

    return method;
}

}  // namespace izi
