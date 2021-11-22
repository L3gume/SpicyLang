#include "spicyresolver.h"

#include "spicy.h"

#include <utility>

namespace spicy::eval {

SpicyResolver::SpicyResolver(SpicyEvaluator &evaluator)
    : m_evaluator(evaluator) {}

void SpicyResolver::resolve(const std::vector<ast::StmtPtrVariant> &stmts) {
    for (const auto& stmt : stmts)
        resolve(stmt);
}

void SpicyResolver::resolveBlockStmt(const ast::BlockStmtPtr &stmt) {
    beginScope();
    resolve(stmt->statements);
    endScope();
}

void SpicyResolver::resolveVarStmt(const ast::VarStmtPtr &stmt) {
    declare(stmt->varName);
    if (stmt->initializer.has_value()) {
        resolve(stmt->initializer.value());
    }
    define(stmt->varName);
}

void SpicyResolver::resolveFuncStmt(const ast::FuncStmtPtr &stmt) {
    declare(stmt->funcName);
    define(stmt->funcName);

    resolveFunction(stmt);
}

void SpicyResolver::resolveExprStmt(const ast::ExprStmtPtr &stmt) {
    resolve(stmt->expression);
}

void SpicyResolver::resolveIfStmt(const ast::IfStmtPtr &stmt) {
    resolve(stmt->condition);
    resolve(stmt->thenBranch);
    if (stmt->elseBranch.has_value())
        resolve(stmt->elseBranch.value());
}

void SpicyResolver::resolvePrintStmt(const ast::PrintStmtPtr &stmt) {
    resolve(stmt->expression);
}

void SpicyResolver::resolveRetStmt(const ast::RetStmtPtr &stmt) {
    if (stmt->value.has_value()) {
        if (m_currentFunction == FunctionType::INITIALIZER)
            error(stmt->ret, "Cannot return a value from an initializer.");
        resolve(stmt->value.value());
    }
}

void SpicyResolver::resolveWhileStmt(const ast::WhileStmtPtr &stmt) {
    resolve(stmt->condition);
    resolve(stmt->loopBody);
}

void SpicyResolver::resolveClassStmt(const ast::ClassStmtPtr &stmt) {
    const auto enclosingClass = m_currentClass;
    m_currentClass = ClassType::CLASS;
    declare(stmt->className);
    define(stmt->className);

    const auto hasSuperClass = stmt->superClass.has_value();
    if (hasSuperClass) {
        if (const auto& super = std::get<ast::VariableExprPtr>(stmt->superClass.value());
            stmt->className.lexeme == super->varName.lexeme) {
            throw RuntimeError(super->varName, "A class cannot inherit from itself.");
        }
        m_currentClass = ClassType::SUBCLASS;
        resolve(stmt->superClass.value());
        beginScope();
        m_scopes.back().insert_or_assign(m_hasher("super"), true);
    }

    beginScope();
    m_scopes.back().insert_or_assign(m_hasher("this"), true);
    for (const auto& method : stmt->methods) {
        auto decl = FunctionType::METHOD;
        if (method->funcName.lexeme == "init")
            decl = FunctionType::INITIALIZER;
        resolveFunction(method, decl);
    }
    endScope();

    if (hasSuperClass) endScope();
    m_currentClass = enclosingClass;
}

void SpicyResolver::resolveVarExpr(const ast::VariableExprPtr &expr) {
    if (!m_scopes.empty()) {
        if (const auto found = m_scopes.back().find(m_hasher(expr->varName.lexeme));
            found != m_scopes.back().end() && !found->second) {
            error(expr->varName, "Can't read local variable in its own initializer");
        }
    }
    resolveLocal(reinterpret_cast<uint64_t>(expr.get()), expr->varName);
}

void SpicyResolver::resolveAssignExpr(const ast::AssignExprPtr &expr) {
    resolve(expr->right);
    resolveLocal(reinterpret_cast<uint64_t>(expr.get()), expr->varName);
}

void SpicyResolver::resolveBinaryExpr(const ast::BinaryExprPtr &expr) {
    resolve(expr->left);
    resolve(expr->right);
}

void SpicyResolver::resolveCallExpr(const ast::CallExprPtr &expr) {
    resolve(expr->callee);
    for (const auto& arg : expr->arguments) {
        resolve(arg);
    }
}

void SpicyResolver::resolveGroupingExpr(const ast::GroupingExprPtr &expr) {
    resolve(expr->expression);
}

void SpicyResolver::resolveLiteralExpr(const ast::LiteralExprPtr &expr) {}

void SpicyResolver::resolveLogicalExpr(const ast::LogicalExprPtr &expr) {
    resolve(expr->left);
    resolve(expr->right);
}

void SpicyResolver::resolveUnaryExpr(const ast::UnaryExprPtr &expr) {
    resolve(expr->right);
}

void SpicyResolver::resolvePostfixExpr(const ast::PostfixExprPtr& expr) {
    resolve(expr->left);
}

void SpicyResolver::resolveFuncExpr(const ast::FuncExprPtr &expr) {
    resolveLambda(expr);
}

void SpicyResolver::resolveGetExpr(const ast::GetExprPtr &expr) {
    resolve(expr->object);
}

void SpicyResolver::resolveSetExpr(const ast::SetExprPtr &expr) {
    resolve(expr->value);
    resolve(expr->object);
}

void SpicyResolver::resolveThisExpr(const ast::ThisExprPtr &expr) {
    if (m_currentClass == ClassType::NONE) {
        error(expr->keyword, "Cannot use 'this' outside of a class.");
        return;
    }
    resolveLocal(reinterpret_cast<uint64_t>(expr.get()), expr->keyword);
}

void SpicyResolver::resolveSuperExpr(const ast::SuperExprPtr &expr) {
    if (m_currentClass == ClassType::NONE) {
        throw RuntimeError(expr->keyword,
                           "Cannot use 'super' outside of a class.");
    } else if (m_currentClass != ClassType::SUBCLASS) {
        throw RuntimeError(expr->keyword,
                           "Cannot use 'super' in a class with no superclass.");
    }
    resolveLocal(reinterpret_cast<uint64_t>(expr.get()), expr->keyword);
}

void SpicyResolver::resolveIndexGetExpr(const ast::IndexGetExprPtr& expr) {
    resolve(expr->lst);
    resolve(expr->idx);
}

void SpicyResolver::resolveIndexSetExpr(const ast::IndexSetExprPtr& expr) {
    resolve(expr->lst);
    resolve(expr->idx);
    resolve(expr->val);
}

struct StmtResolverVisitor {
    SpicyResolver* const resolver;
    explicit StmtResolverVisitor(SpicyResolver* resolver) : resolver(resolver) {}
    void operator()(const ast::ExprStmtPtr& stmt) { resolver->resolveExprStmt(stmt); }
    void operator()(const ast::PrintStmtPtr& stmt) { resolver->resolvePrintStmt(stmt); }
    void operator()(const ast::BlockStmtPtr& stmt) { resolver->resolveBlockStmt(stmt); }
    void operator()(const ast::VarStmtPtr& stmt) { resolver->resolveVarStmt(stmt); }
    void operator()(const ast::IfStmtPtr& stmt) { resolver->resolveIfStmt(stmt); }
    void operator()(const ast::WhileStmtPtr& stmt) { resolver->resolveWhileStmt(stmt); }
    void operator()(const ast::FuncStmtPtr& stmt) { resolver->resolveFuncStmt(stmt); }
    void operator()(const ast::RetStmtPtr& stmt) { resolver->resolveRetStmt(stmt); }
    void operator()(const ast::ClassStmtPtr& stmt) { resolver->resolveClassStmt(stmt); }
};

void SpicyResolver::resolve(const ast::StmtPtrVariant &stmt) {
    std::visit(StmtResolverVisitor(this), stmt);
}

struct ExprResolverVisitor {
    SpicyResolver* const resolver;
    explicit ExprResolverVisitor(SpicyResolver* resolver) : resolver(resolver) {}
    void operator()(const ast::BinaryExprPtr& expr) { resolver->resolveBinaryExpr(expr); }
    void operator()(const ast::GroupingExprPtr& expr) { resolver->resolveGroupingExpr(expr); }
    void operator()(const ast::LiteralExprPtr& expr) { resolver->resolveLiteralExpr(expr); }
    void operator()(const ast::UnaryExprPtr& expr) { resolver->resolveUnaryExpr(expr); }
    void operator()(const ast::ConditionalExprPtr& expr) {}
    void operator()(const ast::PostfixExprPtr& expr) { resolver->resolvePostfixExpr(expr); }
    void operator()(const ast::VariableExprPtr& expr) { resolver->resolveVarExpr(expr); }
    void operator()(const ast::AssignExprPtr& expr) { resolver->resolveAssignExpr(expr); }
    void operator()(const ast::LogicalExprPtr& expr) { resolver->resolveLogicalExpr(expr); }
    void operator()(const ast::CallExprPtr& expr) { resolver->resolveCallExpr(expr); }
    void operator()(const ast::FuncExprPtr& expr) { resolver->resolveFuncExpr(expr); }
    void operator()(const ast::GetExprPtr& expr) { resolver->resolveGetExpr(expr); }
    void operator()(const ast::SetExprPtr& expr) { resolver->resolveSetExpr(expr); }
    void operator()(const ast::ThisExprPtr& expr) { resolver->resolveThisExpr(expr); }
    void operator()(const ast::SuperExprPtr& expr) { resolver->resolveSuperExpr(expr); }
    void operator()(const ast::IndexGetExprPtr& expr) { resolver->resolveIndexGetExpr(expr); }
    void operator()(const ast::IndexSetExprPtr& expr) { resolver->resolveIndexSetExpr(expr); }
};

void SpicyResolver::resolve(const ast::ExprPtrVariant &expr) {
    std::visit(ExprResolverVisitor(this), expr);
}

void SpicyResolver::resolveLocal(uint64_t exprAddr, Token name) {
    resolveLocal(exprAddr, name.lexeme);
}

void SpicyResolver::resolveLocal(uint64_t exprAddr, const std::string& name) {
    for (auto i = m_scopes.size(); i > 0; --i) {
        if (const auto& scope = m_scopes[i - 1].find(m_hasher(name));
            scope != m_scopes[i - 1].end()) {
            m_evaluator.resolve(exprAddr, m_scopes.size() - i);
            return;
        }
    }
}

void SpicyResolver::resolveFunction(const ast::FuncStmtPtr &stmt, FunctionType type) {
    beginScope();
    const auto previousFunction = m_currentFunction;
    m_currentFunction = type;
    for (const auto& param : stmt->funcExpr->parameters) {
        declare(param);
        define(param);
    }
    resolve(stmt->funcExpr->body);
    m_currentFunction = previousFunction;
    endScope();
}

void SpicyResolver::resolveLambda(const ast::FuncExprPtr &expr) {
    beginScope();
    const auto previousFunction = m_currentFunction;
    m_currentFunction = FunctionType::LAMBDA;
    for (const auto& param : expr->parameters) {
        declare(param);
        define(param);
    }
    resolve(expr->body);
    m_currentFunction = previousFunction;
    endScope();
}

void SpicyResolver::beginScope() {
    m_scopes.emplace_back();
}

void SpicyResolver::endScope() {
    m_scopes.pop_back();
}

void SpicyResolver::declare(Token name) {
    if (m_scopes.empty()) return;
    m_scopes.back().insert_or_assign(m_hasher(name.lexeme), false);
}

void SpicyResolver::define(Token name) {
    if (m_scopes.empty()) return;
    m_scopes.back().insert_or_assign(m_hasher(name.lexeme), true);
}

} // namespace spicy
