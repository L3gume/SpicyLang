#include "spicyeval.h"

#include <variant>
#include <optional>
#include <string>
#include <format>

#include "spicy.h"
#include "spicybuiltins.h"

namespace spicy::eval {

namespace typecheck {
void checkUnaryNumOperand(Token op, const SpicyObj& rhs) {
    if (std::holds_alternative<double>(rhs)) return;
    throw RuntimeError(op, "Operand must be a number.");
}

void checkBinaryNumOperands(Token op, const SpicyObj& lhs, const SpicyObj& rhs) {
    if (std::holds_alternative<double>(lhs) && std::holds_alternative<double>(rhs)) return;
    throw RuntimeError(op, "Operands must be numbers.");
}

void checkPlusOperands(Token op, const SpicyObj& lhs, const SpicyObj& rhs) {
    if (std::holds_alternative<double>(lhs) && std::holds_alternative<double>(rhs)) return;
    if (std::holds_alternative<std::string>(lhs) && std::holds_alternative<std::string>(rhs)) return;
    throw RuntimeError(op, "Operands must be numbers or strings.");
}

void checkAppendOperands(Token op, const SpicyObj& lhs, const SpicyObj& rhs) {
    if (op.type == TokenType::APPEND) {
        if (!std::holds_alternative<SpicyListSharedPtr>(lhs))
            throw RuntimeError(op, "Can only append elements to lists.");
    }
    else if (op.type == TokenType::APPEND_FRONT) {
        if (!std::holds_alternative<SpicyListSharedPtr>(rhs))
            throw RuntimeError(op, "Can only append elements to lists.");
    }
}
}

namespace internal {
SpicyObj getObjFromStringLit(const TokenLiteral& lit) {
    const auto& str = std::get<std::string>(lit);
    if (str == "true") return SpicyObj{true};
    if (str == "false") return SpicyObj{false};
    if (str == "nil") return SpicyObj{nullptr};
    if (str == "<spicy_list>") return std::make_shared<SpicyList>();
    return SpicyObj{str};
}
/*
 * the plus operator is a tiny bit complicated
 */
SpicyObj evalPlusBinOp(const SpicyObj& lval, const SpicyObj& rval) {
    if (std::holds_alternative<double>(lval) && std::holds_alternative<double>(rval)) {
        return std::get<double>(lval) + std::get<double>(rval);
    }
    if (std::holds_alternative<std::string>(lval) && std::holds_alternative<std::string>(rval)) {
        return std::get<std::string>(lval) + std::get<std::string>(rval);
    }
    return SpicyObj{nullptr};
}
} // namespace internal

SpicyEvaluator::SpicyEvaluator(bool isRepl) : m_isRepl(isRepl) {
    initBuiltins();
}

struct SpicyExprEvaluator {
    SpicyEvaluator* const eval;
    explicit SpicyExprEvaluator(SpicyEvaluator* eval) : eval(eval) {}
    [[nodiscard]] SpicyObj operator()(const ast::BinaryExprPtr& expr) { return eval->evalBinaryExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::GroupingExprPtr& expr) { return eval->evalGroupingExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::LiteralExprPtr& expr) { return eval->evalLiteralExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::UnaryExprPtr& expr) { return eval->evalUnaryExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::ConditionalExprPtr& expr) { return SpicyObj{nullptr}; }
    [[nodiscard]] SpicyObj operator()(const ast::PostfixExprPtr& expr) { return eval->evalPostfixExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::VariableExprPtr& expr) { return eval->evalVariableExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::AssignExprPtr& expr) { return eval->evalAssignExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::LogicalExprPtr& expr) { return eval->evalLogicalExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::CallExprPtr& expr) { return eval->evalCallExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::FuncExprPtr& expr) { return eval->evalFuncExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::GetExprPtr& expr) { return eval->evalGetExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::SetExprPtr& expr) { return eval->evalSetExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::ThisExprPtr& expr) { return eval->evalThisExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::SuperExprPtr& expr) { return eval->evalSuperExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::IndexGetExprPtr& expr) { return eval->evalIndexGetExpr(expr); }
    [[nodiscard]] SpicyObj operator()(const ast::IndexSetExprPtr& expr) { return eval->evalIndexSetExpr(expr); }
};

SpicyObj SpicyEvaluator::evalExpr(const ast::ExprPtrVariant& expr) {
    m_lastObj = std::move(std::visit(SpicyExprEvaluator(this), expr));
    return m_lastObj;
}

SpicyObj SpicyEvaluator::evalLiteralExpr(const ast::LiteralExprPtr& expr) {
    if (expr->literalVal.has_value()) {
        const auto& val = expr->literalVal.value();
        return std::holds_alternative<std::string>(val)
                ? internal::getObjFromStringLit(val)
                : SpicyObj{std::get<double>(val)};
    }
    return SpicyObj{nullptr};
}

SpicyObj SpicyEvaluator::evalGroupingExpr(const ast::GroupingExprPtr& expr) {
    return evalExpr(expr->expression);
}

SpicyObj SpicyEvaluator::evalUnaryExpr(const ast::UnaryExprPtr& expr) {
    auto rval = evalExpr(expr->right);
    switch (expr->op.type) {
    case TokenType::MINUS:
        typecheck::checkUnaryNumOperand(expr->op, rval);
        return -std::get<double>(rval);
    case TokenType::BANG:
        return !isTrue(rval);
    case TokenType::PLUS_PLUS:
    case TokenType::MINUS_MINUS:
    {
        if (!std::holds_alternative<ast::VariableExprPtr>(expr->right))
            throw RuntimeError(expr->op, "Operand must be a variable.");
        typecheck::checkUnaryNumOperand(expr->op, rval);
        const auto& varExpr = std::get<ast::VariableExprPtr>(expr->right);
        const auto value = expr->op.type == TokenType::PLUS_PLUS ? ++std::get<double>(rval) : --std::get<double>(rval);
        if (const auto& distance = m_locals.find(reinterpret_cast<uint64_t>(varExpr.get()));
            distance != m_locals.end()) {
            m_envMgr.assignAt(distance->second, varExpr->varName, value);
        } else {
            m_envMgr.assignGlobal(varExpr->varName, value);
        }
        return value;
    }
    default:
        throw RuntimeError(expr->op, "Invalid unary operator.");
    }
    return SpicyObj{nullptr};
}

SpicyObj SpicyEvaluator::evalPostfixExpr(const ast::PostfixExprPtr& expr) {
    if (!std::holds_alternative<ast::VariableExprPtr>(expr->left))
        throw RuntimeError(expr->op, "Operand must be a variable.");
    if (!(expr->op.type == TokenType::PLUS_PLUS || expr->op.type == TokenType::MINUS_MINUS))
        throw RuntimeError(expr->op, "Invalid postfix operator.");
    const auto& lval = evalExpr(expr->left);
    typecheck::checkUnaryNumOperand(expr->op, lval);
    const auto& varExpr = std::get<ast::VariableExprPtr>(expr->left);
    const auto& ret = std::get<double>(lval);
    const auto value = expr->op.type == TokenType::PLUS_PLUS ? ret + 1 : ret - 1;
    if (const auto& distance = m_locals.find(reinterpret_cast<uint64_t>(varExpr.get()));
        distance != m_locals.end()) {
        m_envMgr.assignAt(distance->second, varExpr->varName, value);
    } else {
        m_envMgr.assignGlobal(varExpr->varName, value);
    }
    return ret;
}

SpicyObj SpicyEvaluator::evalBinaryExpr(const ast::BinaryExprPtr &expr) {
    const auto& lval = evalExpr(expr->left);
    const auto& rval = evalExpr(expr->right);
    switch (expr->op.type) {
    case TokenType::PLUS:
        typecheck::checkPlusOperands(expr->op, lval, rval);
        return internal::evalPlusBinOp(lval, rval);
    case TokenType::MINUS:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) - std::get<double>(rval);
    case TokenType::SLASH:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) / std::get<double>(rval);
    case TokenType::STAR:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) * std::get<double>(rval);
    case TokenType::GREATER:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) > std::get<double>(rval);
    case TokenType::GREATER_EQUAL:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) >= std::get<double>(rval);
    case TokenType::LESS:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) < std::get<double>(rval);
    case TokenType::LESS_EQUAL:
        typecheck::checkBinaryNumOperands(expr->op, lval, rval);
        return std::get<double>(lval) <= std::get<double>(rval);
    case TokenType::BANG_EQUAL:
        return !areEqual(lval, rval);
    case TokenType::EQUAL_EQUAL:
        return areEqual(lval, rval);
    case TokenType::APPEND: {
            typecheck::checkAppendOperands(expr->op, lval, rval);
            auto lst = std::get<SpicyListSharedPtr>(lval);
            lst->append(expr->op, rval);
            return lst;
        }
    case TokenType::APPEND_FRONT: {
            typecheck::checkAppendOperands(expr->op, lval, rval);
            auto lst = std::get<SpicyListSharedPtr>(rval);
            lst->appendFront(expr->op, lval);
            return lst;
        }
    default:
        throw RuntimeError(expr->op, "Unexpected operator in binary expression.");
    }
    return SpicyObj{nullptr};
}

