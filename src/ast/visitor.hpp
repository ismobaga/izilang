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
struct ArrayExpr;
struct MapExpr;
struct IndexExpr;
struct SetIndexExpr;

struct ExprStmt;
struct BlockStmt;
struct VarStmt;
struct WhileStmt;
struct ReturnStmt;
struct FunctionStmt;
struct IfStmt;
struct ImportStmt;


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
    virtual Value visit(ArrayExpr&) = 0;
    virtual Value visit(MapExpr&) = 0;
    virtual Value visit(IndexExpr&) = 0;
    virtual Value visit(SetIndexExpr&) = 0;
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
};

}  // namespace izi