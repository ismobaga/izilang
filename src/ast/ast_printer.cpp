#include "ast_printer.hpp"

#include <cmath>
#include <sstream>

namespace izi {

// ── Helpers ───────────────────────────────────────────────────────────────────

std::string AstPrinter::literalToString(const Value& v) {
    if (std::holds_alternative<Nil>(v)) {
        return "nil";
    } else if (std::holds_alternative<bool>(v)) {
        return std::get<bool>(v) ? "true" : "false";
    } else if (std::holds_alternative<double>(v)) {
        double num = std::get<double>(v);
        std::ostringstream oss;
        if (num == std::floor(num) && std::isfinite(num)) {
            oss << static_cast<long long>(num);
        } else {
            oss << num;
        }
        return oss.str();
    } else if (std::holds_alternative<std::string>(v)) {
        return "\"" + std::get<std::string>(v) + "\"";
    }
    return "<value>";
}

std::string AstPrinter::sexp(const std::string& tag, const std::vector<std::string>& children) {
    std::string out = "(" + tag;
    for (const auto& child : children) {
        out += " ";
        out += child;
    }
    out += ")";
    return out;
}

std::string AstPrinter::formatParams(const std::vector<std::string>& params) {
    std::string out = "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) out += " ";
        out += params[i];
    }
    out += ")";
    return out;
}

// ── Public API ────────────────────────────────────────────────────────────────

std::string AstPrinter::print(const std::vector<StmtPtr>& program) {
    output_.clear();
    for (const auto& stmt : program) {
        if (stmt) {
            stmt->accept(*this);
            output_ += "\n";
        }
    }
    return output_;
}

std::string AstPrinter::printStmt(Stmt& stmt) {
    std::string saved = output_;
    output_.clear();
    stmt.accept(*this);
    std::string result = output_;
    output_ = saved;
    return result;
}

std::string AstPrinter::printExpr(Expr& expr) {
    expr.accept(*this);
    return result_;
}

// ── Expression visitors ───────────────────────────────────────────────────────

Value AstPrinter::visit(LiteralExpr& expr) {
    result_ = literalToString(expr.value);
    return Nil{};
}

Value AstPrinter::visit(VariableExpr& expr) {
    result_ = expr.name;
    return Nil{};
}

Value AstPrinter::visit(AssignExpr& expr) {
    std::string val = expr.value ? printExpr(*expr.value) : "nil";
    result_ = sexp("=", {expr.name, val});
    return Nil{};
}

Value AstPrinter::visit(BinaryExpr& expr) {
    std::string left = expr.left ? printExpr(*expr.left) : "nil";
    std::string right = expr.right ? printExpr(*expr.right) : "nil";
    result_ = sexp(expr.op.lexeme, {left, right});
    return Nil{};
}

Value AstPrinter::visit(UnaryExpr& expr) {
    std::string right = expr.right ? printExpr(*expr.right) : "nil";
    result_ = sexp(expr.op.lexeme, {right});
    return Nil{};
}

Value AstPrinter::visit(GroupingExpr& expr) {
    std::string inner = expr.expression ? printExpr(*expr.expression) : "nil";
    result_ = sexp("group", {inner});
    return Nil{};
}

Value AstPrinter::visit(CallExpr& expr) {
    std::vector<std::string> parts;
    parts.push_back(expr.callee ? printExpr(*expr.callee) : "nil");
    for (const auto& arg : expr.args) {
        parts.push_back(arg ? printExpr(*arg) : "nil");
    }
    result_ = sexp("call", parts);
    return Nil{};
}

Value AstPrinter::visit(ConditionalExpr& expr) {
    std::string cond = expr.condition ? printExpr(*expr.condition) : "nil";
    std::string thenB = expr.thenBranch ? printExpr(*expr.thenBranch) : "nil";
    std::string elseB = expr.elseBranch ? printExpr(*expr.elseBranch) : "nil";
    result_ = sexp("?:", {cond, thenB, elseB});
    return Nil{};
}

Value AstPrinter::visit(ArrayExpr& expr) {
    std::vector<std::string> elems;
    for (const auto& elem : expr.elements) {
        elems.push_back(elem ? printExpr(*elem) : "nil");
    }
    result_ = sexp("array", elems);
    return Nil{};
}

