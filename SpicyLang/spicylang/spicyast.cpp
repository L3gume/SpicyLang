#include "spicyast.h"

#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace spicy::ast {

// ========================== //
// Expr AST Type Constructors //
// ========================== //
BinaryExpr::BinaryExpr(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

GroupingExpr::GroupingExpr(ExprPtrVariant expression)
    : expression(std::move(expression)) {}

LiteralExpr::LiteralExpr(spicy::OptTokenLiteral value)
    : literalVal(std::move(value)) {}

UnaryExpr::UnaryExpr(spicy::Token op, ExprPtrVariant right)
    : op(std::move(op)), right(std::move(right)) {}

ConditionalExpr::ConditionalExpr(ExprPtrVariant condition,
                                 ExprPtrVariant thenBranch,
                                 ExprPtrVariant elseBranch)
    : condition(std::move(condition)),
      thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

PostfixExpr::PostfixExpr(ExprPtrVariant left, spicy::Token op)
    : left(std::move(left)), op(std::move(op)) {}

VariableExpr::VariableExpr(spicy::Token varName) : varName(std::move(varName)) {}

AssignExpr::AssignExpr(spicy::Token varName, ExprPtrVariant right)
    : varName(std::move(varName)), right(std::move(right)) {}

LogicalExpr::LogicalExpr(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right)
    : left(std::move(left)), op(std::move(op)), right(std::move(right)) {}

CallExpr::CallExpr(ExprPtrVariant callee, spicy::Token paren,
                   std::vector<ExprPtrVariant> arguments)
    : callee(std::move(callee)),
      paren(std::move(paren)),
      arguments(std::move(arguments)) {}

FuncExpr::FuncExpr(std::vector<spicy::Token> parameters,
                   std::vector<StmtPtrVariant> body)
    : parameters(std::move(parameters)), body(std::move(body)) {}

GetExpr::GetExpr(ExprPtrVariant expr, spicy::Token name)
    : object(std::move(expr)), name(std::move(name)) {}

SetExpr::SetExpr(ExprPtrVariant expr, spicy::Token name, ExprPtrVariant value)
    : object(std::move(expr)), name(std::move(name)), value(std::move(value)) {}

ThisExpr::ThisExpr(spicy::Token keyword) : keyword(std::move(keyword)) {}

SuperExpr::SuperExpr(spicy::Token keyword, spicy::Token method)
    : keyword(std::move(keyword)), method(std::move(method)) {}

IndexGetExpr::IndexGetExpr(spicy::Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx) 
    : lbracket(std::move(lbracket)), lst(std::move(arr)), idx(std::move(idx)) {}

IndexSetExpr::IndexSetExpr(Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx, ExprPtrVariant val) 
    : lbracket(std::move(lbracket)), lst(std::move(arr)), idx(std::move(idx)), val(std::move(val)) {}

// ==============================//
// EPV creation helper functions //
// ==============================//
auto createBinaryEPV(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right)
    -> ExprPtrVariant {
  return std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
}

auto createUnaryEPV(spicy::Token op, ExprPtrVariant right) -> ExprPtrVariant {
  return std::make_unique<UnaryExpr>(op, std::move(right));
}

auto createGroupingEPV(ExprPtrVariant right) -> ExprPtrVariant {
  return std::make_unique<GroupingExpr>(std::move(right));
}

auto createLiteralEPV(spicy::OptTokenLiteral literal) -> ExprPtrVariant {
  return std::make_unique<LiteralExpr>(std::move(literal));
}

auto createConditionalEPV(ExprPtrVariant condition, ExprPtrVariant then,
                          ExprPtrVariant elseBranch) -> ExprPtrVariant {
  return std::make_unique<ConditionalExpr>(
      std::move(condition), std::move(then), std::move(elseBranch));
}

auto createPostfixEPV(ExprPtrVariant left, spicy::Token op) -> ExprPtrVariant {
  return std::make_unique<PostfixExpr>(std::move(left), op);
}

auto createVarEPV(spicy::Token varName) -> ExprPtrVariant {
  return std::make_unique<VariableExpr>(varName);
}

auto createAssignEPV(spicy::Token varName, ExprPtrVariant expr) -> ExprPtrVariant {
  return std::make_unique<AssignExpr>(varName, std::move(expr));
}

auto createLogicalEPV(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right)
    -> ExprPtrVariant {
  return std::make_unique<LogicalExpr>(std::move(left), op, std::move(right));
}

auto createCallEPV(ExprPtrVariant callee, spicy::Token paren,
                   std::vector<ExprPtrVariant> arguments) -> ExprPtrVariant {
  return std::make_unique<CallExpr>(std::move(callee), std::move(paren),
                                    std::move(arguments));
}

auto createFuncEPV(std::vector<spicy::Token> params,
                   std::vector<StmtPtrVariant> fnBody) -> ExprPtrVariant {
  return std::make_unique<FuncExpr>(std::move(params), std::move(fnBody));
}

auto createGetEPV(ExprPtrVariant expr, spicy::Token name) -> ExprPtrVariant {
  return std::make_unique<GetExpr>(std::move(expr), std::move(name));
}

auto createSetEPV(ExprPtrVariant expr, spicy::Token name, ExprPtrVariant value)
    -> ExprPtrVariant {
  return std::make_unique<SetExpr>(std::move(expr), std::move(name),
                                   std::move(value));
}

auto createThisEPV(spicy::Token keyword) -> ExprPtrVariant {
  return std::make_unique<ThisExpr>(std::move(keyword));
}

auto createSuperEPV(spicy::Token keyword, spicy::Token method) -> ExprPtrVariant {
  return std::make_unique<SuperExpr>(std::move(keyword), std::move(method));
}

auto createIndexGetEPV(spicy::Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx) -> ExprPtrVariant {
    return std::make_unique<IndexGetExpr>(std::move(lbracket), std::move(arr), std::move(idx));
}

auto createIndexSetEPV(spicy::Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx, ExprPtrVariant val) -> ExprPtrVariant {
    return std::make_unique<IndexSetExpr>(std::move(lbracket), std::move(arr), std::move(idx), std::move(val));
}

// =================== //
// Statment AST types; //
// =================== //
ExprStmt::ExprStmt(ExprPtrVariant expr) : expression(std::move(expr)) {}

PrintStmt::PrintStmt(ExprPtrVariant expr) : expression(std::move(expr)) {}

BlockStmt::BlockStmt(std::vector<StmtPtrVariant> statements)
    : statements(std::move(statements)) {}

VarStmt::VarStmt(spicy::Token varName, std::optional<ExprPtrVariant> initializer)
    : varName(std::move(varName)), initializer(std::move(initializer)) {}

IfStmt::IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
               std::optional<StmtPtrVariant> elseBranch)
    : condition(std::move(condition)),
      thenBranch(std::move(thenBranch)),
      elseBranch(std::move(elseBranch)) {}

