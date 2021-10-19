#pragma once
#ifndef H_SPICYENVIRONMENT
#define H_SPICYENVIRONMENT

#include <cstddef>
#include <exception>
#include <list>
#include <map>
#include <memory>
#include <optional>

#include "spicyutil.h"
#include "spicyobjects.h"
#include "types.h"

namespace spicy::eval {

class Environment : public util::Uncopyable,
                    std::enable_shared_from_this<Environment> {
public:
    using EnvironmentPtr = std::shared_ptr<Environment>;
    explicit Environment(EnvironmentPtr parent = nullptr);

    bool assign(size_t hashedVarName, SpicyObj object);
    void define(size_t hashedVarName, SpicyObj object);
    SpicyObj get(size_t hashedVarName);
    EnvironmentPtr getParent();
    bool isGlobal();
private:
    std::map<size_t, SpicyObj> m_objects;
    EnvironmentPtr m_parent = nullptr;
};

class EnvironmentMgr : public util::Uncopyable {
    Environment::EnvironmentPtr m_global;
    Environment::EnvironmentPtr m_current;
    std::hash<std::string> m_hasher;

public:
    EnvironmentMgr();

    void assign(const Token& token, SpicyObj object);
    void assignAt(uint32_t distance, const Token& token, SpicyObj object);
    void assignGlobal(const Token& token, SpicyObj object);
    void assignGlobal(const std::string& name, SpicyObj object);
    void createNewEnvironment(const std::string& caller);
    void discardEnvironmentsUntil(const Environment::EnvironmentPtr& toRestore,
                                  const std::string& caller);
    void define(const std::string& tokenStr, SpicyObj object);
    void define(const Token& token, SpicyObj object);
    void defineGlobal(const std::string& tokenStr, SpicyObj object);
    SpicyObj get(const Token& token);
    SpicyObj get(uint32_t distance, const Token& token);
    SpicyObj get(uint32_t distance, const std::string& token);
    SpicyObj getGlobal(const Token& token);
    Environment::EnvironmentPtr getCurrentEnvironment();
    void setCurrentEnvironment(Environment::EnvironmentPtr newCurrent,
                               const std::string& caller);

private:
    Environment::EnvironmentPtr ancestor(uint32_t distance);
};

} // namespace spicy::eval

#endif // H_SPICYENVIRONMENT
