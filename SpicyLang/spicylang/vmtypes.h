#pragma once
#ifndef H_VMTYPES
#define H_VMTYPES

#include <vector>
#include <utility>
#include <variant>

#include "spicyobjects.h"

namespace spicy {
    
template<class... Ts>
struct visitor : Ts... {
    using Ts::operator()...;
};

struct LineStart {
    uint32_t line;
    size_t offset;
};

class Chunk {
    static constexpr auto simple_instruction_size = 1ull;
    static constexpr auto constant_instruction_size = 2ull;
    static constexpr auto byte_instruction_size = 2ull;
    static constexpr auto jump_instruction_size = 3ull;
    static constexpr auto invoke_instruction_size = 3ull;
    
    std::vector<uint8_t> bytecode;
    std::vector<SpicyObj> constants;
    std::vector<LineStart> lines;
 
    [[nodiscard]] size_t disassembleSimpleInstruction(const std::string& name, size_t offset) const noexcept;
    [[nodiscard]] size_t disassembleConstantInstruction(const std::string& name, size_t offset) const noexcept;
    [[nodiscard]] size_t disassembleByteInstruction(const std::string& name, size_t offset) const noexcept;
    [[nodiscard]] size_t disassembleJumpInstruction(const std::string& name, int sign, size_t offset) const noexcept;
    [[nodiscard]] size_t disassembleInvokeInstruction(const std::string& name, size_t offset) const noexcept;
    
public:
    enum class OpCode {
        OP_CONSTANT,
        OP_NIL,
        OP_TRUE,
        OP_FALSE,
        OP_POP,
        OP_GET_LOCAL,
        OP_SET_LOCAL,
        OP_GET_GLOBAL,
        OP_DEFINE_GLOBAL,
        OP_SET_GLOBAL,
        OP_GET_UPVALUE,
        OP_SET_UPVALUE,
        OP_GET_PROPERTY,
        OP_SET_PROPERTY,
        OP_GET_SUPER,
        OP_EQUAL,
        OP_GREATER,
        OP_LESS,
        OP_ADD,
        OP_SUBTRACT,
        OP_MULTIPLY,
        OP_DIVIDE,
        OP_NOT,
        OP_NEGATE,
        OP_PRINT,
        OP_JUMP,
        OP_JUMP_IF_FALSE,
        OP_LOOP,
        OP_CALL,
        OP_INVOKE,
        OP_SUPER_INVOKE,
        OP_CLOSURE,
        OP_CLOSE_UPVALUE,
        OP_RETURN,
        OP_CLASS,
        OP_INHERIT,
        OP_METHOD
    };
    
    void appendByte(uint8_t byte, int line) noexcept;
    void disassemble(const std::string &name) const noexcept;
    void setBytecodeValue(size_t offset, uint8_t byte) noexcept;
    [[nodiscard]] size_t disassembleInstruction(size_t offset) const noexcept;
    [[nodiscard]] size_t addConstant(SpicyObj value) noexcept;

    [[nodiscard]] uint32_t getLine(size_t offset) const noexcept;
    [[nodiscard]] int getBytecodeCount() const noexcept;
    [[nodiscard]] std::vector<uint8_t> getBytecode() const noexcept;
    [[nodiscard]] std::vector<SpicyObj> getConstants() const noexcept;
};

enum class FuncType {
    FUNCTION,
    SCRIPT
};

struct Func {
    SpicyObj object = nullptr;
    int arity = 0;
    Chunk chunk = {};
    std::string name = "";
};

struct CallFrame {
    Func function;
    uint8_t instructionPtr;

};

}

#endif
