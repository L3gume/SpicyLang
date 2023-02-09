#include "spicycodegen.h"


namespace spicy {

struct SpicyExprCodeGenerator {
    SpicyCodeGenerator* const gen;
    explicit SpicyExprCodeGenerator(SpicyCodeGenerator* gen) : gen(gen) {}
    [[nodiscard]] llvm::Value*      operator()(const ast::BinaryExprPtr& expr)        { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::GroupingExprPtr& expr)      { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::LiteralExprPtr& expr)       { return gen->generateLiteralExpr(expr); }
    [[nodiscard]] llvm::Value*      operator()(const ast::UnaryExprPtr& expr)         { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::ConditionalExprPtr& expr)   { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::PostfixExprPtr& expr)       { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::VariableExprPtr& expr)      { return gen->generateVariableExpr(expr); }
    [[nodiscard]] llvm::Value*      operator()(const ast::AssignExprPtr& expr)        { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::LogicalExprPtr& expr)       { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::CallExprPtr& expr)          { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::FuncExprPtr& expr)          { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::GetExprPtr& expr)           { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::SetExprPtr& expr)           { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::ThisExprPtr& expr)          { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::SuperExprPtr& expr)         { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::IndexGetExprPtr& expr)      { return nullptr; }
    [[nodiscard]] llvm::Value*      operator()(const ast::IndexSetExprPtr& expr)      { return nullptr; }
};

struct SpicyStmtCodeGenerator {
    SpicyCodeGenerator* const gen;
    explicit SpicyStmtCodeGenerator(SpicyCodeGenerator* gen) : gen(gen) {}
    [[nodiscard]] llvm::Function*   operator()(const ast::ExprStmtPtr& stmt)          { return gen->generateExprStmt(stmt); }
    [[nodiscard]] llvm::Function*   operator()(const ast::PrintStmtPtr& stmt)         { return nullptr; }
    [[nodiscard]] llvm::Function*   operator()(const ast::BlockStmtPtr& stmt)         { return nullptr; }
    [[nodiscard]] llvm::Function*   operator()(const ast::VarStmtPtr& stmt)           { return nullptr; }
    [[nodiscard]] llvm::Function*   operator()(const ast::IfStmtPtr& stmt)            { return nullptr; }
    [[nodiscard]] llvm::Function*   operator()(const ast::WhileStmtPtr& stmt)         { return nullptr; }
    [[nodiscard]] llvm::Function*   operator()(const ast::FuncStmtPtr& stmt)          { return gen->generateFuncStmt(stmt); }
    [[nodiscard]] llvm::Function*   operator()(const ast::RetStmtPtr& stmt)           { return nullptr; }
    [[nodiscard]] llvm::Function*   operator()(const ast::ClassStmtPtr& stmt)         { return nullptr; }
};

auto 
    SpicyCodeGenerator::
    generateStmts(const ast::SpicyProgram& stmts) 
    ->  llvm::Module*
{
    for (const auto& stmt : stmts) {
        const auto* func = generateStmt(stmt);
    }
    // TODO 
    return nullptr;
}

auto
    SpicyCodeGenerator::
    generateStmt(const ast::StmtPtrVariant& stmt) 
    -> llvm::Function*
{
    return std::visit(SpicyStmtCodeGenerator(this), stmt);
}

auto 
    SpicyCodeGenerator::
    generateFuncStmt(const ast::FuncStmtPtr& stmt) 
    -> llvm::Function*
{
    return nullptr;
}

auto 
    SpicyCodeGenerator::
    generateExprStmt(const ast::ExprStmtPtr& stmt) 
    -> llvm::Function*
{
    const auto* val = generateExpr(stmt->expression);
    return nullptr;
}

auto 
    SpicyCodeGenerator::
    generateExpr(const ast::ExprPtrVariant& expr) 
    -> llvm::Value*
{
    return std::visit(SpicyExprCodeGenerator(this), expr);
}

auto SpicyCodeGenerator::generateLiteralExpr(const ast::LiteralExprPtr& expr) -> llvm::Value*
{
    return nullptr;
}

auto SpicyCodeGenerator::generateVariableExpr(const ast::VariableExprPtr& expr) -> llvm::Value*
{
    return nullptr;
}

}
