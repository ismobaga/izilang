#pragma once
#include "common/value.hpp"

namespace izi {

struct BinaryExpr;
struct UnaryExpr;
struct CallExpr;
struct VariableExpr;
struct AssignExpr;
struct LiteralExpr;
struct GroupingExpr;
struct ConditionalExpr;
struct ArrayExpr;
struct MapExpr;
struct SpreadExpr;
struct IndexExpr;
struct SetIndexExpr;
struct FunctionExpr;
struct MatchExpr;
struct PropertyExpr;  // v0.3
struct SetPropertyExpr;  // v0.3
struct ThisExpr;  // v0.3
struct SuperExpr;  // v0.3

struct ExprStmt;
struct BlockStmt;
struct VarStmt;
struct WhileStmt;
struct ReturnStmt;
struct FunctionStmt;
struct IfStmt;
struct ImportStmt;
struct ExportStmt;
struct ReExportStmt;
struct BreakStmt;
struct ContinueStmt;
struct TryStmt;
struct ThrowStmt;
struct ClassStmt;  // v0.3

class ExprVisitor {
   public:
    virtual ~ExprVisitor() = default;

    virtual auto visit(BinaryExpr&) -> Value = 0;
    virtual Value visit(UnaryExpr&) = 0;
    virtual Value visit(CallExpr&) = 0;
    virtual Value visit(LiteralExpr&) = 0;
    virtual Value visit(AssignExpr&) = 0;
    virtual Value visit(VariableExpr&) = 0;
    virtual Value visit(GroupingExpr&) = 0;
    virtual Value visit(ConditionalExpr&) = 0;
    virtual Value visit(ArrayExpr&) = 0;
    virtual Value visit(MapExpr&) = 0;
    virtual Value visit(SpreadExpr&) = 0;
    virtual Value visit(IndexExpr&) = 0;
    virtual Value visit(SetIndexExpr&) = 0;
    virtual Value visit(FunctionExpr&) = 0;
    virtual Value visit(MatchExpr&) = 0;
    virtual Value visit(PropertyExpr&) = 0;  // v0.3
    virtual Value visit(SetPropertyExpr&) = 0;  // v0.3
    virtual Value visit(ThisExpr&) = 0;  // v0.3
    virtual Value visit(SuperExpr&) = 0;  // v0.3
};

class StmtVisitor {
   public:
    virtual ~StmtVisitor() = default;

    virtual void visit(ExprStmt&) = 0;
    virtual void visit(ReturnStmt&) = 0;
    virtual void visit(IfStmt&) = 0;
    virtual void visit(WhileStmt&) = 0;
    virtual void visit(BlockStmt&) = 0;
    virtual void visit(VarStmt&) = 0;
    virtual void visit(FunctionStmt&) = 0;
    virtual void visit(ImportStmt&) = 0;
    virtual void visit(ExportStmt&) = 0;
    virtual void visit(ReExportStmt&) = 0;
    virtual void visit(BreakStmt&) = 0;
    virtual void visit(ContinueStmt&) = 0;
    virtual void visit(TryStmt&) = 0;
    virtual void visit(ThrowStmt&) = 0;
    virtual void visit(ClassStmt&) = 0;  // v0.3
};

}  // namespace izi