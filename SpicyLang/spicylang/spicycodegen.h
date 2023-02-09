#pragma once

#include <map>
#include <vector>

#include "spicyast.h"

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Constant.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

namespace spicy {
    
class SpicyCodeGenerator {
    friend class SpicyExprCodeGenerator;
    friend class SpicyStmtCodeGenerator;
public:
    SpicyCodeGenerator() = default;

    auto generateStmts(const ast::SpicyProgram& stmts) -> llvm::Module*; // TODO: return module? -> likely a function
    auto generateStmt(const ast::StmtPtrVariant& stmt) -> llvm::Function*; // single statement may not be a function
    
private: 
    auto generateFuncStmt(const ast::FuncStmtPtr& stmt) -> llvm::Function*;
    auto generateExprStmt(const ast::ExprStmtPtr& stmt) -> llvm::Function*;
    
    auto generateExpr(const ast::ExprPtrVariant& expr) -> llvm::Value*;
    auto generateLiteralExpr(const ast::LiteralExprPtr& expr) -> llvm::Value*;
    auto generateVariableExpr(const ast::VariableExprPtr& expr) -> llvm::Value*;
    
    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    std::unique_ptr<llvm::Module> m_module = nullptr;
    std::map<std::string, llvm::Value*> namedValues; // TODO: Figure out how to properly do the symbol table thing

};

};
