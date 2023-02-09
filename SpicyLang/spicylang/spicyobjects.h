#pragma once
#ifndef H_SPICYOBJECTS
#define H_SPICYOBJECTS

#include <optional>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <deque>

#include "types.h"
#include "spicyast.h"
#include "spicyutil.h"

namespace spicy {

class FuncObj;
using FuncSharedPtr = std::shared_ptr<FuncObj>;

class BuiltinFunc;
using BuiltinFuncSharedPtr = std::shared_ptr<BuiltinFunc>;

class SpicyClass;
using SpicyClassSharedPtr = std::shared_ptr<SpicyClass>;

class SpicyInstance;
using SpicyInstanceSharedPtr = std::shared_ptr<SpicyInstance>;

class SpicyList;
using SpicyListSharedPtr = std::shared_ptr<SpicyList>;

using SpicyObj = std::variant<
    std::string, double, bool, std::nullptr_t,
    FuncSharedPtr, BuiltinFuncSharedPtr, SpicyClassSharedPtr,
    SpicyInstanceSharedPtr, SpicyListSharedPtr>;

using OptSpicyObj = std::optional<SpicyObj>;

[[nodiscard]]
auto areEqual(const SpicyObj& lhs, const SpicyObj& rhs) -> bool;

[[nodiscard]]
auto isTrue(const SpicyObj& obj) -> bool;

[[nodiscard]]
auto getObjString(const SpicyObj& obj) -> std::string;

namespace eval {
class Environment;
}

class Chunk;

class FuncObj : public util::Uncopyable {
    const ast::FuncExprPtr& m_decl;
    const std::string m_funcName;
    std::shared_ptr<eval::Environment> m_closure;
    bool m_isMethod;
    bool m_isInit;
    
    // TODO: for bytecode vm, improve this when moving bytecode gen to AST traversal instead of single pass Pratt parser
    std::shared_ptr<Chunk> m_chunk;

public:
    FuncObj(const ast::FuncExprPtr& decl,
            const std::string& funcName,
            std::shared_ptr<eval::Environment> closure,
            bool isMethod = false,
            bool isInit = false);
    
    [[nodiscard]] 
    auto arity()        const -> size_t;
    [[nodiscard]] 
    auto getClosure()   const -> std::shared_ptr<eval::Environment>;
    [[nodiscard]] 
    auto getDecl()      const -> const ast::FuncExprPtr&;
    [[nodiscard]] 
    auto getBodyStmts() const -> std::vector<ast::StmtPtrVariant>&;
    [[nodiscard]] 
    auto getFuncName()  const -> const std::string&;
    [[nodiscard]] 
    auto isMethod()     const -> bool;
    [[nodiscard]] 
    auto isInit()       const -> bool;
    [[nodiscard]] 
    auto getParams()    const -> const std::vector<Token>&;
};

class BuiltinFunc : public util::Uncopyable {
protected:
    std::string m_funcName = "";
    std::shared_ptr<eval::Environment> m_closure;
    std::vector<SpicyObj> m_args;

public:
    BuiltinFunc(const std::string& funcName, std::shared_ptr<eval::Environment> closure = nullptr);

    void setArgs(const std::vector<SpicyObj>& args);

    virtual size_t arity() = 0;
    virtual SpicyObj run() = 0;
    std::string getFuncName() {
        return m_funcName;
    }
};

class SpicyClass : public util::Uncopyable {
protected:
    const std::string m_className;
    std::optional<SpicyClassSharedPtr> m_superClass;
    std::hash<std::string> m_hasher;
    std::map<size_t, SpicyObj> m_methods;

public:
    SpicyClass(const std::string& name, std::optional<SpicyClassSharedPtr> superClass,
               const std::vector<std::pair<std::string, SpicyObj>>& methods);

    [[nodiscard]] std::string getClassName() const;
    [[nodiscard]] std::optional<SpicyClassSharedPtr> getSuperClass() const;
    [[nodiscard]] std::optional<SpicyObj> findMethod(const std::string& methodName) const;
};

class SpicyInstance : public util::Uncopyable {
    const SpicyClassSharedPtr m_class;
    std::hash<std::string> m_hasher;
    std::map<size_t, SpicyObj> m_fields;

public:
    explicit SpicyInstance(SpicyClassSharedPtr class_);

    [[nodiscard]] std::string toString() const;
    [[nodiscard]] SpicyObj get(const Token& fieldName) const;
    [[nodiscard]] SpicyObj get(const std::string& fieldName) const;
    void set(const Token& fieldName, SpicyObj value);
};

class SpicyList : public util::Uncopyable {
    std::deque<SpicyObj> m_list;
public:
    void append(const Token& lstName, SpicyObj val);
    void appendFront(const Token& lstName, SpicyObj val);
    SpicyObj get(const Token& lstName, int idx);
    SpicyObj set(const Token& lstName, int idx, SpicyObj val);
    SpicyObj back();
    SpicyObj front();
    SpicyObj size();
    
    std::string toString();
    
    friend bool operator==(const SpicyList& lhs, const SpicyList& rhs);
};

} // namespace spicy

#endif // SPICY_SPICYOBJECTS_H
