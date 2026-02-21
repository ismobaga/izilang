#include "semantic_analyzer.hpp"
#include <iostream>

namespace izi {

void SemanticAnalyzer::analyze(const std::vector<StmtPtr>& program) {
    diagnostics_.clear();
    currentScope_ = std::make_shared<Scope>();
    currentScope_->type = Scope::Type::Global;

    // Pre-populate global scope with built-in function names
    auto anyType = TypeAnnotation::simple(TypeAnnotation::Kind::Any);
    for (const auto& builtin :
         {"print", "clock", "len", "str", "spawn", "await", "sleep", "mutex", "lock", "unlock", "trylock",
          "thread_spawn", "push", "pop", "shift", "unshift", "splice",
          "keys", "values", "hasKey", "has", "delete", "entries", "Set", "setAdd", "setHas", "setDelete", "setSize",
          "sqrt", "pow", "abs", "floor", "ceil", "round", "sin", "cos", "tan", "min", "max", "substring", "split",
          "join", "toUpper", "toLower", "trim", "replace", "startsWith", "endsWith", "indexOf", "map", "filter",
          "reduce", "sort", "reverse", "concat", "slice", "readFile", "writeFile", "appendFile", "fileExists",
          "readLine", "input", "num", "int", "bool", "type", "error", "assert", "range", "zip", "enumerate",
          "format", "parse", "exit", "getenv", "args", "open", "close", "read", "write"}) {
        currentScope_->variables[builtin] = TypeAnnotation::simple(TypeAnnotation::Kind::Any);
        currentScope_->usedVariables.insert(builtin);  // mark builtins as used
    }

    for (const auto& stmt : program) {
        if (stmt) {
            stmt->accept(*this);
        }
    }

    // Check for unused variables in global scope
    for (const auto& [name, type] : currentScope_->variables) {
        if (currentScope_->usedVariables.find(name) == currentScope_->usedVariables.end()) {
            addWarning("Unused variable '" + name + "'", 0, 0);
        }
    }
}

// Helper methods
void SemanticAnalyzer::addError(const std::string& message, int line, int column) {
    diagnostics_.emplace_back(SemanticDiagnostic::Severity::Error, message, line, column);
}

void SemanticAnalyzer::addWarning(const std::string& message, int line, int column) {
    diagnostics_.emplace_back(SemanticDiagnostic::Severity::Warning, message, line, column);
}

void SemanticAnalyzer::addInfo(const std::string& message, int line, int column) {
    diagnostics_.emplace_back(SemanticDiagnostic::Severity::Info, message, line, column);
}

void SemanticAnalyzer::enterScope(Scope::Type type) {
    auto newScope = std::make_shared<Scope>();
    newScope->type = type;
    newScope->parent = currentScope_;
    currentScope_ = newScope;
}

void SemanticAnalyzer::exitScope() {
    // Check for unused variables in this scope
    for (const auto& [name, type] : currentScope_->variables) {
        if (currentScope_->usedVariables.find(name) == currentScope_->usedVariables.end()) {
            addWarning("Unused variable '" + name + "'", 0, 0);
        }
    }

    if (currentScope_->parent) {
        currentScope_ = currentScope_->parent;
    }
}

void SemanticAnalyzer::defineVariable(const std::string& name, TypePtr type, int line, int column) {
    if (currentScope_->variables.find(name) != currentScope_->variables.end()) {
        addError("Variable '" + name + "' already defined in this scope", line, column);
    }
    // Check for shadowing in parent scopes
    if (currentScope_->type != Scope::Type::Global) {
        auto scope = currentScope_->parent;
        while (scope) {
            if (scope->variables.find(name) != scope->variables.end()) {
                std::string scopeDesc = (scope->type == Scope::Type::Global)    ? "global scope"
                                        : (scope->type == Scope::Type::Function) ? "enclosing function scope"
                                                                                  : "outer block scope";
                addWarning("Variable '" + name + "' shadows variable from " + scopeDesc, line, column);
                break;
            }
            scope = scope->parent;
        }
    }
    currentScope_->variables[name] = std::move(type);
}

void SemanticAnalyzer::markVariableUsed(const std::string& name) {
    auto scope = currentScope_;
    while (scope) {
        if (scope->variables.find(name) != scope->variables.end()) {
            scope->usedVariables.insert(name);
            return;
        }
        scope = scope->parent;
    }
}

TypeAnnotation* SemanticAnalyzer::lookupVariable(const std::string& name) {
    auto scope = currentScope_;
    while (scope) {
        auto it = scope->variables.find(name);
        if (it != scope->variables.end()) {
            return it->second.get();
        }
        scope = scope->parent;
    }
    return nullptr;
}

TypePtr SemanticAnalyzer::inferType(Expr& expr) {
    // Try to infer type from literal values
    if (auto* literal = dynamic_cast<LiteralExpr*>(&expr)) {
        return valueToType(literal->value);
    }

    // Try to infer from variable reference
    if (auto* var = dynamic_cast<VariableExpr*>(&expr)) {
        auto* varType = lookupVariable(var->name);
        if (varType) {
            return TypeAnnotation::simple(varType->kind);
        }
    }

    // For now, return Any type for other expressions
    return TypeAnnotation::simple(TypeAnnotation::Kind::Any);
}

bool SemanticAnalyzer::isExplicitlyTyped(const TypeAnnotation* type) {
    return type != nullptr && type->kind != TypeAnnotation::Kind::Any;
}

bool SemanticAnalyzer::areTypesCompatible(const TypeAnnotation& expected, const TypeAnnotation& actual) {
    return expected.isCompatibleWith(actual);
}

TypePtr SemanticAnalyzer::valueToType(const Value& value) {
    if (std::holds_alternative<Nil>(value)) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::Nil);
    } else if (std::holds_alternative<bool>(value)) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::Bool);
    } else if (std::holds_alternative<double>(value)) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::Number);
    } else if (std::holds_alternative<std::string>(value)) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::String);
    } else if (std::holds_alternative<std::shared_ptr<Array>>(value)) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::Array);
    } else if (std::holds_alternative<std::shared_ptr<Map>>(value)) {
        return TypeAnnotation::simple(TypeAnnotation::Kind::Map);
    }
    return TypeAnnotation::simple(TypeAnnotation::Kind::Any);
}

