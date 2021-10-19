#include "spicyenvironment.h"

#include "spicy.h"

namespace spicy::eval {

namespace {
class UndefinedVarAccess : public std::exception {};
class UninitializedVarAccess : public std::exception {};
}

/*
 * Environement
 */
Environment::Environment(EnvironmentPtr parent)
    : m_parent(std::move(parent)) {}

bool Environment::assign(size_t hashedVarName, SpicyObj object) {
    if (const auto& iter = m_objects.find(hashedVarName);
        iter != m_objects.end()) {
        m_objects.insert_or_assign(hashedVarName, object);
        return true;
    }
    if (m_parent != nullptr) {
        m_parent->assign(hashedVarName, std::move(object));
    }
    throw UndefinedVarAccess{};
}

void Environment::define(size_t hashedVarName, SpicyObj object) {
    m_objects.insert_or_assign(hashedVarName, object);
}

SpicyObj Environment::get(size_t hashedVarName) {
    if (const auto& iter = m_objects.find(hashedVarName);
        iter != m_objects.end()) {
        const auto& val = iter->second;
        if (std::holds_alternative<std::nullptr_t>(val))
            throw UninitializedVarAccess{};
        return val;
    }
    if (m_parent != nullptr)
        return m_parent->get(hashedVarName);
    throw UndefinedVarAccess{};
}

Environment::EnvironmentPtr Environment::getParent() {
    return m_parent;
}

bool Environment::isGlobal() {
    return m_parent != nullptr;
}

/*
 * EnvironmentMgr
 */
EnvironmentMgr::EnvironmentMgr()
    : m_current(std::make_shared<Environment>(nullptr)) {
    m_global = m_current;
    dbgPrint("EnvironmentMgr constructed! Global env = "
             + std::to_string(reinterpret_cast<uint64_t>(m_current.get())));
}

void EnvironmentMgr::assign(const Token &token, SpicyObj object) {
    try {
        m_current->assign(m_hasher(token.lexeme), std::move(object));
    }  catch (const UndefinedVarAccess& e) {
        throw RuntimeError(token, "Undefined variable.");
    }
}

void EnvironmentMgr::assignAt(uint32_t distance, const Token &token, SpicyObj object) {
    ancestor(distance)->assign(m_hasher(token.lexeme), std::move(object));
}

void EnvironmentMgr::assignGlobal(const Token &token, SpicyObj object) {
    assignGlobal(token.lexeme, object);
}

void EnvironmentMgr::assignGlobal(const std::string &name, SpicyObj object) {
    m_global->assign(m_hasher(name), std::move(object));
}

void EnvironmentMgr::createNewEnvironment(const std::string &caller) {
    m_current = std::make_shared<Environment>(m_current);
    dbgPrint(caller + " requested new environement: " + std::to_string(reinterpret_cast<uint64_t>(m_current.get())));
}

void EnvironmentMgr::discardEnvironmentsUntil(const Environment::EnvironmentPtr &toRestore, const std::string &caller) {
    dbgPrint("discarding environements until: "
             + std::to_string(reinterpret_cast<uint64_t>(toRestore.get()))
             + "\ncaller: " + caller
             + "\ncurrent env:"
             + std::to_string(reinterpret_cast<uint64_t>(m_current.get())));
    while (!m_current->isGlobal()
           && m_current.get() != toRestore.get()) {
        dbgPrint("discarding env: " + std::to_string(reinterpret_cast<uint64_t>(m_current.get())));
        m_current = m_current->getParent();
    }
}

void EnvironmentMgr::define(const std::string &tokenStr, SpicyObj object) {
    m_current->define(m_hasher(tokenStr), std::move(object));
}

void EnvironmentMgr::define(const Token &token, SpicyObj object) {
    m_current->define(m_hasher(token.lexeme), std::move(object));
}

void EnvironmentMgr::defineGlobal(const std::string &tokenStr, SpicyObj object) {
    m_global->define(m_hasher(tokenStr), std::move(object));
}

SpicyObj EnvironmentMgr::get(const Token &token) {
    try {
        return m_current->get(m_hasher(token.lexeme));
    } catch (const UndefinedVarAccess& ex) {
        throw RuntimeError(token, "Undefined variable.");
    } catch (const UninitializedVarAccess& ex) {
        throw RuntimeError(token, "Uninitialized variable.");
    }
}

SpicyObj EnvironmentMgr::get(uint32_t distance, const Token &token) {
    return ancestor(distance)->get(m_hasher(token.lexeme));
}

SpicyObj EnvironmentMgr::get(uint32_t distance, const std::string &token) {
    return ancestor(distance)->get(m_hasher(token));
}

SpicyObj EnvironmentMgr::getGlobal(const Token &token) {
    try {
        return m_global->get(m_hasher(token.lexeme));
    } catch (const UndefinedVarAccess& ex) {
        throw RuntimeError(token, "Undefined variable.");
    } catch (const UninitializedVarAccess& ex) {
        throw RuntimeError(token, "Uninitialized variable.");
    }
}

Environment::EnvironmentPtr EnvironmentMgr::getCurrentEnvironment() {
    return m_current;
}

void EnvironmentMgr::setCurrentEnvironment(Environment::EnvironmentPtr newCurrent, const std::string &caller) {
    dbgPrint(caller + " requested setting current env to: "
             + std::to_string(reinterpret_cast<uint64_t>(newCurrent.get()))
             + "\ncurrent env: "
             + std::to_string(reinterpret_cast<uint64_t>(m_current.get())));
    m_current = std::move(newCurrent);
}

Environment::EnvironmentPtr EnvironmentMgr::ancestor(uint32_t distance) {
    auto env = m_current;
    for (auto i = 0; i < distance; ++i) {
        env = env->getParent();
    }
    return env;
}

} // namespace spicy::eval