SpicyObj SpicyEvaluator::evalVariableExpr(const ast::VariableExprPtr &expr) {
    return lookUpVariable(expr->varName, reinterpret_cast<uint64_t>(expr.get()));
}

SpicyObj SpicyEvaluator::evalAssignExpr(const ast::AssignExprPtr &expr) {
    auto value = evalExpr(expr->right);
    if (const auto& distance = m_locals.find(reinterpret_cast<uint64_t>(expr.get()));
        distance != m_locals.end()) {
        m_envMgr.assignAt(distance->second, expr->varName, value);
    } else {
        m_envMgr.assignGlobal(expr->varName, value);
    }
    return value;
}

SpicyObj SpicyEvaluator::evalLogicalExpr(const ast::LogicalExprPtr &expr) {
    const auto& lhs = evalExpr(expr->left);
    if (expr->op.type == TokenType::OR) {
        if (isTrue(lhs)) return lhs;
    } else {
        if (!isTrue(lhs)) return lhs;
    }
    return evalExpr(expr->right);
}

SpicyObj SpicyEvaluator::evalCallExpr(const ast::CallExprPtr &expr) {
    auto callee = evalExpr(expr->callee);

    if (std::holds_alternative<BuiltinFuncSharedPtr>(callee)) {
        return evalBuiltInCall(std::get<BuiltinFuncSharedPtr>(callee), expr);
    }

    auto instanceOrNull = ([&]() -> SpicyObj {
        if (std::holds_alternative<SpicyClassSharedPtr>(callee))
            return std::make_shared<SpicyInstance>(
                        std::get<SpicyClassSharedPtr>(callee));
        return SpicyObj{nullptr};
    })();

    const auto func = ([&]() -> FuncSharedPtr {
        if (std::holds_alternative<SpicyClassSharedPtr>(callee)) {
            auto instance = std::get<SpicyInstanceSharedPtr>(instanceOrNull);
            try {
                return bindInstance(std::get<FuncSharedPtr>(instance->get("init")), instance);
            } catch (RuntimeError& err) {
                runtimeError(err);
                return nullptr;
            }
        }
        if (std::holds_alternative<FuncSharedPtr>(callee))
            return std::get<FuncSharedPtr>(callee);
        throw RuntimeError(expr->paren, "Attempted to invoke a non-function.");
    })();

    if (func == nullptr)
        return instanceOrNull;

    if (expr->arguments.size() != func->arity())
        throw RuntimeError(expr->paren, std::format("Expected {} argurments but got {}.", func->arity(), expr->arguments.size()));

    std::vector<SpicyObj> args;
    for (const auto& arg : expr->arguments)
        args.emplace_back(evalExpr(arg));

    const auto& prevEnv = m_envMgr.getCurrentEnvironment();
    m_envMgr.setCurrentEnvironment(func->getClosure(), func->getFuncName());
    m_envMgr.createNewEnvironment(func->getFuncName());

    const auto& params = func->getParams();
    auto param = params.begin();
    auto arg = args.begin();
    for (; param != params.end() && arg != args.end(); ++param, ++arg) {
        m_envMgr.define(*param, *arg);
    }

    auto ret = execStmts(func->getBodyStmts());

    if (func->isInit())
        ret = m_envMgr.get(0, "this");

    if (!func->isMethod())
        m_envMgr.discardEnvironmentsUntil(func->getClosure(), func->getFuncName());
    else
        m_envMgr.discardEnvironmentsUntil(func->getClosure()->getParent(), func->getFuncName());

    m_envMgr.setCurrentEnvironment(prevEnv, func->getFuncName());

    if (ret.has_value())
        return ret.value();

    return instanceOrNull;
}

