#include "optimizer.hpp"
#include <cmath>

namespace izi {

std::vector<StmtPtr> Optimizer::optimize(std::vector<StmtPtr> program) {
    std::vector<StmtPtr> optimized;
    for (auto& stmt : program) {
        auto opt = optimizeStmt(std::move(stmt));
        if (opt) {
            optimized.push_back(std::move(opt));
        }
    }
    return optimized;
}

ExprPtr Optimizer::optimizeExpr(ExprPtr expr) {
    if (!expr) return nullptr;

    // Visit the expression to optimize it
    expr->accept(*this);

    // Return the optimized expression
    return std::move(currentExpr);
}

StmtPtr Optimizer::optimizeStmt(StmtPtr stmt) {
    if (!stmt) return nullptr;

    // Visit the statement to optimize it
    stmt->accept(*this);

    // Return the optimized statement
    return std::move(currentStmt);
}

bool Optimizer::isConstant(const Expr& expr) const {
    return dynamic_cast<const LiteralExpr*>(&expr) != nullptr;
}

Value Optimizer::evaluateConstantBinary(const Value& left, TokenType op, const Value& right) {
    // Only handle number operations for now
    if (!std::holds_alternative<double>(left) || !std::holds_alternative<double>(right)) {
        return Nil{};  // Not a constant we can fold
    }

    double l = std::get<double>(left);
    double r = std::get<double>(right);

    switch (op) {
        case TokenType::PLUS:
            return l + r;
        case TokenType::MINUS:
            return l - r;
        case TokenType::STAR:
            return l * r;
        case TokenType::SLASH:
            return l / r;
        case TokenType::GREATER:
            return l > r;
        case TokenType::GREATER_EQUAL:
            return l >= r;
        case TokenType::LESS:
            return l < r;
        case TokenType::LESS_EQUAL:
            return l <= r;
        case TokenType::EQUAL_EQUAL:
            return l == r;
        case TokenType::BANG_EQUAL:
            return l != r;
        default:
            return Nil{};
    }
}

Value Optimizer::evaluateConstantUnary(TokenType op, const Value& right) {
    switch (op) {
        case TokenType::MINUS:
            if (std::holds_alternative<double>(right)) {
                return -std::get<double>(right);
            }
            break;
        case TokenType::BANG:
            return !isTruthy(right);
        default:
            break;
    }
    return Nil{};
}

// Expression visitors

Value Optimizer::visit(BinaryExpr& expr) {
    // Optimize children first
    auto left = optimizeExpr(std::move(expr.left));
    auto right = optimizeExpr(std::move(expr.right));

    // Constant folding: if both operands are constants, evaluate at compile time
    if (isConstant(*left) && isConstant(*right)) {
        auto* leftLit = static_cast<LiteralExpr*>(left.get());
        auto* rightLit = static_cast<LiteralExpr*>(right.get());

        Value result = evaluateConstantBinary(leftLit->value, expr.op.type, rightLit->value);

        // If we got a valid result, return a literal expression
        if (!std::holds_alternative<Nil>(result)) {
            currentExpr = std::make_unique<LiteralExpr>(result);
            return result;
        }
    }

    // No optimization possible, return the original expression with optimized children
    expr.left = std::move(left);
    expr.right = std::move(right);
    currentExpr = std::make_unique<BinaryExpr>(std::move(expr.left), expr.op, std::move(expr.right));
    return Nil{};
}

Value Optimizer::visit(UnaryExpr& expr) {
    // Optimize child first
    auto right = optimizeExpr(std::move(expr.right));

    // Constant folding: if operand is constant, evaluate at compile time
    if (isConstant(*right)) {
        auto* rightLit = static_cast<LiteralExpr*>(right.get());
        Value result = evaluateConstantUnary(expr.op.type, rightLit->value);

        if (!std::holds_alternative<Nil>(result)) {
            currentExpr = std::make_unique<LiteralExpr>(result);
            return result;
        }
    }

    // No optimization possible, return the original expression with optimized child
    expr.right = std::move(right);
    currentExpr = std::make_unique<UnaryExpr>(expr.op, std::move(expr.right));
    return Nil{};
}

Value Optimizer::visit(CallExpr& expr) {
    // Optimize callee and arguments
    expr.callee = optimizeExpr(std::move(expr.callee));
    for (auto& arg : expr.args) {
        arg = optimizeExpr(std::move(arg));
    }
    currentExpr = std::make_unique<CallExpr>(std::move(expr.callee), std::move(expr.args));
    return Nil{};
}

Value Optimizer::visit(LiteralExpr& expr) {
    // Literals are already optimized
    currentExpr = std::make_unique<LiteralExpr>(expr.value);
    return expr.value;
}

Value Optimizer::visit(AssignExpr& expr) {
    expr.value = optimizeExpr(std::move(expr.value));
    currentExpr = std::make_unique<AssignExpr>(expr.name, std::move(expr.value));
    return Nil{};
}

Value Optimizer::visit(VariableExpr& expr) {
    if (expr.value) {
        expr.value = optimizeExpr(std::move(expr.value));
    }
    currentExpr = std::make_unique<VariableExpr>(expr.name, std::move(expr.value));
    return Nil{};
}

Value Optimizer::visit(GroupingExpr& expr) {
    expr.expression = optimizeExpr(std::move(expr.expression));
    currentExpr = std::make_unique<GroupingExpr>(std::move(expr.expression));
    return Nil{};
}

Value Optimizer::visit(ConditionalExpr& expr) {
    expr.condition = optimizeExpr(std::move(expr.condition));
    expr.thenBranch = optimizeExpr(std::move(expr.thenBranch));
    expr.elseBranch = optimizeExpr(std::move(expr.elseBranch));

    // Constant folding: if condition is a literal, only keep the appropriate branch
    if (auto* lit = dynamic_cast<LiteralExpr*>(expr.condition.get())) {
        if (isTruthy(lit->value)) {
            currentExpr = std::move(expr.thenBranch);
        } else {
            currentExpr = std::move(expr.elseBranch);
        }
        return Nil{};
    }

    currentExpr = std::make_unique<ConditionalExpr>(std::move(expr.condition), std::move(expr.thenBranch),
                                                    std::move(expr.elseBranch));
    return Nil{};
}

Value Optimizer::visit(ArrayExpr& expr) {
    for (auto& elem : expr.elements) {
        elem = optimizeExpr(std::move(elem));
    }
    currentExpr = std::make_unique<ArrayExpr>(std::move(expr.elements));
    return Nil{};
}

Value Optimizer::visit(MapExpr& expr) {
    for (auto& entry : expr.entries) {
        entry.second = optimizeExpr(std::move(entry.second));
    }
    currentExpr = std::make_unique<MapExpr>(std::move(expr.entries));
    return Nil{};
}

Value Optimizer::visit(SpreadExpr& expr) {
    expr.argument = optimizeExpr(std::move(expr.argument));
    currentExpr = std::make_unique<SpreadExpr>(std::move(expr.argument));
    return Nil{};
}

Value Optimizer::visit(IndexExpr& expr) {
    expr.collection = optimizeExpr(std::move(expr.collection));
    expr.index = optimizeExpr(std::move(expr.index));
    currentExpr = std::make_unique<IndexExpr>(std::move(expr.collection), std::move(expr.index));
    return Nil{};
}

Value Optimizer::visit(SetIndexExpr& expr) {
    expr.collection = optimizeExpr(std::move(expr.collection));
    expr.index = optimizeExpr(std::move(expr.index));
    expr.value = optimizeExpr(std::move(expr.value));
    currentExpr =
        std::make_unique<SetIndexExpr>(std::move(expr.collection), std::move(expr.index), std::move(expr.value));
    return Nil{};
}

Value Optimizer::visit(FunctionExpr& expr) {
    // Optimize function body with dead code elimination
    std::vector<StmtPtr> optimizedBody;
    bool foundReturn = false;

    for (auto& stmt : expr.body) {
        // Dead code elimination: don't include statements after return
        if (foundReturn) {
            continue;
        }

        auto opt = optimizeStmt(std::move(stmt));
        if (opt) {
            optimizedBody.push_back(std::move(opt));

            // Check if this is a return statement
            if (dynamic_cast<ReturnStmt*>(optimizedBody.back().get())) {
                foundReturn = true;
            }
        }
    }
    currentExpr = std::make_unique<FunctionExpr>(expr.params, std::move(optimizedBody));
    return Nil{};
}

Value Optimizer::visit(MatchExpr& expr) {
    expr.value = optimizeExpr(std::move(expr.value));
    for (auto& matchCase : expr.cases) {
        if (matchCase.guard) {
            matchCase.guard = optimizeExpr(std::move(matchCase.guard));
        }
        matchCase.result = optimizeExpr(std::move(matchCase.result));
    }
    currentExpr = std::make_unique<MatchExpr>(std::move(expr.value), std::move(expr.cases));
    return Nil{};
}

Value Optimizer::visit(PropertyExpr& expr) {
    expr.object = optimizeExpr(std::move(expr.object));
    currentExpr = std::make_unique<PropertyExpr>(std::move(expr.object), expr.property);
    return Nil{};
}

Value Optimizer::visit(SetPropertyExpr& expr) {
    expr.object = optimizeExpr(std::move(expr.object));
    expr.value = optimizeExpr(std::move(expr.value));
    currentExpr = std::make_unique<SetPropertyExpr>(std::move(expr.object), expr.property, std::move(expr.value));
    return Nil{};
}

Value Optimizer::visit(ThisExpr& expr) {
    currentExpr = std::make_unique<ThisExpr>();
    return Nil{};
}

Value Optimizer::visit(SuperExpr& expr) {
    currentExpr = std::make_unique<SuperExpr>(expr.method);
    return Nil{};
}

Value Optimizer::visit(AwaitExpr& expr) {
    expr.value = optimizeExpr(std::move(expr.value));
    currentExpr = std::make_unique<AwaitExpr>(std::move(expr.value));
    return Nil{};
}

// Statement visitors

void Optimizer::visit(ExprStmt& stmt) {
    stmt.expr = optimizeExpr(std::move(stmt.expr));
    currentStmt = std::make_unique<ExprStmt>(std::move(stmt.expr));
}

void Optimizer::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        stmt.value = optimizeExpr(std::move(stmt.value));
    }
    currentStmt = std::make_unique<ReturnStmt>(std::move(stmt.value));
}