WhileStmt::WhileStmt(ExprPtrVariant condition, StmtPtrVariant loopBody)
    : condition(std::move(condition)), loopBody(std::move(loopBody)) {}

FuncStmt::FuncStmt(spicy::Token funcName, FuncExprPtr funcExpr)
    : funcName(std::move(funcName)), funcExpr(std::move(funcExpr)) {}

RetStmt::RetStmt(spicy::Token ret, std::optional<ExprPtrVariant> value)
    : ret(std::move(ret)), value(std::move(value)) {}

ClassStmt::ClassStmt(spicy::Token className, std::optional<ExprPtrVariant> superClass,
                     std::vector<FuncStmtPtr> methods)
    : className(std::move(className)),
      superClass(std::move(superClass)),
      methods(std::move(methods)) {}

// ============================================================= //
// Helper functions to create StmtPtrVariants for each Stmt type //
// ============================================================= //
auto createExprSPV(ExprPtrVariant expr) -> StmtPtrVariant {
  return std::make_unique<ExprStmt>(std::move(expr));
}

auto createPrintSPV(ExprPtrVariant expr) -> StmtPtrVariant {
  return std::make_unique<PrintStmt>(std::move(expr));
}

auto createBlockSPV(std::vector<StmtPtrVariant> statements) -> StmtPtrVariant {
  return std::make_unique<BlockStmt>(std::move(statements));
}

auto createVarSPV(spicy::Token varName, std::optional<ExprPtrVariant> initializer)
    -> StmtPtrVariant {
  return std::make_unique<VarStmt>(varName, std::move(initializer));
}

auto createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                 std::optional<StmtPtrVariant> elseBranch) -> StmtPtrVariant {
  return std::make_unique<IfStmt>(std::move(condition), std::move(thenBranch),
                                  std::move(elseBranch));
}

auto createWhileSPV(ExprPtrVariant condition, StmtPtrVariant loopBody)
    -> StmtPtrVariant {
  return std::make_unique<WhileStmt>(std::move(condition), std::move(loopBody));
}

auto createFuncSPV(spicy::Token fName, FuncExprPtr funcExpr) -> StmtPtrVariant {
  return std::make_unique<FuncStmt>(std::move(fName), std::move(funcExpr));
}

auto createRetSPV(spicy::Token ret, std::optional<ExprPtrVariant> value)
    -> StmtPtrVariant {
  return std::make_unique<RetStmt>(std::move(ret), std::move(value));
}

auto createClassSPV(spicy::Token className, std::optional<ExprPtrVariant> superClass,
                    std::vector<FuncStmtPtr> methods) -> StmtPtrVariant {
  return std::make_unique<ClassStmt>(std::move(className),
                                     std::move(superClass), std::move(methods));
}

}