SpicyObj SpicyEvaluator::evalGetExpr(const ast::GetExprPtr &expr) {
    const auto& obj = evalExpr(expr->object);
    if (!std::holds_alternative<SpicyInstanceSharedPtr>(obj)) {
        throw RuntimeError(expr->name, "Only class instances have properties.");
    }
    auto prop = std::get<SpicyInstanceSharedPtr>(obj)->get(expr->name);
    if (std::holds_alternative<FuncSharedPtr>(prop)) {
        prop = SpicyObj{bindInstance(std::get<FuncSharedPtr>(prop), std::get<SpicyInstanceSharedPtr>(obj))};
    }
    return prop;
}

SpicyObj SpicyEvaluator::evalSetExpr(const ast::SetExprPtr &expr) {
    const auto& obj = evalExpr(expr->object);
    if (!std::holds_alternative<SpicyInstanceSharedPtr>(obj)) {
        throw RuntimeError(expr->name, "Only instances have fields.");
    }
    const auto& value = evalExpr(expr->value);
    std::get<SpicyInstanceSharedPtr>(obj)->set(expr->name, value);
    return value;
}

SpicyObj SpicyEvaluator::evalThisExpr(const ast::ThisExprPtr &expr) {
    return lookUpVariable(expr->keyword, reinterpret_cast<uint64_t>(expr.get()));
}

