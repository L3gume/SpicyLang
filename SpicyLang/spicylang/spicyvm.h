#pragma once
#ifndef H_SPICYVM
#define H_SPICYVM

#include <vector>
#include <tuple>
#include "vmtypes.h"

namespace spicy {

template<typename T>
using Stack = std::vector<T>;
    
class SpicyVM {
    bool trace_execution;
    Stack<Stack<SpicyObj>> stack;
    unsigned long current_stack = 0l;
    unsigned long program_counter = 0l;
public:
    explicit SpicyVM(bool trace_execution);
    void disassemble(const Chunk& chunk);
    void execute(const Chunk& chunk);
private:
    void reset();
    [[nodiscard]] uint8_t readByte(const Chunk& chunk);
    
    void push(SpicyObj value);
    SpicyObj pop();
    
    std::string printStack();
};

}// namespace spicy

#endif // H_SPICYVM
