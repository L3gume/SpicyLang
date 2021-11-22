#ifndef SPICY_SPICYRESOLVER_H
#define SPICY_SPICYRESOLVER_H

#include <memory>
#include <map>
#include <vector>

#include "spicyeval.h"

namespace spicy::eval {

enum class FunctionType {
    NONE,
    FUNCTION,
    INITIALIZER,
    METHOD,
    LAMBDA
};

enum class ClassType {
    NONE,
    CLASS,
    SUBCLASS
};

class SpicyResolver {
    SpicyEvaluator& m_evaluator;
    std::vector<std::map<size_t, bool>> m_scopes;
    std::hash<std::string> m_hasher;
    ClassType m_currentClass = ClassType::NONE;
    FunctionType m_currentFunction = FunctionType::NONE;
public:
    explicit SpicyResolver(SpicyEvaluator& evaluator);

    void resolve(const std::vector<ast::StmtPtrVariant>& stmts);
    void resolve(const ast::StmtPtrVariant& stmt);

private:
    void resolveBlockStmt(const ast::BlockStmtPtr& stmt);
    void resolveVarStmt(const ast::VarStmtPtr& stmt);
    void resolveFuncStmt(const ast::FuncStmtPtr& stmt);
    void resolveExprStmt(const ast::ExprStmtPtr& stmt);
    void resolveIfStmt(const ast::IfStmtPtr& stmt);
    void resolvePrintStmt(const ast::PrintStmtPtr& stmt);
    void resolveRetStmt(const ast::RetStmtPtr& stmt);
    void resolveWhileStmt(const ast::WhileStmtPtr& stmt);
    void resolveClassStmt(const ast::ClassStmtPtr& stmt);

    void resolveVarExpr(const ast::VariableExprPtr& expr);
    void resolveAssignExpr(const ast::AssignExprPtr& expr);
    void resolveBinaryExpr(const ast::BinaryExprPtr& expr);
    void resolveCallExpr(const ast::CallExprPtr& expr);
    void resolveGroupingExpr(const ast::GroupingExprPtr& expr);
    void resolveLiteralExpr(const ast::LiteralExprPtr& expr);
    void resolveLogicalExpr(const ast::LogicalExprPtr& expr);
    void resolveUnaryExpr(const ast::UnaryExprPtr& expr);
    void resolvePostfixExpr(const ast::PostfixExprPtr& expr);
    void resolveFuncExpr(const ast::FuncExprPtr& expr);
    void resolveGetExpr(const ast::GetExprPtr& expr);
    void resolveSetExpr(const ast::SetExprPtr& expr);
    void resolveThisExpr(const ast::ThisExprPtr& expr);
    void resolveSuperExpr(const ast::SuperExprPtr& expr);
    void resolveIndexGetExpr(const ast::IndexGetExprPtr& expr);
    void resolveIndexSetExpr(const ast::IndexSetExprPtr& expr);

    void resolve(const ast::ExprPtrVariant& expr);
    void resolveLocal(uint64_t exprAddr, Token name);
    void resolveLocal(uint64_t exprAddr, const std::string& name);
    void resolveFunction(const ast::FuncStmtPtr& stmt, FunctionType type = FunctionType::FUNCTION);
    void resolveLambda(const ast::FuncExprPtr& expr);

    void beginScope();
    void endScope();
    void declare(Token name);
    void define(Token name);

    friend struct StmtResolverVisitor;
    friend struct ExprResolverVisitor;
};

} // namespace spicy

#endif // SPICY_SPICYRESOLVER_H