SpicyObj SpicyEvaluator::evalSuperExpr(const ast::SuperExprPtr &expr) {
    const auto distance = m_locals[reinterpret_cast<uint64_t>(expr.get())];
    const auto superClass = std::get<SpicyClassSharedPtr>(m_envMgr.get(distance, "super"));
    const auto instance = std::get<SpicyInstanceSharedPtr>(m_envMgr.get(distance - 1, "this"));
    auto method = superClass->findMethod(expr->method.lexeme);
    if (!method.has_value())
        throw RuntimeError(expr->method, std::format("Attempted to access undefined property {} on super.", expr->method.lexeme));
    return bindInstance(std::get<FuncSharedPtr>(method.value()), instance);
}

SpicyObj SpicyEvaluator::evalFuncExpr(const ast::FuncExprPtr &expr) {
    auto closure = m_envMgr.getCurrentEnvironment();
    // make sure to use a UNIQUE name for lambdas
    //const auto lambdaName = boost::uuids::to_string(expr->uuid);
    //const auto func = std::make_shared<FuncObj>(expr, "lambda", std::move(closure));
    return std::make_shared<FuncObj>(expr, "___lambda", std::move(closure));
}

SpicyObj SpicyEvaluator::evalIndexGetExpr(const ast::IndexGetExprPtr& expr) {
    const auto lstObj = evalExpr(expr->lst);
    if (!std::holds_alternative<SpicyListSharedPtr>(lstObj))
        throw RuntimeError(expr->lbracket, "Can only perform indexing operations on lists.");
    const auto idxObj = evalExpr(expr->idx);
    if (!std::holds_alternative<double>(idxObj))
        throw RuntimeError(expr->lbracket, "Index expression must evaluate to a number.");
    return std::get<SpicyListSharedPtr>(lstObj)->get(expr->lbracket, static_cast<int>(std::get<double>(idxObj)));
}

SpicyObj SpicyEvaluator::evalIndexSetExpr(const ast::IndexSetExprPtr& expr) {
    const auto& lstObj = evalExpr(expr->lst);
    if (!std::holds_alternative<SpicyListSharedPtr>(lstObj))
        throw RuntimeError(expr->lbracket, "Can only perform indexing operations on lists.");
    const auto idxObj = evalExpr(expr->idx);
    if (!std::holds_alternative<double>(idxObj))
        throw RuntimeError(expr->lbracket, "Index expression must evaluate to a number.");
    const auto valObj = evalExpr(expr->val);
    std::get<SpicyListSharedPtr>(lstObj)->set(expr->lbracket, static_cast<int>(std::get<double>(idxObj)), valObj);
    return lstObj;
}