Value AstPrinter::visit(MapExpr& expr) {
    std::vector<std::string> entries;
    for (const auto& [key, val] : expr.entries) {
        entries.push_back("\"" + key + "\":" + (val ? printExpr(*val) : "nil"));
    }
    result_ = sexp("map", entries);
    return Nil{};
}

Value AstPrinter::visit(SpreadExpr& expr) {
    std::string arg = expr.argument ? printExpr(*expr.argument) : "nil";
    result_ = sexp("spread", {arg});
    return Nil{};
}

Value AstPrinter::visit(IndexExpr& expr) {
    std::string coll = expr.collection ? printExpr(*expr.collection) : "nil";
    std::string idx = expr.index ? printExpr(*expr.index) : "nil";
    result_ = sexp("index", {coll, idx});
    return Nil{};
}

Value AstPrinter::visit(SetIndexExpr& expr) {
    std::string coll = expr.collection ? printExpr(*expr.collection) : "nil";
    std::string idx = expr.index ? printExpr(*expr.index) : "nil";
    std::string val = expr.value ? printExpr(*expr.value) : "nil";
    result_ = sexp("set-index", {coll, idx, val});
    return Nil{};
}

Value AstPrinter::visit(PropertyExpr& expr) {
    std::string obj = expr.object ? printExpr(*expr.object) : "nil";
    result_ = sexp(".", {obj, expr.property});
    return Nil{};
}

Value AstPrinter::visit(SetPropertyExpr& expr) {
    std::string obj = expr.object ? printExpr(*expr.object) : "nil";
    std::string val = expr.value ? printExpr(*expr.value) : "nil";
    result_ = sexp(".=", {obj, expr.property, val});
    return Nil{};
}

Value AstPrinter::visit(ThisExpr&) {
    result_ = "this";
    return Nil{};
}

Value AstPrinter::visit(SuperExpr& expr) {
    result_ = sexp("super", {expr.method});
    return Nil{};
}

Value AstPrinter::visit(FunctionExpr& expr) {
    std::vector<std::string> parts;
    parts.push_back(formatParams(expr.params));
    for (const auto& stmt : expr.body) {
        if (stmt) parts.push_back(printStmt(*stmt));
    }
    std::string tag = expr.isAsync ? "async fn" : "fn";
    result_ = sexp(tag, parts);
    return Nil{};
}

Value AstPrinter::visit(AwaitExpr& expr) {
    std::string val = expr.value ? printExpr(*expr.value) : "nil";
    result_ = sexp("await", {val});
    return Nil{};
}

Value AstPrinter::visit(MatchExpr& expr) {
    std::vector<std::string> parts;
    parts.push_back(expr.value ? printExpr(*expr.value) : "nil");
    for (const auto& mc : expr.cases) {
        std::string caseStr = "(case";
        if (mc.guard) caseStr += " (if " + printExpr(*mc.guard) + ")";
        caseStr += " " + (mc.result ? printExpr(*mc.result) : "nil") + ")";
        parts.push_back(caseStr);
    }
    result_ = sexp("match", parts);
    return Nil{};
}

// ── Statement visitors ────────────────────────────────────────────────────────

void AstPrinter::visit(ExprStmt& stmt) {
    output_ += stmt.expr ? printExpr(*stmt.expr) : "nil";
}

void AstPrinter::visit(BlockStmt& stmt) {
    std::vector<std::string> parts;
    for (const auto& s : stmt.statements) {
        if (s) parts.push_back(printStmt(*s));
    }
    output_ += sexp("block", parts);
}

void AstPrinter::visit(VarStmt& stmt) {
    std::string init = stmt.initializer ? printExpr(*stmt.initializer) : "nil";
    output_ += sexp("var", {stmt.name.empty() ? "<pattern>" : stmt.name, init});
}

void AstPrinter::visit(WhileStmt& stmt) {
    std::string cond = stmt.condition ? printExpr(*stmt.condition) : "nil";
    std::string body = stmt.body ? printStmt(*stmt.body) : "nil";
    output_ += sexp("while", {cond, body});
}

