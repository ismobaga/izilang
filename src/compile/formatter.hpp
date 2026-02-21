#pragma once

#include "ast/stmt.hpp"
#include "ast/expr.hpp"
#include "ast/visitor.hpp"
#include "ast/pattern.hpp"
#include "ast/type.hpp"
#include "common/value.hpp"
#include <string>
#include <vector>

namespace izi {

/**
 * Style configuration for the IziLang formatter.
 * Can be loaded from a `.izifmt.toml` file.
 */
struct FormatterConfig {
    int indentSize = 4;      // Number of spaces per indent level
    int maxLineLength = 100; // Maximum line length (informational)

    // Load configuration from a TOML file.
    // Returns a default-constructed config if the file cannot be opened.
    static FormatterConfig load(const std::string& path);
};

/**
 * IziLang source code formatter.
 *
 * Traverses a parsed AST and emits canonical, consistently-styled source code:
 *   - Configurable indentation (default: 4-space)
 *   - Spaces around binary operators
 *   - Space after commas
 *   - Opening brace on the same line (K&R style)
 *   - Blank line between top-level declarations
 */
class Formatter : public ExprVisitor, public StmtVisitor {
   public:
    Formatter() = default;
    explicit Formatter(FormatterConfig cfg);

    // Format a complete program and return the formatted source string.
    std::string format(const std::vector<StmtPtr>& program);

    // Expression visitors (store result in currentExpr_)
    Value visit(BinaryExpr& expr) override;
    Value visit(UnaryExpr& expr) override;
    Value visit(CallExpr& expr) override;
    Value visit(LiteralExpr& expr) override;
    Value visit(AssignExpr& expr) override;
    Value visit(VariableExpr& expr) override;
    Value visit(GroupingExpr& expr) override;
    Value visit(ConditionalExpr& expr) override;
    Value visit(ArrayExpr& expr) override;
    Value visit(MapExpr& expr) override;
    Value visit(SpreadExpr& expr) override;
    Value visit(IndexExpr& expr) override;
    Value visit(SetIndexExpr& expr) override;
    Value visit(FunctionExpr& expr) override;
    Value visit(MatchExpr& expr) override;
    Value visit(PropertyExpr& expr) override;
    Value visit(SetPropertyExpr& expr) override;
    Value visit(ThisExpr& expr) override;
    Value visit(SuperExpr& expr) override;
    Value visit(AwaitExpr& expr) override;

    // Statement visitors (append to output_)
    void visit(ExprStmt& stmt) override;
    void visit(ReturnStmt& stmt) override;
    void visit(IfStmt& stmt) override;
    void visit(WhileStmt& stmt) override;
    void visit(BlockStmt& stmt) override;
    void visit(VarStmt& stmt) override;
    void visit(FunctionStmt& stmt) override;
    void visit(ImportStmt& stmt) override;
    void visit(ExportStmt& stmt) override;
    void visit(ReExportStmt& stmt) override;
    void visit(BreakStmt& stmt) override;
    void visit(ContinueStmt& stmt) override;
    void visit(TryStmt& stmt) override;
    void visit(ThrowStmt& stmt) override;
    void visit(ClassStmt& stmt) override;

   private:
    FormatterConfig config_;   // Style configuration
    std::string output_;       // Accumulated formatted output
    int indentLevel_ = 0;      // Current indentation depth
    std::string currentExpr_;  // Result of the most recent expression visit

    // Helpers
    std::string indent() const;
    std::string formatExpr(Expr& expr);
    std::string formatType(const TypeAnnotation& type) const;
    std::string formatPattern(const Pattern& pattern) const;
    std::string tokenTypeToStr(TokenType type) const;
    void formatFunctionBody(const std::vector<std::string>& params,
                            const std::vector<TypePtr>& paramTypes,
                            const TypePtr& returnType,
                            const std::vector<StmtPtr>& body);
};

}  // namespace izi