void Optimizer::visit(IfStmt& stmt) {
    stmt.condition = optimizeExpr(std::move(stmt.condition));
    stmt.thenBranch = optimizeStmt(std::move(stmt.thenBranch));
    if (stmt.elseBranch) {
        stmt.elseBranch = optimizeStmt(std::move(stmt.elseBranch));
    }

    // Constant folding for if statements: if condition is a constant, eliminate dead branch
    if (isConstant(*stmt.condition)) {
        auto* condLit = static_cast<LiteralExpr*>(stmt.condition.get());
        if (isTruthy(condLit->value)) {
            // Condition is always true, keep only then branch
            currentStmt = std::move(stmt.thenBranch);
        } else {
            // Condition is always false, keep only else branch (or nothing)
            currentStmt = std::move(stmt.elseBranch);
        }
    } else {
        currentStmt =
            std::make_unique<IfStmt>(std::move(stmt.condition), std::move(stmt.thenBranch), std::move(stmt.elseBranch));
    }
}

void Optimizer::visit(WhileStmt& stmt) {
    stmt.condition = optimizeExpr(std::move(stmt.condition));
    stmt.body = optimizeStmt(std::move(stmt.body));

    // Dead code elimination: if condition is constant false, remove the loop
    if (isConstant(*stmt.condition)) {
        auto* condLit = static_cast<LiteralExpr*>(stmt.condition.get());
        if (!isTruthy(condLit->value)) {
            // Loop never executes, return null to eliminate it
            currentStmt = nullptr;
            return;
        }
    }

    currentStmt = std::make_unique<WhileStmt>(std::move(stmt.condition), std::move(stmt.body));
}

