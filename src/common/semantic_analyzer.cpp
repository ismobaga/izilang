#include "semantic_analyzer.hpp"
#include <iostream>

namespace izi {

void SemanticAnalyzer::analyze(const std::vector<StmtPtr>& program) {
    diagnostics_.clear();
    currentScope_ = std::make_shared<Scope>();
    
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

void SemanticAnalyzer::enterScope() {
    auto newScope = std::make_shared<Scope>();
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
    // For now, return Any type for untyped expressions
    return TypeAnnotation::simple(TypeAnnotation::Kind::Any);
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

Value SemanticAnalyzer::visit(CallExpr& expr) {
    expr.callee->accept(*this);
    for (auto& arg : expr.args) {
        arg->accept(*this);
    }
    return Nil{};
}

Value SemanticAnalyzer::visit(VariableExpr& expr) {
    markVariableUsed(expr.name);
    return Nil{};
}

Value SemanticAnalyzer::visit(AssignExpr& expr) {
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
    TypePtr varType = stmt.typeAnnotation ? 
        TypeAnnotation::simple(stmt.typeAnnotation->kind) : 
        TypeAnnotation::simple(TypeAnnotation::Kind::Any);
    
    if (stmt.initializer) {
        stmt.initializer->accept(*this);
        // TODO: Check type compatibility if type annotation present
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
    
    TypePtr returnType = stmt.returnType ? 
        TypeAnnotation::simple(stmt.returnType->kind) : 
        TypeAnnotation::simple(TypeAnnotation::Kind::Any);
    
    auto funcType = TypeAnnotation::function(std::move(paramTypes), std::move(returnType));
    defineVariable(stmt.name, std::move(funcType), 0, 0);
    
    // Analyze function body
    enterScope();
    bool wasInFunction = inFunction_;
    inFunction_ = true;
    
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
    // Nothing to analyze for imports
}

void SemanticAnalyzer::visit(ExportStmt& stmt) {
    stmt.declaration->accept(*this);
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
        stmt.catchBlock->accept(*this);
    }
    if (stmt.finallyBlock) {
        stmt.finallyBlock->accept(*this);
    }
}

void SemanticAnalyzer::visit(ThrowStmt& stmt) {
    stmt.value->accept(*this);
}

void SemanticAnalyzer::visit(ClassStmt& stmt) {
    // TODO: Implement class semantic analysis
    addInfo("Class '" + stmt.name + "' semantic analysis not yet implemented", 0, 0);
}

} // namespace izi
