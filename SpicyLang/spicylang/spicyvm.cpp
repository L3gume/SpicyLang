#include "spicyvm.h"

#include <iostream>
#include <format>

namespace spicy {
    SpicyVM::SpicyVM(bool trace_execution) 
        : trace_execution(trace_execution) {}
    
    void SpicyVM::disassemble(const Chunk& chunk) {
        chunk.disassemble("TODO");
    }
    
    // TODO: return type for status?
    void SpicyVM::execute(const Chunk& chunk) {
        reset();
        
        while (program_counter < chunk.getBytecodeCount()) {
            if (trace_execution) {
                printStack();
                auto discarded = chunk.disassembleInstruction(program_counter);
            }
            switch (static_cast<Chunk::OpCode>(readByte(chunk))) {
            case Chunk::OpCode::OP_CONSTANT: {
                const auto& constants = chunk.getConstants();
                const auto& obj = constants[readByte(chunk)];
                push(obj);
                break;
            }
            case Chunk::OpCode::OP_RETURN: {
                std::cout << getObjString(pop()) << '\n';
                return;
            }
            }
        }
    }
    
    void SpicyVM::reset() {
        stack.clear();
        current_stack = 0l;
        program_counter = 0l;
    }
    
    uint8_t SpicyVM::readByte(const Chunk& chunk) {
		const auto& code = chunk.getBytecode();
        return code[program_counter++];
    }
    
    void SpicyVM::push(SpicyObj value) {
        stack[current_stack].emplace_back(std::move(value));
    }
    
    SpicyObj SpicyVM::pop() {
        const auto value = stack[current_stack].back();
        stack[current_stack].pop_back();
        return value;
    }
    
    std::string SpicyVM::printStack() {
        std::cout << '\t';
        for (const auto& value : stack[current_stack]) {
            std::cout << std::format("[ {} ]", getObjString(value));
        }
        std::cout << '\n';
    }
}
