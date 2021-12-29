#include "vmtypes.h"

#include <iostream>
#include <format>
#include <algorithm>

size_t spicy::Chunk::disassembleSimpleInstruction(const std::string& name, size_t offset) const noexcept {
    std::cout << name << '\n';
    return offset + simple_instruction_size;
}

size_t spicy::Chunk::disassembleConstantInstruction(const std::string& name, size_t offset) const noexcept {
    const auto constant = bytecode[offset + 1];
    std::cout << std::format("{} {:4d} '{}'\n", name, constant, getObjString(constants[constant]));
    return offset + constant_instruction_size;
}

size_t spicy::Chunk::disassembleByteInstruction(const std::string& name, size_t offset) const noexcept {
    const auto slot = bytecode[offset + 1];
    std::cout << std::format("{} {:4d}\n", name, slot);
    return offset + byte_instruction_size;
}

size_t spicy::Chunk::disassembleJumpInstruction(const std::string& name, int sign, size_t offset) const noexcept {
    const auto jump = static_cast<uint16_t>(bytecode[offset + 1] << 8) | bytecode[offset + 2];
    std::cout << std::format("{} {:4d} -> {}\n", name.c_str(), offset, offset + 3 + (sign * jump));
    return offset + jump_instruction_size;
}

size_t spicy::Chunk::disassembleInvokeInstruction(const std::string& name, size_t offset) const noexcept {
    const auto constant = bytecode[offset + 1];
    const auto argCount = bytecode[offset + 2];
    std::cout << std::format("{} ({} args) {:4d} '{}'\n", name, argCount, constant, getObjString(constants[constant]));
    return offset + invoke_instruction_size;
}

void spicy::Chunk::appendByte(uint8_t byte, int line) noexcept {
    bytecode.emplace_back(byte);
    // Do not add a new line if the previous line is the same (multiple intructions per line)
    if (!lines.empty() && lines.back().line == line) return;
    lines.emplace_back(line, bytecode.size() - 1);
}

void spicy::Chunk::disassemble(const std::string& name) const noexcept {
    std::cout << std::format("== {} ==\n", name);
    for (auto offset = 0ull; offset < bytecode.size();) {
        offset = disassembleInstruction(offset);
    }
}

void spicy::Chunk::setBytecodeValue(size_t offset, uint8_t byte) noexcept {
    bytecode[offset] = byte;
}

size_t spicy::Chunk::disassembleInstruction(size_t offset) const noexcept {
    const auto line = getLine(offset);
    std::cout << std::format("{:04d} ", offset);
    std::cout << std::format(offset > 0 && line == getLine(offset - 1) ? "\t| " : "{:4d} ", line);
    const auto instr = static_cast<OpCode>(bytecode[offset]);
    switch (instr) {
    case OpCode::OP_CONSTANT:
        return disassembleConstantInstruction("OP_CONSTANT", offset);
    case OpCode::OP_RETURN:
        return disassembleSimpleInstruction("OP_RETURN", offset);
    case OpCode::OP_ADD:    
        return disassembleSimpleInstruction("OP_ADD", offset);
    case OpCode::OP_SUBTRACT:    
        return disassembleSimpleInstruction("OP_SUBTRACT", offset);
    case OpCode::OP_MULTIPLY:    
        return disassembleSimpleInstruction("OP_MULTIPLY", offset);
    case OpCode::OP_DIVIDE:    
        return disassembleSimpleInstruction("OP_DIVIDE", offset);
    case OpCode::OP_NEGATE:    
        return disassembleSimpleInstruction("OP_NEGATE", offset);
    default:
        std::cout << std::format("Unknown opcode: {}\n", static_cast<uint8_t>(instr));
        return offset + simple_instruction_size;
    }
    // unreachable
    return offset;
}

size_t spicy::Chunk::addConstant(SpicyObj value) noexcept {
    constants.emplace_back(value);
    return constants.size() - 1;
}

uint32_t spicy::Chunk::getLine(size_t offset) const noexcept {
    auto start = 0;
    auto linesEnd = lines.size() - 1;
    auto end = linesEnd;
    while (true) {
        auto mid = (start + end) / 2;
        const auto& lineStart = lines[mid];
        if (offset < lineStart.offset) {
            end = mid - 1;
        } else if (mid == linesEnd || offset < lines[mid + 1].offset) {
            return lineStart.line;
        } else {
            start = mid + 1;
        }
    }
}

int spicy::Chunk::getBytecodeCount() const noexcept {
    return bytecode.size();
}

std::vector<uint8_t> spicy::Chunk::getBytecode() const noexcept {
    return bytecode;
}

std::vector<spicy::SpicyObj> spicy::Chunk::getConstants() const noexcept {
    return constants;
}
