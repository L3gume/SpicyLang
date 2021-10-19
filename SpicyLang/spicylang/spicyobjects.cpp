#include "spicyobjects.h"

namespace spicy {

// ======================== FuncObj ================================
FuncObj::FuncObj(const ast::FuncExprPtr &decl, const std::string &funcName, std::shared_ptr<eval::Environment> closure, bool isMethod, bool isInit)
    : m_decl(decl), m_funcName(funcName), m_closure(closure), m_isMethod(isMethod), m_isInit(isInit) {}

size_t FuncObj::arity() const {
    return m_decl->parameters.size();
}

std::shared_ptr<eval::Environment> FuncObj::getClosure() const {
    return m_closure;
}

const ast::FuncExprPtr &FuncObj::getDecl() const {
    return m_decl;
}

std::vector<ast::StmtPtrVariant> &FuncObj::getBodyStmts() const {
    return m_decl->body;
}

const std::string &FuncObj::getFuncName() const {
    return m_funcName;
}

bool FuncObj::isMethod() const {
    return m_isMethod;
}

bool FuncObj::isInit() const {
    return m_isInit;
}

const std::vector<Token> &FuncObj::getParams() const {
    return m_decl->parameters;
}

// ======================== BuiltinFunc ================================
BuiltinFunc::BuiltinFunc(const std::string &funcName, std::shared_ptr<eval::Environment> closure)
    : m_funcName(funcName), m_closure(closure) {}

void BuiltinFunc::setArgs(const std::vector<SpicyObj> &args) {
    m_args = args;
}

// ======================== SpicyClass ================================
SpicyClass::SpicyClass(const std::string &name, std::optional<SpicyClassSharedPtr> superClass, const std::vector<std::pair<std::string, SpicyObj> > &methods)
    : m_className(name), m_superClass(superClass) {
    for (const auto& [methodName, methodObj] : methods) {
        m_methods.insert_or_assign(m_hasher(methodName), methodObj);
    }
}

std::string SpicyClass::getClassName() const {
    return m_className;
}

std::optional<SpicyClassSharedPtr> SpicyClass::getSuperClass() const {
    return m_superClass;
}

std::optional<SpicyObj> SpicyClass::findMethod(const std::string &methodName) const {
    if (auto iter = m_methods.find(m_hasher(methodName));
        iter != m_methods.end()) {
        return iter->second;
    }

    if (m_superClass.has_value()) {
        return m_superClass.value()->findMethod(methodName);
    }

    return std::nullopt;
}

// ======================== SpicyInstance ================================
SpicyInstance::SpicyInstance(SpicyClassSharedPtr class_) : m_class(class_) {}

std::string SpicyInstance::toString() const {
    return "Instance of " + m_class->getClassName();
}

SpicyObj SpicyInstance::get(const Token &fieldName) const {
    if (auto iter = m_fields.find(m_hasher(fieldName.lexeme));
        iter != m_fields.end()) {
        return iter->second;
    }
    const auto& method = m_class->findMethod(fieldName.lexeme);
    if (method.has_value()) return method.value();

    throw RuntimeError(fieldName, "Undefined property '" + fieldName.lexeme + "'."); // TODO: actual exception type for this
}

SpicyObj SpicyInstance::get(const std::string &fieldName) const {
    if (auto iter = m_fields.find(m_hasher(fieldName));
        iter != m_fields.end()) {
        return iter->second;
    }
    const auto& method = m_class->findMethod(fieldName);
    if (method.has_value()) return method.value();
    throw std::exception{}; // TODO
}

void SpicyInstance::set(const Token &fieldName, SpicyObj value) {
    m_fields.insert_or_assign(m_hasher(fieldName.lexeme), std::move(value));
}

bool areEqual(const SpicyObj &lhs, const SpicyObj &rhs) {
    // if the indices aren't equal then they're obviously not equal
    if (lhs.index() == rhs.index()) {
        switch (lhs.index()) {
        case 0:
            return std::get<std::string>(lhs) == std::get<std::string>(rhs);
        case 1:
            return std::get<double>(lhs) == std::get<double>(rhs);
        case 2:
            return std::get<bool>(lhs) == std::get<bool>(rhs);
        case 3:
            return true;
        case 4:
            return std::get<FuncSharedPtr>(lhs)->getFuncName()
                    == std::get<FuncSharedPtr>(rhs)->getFuncName();
        case 5:
            return std::get<BuiltinFuncSharedPtr>(lhs)->getFuncName()
                    == std::get<BuiltinFuncSharedPtr>(rhs)->getFuncName();
        case 6:
            return std::get<SpicyClassSharedPtr>(lhs)->getClassName()
                    == std::get<SpicyClassSharedPtr>(rhs)->getClassName();
        case 7:
            return std::get<SpicyInstanceSharedPtr>(lhs).get()
                    == std::get<SpicyInstanceSharedPtr>(rhs).get();
        default:
        static_assert (std::variant_size_v<SpicyObj> == 8,
            "SpicyObj cases missing in areEqual()!");
        }
    }
    return false;
}

struct ObjectStringVisitor {
    std::string operator()(const std::string& str) { return str; }
    std::string operator()(const double& dbl) { return std::to_string(dbl); }
    std::string operator()(const bool& bol) { return bol ? "true" : "false"; }
    std::string operator()(const std::nullptr_t& nilptr) { return "nil"; }
    std::string operator()(const FuncSharedPtr& ptr) { return ptr->isMethod() ? "<method " + ptr->getFuncName() + ">" : "<fn " + ptr->getFuncName() + ">"; }
    std::string operator()(const BuiltinFuncSharedPtr& ptr) { return "<builtin " + ptr->getFuncName() + ">"; }
    std::string operator()(const SpicyClassSharedPtr& ptr) { return ptr->getClassName(); }
    std::string operator()(const SpicyInstanceSharedPtr& ptr) { return ptr->toString(); }
};

std::string getObjString(const SpicyObj &obj)
{
    return std::visit(ObjectStringVisitor{}, obj);
}

bool isTrue(const SpicyObj &obj)
{
    if (std::holds_alternative<std::nullptr_t>(obj)) return false;
    if (std::holds_alternative<bool>(obj)) return std::get<bool>(obj);
    if (std::holds_alternative<FuncSharedPtr>(obj))
        return std::get<FuncSharedPtr>(obj) == nullptr;
    if (std::holds_alternative<BuiltinFuncSharedPtr>(obj))
        return std::get<BuiltinFuncSharedPtr>(obj) == nullptr;
    if (std::holds_alternative<SpicyClassSharedPtr>(obj))
        return std::get<SpicyClassSharedPtr>(obj) == nullptr;
    if (std::holds_alternative<SpicyInstanceSharedPtr>(obj))
        return std::get<SpicyInstanceSharedPtr>(obj) == nullptr;
    return true;
}










} // namespace spicy
