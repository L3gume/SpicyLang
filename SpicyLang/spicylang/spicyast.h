#pragma once
#ifndef H_SPICYAST
#define H_SPICYAST

#include <memory>
#include <optional>
#include <variant>
#include <vector>

#include "types.h"
#include "spicyutil.h"

namespace spicy::ast {

// Expression types forward-declaration
struct BinaryExpr;
struct GroupingExpr;
struct LiteralExpr;
struct UnaryExpr;
struct ConditionalExpr;
struct PostfixExpr;
struct VariableExpr;
struct AssignExpr;
struct LogicalExpr;
struct CallExpr;
struct FuncExpr;
struct GetExpr;
struct SetExpr;
struct ThisExpr;
struct SuperExpr;
struct IndexGetExpr;
struct IndexSetExpr;

// Expression pointer types
using BinaryExprPtr = std::unique_ptr<BinaryExpr>;
using GroupingExprPtr = std::unique_ptr<GroupingExpr>;
using LiteralExprPtr = std::unique_ptr<LiteralExpr>;
using UnaryExprPtr = std::unique_ptr<UnaryExpr>;
using ConditionalExprPtr = std::unique_ptr<ConditionalExpr>;
using PostfixExprPtr = std::unique_ptr<PostfixExpr>;
using VariableExprPtr = std::unique_ptr<VariableExpr>;
using AssignExprPtr = std::unique_ptr<AssignExpr>;
using LogicalExprPtr = std::unique_ptr<LogicalExpr>;
using CallExprPtr = std::unique_ptr<CallExpr>;
using FuncExprPtr = std::unique_ptr<FuncExpr>;
using GetExprPtr = std::unique_ptr<GetExpr>;
using SetExprPtr = std::unique_ptr<SetExpr>;
using ThisExprPtr = std::unique_ptr<ThisExpr>;
using SuperExprPtr = std::unique_ptr<SuperExpr>;
using IndexGetExprPtr = std::unique_ptr<IndexGetExpr>;
using IndexSetExprPtr = std::unique_ptr<IndexSetExpr>;

using ExprPtrVariant = std::variant<
    BinaryExprPtr, GroupingExprPtr, LiteralExprPtr, UnaryExprPtr,
    ConditionalExprPtr, PostfixExprPtr, VariableExprPtr, AssignExprPtr,
    LogicalExprPtr, CallExprPtr, FuncExprPtr, GetExprPtr,
    SetExprPtr, ThisExprPtr, SuperExprPtr, IndexGetExprPtr,
    IndexSetExprPtr>;

// Statement types forward-declaration
struct ExprStmt;
struct PrintStmt;
struct BlockStmt;
struct VarStmt;
struct IfStmt;
struct WhileStmt;
struct FuncStmt;
struct RetStmt;
struct ClassStmt;

// Statement pointer types
using ExprStmtPtr = std::unique_ptr<ExprStmt>;
using PrintStmtPtr = std::unique_ptr<PrintStmt>;
using BlockStmtPtr = std::unique_ptr<BlockStmt>;
using VarStmtPtr = std::unique_ptr<VarStmt>;
using IfStmtPtr = std::unique_ptr<IfStmt>;
using WhileStmtPtr = std::unique_ptr<WhileStmt>;
using FuncStmtPtr = std::unique_ptr<FuncStmt>;
using RetStmtPtr = std::unique_ptr<RetStmt>;
using ClassStmtPtr = std::unique_ptr<ClassStmt>;

using StmtPtrVariant = std::variant<
    ExprStmtPtr, PrintStmtPtr, BlockStmtPtr, VarStmtPtr,
    IfStmtPtr, WhileStmtPtr, FuncStmtPtr,
    RetStmtPtr, ClassStmtPtr>;

using SpicyProgram = std::vector<StmtPtrVariant>;

// Helper functions to make passing pointers easier
auto createBinaryEPV(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right) -> ExprPtrVariant;
auto createUnaryEPV(spicy::Token op, ExprPtrVariant right) -> ExprPtrVariant;
auto createGroupingEPV(ExprPtrVariant right) -> ExprPtrVariant;
auto createLiteralEPV(spicy::OptTokenLiteral literal) -> ExprPtrVariant;
auto createConditionalEPV(ExprPtrVariant condition, ExprPtrVariant then, ExprPtrVariant elseBranch) -> ExprPtrVariant;
auto createPostfixEPV(ExprPtrVariant left, spicy::Token op) -> ExprPtrVariant;
auto createVarEPV(spicy::Token varName) -> ExprPtrVariant;
auto createAssignEPV(spicy::Token varName, ExprPtrVariant expr) -> ExprPtrVariant;
auto createLogicalEPV(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right) -> ExprPtrVariant;
auto createCallEPV(ExprPtrVariant callee, spicy::Token paren, std::vector<ExprPtrVariant> arguments) -> ExprPtrVariant;
auto createFuncEPV(std::vector<spicy::Token> params, std::vector<StmtPtrVariant> fnBody) -> ExprPtrVariant;
auto createGetEPV(ExprPtrVariant expr, spicy::Token name) -> ExprPtrVariant;
auto createSetEPV(ExprPtrVariant expr, spicy::Token name, ExprPtrVariant value) -> ExprPtrVariant;
auto createThisEPV(spicy::Token keyword) -> ExprPtrVariant;
auto createSuperEPV(spicy::Token keyword, spicy::Token method) -> ExprPtrVariant;
auto createIndexGetEPV(spicy::Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx) -> ExprPtrVariant;
auto createIndexSetEPV(spicy::Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx, ExprPtrVariant val) -> ExprPtrVariant;

// Helper functions to create StmtPtrVariants for each Stmt type
auto createExprSPV(ExprPtrVariant expr) -> StmtPtrVariant;
auto createPrintSPV(ExprPtrVariant expr) -> StmtPtrVariant;
auto createBlockSPV(std::vector<StmtPtrVariant> statements) -> StmtPtrVariant;
auto createVarSPV(spicy::Token varName, std::optional<ExprPtrVariant> initializer) -> StmtPtrVariant;
auto createIfSPV(ExprPtrVariant condition, StmtPtrVariant thenBranch, std::optional<StmtPtrVariant> elseBranch) -> StmtPtrVariant;
auto createWhileSPV(ExprPtrVariant condition, StmtPtrVariant loopBody) -> StmtPtrVariant;
auto createFuncSPV(spicy::Token fName, FuncExprPtr funcExpr) -> StmtPtrVariant;
auto createRetSPV(spicy::Token ret, std::optional<ExprPtrVariant> value) -> StmtPtrVariant;
auto createClassSPV(spicy::Token className, std::optional<ExprPtrVariant> superClass, std::vector<FuncStmtPtr> methods) -> StmtPtrVariant;

// Expression AST Types:
struct BinaryExpr final : public util::Uncopyable {
  ExprPtrVariant left;
  spicy::Token op;
  ExprPtrVariant right;
  BinaryExpr(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right);
};

struct GroupingExpr final : public util::Uncopyable {
  ExprPtrVariant expression;
  explicit GroupingExpr(ExprPtrVariant expression);
};

struct LiteralExpr final : public util::Uncopyable {
  spicy::OptTokenLiteral literalVal;
  explicit LiteralExpr(spicy::OptTokenLiteral value);
};

struct UnaryExpr final : public util::Uncopyable {
  spicy::Token op;
  ExprPtrVariant right;
  UnaryExpr(spicy::Token op, ExprPtrVariant right);
};

struct ConditionalExpr final : public util::Uncopyable {
  ExprPtrVariant condition;
  ExprPtrVariant thenBranch;
  ExprPtrVariant elseBranch;
  ConditionalExpr(ExprPtrVariant condition, ExprPtrVariant thenBranch,
                  ExprPtrVariant elseBranch);
};

struct PostfixExpr final : public util::Uncopyable {
  ExprPtrVariant left;
  spicy::Token op;
  PostfixExpr(ExprPtrVariant left, spicy::Token op);
};

struct VariableExpr final : public util::Uncopyable {
  spicy::Token varName;
  explicit VariableExpr(spicy::Token varName);
};

struct AssignExpr final : public util::Uncopyable {
  spicy::Token varName;
  ExprPtrVariant right;
  AssignExpr(spicy::Token varName, ExprPtrVariant right);
};

struct LogicalExpr final : public util::Uncopyable {
  ExprPtrVariant left;
  spicy::Token op;
  ExprPtrVariant right;
  LogicalExpr(ExprPtrVariant left, spicy::Token op, ExprPtrVariant right);
};

struct CallExpr final : public util::Uncopyable {
  ExprPtrVariant callee;
  spicy::Token paren;
  std::vector<ExprPtrVariant> arguments;
  CallExpr(ExprPtrVariant callee, spicy::Token paren,
           std::vector<ExprPtrVariant> arguments);
};

struct FuncExpr final : public util::Uncopyable {
  std::vector<spicy::Token> parameters;
  std::vector<StmtPtrVariant> body;
  FuncExpr(std::vector<spicy::Token> parameters, std::vector<StmtPtrVariant> body);
};

struct GetExpr final : public util::Uncopyable {
  ExprPtrVariant object;
  spicy::Token name;
  GetExpr(ExprPtrVariant expr, spicy::Token name);
};

struct SetExpr final : public util::Uncopyable {
  ExprPtrVariant object;
  spicy::Token name;
  ExprPtrVariant value;
  SetExpr(ExprPtrVariant expr, spicy::Token name, ExprPtrVariant value);
};

struct ThisExpr final : public util::Uncopyable {
  spicy::Token keyword;
  explicit ThisExpr(spicy::Token keyword);
};

struct SuperExpr final : public util::Uncopyable {
  spicy::Token keyword;
  spicy::Token method;
  explicit SuperExpr(spicy::Token keyword, spicy::Token method);
};

struct IndexGetExpr final : public util::Uncopyable {
    Token lbracket;
    ExprPtrVariant lst;
    ExprPtrVariant idx;
    explicit IndexGetExpr(Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx);
};

struct IndexSetExpr final : public util::Uncopyable {
    Token lbracket;
    ExprPtrVariant lst;
    ExprPtrVariant idx;
    ExprPtrVariant val;
    explicit IndexSetExpr(Token lbracket, ExprPtrVariant arr, ExprPtrVariant idx, ExprPtrVariant val);
};

// Statment AST types;
struct ExprStmt final : public util::Uncopyable {
  ExprPtrVariant expression;
  explicit ExprStmt(ExprPtrVariant expr);
};

struct PrintStmt final : public util::Uncopyable {
  ExprPtrVariant expression;
  explicit PrintStmt(ExprPtrVariant expression);
};

struct BlockStmt final : public util::Uncopyable {
  std::vector<StmtPtrVariant> statements;
  explicit BlockStmt(std::vector<StmtPtrVariant> statements);
};

struct VarStmt final : public util::Uncopyable {
  spicy::Token varName;
  std::optional<ExprPtrVariant> initializer;
  explicit VarStmt(spicy::Token varName, std::optional<ExprPtrVariant> initializer);
};

struct IfStmt final : public util::Uncopyable {
  ExprPtrVariant condition;
  StmtPtrVariant thenBranch;
  std::optional<StmtPtrVariant> elseBranch;
  explicit IfStmt(ExprPtrVariant condition, StmtPtrVariant thenBranch,
                  std::optional<StmtPtrVariant> elseBranch);
};

struct WhileStmt final : public util::Uncopyable {
  ExprPtrVariant condition;
  StmtPtrVariant loopBody;
  explicit WhileStmt(ExprPtrVariant condition, StmtPtrVariant loopBody);
};

struct FuncStmt : public util::Uncopyable {
  spicy::Token funcName;
  FuncExprPtr funcExpr;
  FuncStmt(spicy::Token funcName, FuncExprPtr funcExpr);
};

struct RetStmt : public util::Uncopyable {
  spicy::Token ret;
  std::optional<ExprPtrVariant> value;
  RetStmt(spicy::Token ret, std::optional<ExprPtrVariant> value);
};

struct ClassStmt : public util::Uncopyable {
  spicy::Token className;
  std::optional<ExprPtrVariant> superClass;
  std::vector<FuncStmtPtr> methods;
  ClassStmt(spicy::Token className, std::optional<ExprPtrVariant> superClass,
            std::vector<FuncStmtPtr> methods);
};

}
#endif
