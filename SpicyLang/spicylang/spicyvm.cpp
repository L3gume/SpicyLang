#include "spicyvm.h"
#include "spicy.h"

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
        
        auto binary = [&](auto op) {
            if (!std::holds_alternative<double>(peek(0)) ||
                !std::holds_alternative<double>(peek(1))) {
                runtimeError("Operands must be numbers.", chunk);
                return false;
            }
            auto&& b = pop();
            auto&& a = pop();
            push(op(std::get<double>(a), std::get<double>(b)));
            return true;
        };
        
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
            case Chunk::OpCode::OP_NIL:
                push(nullptr);
                break;
            case Chunk::OpCode::OP_TRUE:
                push(true);
                break;
            case Chunk::OpCode::OP_FALSE:
                push(false);
                break;
            case Chunk::OpCode::OP_NEGATE: {
                if (!std::holds_alternative<double>(peek(0))) {
                    runtimeError("Operand must be a number.", chunk);
                    return;
                }
                push(-std::get<double>(pop()));
                break;
            }
            case Chunk::OpCode::OP_NOT:
                push(!isTrue(pop()));
                break;
            case Chunk::OpCode::OP_POP:
                pop();
                break;
            case Chunk::OpCode::OP_DEFINE_GLOBAL: {
                const auto& constants = chunk.getConstants();
                const auto& name = std::get<std::string>(constants[readByte(chunk)]);
                const auto val = peek(0);
                globals.insert({ name, val });
                pop();
                break;
            }
            case Chunk::OpCode::OP_GET_GLOBAL: {
                const auto& constants = chunk.getConstants();
                const auto& name = std::get<std::string>(constants[readByte(chunk)]);
                if (!globals.contains(name)) {
                    runtimeError(std::format("Undefined variable {}.", name), chunk);
                    return;
                }
                push(globals[name]);
                break;
            }
            case Chunk::OpCode::OP_EQUAL: {
                auto&& b = pop();
                auto&& a = pop();
                push(areEqual(a, b));
                break;
            }
            case Chunk::OpCode::OP_GREATER:
                if (!binary([](double a, double b) { return a > b; })) return;
                break;
            case Chunk::OpCode::OP_LESS:
                if (!binary([](double a, double b) { return a < b; })) return;
                break;
            case Chunk::OpCode::OP_ADD: {
                if (std::holds_alternative<std::string>(peek(0)) &&
                    std::holds_alternative<std::string>(peek(1))) {
                    auto&& b = pop();
                    auto&& a = pop();
                    push(std::move(std::get<std::string>(a) + std::get<std::string>(b)));
                } else if (std::holds_alternative<double>(peek(0)) &&
                    std::holds_alternative<double>(peek(1))) {
                    auto&& b = pop();
                    auto&& a = pop();
                    push(std::move(std::get<double>(a) + std::get<double>(b)));
                } else {
                    runtimeError("Operands must be either numbers or strings.", chunk);
                    return;
                }
                break;
            }
            case Chunk::OpCode::OP_SUBTRACT: 
                if (!binary([](double a, double b) { return a - b; })) return;
                break;
            case Chunk::OpCode::OP_MULTIPLY:
                if (!binary([](double a, double b) { return a * b; })) return;
                break;
            case Chunk::OpCode::OP_DIVIDE:
                if (!binary([](double a, double b) { return a / b; })) return;
                break;
            case Chunk::OpCode::OP_PRINT:
                std::cout << '\n' << getObjString(pop()) << '\n';
                break;
            case Chunk::OpCode::OP_RETURN:
                //std::cout << '\n' << getObjString(pop()) << '\n';
                return;
            }
        }
    }
    
    void SpicyVM::reset() {
        current_stack = 0l;
        program_counter = 0l;
        stack.clear();
        stack.emplace_back(Stack<SpicyObj>{});
        globals.clear();
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

    SpicyObj& SpicyVM::peek(int distance) {
        const auto top = stack[current_stack].size() - 1;
        return stack[current_stack][top - distance];
    }
    
    void SpicyVM::printStack() {
        std::cout << "Stack" << '\t';
        for (const auto& value : stack[current_stack]) {
            std::cout << std::format("[ {} ]", getObjString(value));
        }
        std::cout << '\n';
    }
    
    void SpicyVM::runtimeError(const std::string& msg, const Chunk& chunk) {
        error(chunk.getLine(program_counter - 1), msg);
        reset();
    }
}
