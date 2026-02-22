#include "formatter.hpp"
#include <sstream>
#include <cmath>
#include <fstream>
#include <cctype>

namespace izi {

// ── FormatterConfig ───────────────────────────────────────────────────────────

static std::string trimStr(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

FormatterConfig FormatterConfig::load(const std::string& path) {
    FormatterConfig config;
    std::ifstream f(path);
    if (!f.is_open()) return config;

    std::string line;
    while (std::getline(f, line)) {
        // Strip inline comments
        auto commentPos = line.find('#');
        if (commentPos != std::string::npos) line = line.substr(0, commentPos);
        auto eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;
        std::string key = trimStr(line.substr(0, eqPos));
        std::string val = trimStr(line.substr(eqPos + 1));
        if (val.empty()) continue;

        // Parse a positive integer option, storing in `dest` on success.
        auto parsePositiveInt = [&](int& dest) {
            try {
                int v = std::stoi(val);
                if (v > 0) dest = v;
            } catch (...) {
                // Ignore malformed values
            }
        };

        if (key == "indent_size") {
            parsePositiveInt(config.indentSize);
        } else if (key == "max_line_length") {
            parsePositiveInt(config.maxLineLength);
        }
    }
    return config;
}

// ── Formatter constructor ─────────────────────────────────────────────────────

Formatter::Formatter(FormatterConfig cfg) : config_(std::move(cfg)) {}

// Helper: escape a raw string value for output as a quoted string literal
static std::string escapeString(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:   out += c;      break;
        }
    }
    return out;
}

// ── Public API ────────────────────────────────────────────────────────────────

