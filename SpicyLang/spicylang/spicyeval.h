#pragma once
#ifndef H_SPICYEVAL
#define H_SPICYEVAL

#include "spicyast.h"
#include "spicyobjects.h"
#include "spicyenvironment.h"

namespace spicy::eval {

struct SpicyExprEvaluator;
struct SpicyStmtExecutor;

class SpicyEvaluator {
    EnvironmentMgr m_envMgr{};
    std::map<uint64_t, uint32_t> m_locals{};
    const bool m_isRepl;
    SpicyObj m_lastObj{};

public:
    explicit SpicyEvaluator(bool isRepl = false);

    SpicyObj evalExpr(const ast::ExprPtrVariant& expr);
    OptSpicyObj execStmt(const ast::StmtPtrVariant& stmt);
    OptSpicyObj execStmts(const std::vector<ast::StmtPtrVariant>& stmts);

    void resolve(uint64_t exprAddr, uint32_t depth);
    SpicyObj getLastObj();

private:
    void initBuiltins();

    SpicyObj evalLiteralExpr(const ast::LiteralExprPtr& expr);
    SpicyObj evalGroupingExpr(const ast::GroupingExprPtr& expr);
    SpicyObj evalUnaryExpr(const ast::UnaryExprPtr& expr);
    SpicyObj evalPostfixExpr(const ast::PostfixExprPtr& expr);
    SpicyObj evalBinaryExpr(const ast::BinaryExprPtr& expr);
    SpicyObj evalVariableExpr(const ast::VariableExprPtr& expr);
    SpicyObj evalAssignExpr(const ast::AssignExprPtr& expr);
    SpicyObj evalLogicalExpr(const ast::LogicalExprPtr& expr);
    SpicyObj evalCallExpr(const ast::CallExprPtr& expr);
    SpicyObj evalGetExpr(const ast::GetExprPtr& expr);
    SpicyObj evalSetExpr(const ast::SetExprPtr& expr);
    SpicyObj evalThisExpr(const ast::ThisExprPtr& expr);
    SpicyObj evalSuperExpr(const ast::SuperExprPtr& expr);
    SpicyObj evalFuncExpr(const ast::FuncExprPtr& expr);
    SpicyObj evalIndexGetExpr(const ast::IndexGetExprPtr& expr);
    SpicyObj evalIndexSetExpr(const ast::IndexSetExprPtr& expr);

    OptSpicyObj execExpressionStmt(const ast::ExprStmtPtr& stmt);
    OptSpicyObj execPrintStmt(const ast::PrintStmtPtr& stmt);
    OptSpicyObj execBlockStmt(const ast::BlockStmtPtr& stmt);
    OptSpicyObj execVarStmt(const ast::VarStmtPtr& stmt);
    OptSpicyObj execIfStmt(const ast::IfStmtPtr& stmt);
    OptSpicyObj execWhileStmt(const ast::WhileStmtPtr& stmt);
    OptSpicyObj execFuncStmt(const ast::FuncStmtPtr& stmt);
    OptSpicyObj execRetStmt(const ast::RetStmtPtr& stmt);
    OptSpicyObj execClassStmt(const ast::ClassStmtPtr& stmt);

    SpicyObj lookUpVariable(Token name, uint64_t exprAddr);
    FuncSharedPtr bindInstance(const FuncSharedPtr& method, SpicyInstanceSharedPtr instance);
    SpicyObj evalBuiltInCall(const BuiltinFuncSharedPtr& builtin, const ast::CallExprPtr& expr);

    friend struct SpicyExprEvaluator;
    friend struct SpicyStmtExecutor;
};

} // namespace spicy::eval

#endif // H_SPICYEVAL
