#include "spicyastprinter.h"

#include <iterator>
#include <utility>
#include <vector>
#include <format>

#include "spicyast.h"
#include "spicyscanner.h"
#include "types.h"

namespace spicy::ast {
//=========================//
// Expr Printing functions //
//=========================//
namespace {
    
struct LiteralPrintVisitor {
    [[nodiscard]]
    std::string operator()(const std::string& stringLiteral) {
        return "\"" + stringLiteral + "\"";
    }
    
    [[nodiscard]]
    std::string operator()(const double& numLiteral) {
        auto result = std::to_string(numLiteral);
        auto pos = result.find(".000000");
        if (pos != std::string::npos)
          result.erase(pos, std::string::npos);
        else
          result.erase(result.find_last_not_of('0') + 1, std::string::npos);
        return result;
    }
};

auto getLiteralString(spicy::OptTokenLiteral literal) -> std::string {
    if (!literal.has_value()) return "nil";
    return std::visit(LiteralPrintVisitor{}, literal.value());
}

auto parenthesize(const std::string& val)
    -> std::string {
    return std::format("({})", val);
}

auto parenthesize(const std::string& name, const ExprPtrVariant& expr)
    -> std::string {
    return std::format("({} {})", name, printer::toString(expr));
}

auto parenthesize(const std::string& name, const ExprPtrVariant& expr1,
                  const ExprPtrVariant& expr2) -> std::string {
    return std::format("({} {} {})", name, printer::toString(expr1), printer::toString(expr2));
}

auto printBinaryExpr(const BinaryExprPtr& expr) -> std::string {
    return parenthesize(expr->op.lexeme, expr->left, expr->right);
}

auto printGroupingExpr(const GroupingExprPtr& expr) -> std::string {
    std::string name = "group";
    return parenthesize(name, expr->expression);
}

auto printLiteralExpr(const LiteralExprPtr& expr) -> std::string {
    return expr->literalVal.has_value()
             ? getLiteralString(expr->literalVal.value())
             : "nil";
}

auto printUnaryExpr(const UnaryExprPtr& expr) -> std::string {
    return parenthesize(expr->op.lexeme, expr->right);
}

auto printConditionalExpr(const ConditionalExprPtr& expr) -> std::string {
    return parenthesize(": " + parenthesize("?", expr->condition),
      expr->thenBranch, expr->elseBranch);
}

auto printPostfixExpr(const PostfixExprPtr& expr) -> std::string {
    return parenthesize("POSTFIX " + expr->op.lexeme, expr->left);
}

auto printVariableExpr(const VariableExprPtr& expr) -> std::string {
    return parenthesize(expr->varName.lexeme);
}

auto printAssignExpr(const AssignExprPtr& expr) -> std::string {
    return parenthesize("= " + expr->varName.lexeme, expr->right) + ";";
}

auto printLogicalExpr(const LogicalExprPtr& expr) -> std::string {
    return parenthesize(expr->op.lexeme, expr->left, expr->right);
}
// myGloriousFn(arg1, expr1+expr2)
// ( ((arg1), (+ expr1 expr2)) myGloriousFn )
auto printCallExpr(const CallExprPtr& expr) -> std::string {
    std::string result;
    for (size_t i = 0; i < expr->arguments.size(); ++i) {
        if (i > 0 && i < expr->arguments.size() - 1) result += ", ";
            result += "(" + printer::toString(expr->arguments[i]) + ")";
    }
    return parenthesize("(" + result + ")", expr->callee);
}

auto printFuncExpr(const FuncExprPtr& expr) -> std::string {
    std::string funcStr = "(";
    for (size_t i = 0; i < expr->parameters.size(); ++i) {
        if (0 != i) funcStr += ", ";
        funcStr += expr->parameters[i].lexeme;
    }
    funcStr += ") {\n";
    for (const auto& bodyStmt : expr->body) {
        for (const auto& str : printer::toString(bodyStmt)) {
            funcStr += str + "\n";
        }
    }
    funcStr += "\n}";
    return funcStr;
}

auto printGetExpr(const GetExprPtr& expr) -> std::string {
    std::string str = printer::toString(expr->object);
    return std::format("{}.( get {} )", str, expr->name.lexeme);
}

auto printSetExpr(const SetExprPtr& expr) -> std::string {
    std::string str = printer::toString(expr->object);
    std::string val = printer::toString(expr->value);
    return std::format("{}.( set {} ) = {}", str, expr->name.lexeme, val);
}

auto printThisExpr(const ThisExprPtr& expr) -> std::string {
    return "( this )";
}

auto printSuperExpr(const SuperExprPtr& expr) -> std::string {
    return std::format("( super.{} )", expr->method.toString());
}

auto printIndexGetExpr(const IndexGetExprPtr& expr) -> std::string {
    return std::format("( IndexGet {}[{}] )", printer::toString(expr->lst), printer::toString(expr->idx));
}

auto printIndexSetExpr(const IndexSetExprPtr& expr) -> std::string {
    return std::format("( IndexSet {}[{}] = {} )", printer::toString(expr->lst), printer::toString(expr->idx), printer::toString(expr->val));
}
}  // namespace 

struct ExprPtrPrintVisitor {
    [[nodiscard]]
    std::string operator()(const BinaryExprPtr& expr) {
        return printBinaryExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const GroupingExprPtr& expr) {
        return printGroupingExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const LiteralExprPtr& expr) {
        return printLiteralExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const UnaryExprPtr& expr) {
        return printUnaryExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const ConditionalExprPtr& expr) { 
        return printConditionalExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const PostfixExprPtr& expr) { 
        return printPostfixExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const VariableExprPtr& expr) {
        return printVariableExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const AssignExprPtr& expr) {
        return printAssignExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const LogicalExprPtr& expr) {
        return printLogicalExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const CallExprPtr& expr) {
        return printCallExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const FuncExprPtr& expr) {
        return printFuncExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const GetExprPtr& expr) {
        return printGetExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const SetExprPtr& expr) {
        return printSetExpr(expr); 
    }
    [[nodiscard]]
    std::string operator()(const ThisExprPtr& expr) {
        return printThisExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const SuperExprPtr& expr) {
        return printSuperExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const IndexGetExprPtr& expr) {
        return printIndexGetExpr(expr);
    }
    [[nodiscard]]
    std::string operator()(const IndexSetExprPtr& expr) {
        return printIndexSetExpr(expr);
    }
};

auto printer::toString(const ExprPtrVariant &expr) -> std::string {
    return std::visit(ExprPtrPrintVisitor{}, expr);
}

namespace {

auto printExprStmt(const ExprStmtPtr& stmt) -> std::string {
    return parenthesize("", stmt->expression) + ";";
}

auto printPrintStmt(const PrintStmtPtr& stmt) -> std::string {
    return parenthesize("print", stmt->expression) + ";";
}

auto printBlockStmt(const BlockStmtPtr& blkStmts) -> std::vector<std::string> {
    std::vector<std::string> blkStmtStrsVec;
    blkStmtStrsVec.emplace_back("{");
    for (auto& stmt : blkStmts->statements) {
    auto stmtStr = printer::toString(stmt);
    if (!stmtStr.empty())
        std::move(stmtStr.begin(), stmtStr.end(),
            std::back_inserter(blkStmtStrsVec));
    }
    blkStmtStrsVec.emplace_back("}");
    return blkStmtStrsVec;
}

auto printVarStmt(const VarStmtPtr& stmt) -> std::string {
    return std::format("var {}{}", stmt->varName.lexeme, 
        stmt->initializer.has_value() ? std::format("( = {});", printer::toString(stmt->initializer.value())) : "");
}

auto printIfStmt(const IfStmtPtr& stmt) -> std::vector<std::string> {
    std::vector<std::string> ifStmtStrVec;
    ifStmtStrVec.emplace_back(std::format("( if ({})", printer::toString(stmt->condition)));
    auto thenBranchVec = printer::toString(stmt->thenBranch);
    std::move(thenBranchVec.begin(), thenBranchVec.end(),
            std::back_inserter(ifStmtStrVec));
    if (stmt->elseBranch.has_value()) {
        ifStmtStrVec.emplace_back(" else ");
        auto elseBranchVec = printer::toString(stmt->elseBranch.value());
        std::move(elseBranchVec.begin(), elseBranchVec.end(),
                  std::back_inserter(ifStmtStrVec));
        }
    ifStmtStrVec.emplace_back(" );");
    return ifStmtStrVec;
}

auto printWhileStmt(const WhileStmtPtr& stmt) {
    std::vector<std::string> whileStmtStrVec;
    whileStmtStrVec.emplace_back(std::format("( while ({})", printer::toString(stmt->condition)));
    auto loopBodyVec = printer::toString(stmt->loopBody);
    std::move(loopBodyVec.begin(), loopBodyVec.end(),
            std::back_inserter(whileStmtStrVec));
    whileStmtStrVec.emplace_back(" );");
    return whileStmtStrVec;
}

// Token funcName, FuncExprPtr funcExpr
auto printFuncStmt(const FuncStmtPtr& stmt) -> std::vector<std::string> {
    std::vector<std::string> funcStrVec;
    funcStrVec.emplace_back("( ( " + stmt->funcName.lexeme + ") ");
    funcStrVec.emplace_back(printFuncExpr(stmt->funcExpr));
    funcStrVec.emplace_back(")");
    return funcStrVec;
}

auto printRetStmt(const RetStmtPtr& stmt) -> std::string {
    std::string value = stmt->value.has_value()
                          ? printer::toString(stmt->value.value())
                          : " ";
    return std::format("( return {} );", value);
}

auto printClassStmt(const ClassStmtPtr& stmt) -> std::vector<std::string> {
    std::vector<std::string> strVec;
    strVec.emplace_back("(CLASS " + stmt->className.lexeme + " )");
    if (stmt->superClass.has_value())
        strVec.emplace_back(" < "
                        + printer::toString(stmt->superClass.value()));
    strVec.emplace_back("{");
    for (const auto& method : stmt->methods) {
        std::vector<std::string> methodStr = printFuncStmt(method);
        std::move(methodStr.begin(), methodStr.end(), std::back_inserter(strVec));
    }
    strVec.emplace_back("}");
    return strVec;
}

}  // namespace

struct StmtPtrPrintVisitor {
    [[nodiscard]]
    std::vector<std::string> operator()(const ExprStmtPtr& stmt) {
        return std::vector(1, printExprStmt(stmt));
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const PrintStmtPtr& stmt) {
        return std::vector(1, printPrintStmt(stmt));
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const BlockStmtPtr& stmt) {
        return printBlockStmt(stmt);
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const VarStmtPtr& stmt) {
        return std::vector(1, printVarStmt(stmt));
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const IfStmtPtr& stmt) {
        return printIfStmt(stmt);
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const WhileStmtPtr& stmt) {
        return printWhileStmt(stmt);
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const FuncStmtPtr& stmt) {
        return printFuncStmt(stmt);
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const RetStmtPtr& stmt) {
        return std::vector(1, printRetStmt(stmt));
    }
    [[nodiscard]]
    std::vector<std::string> operator()(const ClassStmtPtr& stmt) {
        return printClassStmt(stmt);
    }
};

auto printer::toString(const StmtPtrVariant& statement)
    -> std::vector<std::string> {
    return std::visit(StmtPtrPrintVisitor{}, statement);
}

auto printer::toString(const std::vector<StmtPtrVariant>& statements)
    -> std::vector<std::string> {
    std::vector<std::string> stmtStrsVec;
    for (const auto& stmt : statements) {
        auto stmtStr = printer::toString(stmt);
        if (!stmtStr.empty())
        std::move(stmtStr.begin(), stmtStr.end(),
                    std::back_inserter(stmtStrsVec));
    }
    return stmtStrsVec;
}
}