void Optimizer::visit(BlockStmt& stmt) {
    std::vector<StmtPtr> optimizedStmts;
    bool foundReturn = false;

    for (auto& s : stmt.statements) {
        // Dead code elimination: don't include statements after return
        if (foundReturn) {
            continue;
        }

        auto opt = optimizeStmt(std::move(s));
        if (opt) {
            optimizedStmts.push_back(std::move(opt));

            // Check if this is a return statement
            if (dynamic_cast<ReturnStmt*>(optimizedStmts.back().get())) {
                foundReturn = true;
            }
        }
    }

    currentStmt = std::make_unique<BlockStmt>(std::move(optimizedStmts));
}

void Optimizer::visit(VarStmt& stmt) {
    if (stmt.initializer) {
        stmt.initializer = optimizeExpr(std::move(stmt.initializer));
    }

    if (stmt.pattern) {
        currentStmt = std::make_unique<VarStmt>(std::move(stmt.pattern), std::move(stmt.initializer),
                                                std::move(stmt.typeAnnotation));
    } else {
        currentStmt = std::make_unique<VarStmt>(stmt.name, std::move(stmt.initializer), std::move(stmt.typeAnnotation));
    }
}

void Optimizer::visit(FunctionStmt& stmt) {
    // Optimize function body with dead code elimination
    std::vector<StmtPtr> optimizedBody;
    bool foundReturn = false;

    for (auto& s : stmt.body) {
        // Dead code elimination: don't include statements after return
        if (foundReturn) {
            continue;
        }

        auto opt = optimizeStmt(std::move(s));
        if (opt) {
            optimizedBody.push_back(std::move(opt));

            // Check if this is a return statement
            if (dynamic_cast<ReturnStmt*>(optimizedBody.back().get())) {
                foundReturn = true;
            }
        }
    }

    currentStmt = std::make_unique<FunctionStmt>(stmt.name, stmt.params, std::move(optimizedBody),
                                                 std::move(stmt.paramTypes), std::move(stmt.returnType));
}