std::string Formatter::format(const std::vector<StmtPtr>& program) {
    output_.clear();
    indentLevel_ = 0;

    for (size_t i = 0; i < program.size(); ++i) {
        if (!program[i]) continue;

        // Blank line between top-level declarations (functions, classes)
        if (i > 0) {
            bool prevIsDecl = dynamic_cast<FunctionStmt*>(program[i - 1].get()) != nullptr ||
                              dynamic_cast<ClassStmt*>(program[i - 1].get()) != nullptr;
            bool curIsDecl = dynamic_cast<FunctionStmt*>(program[i].get()) != nullptr ||
                             dynamic_cast<ClassStmt*>(program[i].get()) != nullptr;
            if (prevIsDecl || curIsDecl) {
                output_ += "\n";
            }
        }

        program[i]->accept(*this);
    }

    return output_;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

std::string Formatter::indent() const {
    return std::string(static_cast<size_t>(indentLevel_) * static_cast<size_t>(config_.indentSize), ' ');
}

std::string Formatter::formatExpr(Expr& expr) {
    expr.accept(*this);
    return currentExpr_;
}

std::string Formatter::formatType(const TypeAnnotation& type) const {
    switch (type.kind) {
        case TypeAnnotation::Kind::Number:  return "Number";
        case TypeAnnotation::Kind::String:  return "String";
        case TypeAnnotation::Kind::Bool:    return "Bool";
        case TypeAnnotation::Kind::Nil:     return "Nil";
        case TypeAnnotation::Kind::Any:     return "Any";
        case TypeAnnotation::Kind::Void:    return "Void";
        case TypeAnnotation::Kind::Array: {
            if (type.elementType) {
                return "Array<" + formatType(*type.elementType) + ">";
            }
            return "Array";
        }
        case TypeAnnotation::Kind::Map: {
            if (type.keyType && type.valueType) {
                return "Map<" + formatType(*type.keyType) + ", " + formatType(*type.valueType) + ">";
            }
            return "Map";
        }
        case TypeAnnotation::Kind::Function: {
            std::string s = "Function(";
            for (size_t i = 0; i < type.paramTypes.size(); ++i) {
                if (i > 0) s += ", ";
                s += formatType(*type.paramTypes[i]);
            }
            s += ")";
            if (type.valueType) {
                s += " -> " + formatType(*type.valueType);
            }
            return s;
        }
    }
    return "Any";
}

std::string Formatter::formatPattern(const Pattern& pattern) const {
    if (auto* wp = dynamic_cast<const WildcardPattern*>(&pattern)) {
        (void)wp;
        return "_";
    }
    if (auto* lp = dynamic_cast<const LiteralPattern*>(&pattern)) {
        const Value& v = lp->value;
        if (std::holds_alternative<Nil>(v)) return "nil";
        if (std::holds_alternative<bool>(v)) return std::get<bool>(v) ? "true" : "false";
        if (std::holds_alternative<double>(v)) {
            double num = std::get<double>(v);
            std::ostringstream oss;
            if (num == std::floor(num) && std::isfinite(num)) {
                oss << static_cast<long long>(num);
            } else {
                oss << num;
            }
            return oss.str();
        }
        if (std::holds_alternative<std::string>(v)) {
            return "\"" + escapeString(std::get<std::string>(v)) + "\"";
        }
        return "_";
    }
    if (auto* vp = dynamic_cast<const VariablePattern*>(&pattern)) {
        return vp->name;
    }
    if (auto* ap = dynamic_cast<const ArrayPattern*>(&pattern)) {
        std::string s = "[";
        for (size_t i = 0; i < ap->elements.size(); ++i) {
            if (i > 0) s += ", ";
            s += formatPattern(*ap->elements[i]);
        }
        s += "]";
        return s;
    }
    if (auto* mp = dynamic_cast<const MapPattern*>(&pattern)) {
        std::string s = "{";
        for (size_t i = 0; i < mp->keys.size(); ++i) {
            if (i > 0) s += ", ";
            s += mp->keys[i];
        }
        s += "}";
        return s;
    }
    return "_";
}

std::string Formatter::tokenTypeToStr(TokenType type) const {
    switch (type) {
        case TokenType::PLUS:          return "+";
        case TokenType::MINUS:         return "-";
        case TokenType::STAR:          return "*";
        case TokenType::SLASH:         return "/";
        case TokenType::PERCENT:       return "%";
        case TokenType::EQUAL_EQUAL:   return "==";
        case TokenType::BANG_EQUAL:    return "!=";
        case TokenType::LESS:          return "<";
        case TokenType::LESS_EQUAL:    return "<=";
        case TokenType::GREATER:       return ">";
        case TokenType::GREATER_EQUAL: return ">=";
        case TokenType::AND:           return "and";
        case TokenType::OR:            return "or";
        case TokenType::QUESTION_QUESTION: return "??";
        case TokenType::BANG:          return "!";
        case TokenType::PLUS_EQUAL:    return "+=";
        case TokenType::MINUS_EQUAL:   return "-=";
        case TokenType::STAR_EQUAL:    return "*=";
        case TokenType::SLASH_EQUAL:   return "/=";
        case TokenType::PERCENT_EQUAL: return "%=";
        default:                       return "?";
    }
}

void Formatter::formatFunctionBody(const std::vector<std::string>& params,
                                   const std::vector<TypePtr>& paramTypes,
                                   const TypePtr& returnType,
                                   const std::vector<StmtPtr>& body) {
    output_ += "(";
    for (size_t i = 0; i < params.size(); ++i) {
        if (i > 0) output_ += ", ";
        output_ += params[i];
        if (i < paramTypes.size() && paramTypes[i]) {
            output_ += ": " + formatType(*paramTypes[i]);
        }
    }
    output_ += ")";
    if (returnType) {
        output_ += ": " + formatType(*returnType);
    }
    output_ += " {\n";
    indentLevel_++;
    for (auto& stmt : body) {
        if (stmt) stmt->accept(*this);
    }
    indentLevel_--;
    output_ += indent() + "}";
}

// ── Expression Visitors ───────────────────────────────────────────────────────

Value Formatter::visit(LiteralExpr& expr) {
    const Value& v = expr.value;
    if (std::holds_alternative<Nil>(v)) {
        currentExpr_ = "nil";
    } else if (std::holds_alternative<bool>(v)) {
        currentExpr_ = std::get<bool>(v) ? "true" : "false";
    } else if (std::holds_alternative<double>(v)) {
        double num = std::get<double>(v);
        std::ostringstream oss;
        if (num == std::floor(num) && std::isfinite(num)) {
            oss << static_cast<long long>(num);
        } else {
            oss << num;
        }
        currentExpr_ = oss.str();
    } else if (std::holds_alternative<std::string>(v)) {
        currentExpr_ = "\"" + escapeString(std::get<std::string>(v)) + "\"";
    } else {
        currentExpr_ = "nil";
    }
    return Nil{};
}

Value Formatter::visit(VariableExpr& expr) {
    currentExpr_ = expr.name;
    return Nil{};
}

Value Formatter::visit(AssignExpr& expr) {
    std::string val = formatExpr(*expr.value);
    currentExpr_ = expr.name + " = " + val;
    return Nil{};
}

Value Formatter::visit(BinaryExpr& expr) {
    std::string left = formatExpr(*expr.left);
    std::string right = formatExpr(*expr.right);
    currentExpr_ = left + " " + tokenTypeToStr(expr.op.type) + " " + right;
    return Nil{};
}

Value Formatter::visit(UnaryExpr& expr) {
    std::string op = tokenTypeToStr(expr.op.type);
    std::string right = formatExpr(*expr.right);
    // No space between unary operator and operand
    currentExpr_ = op + right;
    return Nil{};
}

Value Formatter::visit(GroupingExpr& expr) {
    std::string inner = formatExpr(*expr.expression);
    currentExpr_ = "(" + inner + ")";
    return Nil{};
}

Value Formatter::visit(ConditionalExpr& expr) {
    std::string cond  = formatExpr(*expr.condition);
    std::string then  = formatExpr(*expr.thenBranch);
    std::string els   = formatExpr(*expr.elseBranch);
    currentExpr_ = cond + " ? " + then + " : " + els;
    return Nil{};
}

Value Formatter::visit(CallExpr& expr) {
    std::string callee = formatExpr(*expr.callee);
    std::string s = callee + "(";
    for (size_t i = 0; i < expr.args.size(); ++i) {
        if (i > 0) s += ", ";
        s += formatExpr(*expr.args[i]);
    }
    s += ")";
    currentExpr_ = s;
    return Nil{};
}

Value Formatter::visit(ArrayExpr& expr) {
    std::string s = "[";
    for (size_t i = 0; i < expr.elements.size(); ++i) {
        if (i > 0) s += ", ";
        s += formatExpr(*expr.elements[i]);
    }
    s += "]";
    currentExpr_ = s;
    return Nil{};
}

Value Formatter::visit(MapExpr& expr) {
    std::string s = "{";
    for (size_t i = 0; i < expr.entries.size(); ++i) {
        if (i > 0) s += ", ";
        s += "\"" + escapeString(expr.entries[i].first) + "\": " + formatExpr(*expr.entries[i].second);
    }
    s += "}";
    currentExpr_ = s;
    return Nil{};
}

Value Formatter::visit(SpreadExpr& expr) {
    currentExpr_ = "..." + formatExpr(*expr.argument);
    return Nil{};
}

Value Formatter::visit(IndexExpr& expr) {
    std::string coll = formatExpr(*expr.collection);
    std::string idx  = formatExpr(*expr.index);
    currentExpr_ = coll + "[" + idx + "]";
    return Nil{};
}

Value Formatter::visit(SetIndexExpr& expr) {
    std::string coll = formatExpr(*expr.collection);
    std::string idx  = formatExpr(*expr.index);
    std::string val  = formatExpr(*expr.value);
    currentExpr_ = coll + "[" + idx + "] = " + val;
    return Nil{};
}

Value Formatter::visit(PropertyExpr& expr) {
    std::string obj = formatExpr(*expr.object);
    currentExpr_ = obj + "." + expr.property;
    return Nil{};
}

Value Formatter::visit(SetPropertyExpr& expr) {
    std::string obj = formatExpr(*expr.object);
    std::string val = formatExpr(*expr.value);
    currentExpr_ = obj + "." + expr.property + " = " + val;
    return Nil{};
}

Value Formatter::visit(ThisExpr&) {
    currentExpr_ = "this";
    return Nil{};
}

Value Formatter::visit(SuperExpr& expr) {
    currentExpr_ = "super." + expr.method;
    return Nil{};
}

Value Formatter::visit(AwaitExpr& expr) {
    currentExpr_ = "await " + formatExpr(*expr.value);
    return Nil{};
}

Value Formatter::visit(FunctionExpr& expr) {
    std::string s = (expr.isAsync ? "async fn" : "fn");
    s += "(";
    for (size_t i = 0; i < expr.params.size(); ++i) {
        if (i > 0) s += ", ";
        s += expr.params[i];
    }
    s += ") {\n";

    // Temporarily capture function body into output_ using indentation
    int savedIndent = indentLevel_;
    indentLevel_++;
    std::string savedOutput = output_;
    output_ = "";

    for (auto& stmt : expr.body) {
        if (stmt) stmt->accept(*this);
    }

    std::string bodyStr = output_;
    output_ = savedOutput;
    indentLevel_ = savedIndent;

    s += bodyStr;
    s += indent() + "}";
    currentExpr_ = s;
    return Nil{};
}

Value Formatter::visit(MatchExpr& expr) {
    std::string s = "match " + formatExpr(*expr.value) + " {\n";
    indentLevel_++;
    for (auto& mc : expr.cases) {
        s += indent() + formatPattern(*mc.pattern);
        if (mc.guard) {
            s += " if " + formatExpr(*mc.guard);
        }
        s += " => " + formatExpr(*mc.result) + ",\n";
    }
    indentLevel_--;
    s += indent() + "}";
    currentExpr_ = s;
    return Nil{};
}

// ── Statement Visitors ────────────────────────────────────────────────────────

void Formatter::visit(ExprStmt& stmt) {
    output_ += indent() + formatExpr(*stmt.expr) + ";\n";
}

void Formatter::visit(VarStmt& stmt) {
    std::string s = indent() + "var ";
    if (!stmt.name.empty()) {
        s += stmt.name;
        if (stmt.typeAnnotation) {
            s += ": " + formatType(*stmt.typeAnnotation);
        }
    } else if (stmt.pattern) {
        s += formatPattern(*stmt.pattern);
    }
    if (stmt.initializer) {
        s += " = " + formatExpr(*stmt.initializer);
    }
    s += ";\n";
    output_ += s;
}

void Formatter::visit(BlockStmt& stmt) {
    output_ += indent() + "{\n";
    indentLevel_++;
    for (auto& s : stmt.statements) {
        if (s) s->accept(*this);
    }
    indentLevel_--;
    output_ += indent() + "}\n";
}

void Formatter::visit(IfStmt& stmt) {
    output_ += indent() + "if (" + formatExpr(*stmt.condition) + ") {\n";
    indentLevel_++;
    if (auto* block = dynamic_cast<BlockStmt*>(stmt.thenBranch.get())) {
        for (auto& s : block->statements) {
            if (s) s->accept(*this);
        }
    } else {
        if (stmt.thenBranch) stmt.thenBranch->accept(*this);
    }
    indentLevel_--;
    if (stmt.elseBranch) {
        // Check if else branch is another if (else if chain)
        if (auto* elseIf = dynamic_cast<IfStmt*>(stmt.elseBranch.get())) {
            output_ += indent() + "} else if (" + formatExpr(*elseIf->condition) + ") {\n";
            indentLevel_++;
            if (auto* block = dynamic_cast<BlockStmt*>(elseIf->thenBranch.get())) {
                for (auto& s : block->statements) {
                    if (s) s->accept(*this);
                }
            } else {
                if (elseIf->thenBranch) elseIf->thenBranch->accept(*this);
            }
            indentLevel_--;
            // Recurse for nested else-if
            if (elseIf->elseBranch) {
                std::string savedOutput = output_;
                output_ = "";
                elseIf->elseBranch->accept(*this);
                std::string elseBody = output_;
                output_ = savedOutput;
                // Prepend "} else " to the inner if/block output (strip leading indent)
                output_ += indent() + "} else " + elseBody.substr(indent().size());
            } else {
                output_ += indent() + "}\n";
            }
        } else {
            output_ += indent() + "} else {\n";
            indentLevel_++;
            if (auto* block = dynamic_cast<BlockStmt*>(stmt.elseBranch.get())) {
                for (auto& s : block->statements) {
                    if (s) s->accept(*this);
                }
            } else {
                stmt.elseBranch->accept(*this);
            }
            indentLevel_--;
            output_ += indent() + "}\n";
        }
    } else {
        output_ += indent() + "}\n";
    }
}

void Formatter::visit(WhileStmt& stmt) {
    output_ += indent() + "while (" + formatExpr(*stmt.condition) + ") {\n";
    indentLevel_++;
    if (auto* block = dynamic_cast<BlockStmt*>(stmt.body.get())) {
        for (auto& s : block->statements) {
            if (s) s->accept(*this);
        }
    } else {
        if (stmt.body) stmt.body->accept(*this);
    }
    indentLevel_--;
    output_ += indent() + "}\n";
}

void Formatter::visit(ReturnStmt& stmt) {
    if (stmt.value) {
        output_ += indent() + "return " + formatExpr(*stmt.value) + ";\n";
    } else {
        output_ += indent() + "return;\n";
    }
}

void Formatter::visit(FunctionStmt& stmt) {
    output_ += indent();
    if (stmt.isAsync) output_ += "async ";
    output_ += "fn " + stmt.name;
    formatFunctionBody(stmt.params, stmt.paramTypes, stmt.returnType, stmt.body);
    output_ += "\n";
}

void Formatter::visit(ClassStmt& stmt) {
    output_ += indent() + "class " + stmt.name;
    if (!stmt.superclass.empty()) {
        output_ += " extends " + stmt.superclass;
    }
    output_ += " {\n";
    indentLevel_++;

    for (auto& field : stmt.fields) {
        if (field) field->accept(*this);
    }
    if (!stmt.fields.empty() && !stmt.methods.empty()) {
        output_ += "\n";
    }
    for (auto& method : stmt.methods) {
        if (method) {
            output_ += indent();
            if (method->isAsync) output_ += "async ";
            output_ += "fn " + method->name;
            formatFunctionBody(method->params, method->paramTypes, method->returnType, method->body);
            output_ += "\n";
        }
    }

    indentLevel_--;
    output_ += indent() + "}\n";
}

void Formatter::visit(ImportStmt& stmt) {
    if (stmt.isWildcard) {
        output_ += indent() + "import * as " + stmt.wildcardAlias + " from \"" + stmt.module + "\";\n";
    } else if (!stmt.namedImports.empty()) {
        output_ += indent() + "import { ";
        for (size_t i = 0; i < stmt.namedImports.size(); ++i) {
            if (i > 0) output_ += ", ";
            output_ += stmt.namedImports[i];
            if (i < stmt.namedAliases.size() && !stmt.namedAliases[i].empty() &&
                stmt.namedAliases[i] != stmt.namedImports[i]) {
                output_ += " as " + stmt.namedAliases[i];
            }
        }
        output_ += " } from \"" + stmt.module + "\";\n";
    } else {
        output_ += indent() + "import \"" + stmt.module + "\";\n";
    }
}

void Formatter::visit(ExportStmt& stmt) {
    output_ += indent() + "export ";
    if (stmt.isDefault) {
        output_ += "default ";
        if (stmt.declaration) {
            // Remove leading indent from declaration since we already printed it
            std::string savedOutput = output_;
            output_ = "";
            stmt.declaration->accept(*this);
            std::string declStr = output_;
            output_ = savedOutput;
            // Strip leading whitespace from the declaration output
            size_t pos = 0;
            while (pos < declStr.size() && (declStr[pos] == ' ' || declStr[pos] == '\t')) ++pos;
            output_ += declStr.substr(pos);
        } else if (stmt.defaultExpr) {
            output_ += formatExpr(*stmt.defaultExpr) + ";\n";
        }
    } else if (stmt.declaration) {
        std::string savedOutput = output_;
        output_ = "";
        stmt.declaration->accept(*this);
        std::string declStr = output_;
        output_ = savedOutput;
        size_t pos = 0;
        while (pos < declStr.size() && (declStr[pos] == ' ' || declStr[pos] == '\t')) ++pos;
        output_ += declStr.substr(pos);
    }
}

void Formatter::visit(ReExportStmt& stmt) {
    if (stmt.isWildcard) {
        output_ += indent() + "export * from \"" + stmt.module + "\";\n";
    } else {
        output_ += indent() + "export { ";
        for (size_t i = 0; i < stmt.names.size(); ++i) {
            if (i > 0) output_ += ", ";
            output_ += stmt.names[i];
        }
        output_ += " } from \"" + stmt.module + "\";\n";
    }
}

void Formatter::visit(BreakStmt&) {
    output_ += indent() + "break;\n";
}

void Formatter::visit(ContinueStmt&) {
    output_ += indent() + "continue;\n";
}

void Formatter::visit(TryStmt& stmt) {
    output_ += indent() + "try {\n";
    indentLevel_++;
    if (auto* block = dynamic_cast<BlockStmt*>(stmt.tryBlock.get())) {
        for (auto& s : block->statements) {
            if (s) s->accept(*this);
        }
    } else {
        if (stmt.tryBlock) stmt.tryBlock->accept(*this);
    }
    indentLevel_--;

    if (stmt.catchBlock) {
        output_ += indent() + "} catch (" + stmt.catchVariable + ") {\n";
        indentLevel_++;
        if (auto* block = dynamic_cast<BlockStmt*>(stmt.catchBlock.get())) {
            for (auto& s : block->statements) {
                if (s) s->accept(*this);
            }
        } else {
            stmt.catchBlock->accept(*this);
        }
        indentLevel_--;
    }

    if (stmt.finallyBlock) {
        output_ += indent() + "} finally {\n";
        indentLevel_++;
        if (auto* block = dynamic_cast<BlockStmt*>(stmt.finallyBlock.get())) {
            for (auto& s : block->statements) {
                if (s) s->accept(*this);
            }
        } else {
            stmt.finallyBlock->accept(*this);
        }
        indentLevel_--;
    }

    output_ += indent() + "}\n";
}

void Formatter::visit(ThrowStmt& stmt) {
    output_ += indent() + "throw " + formatExpr(*stmt.value) + ";\n";
}

}  // namespace izi