// Expression visitors (stub implementations for now)
Value SemanticAnalyzer::visit(BinaryExpr& expr) {
    expr.left->accept(*this);
    expr.right->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(UnaryExpr& expr) {
    expr.right->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(LiteralExpr& expr) {
    return Nil{};
}

Value SemanticAnalyzer::visit(GroupingExpr& expr) {
    expr.expression->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(ConditionalExpr& expr) {
    // Analyze condition
    expr.condition->accept(*this);

    // Analyze both branches
    expr.thenBranch->accept(*this);
    expr.elseBranch->accept(*this);

    // Note: In a full type system, we would check that both branches
    // return compatible types. For now, we just ensure both are analyzed.
    return Nil{};
}

Value SemanticAnalyzer::visit(CallExpr& expr) {
    expr.callee->accept(*this);

    // Check if callee is a function and validate parameter types
    if (auto* varExpr = dynamic_cast<VariableExpr*>(expr.callee.get())) {
        auto* funcType = lookupVariable(varExpr->name);

        // If function has type information, validate parameter count and types
        if (funcType && funcType->kind == TypeAnnotation::Kind::Function) {
            // Check parameter count with proper pluralization
            if (expr.args.size() != funcType->paramTypes.size()) {
                std::string expectedWord = (funcType->paramTypes.size() == 1) ? "argument" : "arguments";
                std::string gotWord = (expr.args.size() == 1) ? "argument" : "arguments";
                addError("Function '" + varExpr->name + "' expects " + std::to_string(funcType->paramTypes.size()) +
                             " " + expectedWord + " but got " + std::to_string(expr.args.size()) + " " + gotWord,
                         0, 0);
            } else {
                // Check parameter types
                for (size_t i = 0; i < expr.args.size(); ++i) {
                    expr.args[i]->accept(*this);

                    // Only validate if parameter has explicit type annotation
                    if (isExplicitlyTyped(funcType->paramTypes[i].get())) {
                        TypePtr argType = inferType(*expr.args[i]);

                        if (!areTypesCompatible(*funcType->paramTypes[i], *argType)) {
                            addError("Function '" + varExpr->name + "' expects parameter at position " +
                                         std::to_string(i + 1) + " to be " + funcType->paramTypes[i]->toString() +
                                         " but got " + argType->toString(),
                                     0, 0);
                        }
                    }
                }
            }
            return Nil{};
        }
    }

    // For non-typed or unknown functions, just visit arguments
    for (auto& arg : expr.args) {
        arg->accept(*this);
    }
    return Nil{};
}

Value SemanticAnalyzer::visit(VariableExpr& expr) {
    if (lookupVariable(expr.name) == nullptr) {
        addError("Undefined variable '" + expr.name + "'", 0, 0);
    }
    markVariableUsed(expr.name);
    return Nil{};
}

Value SemanticAnalyzer::visit(AssignExpr& expr) {
    if (lookupVariable(expr.name) == nullptr) {
        addError("Undefined variable '" + expr.name + "'", 0, 0);
    }
    markVariableUsed(expr.name);
    expr.value->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(ArrayExpr& expr) {
    for (auto& elem : expr.elements) {
        elem->accept(*this);
    }
    return Nil{};
}

Value SemanticAnalyzer::visit(MapExpr& expr) {
    for (auto& [key, value] : expr.entries) {
        value->accept(*this);
    }
    return Nil{};
}

Value SemanticAnalyzer::visit(SpreadExpr& expr) {
    // Analyze the argument expression
    expr.argument->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(IndexExpr& expr) {
    expr.collection->accept(*this);
    expr.index->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(SetIndexExpr& expr) {
    expr.collection->accept(*this);
    expr.index->accept(*this);
    expr.value->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(FunctionExpr& expr) {
    enterScope(Scope::Type::Function);
    bool wasInFunction = inFunction_;
    inFunction_ = true;

    // Define parameters in function scope
    for (const auto& param : expr.params) {
        defineVariable(param, TypeAnnotation::simple(TypeAnnotation::Kind::Any), 0, 0);
    }

    for (const auto& s : expr.body) {
        s->accept(*this);
    }

    inFunction_ = wasInFunction;
    exitScope();
    return Nil{};
}

Value SemanticAnalyzer::visit(MatchExpr& expr) {
    expr.value->accept(*this);
    for (auto& matchCase : expr.cases) {
        if (matchCase.guard) {
            matchCase.guard->accept(*this);
        }
        matchCase.result->accept(*this);
    }
    return Nil{};
}

Value SemanticAnalyzer::visit(PropertyExpr& expr) {
    expr.object->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(SetPropertyExpr& expr) {
    expr.object->accept(*this);
    expr.value->accept(*this);
    return Nil{};
}

Value SemanticAnalyzer::visit(ThisExpr& expr) {
    if (!inMethod_) {
        addError("'this' can only be used inside class methods. Use 'this' within a method defined inside a class.", 0,
                 0);
    }
    return Nil{};
}

Value SemanticAnalyzer::visit(SuperExpr& expr) {
    if (!inMethod_) {
        addError("'super' can only be used inside class methods. Use 'super' within a method of a subclass.", 0, 0);
    }
    // Could add additional check to ensure the class has a superclass, but that requires more context
    return Nil{};
}

Value SemanticAnalyzer::visit(AwaitExpr& expr) {
    expr.value->accept(*this);
    return Nil{};
}

// Statement visitors
void SemanticAnalyzer::visit(ExprStmt& stmt) {
    stmt.expr->accept(*this);
}

void SemanticAnalyzer::visit(BlockStmt& stmt) {
    enterScope();
    bool previousReturned = hasReturnedInCurrentBlock_;
    hasReturnedInCurrentBlock_ = false;

    for (const auto& s : stmt.statements) {
        if (hasReturnedInCurrentBlock_) {
            addWarning("Unreachable code after return statement", 0, 0);
            break;
        }
        s->accept(*this);
    }

    hasReturnedInCurrentBlock_ = previousReturned;
    exitScope();
}

void SemanticAnalyzer::visit(VarStmt& stmt) {
    // Analyze initializer first (only once for all cases)
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
    }

    // Handle destructuring patterns
    if (stmt.pattern != nullptr) {
        if (auto* arrayPattern = dynamic_cast<ArrayPattern*>(stmt.pattern.get())) {
            // Register all variables from array pattern
            for (const auto& elem : arrayPattern->elements) {
                if (auto* varPattern = dynamic_cast<VariablePattern*>(elem.get())) {
                    TypePtr varType = TypeAnnotation::simple(TypeAnnotation::Kind::Any);
                    defineVariable(varPattern->name, std::move(varType), 0, 0);
                }
            }
        } else if (auto* mapPattern = dynamic_cast<MapPattern*>(stmt.pattern.get())) {
            // Register all variables from map pattern
            for (const auto& key : mapPattern->keys) {
                TypePtr varType = TypeAnnotation::simple(TypeAnnotation::Kind::Any);
                defineVariable(key, std::move(varType), 0, 0);
            }
        }
        return;
    }

    // Simple variable declaration
    TypePtr varType = stmt.typeAnnotation ? TypeAnnotation::simple(stmt.typeAnnotation->kind)
                                          : TypeAnnotation::simple(TypeAnnotation::Kind::Any);

    // Type check if both type annotation and initializer are present
    if (stmt.initializer && stmt.typeAnnotation && isExplicitlyTyped(varType.get())) {
        TypePtr initType = inferType(*stmt.initializer);

        // Check if the initializer type is compatible with the declared type
        if (!areTypesCompatible(*varType, *initType)) {
            addError("Type mismatch: variable '" + stmt.name + "' declared as " + varType->toString() +
                         " but initialized with " + initType->toString(),
                     0, 0);
        }
    }

    defineVariable(stmt.name, std::move(varType), 0, 0);
}

void SemanticAnalyzer::visit(WhileStmt& stmt) {
    stmt.condition->accept(*this);

    bool wasInLoop = inLoop_;
    inLoop_ = true;
    stmt.body->accept(*this);
    inLoop_ = wasInLoop;
}

void SemanticAnalyzer::visit(IfStmt& stmt) {
    stmt.condition->accept(*this);
    stmt.thenBranch->accept(*this);
    if (stmt.elseBranch) {
        stmt.elseBranch->accept(*this);
    }
}

void SemanticAnalyzer::visit(FunctionStmt& stmt) {
    // Define function type
    std::vector<TypePtr> paramTypes;
    for (size_t i = 0; i < stmt.params.size(); ++i) {
        if (i < stmt.paramTypes.size() && stmt.paramTypes[i]) {
            paramTypes.push_back(TypeAnnotation::simple(stmt.paramTypes[i]->kind));
        } else {
            paramTypes.push_back(TypeAnnotation::simple(TypeAnnotation::Kind::Any));
        }
    }

    TypePtr returnType = stmt.returnType ? TypeAnnotation::simple(stmt.returnType->kind)
                                         : TypeAnnotation::simple(TypeAnnotation::Kind::Any);

    auto funcType = TypeAnnotation::function(std::move(paramTypes), std::move(returnType));
    defineVariable(stmt.name, std::move(funcType), 0, 0);

    // Analyze function body
    enterScope(Scope::Type::Function);
    bool wasInFunction = inFunction_;
    inFunction_ = true;

    // Define parameters in function scope
    for (size_t i = 0; i < stmt.params.size(); ++i) {
        TypePtr paramType;
        if (i < stmt.paramTypes.size() && stmt.paramTypes[i]) {
            paramType = TypeAnnotation::simple(stmt.paramTypes[i]->kind);
        } else {
            paramType = TypeAnnotation::simple(TypeAnnotation::Kind::Any);
        }
        defineVariable(stmt.params[i], std::move(paramType), 0, 0);
    }

    for (const auto& s : stmt.body) {
        s->accept(*this);
    }

    inFunction_ = wasInFunction;
    exitScope();
}

void SemanticAnalyzer::visit(ReturnStmt& stmt) {
    if (!inFunction_) {
        addError("Return statement outside of function", 0, 0);
    }
    hasReturnedInCurrentBlock_ = true;

    if (stmt.value) {
        stmt.value->accept(*this);
    }
}

void SemanticAnalyzer::visit(ImportStmt& stmt) {
    // Register imported names in current scope to prevent false "undefined variable" errors
    if (stmt.isWildcard && !stmt.wildcardAlias.empty()) {
        // Only define if not already in scope (avoid conflict with built-ins)
        if (lookupVariable(stmt.wildcardAlias) == nullptr) {
            defineVariable(stmt.wildcardAlias, TypeAnnotation::simple(TypeAnnotation::Kind::Any), 0, 0);
        }
    } else {
        for (size_t i = 0; i < stmt.namedImports.size(); ++i) {
            const std::string& alias = (i < stmt.namedAliases.size() && !stmt.namedAliases[i].empty())
                                           ? stmt.namedAliases[i]
                                           : stmt.namedImports[i];
            // Only define if not already in scope (avoid conflict with built-ins)
            if (lookupVariable(alias) == nullptr) {
                defineVariable(alias, TypeAnnotation::simple(TypeAnnotation::Kind::Any), 0, 0);
            }
        }
    }
}

void SemanticAnalyzer::visit(ExportStmt& stmt) {
    if (stmt.isDefault) {
        if (stmt.declaration) {
            stmt.declaration->accept(*this);
        } else if (stmt.defaultExpr) {
            stmt.defaultExpr->accept(*this);
        }
        return;
    }
    if (stmt.declaration) {
        stmt.declaration->accept(*this);
    }
}

void SemanticAnalyzer::visit(ReExportStmt& /*stmt*/) {
    // Nothing to analyze for re-exports
}

void SemanticAnalyzer::visit(BreakStmt& stmt) {
    if (!inLoop_) {
        addError("Break statement outside of loop", 0, 0);
    }
}

void SemanticAnalyzer::visit(ContinueStmt& stmt) {
    if (!inLoop_) {
        addError("Continue statement outside of loop", 0, 0);
    }
}

void SemanticAnalyzer::visit(TryStmt& stmt) {
    stmt.tryBlock->accept(*this);
    if (stmt.catchBlock) {
        // Enter a scope for the catch block so the catch variable is accessible
        enterScope();
        if (!stmt.catchVariable.empty()) {
            defineVariable(stmt.catchVariable, TypeAnnotation::simple(TypeAnnotation::Kind::Any), 0, 0);
        }
        // Visit the catch block's body directly to avoid entering another scope
        if (auto* block = dynamic_cast<BlockStmt*>(stmt.catchBlock.get())) {
            bool previousReturned = hasReturnedInCurrentBlock_;
            hasReturnedInCurrentBlock_ = false;
            for (const auto& s : block->statements) {
                if (hasReturnedInCurrentBlock_) {
                    addWarning("Unreachable code after return statement", 0, 0);
                    break;
                }
                s->accept(*this);
            }
            hasReturnedInCurrentBlock_ = previousReturned;
        } else {
            stmt.catchBlock->accept(*this);
        }
        exitScope();
    }
    if (stmt.finallyBlock) {
        stmt.finallyBlock->accept(*this);
    }
}

void SemanticAnalyzer::visit(ThrowStmt& stmt) {
    stmt.value->accept(*this);
}

void SemanticAnalyzer::visit(ClassStmt& stmt) {
    // Define class name in current scope (so it can be used as a callable)
    defineVariable(stmt.name, TypeAnnotation::simple(TypeAnnotation::Kind::Any), 0, 0);

    currentClassName_ = stmt.name;
    currentClassFields_.clear();
    currentClassMethods_.clear();

    // Enter a new scope for the class
    enterScope();

    // Check for duplicate fields
    for (const auto& field : stmt.fields) {
        if (currentClassFields_.count(field->name)) {
            addError("Duplicate field '" + field->name + "' in class '" + stmt.name + "'", 0, 0);
        } else {
            currentClassFields_.insert(field->name);
        }

        // Define field type in class scope (not global scope)
        TypePtr fieldType = field->typeAnnotation ? TypeAnnotation::simple(field->typeAnnotation->kind)
                                                  : TypeAnnotation::simple(TypeAnnotation::Kind::Any);

        // Check type of initializer if present
        if (field->initializer) {
            field->initializer->accept(*this);

            // Check type compatibility
            if (field->typeAnnotation && isExplicitlyTyped(fieldType.get())) {
                TypePtr initType = inferType(*field->initializer);
                if (!areTypesCompatible(*fieldType, *initType)) {
                    addError("Type mismatch: field '" + field->name + "' declared as " + fieldType->toString() +
                                 " but initialized with " + initType->toString(),
                             0, 0);
                }
            }
        }
    }

    // Check for duplicate methods and validate constructor naming
    for (const auto& method : stmt.methods) {
        if (currentClassMethods_.count(method->name)) {
            addError("Duplicate method '" + method->name + "' in class '" + stmt.name + "'", 0, 0);
        } else {
            currentClassMethods_.insert(method->name);
        }

        // Validate constructor naming rules
        if (method->name == "constructor") {
            // Constructor is valid - this is the expected name
        } else if (method->name == stmt.name) {
            // Constructor with class name - should be named "constructor"
            addError("Constructor should be named 'constructor', not '" + method->name + "'", 0, 0);
        }

        // Analyze method body with method context
        bool wasInMethod = inMethod_;
        inMethod_ = true;

        enterScope();
        bool wasInFunction = inFunction_;
        inFunction_ = true;

        // Define parameters in function scope
        for (size_t i = 0; i < method->params.size(); ++i) {
            TypePtr paramType;
            if (i < method->paramTypes.size() && method->paramTypes[i]) {
                paramType = TypeAnnotation::simple(method->paramTypes[i]->kind);
            } else {
                paramType = TypeAnnotation::simple(TypeAnnotation::Kind::Any);
            }
            defineVariable(method->params[i], std::move(paramType), 0, 0);
        }

        // Analyze method body
        for (const auto& s : method->body) {
            s->accept(*this);
        }

        inFunction_ = wasInFunction;
        exitScope();
        inMethod_ = wasInMethod;
    }

    // Exit class scope
    exitScope();

    currentClassName_.clear();
    currentClassFields_.clear();
    currentClassMethods_.clear();
}

}  // namespace izi