void Optimizer::visit(ImportStmt& stmt) {
    // Import statements don't need optimization
    if (stmt.isWildcard) {
        currentStmt = std::make_unique<ImportStmt>(stmt.module, stmt.wildcardAlias, stmt.isWildcard);
    } else if (!stmt.namedImports.empty()) {
        currentStmt = std::make_unique<ImportStmt>(stmt.module, stmt.namedImports);
    } else {
        currentStmt = std::make_unique<ImportStmt>(stmt.module);
    }
}

void Optimizer::visit(ExportStmt& stmt) {
    if (stmt.isDefault) {
        if (stmt.declaration) {
            stmt.declaration = optimizeStmt(std::move(stmt.declaration));
            currentStmt = std::make_unique<ExportStmt>(std::move(stmt.declaration), true);
        } else if (stmt.defaultExpr) {
            stmt.defaultExpr = optimizeExpr(std::move(stmt.defaultExpr));
            currentStmt = std::make_unique<ExportStmt>(std::move(stmt.defaultExpr));
        }
        return;
    }
    stmt.declaration = optimizeStmt(std::move(stmt.declaration));
    currentStmt = std::make_unique<ExportStmt>(std::move(stmt.declaration));
}

void Optimizer::visit(ReExportStmt& stmt) {
    // Re-exports have no sub-expressions to optimize
    if (stmt.isWildcard) {
        currentStmt = std::make_unique<ReExportStmt>(stmt.module);
    } else {
        currentStmt = std::make_unique<ReExportStmt>(stmt.module, stmt.names);
    }
}

void Optimizer::visit(BreakStmt& stmt) {
    currentStmt = std::make_unique<BreakStmt>();
}

void Optimizer::visit(ContinueStmt& stmt) {
    currentStmt = std::make_unique<ContinueStmt>();
}

void Optimizer::visit(TryStmt& stmt) {
    stmt.tryBlock = optimizeStmt(std::move(stmt.tryBlock));
    if (stmt.catchBlock) {
        stmt.catchBlock = optimizeStmt(std::move(stmt.catchBlock));
    }
    if (stmt.finallyBlock) {
        stmt.finallyBlock = optimizeStmt(std::move(stmt.finallyBlock));
    }

    currentStmt = std::make_unique<TryStmt>(std::move(stmt.tryBlock), stmt.catchVariable, std::move(stmt.catchBlock),
                                            std::move(stmt.finallyBlock));
}

void Optimizer::visit(ThrowStmt& stmt) {
    stmt.value = optimizeExpr(std::move(stmt.value));
    currentStmt = std::make_unique<ThrowStmt>(stmt.keyword, std::move(stmt.value));
}

void Optimizer::visit(ClassStmt& stmt) {
    // Optimize field initializers
    for (auto& field : stmt.fields) {
        if (field->initializer) {
            field->initializer = optimizeExpr(std::move(field->initializer));
        }
    }

    // Optimize method bodies
    for (auto& method : stmt.methods) {
        std::vector<StmtPtr> optimizedBody;
        for (auto& s : method->body) {
            auto opt = optimizeStmt(std::move(s));
            if (opt) {
                optimizedBody.push_back(std::move(opt));
            }
        }
        method->body = std::move(optimizedBody);
    }

    currentStmt =
        std::make_unique<ClassStmt>(stmt.name, stmt.superclass, std::move(stmt.fields), std::move(stmt.methods));
}

}  // namespace izi