struct SpicyStmtExecutor {
    SpicyEvaluator* const eval;
    explicit SpicyStmtExecutor(SpicyEvaluator* eval) : eval(eval) {}
    OptSpicyObj operator()(const ast::ExprStmtPtr& stmt) { return eval->execExpressionStmt(stmt); }
    OptSpicyObj operator()(const ast::PrintStmtPtr& stmt) { return eval->execPrintStmt(stmt); }
    OptSpicyObj operator()(const ast::BlockStmtPtr& stmt) { return eval->execBlockStmt(stmt); }
    OptSpicyObj operator()(const ast::VarStmtPtr& stmt) { return eval->execVarStmt(stmt); }
    OptSpicyObj operator()(const ast::IfStmtPtr& stmt) { return eval->execIfStmt(stmt); }
    OptSpicyObj operator()(const ast::WhileStmtPtr& stmt) { return eval->execWhileStmt(stmt); }
    OptSpicyObj operator()(const ast::FuncStmtPtr& stmt) { return eval->execFuncStmt(stmt); }
    OptSpicyObj operator()(const ast::RetStmtPtr& stmt) { return eval->execRetStmt(stmt); }
    OptSpicyObj operator()(const ast::ClassStmtPtr& stmt) {return eval->execClassStmt(stmt); }
};

OptSpicyObj SpicyEvaluator::execStmt(const ast::StmtPtrVariant &stmt) {
    return std::visit(SpicyStmtExecutor(this), stmt);
}

OptSpicyObj SpicyEvaluator::execStmts(const std::vector<ast::StmtPtrVariant> &stmts) {
    OptSpicyObj result = std::nullopt;
    try {
        for (const auto& st : stmts) {
            result = execStmt(st);
            if (std::holds_alternative<ast::RetStmtPtr>(st) || result.has_value()) break;
        }
    } catch (const RuntimeError& err) {
        runtimeError(err);
    }
    return result;
}

void SpicyEvaluator::resolve(uint64_t exprAddr, uint32_t depth) {
    m_locals.insert_or_assign(exprAddr, depth);
}

SpicyObj SpicyEvaluator::getLastObj() {
    return m_lastObj;
}

void SpicyEvaluator::initBuiltins() {
    m_envMgr.defineGlobal("clock", std::make_shared<ClockBuiltIn>());
    m_envMgr.defineGlobal("str", std::make_shared<StrBuiltIn>());
    m_envMgr.defineGlobal("sqrt", std::make_shared<SqrtBuiltIn>());
    m_envMgr.defineGlobal("len", std::make_shared<LenBuiltIn>());
    m_envMgr.defineGlobal("front", std::make_shared<FrontBuiltIn>());
    m_envMgr.defineGlobal("back", std::make_shared<BackBuiltIn>());
}

OptSpicyObj SpicyEvaluator::execExpressionStmt(const ast::ExprStmtPtr &stmt) {
    // TODO: need to hook in a value to determine if we're in a repl loop or not (?)
    evalExpr(stmt->expression);
    return std::nullopt;
}

OptSpicyObj SpicyEvaluator::execPrintStmt(const ast::PrintStmtPtr &stmt) {
    const auto& value = evalExpr(stmt->expression);
    std::cout << getObjString(value) << '\n';
    return std::nullopt;
}

OptSpicyObj SpicyEvaluator::execBlockStmt(const ast::BlockStmtPtr &stmt) {
    OptSpicyObj result = std::nullopt;
    const auto& prevEnv = m_envMgr.getCurrentEnvironment();
    m_envMgr.createNewEnvironment("execBlockStmt");
    result = execStmts(stmt->statements);
    m_envMgr.setCurrentEnvironment(prevEnv, "execBlockStmt");
    return result;
}

OptSpicyObj SpicyEvaluator::execVarStmt(const ast::VarStmtPtr &stmt) {
    if (stmt->initializer.has_value()) {
        m_envMgr.define(stmt->varName, evalExpr(stmt->initializer.value()));
    } else {
        m_envMgr.define(stmt->varName, SpicyObj{nullptr});
    }
    return std::nullopt;
}

OptSpicyObj SpicyEvaluator::execIfStmt(const ast::IfStmtPtr &stmt) {
    if (isTrue(evalExpr(stmt->condition))) {
        auto result = execStmt(stmt->thenBranch);
        if (result.has_value()) return result.value();
    } else if (stmt->elseBranch.has_value()) {
        auto result = execStmt(stmt->elseBranch.value());
        if (result.has_value()) return result.value();
    }
    return std::nullopt;
}