void AstPrinter::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        output_ += sexp("return", {printExpr(*stmt.value)});
    } else {
        output_ += "(return)";
    }
}

void AstPrinter::visit(FunctionStmt& stmt) {
    std::vector<std::string> parts;
    parts.push_back(stmt.name);
    parts.push_back(formatParams(stmt.params));
    for (const auto& s : stmt.body) {
        if (s) parts.push_back(printStmt(*s));
    }
    std::string tag = stmt.isAsync ? "async fn" : "fn";
    output_ += sexp(tag, parts);
}

void AstPrinter::visit(IfStmt& stmt) {
    std::string cond = stmt.condition ? printExpr(*stmt.condition) : "nil";
    std::string thenB = stmt.thenBranch ? printStmt(*stmt.thenBranch) : "nil";
    if (stmt.elseBranch) {
        output_ += sexp("if", {cond, thenB, printStmt(*stmt.elseBranch)});
    } else {
        output_ += sexp("if", {cond, thenB});
    }
}

void AstPrinter::visit(ImportStmt& stmt) {
    if (stmt.isWildcard) {
        output_ += sexp("import*", {"\"" + stmt.module + "\"", "as", stmt.wildcardAlias});
    } else if (!stmt.namedImports.empty()) {
        std::vector<std::string> parts = {"\"" + stmt.module + "\""};
        for (size_t i = 0; i < stmt.namedImports.size(); ++i) {
            std::string entry = stmt.namedImports[i];
            if (i < stmt.namedAliases.size() && !stmt.namedAliases[i].empty()) {
                entry += " as " + stmt.namedAliases[i];
            }
            parts.push_back(entry);
        }
        output_ += sexp("import", parts);
    } else {
        output_ += sexp("import", {"\"" + stmt.module + "\""});
    }
}

void AstPrinter::visit(ExportStmt& stmt) {
    if (stmt.isDefault) {
        if (stmt.declaration) {
            output_ += sexp("export default", {printStmt(*stmt.declaration)});
        } else if (stmt.defaultExpr) {
            output_ += sexp("export default", {printExpr(*stmt.defaultExpr)});
        } else {
            output_ += "(export default)";
        }
    } else if (stmt.declaration) {
        output_ += sexp("export", {printStmt(*stmt.declaration)});
    } else {
        output_ += "(export)";
    }
}

void AstPrinter::visit(ReExportStmt& stmt) {
    std::vector<std::string> parts = {"\"" + stmt.module + "\""};
    if (stmt.isWildcard) {
        parts.push_back("*");
    } else {
        for (const auto& name : stmt.names) {
            parts.push_back(name);
        }
    }
    output_ += sexp("re-export", parts);
}

void AstPrinter::visit(BreakStmt&) {
    output_ += "break";
}

void AstPrinter::visit(ContinueStmt&) {
    output_ += "continue";
}

void AstPrinter::visit(TryStmt& stmt) {
    std::vector<std::string> parts;
    parts.push_back(stmt.tryBlock ? printStmt(*stmt.tryBlock) : "nil");
    if (stmt.catchBlock) {
        parts.push_back(sexp("catch", {stmt.catchVariable, printStmt(*stmt.catchBlock)}));
    }
    if (stmt.finallyBlock) {
        parts.push_back(sexp("finally", {printStmt(*stmt.finallyBlock)}));
    }
    output_ += sexp("try", parts);
}

void AstPrinter::visit(ThrowStmt& stmt) {
    std::string val = stmt.value ? printExpr(*stmt.value) : "nil";
    output_ += sexp("throw", {val});
}

void AstPrinter::visit(ClassStmt& stmt) {
    std::vector<std::string> parts;
    parts.push_back(stmt.name);
    if (!stmt.superclass.empty()) {
        parts.push_back(sexp("extends", {stmt.superclass}));
    }
    for (const auto& field : stmt.fields) {
        if (field) parts.push_back(printStmt(*field));
    }
    for (const auto& method : stmt.methods) {
        if (method) parts.push_back(printStmt(*method));
    }
    output_ += sexp("class", parts);
}

}  // namespace izi