OptSpicyObj SpicyEvaluator::execWhileStmt(const ast::WhileStmtPtr &stmt) {
    OptSpicyObj result = std::nullopt;
    while (isTrue(evalExpr(stmt->condition)) && !result.has_value()) {
        result = execStmt(stmt->loopBody);
    }
    return result;
}

OptSpicyObj SpicyEvaluator::execFuncStmt(const ast::FuncStmtPtr &stmt) {
    auto func = std::make_shared<FuncObj>(
                    stmt->funcExpr,
                    stmt->funcName.lexeme,
                    m_envMgr.getCurrentEnvironment()
                );
    m_envMgr.define(stmt->funcName, func);
    return std::nullopt;
}

OptSpicyObj SpicyEvaluator::execRetStmt(const ast::RetStmtPtr &stmt) {
    return stmt->value.has_value()
              ? std::make_optional(evalExpr(stmt->value.value()))
              : std::nullopt;
}

OptSpicyObj SpicyEvaluator::execClassStmt(const ast::ClassStmtPtr &stmt) {
    m_envMgr.define(stmt->className, std::format("<class {}>", stmt->className.lexeme));
    std::vector<std::pair<std::string, SpicyObj>> methods;

    const auto hasSuperClass = stmt->superClass.has_value();
    auto superClass = [&]() -> std::optional<SpicyClassSharedPtr> {
        if (hasSuperClass) {
            auto superClassObj = evalExpr(stmt->superClass.value());
            if (!std::holds_alternative<SpicyClassSharedPtr>(superClassObj))
                throw RuntimeError(stmt->className, "Superclass must be a class; cannot inherit from a non-class.");
            return std::get<SpicyClassSharedPtr>(superClassObj);
        }
        return std::nullopt;
    }();

    if (hasSuperClass) {
        m_envMgr.createNewEnvironment("execClassStmt");
        m_envMgr.define("super", superClass.value());
    }

    for (const auto& method : stmt->methods) {
        methods.emplace_back(
            method->funcName.lexeme,
            std::make_shared<FuncObj>(
                method->funcExpr,
                method->funcName.lexeme,
                m_envMgr.getCurrentEnvironment(),
                true,
                method->funcName.lexeme == "init")
        );
    }

    auto class_ = std::make_shared<SpicyClass>(
                    stmt->className.lexeme,
                    std::move(superClass),
                    std::move(methods)
                );

    if (hasSuperClass)
        m_envMgr.setCurrentEnvironment(
                    m_envMgr.getCurrentEnvironment()->getParent(),
                    "execClassStmt");

    m_envMgr.assign(stmt->className, std::move(class_));
    return std::nullopt;
}

SpicyObj SpicyEvaluator::lookUpVariable(Token name, uint64_t exprAddr) {
    if (const auto distance = m_locals.find(exprAddr);
        distance != m_locals.end())
        return m_envMgr.get(distance->second, name);
    else
        return m_envMgr.getGlobal(name);
}

FuncSharedPtr SpicyEvaluator::bindInstance(const FuncSharedPtr &method, SpicyInstanceSharedPtr instance) {
    const auto envToRestore = m_envMgr.getCurrentEnvironment();
    m_envMgr.setCurrentEnvironment(method->getClosure(), method->getFuncName());
    m_envMgr.createNewEnvironment(method->getFuncName());
    const auto methodClosure = m_envMgr.getCurrentEnvironment();
    m_envMgr.define("this", instance);
    m_envMgr.setCurrentEnvironment(envToRestore, method->getFuncName());
    return std::make_shared<FuncObj>(
                method->getDecl(),
                method->getFuncName(),
                methodClosure,
                method->isMethod(),
                method->isInit()
                );
}

SpicyObj SpicyEvaluator::evalBuiltInCall(const BuiltinFuncSharedPtr &builtin, const ast::CallExprPtr &expr) {
    if (builtin->arity() != expr->arguments.size())
        throw RuntimeError(expr->paren, std::format("Expected {} args but got {}.", builtin->arity(), expr->arguments.size()));

    std::vector<SpicyObj> args;
    for (const auto& arg : expr->arguments)
        args.emplace_back(evalExpr(arg));

    builtin->setArgs(args);
    return builtin->run();
}

} // namespace spicy::eval
