#pragma once
#ifndef H_SPICYVM
#define H_SPICYVM

#include <vector>
#include <tuple>
#include <unordered_map>
#include "vmtypes.h"
#include "spicy.h"

namespace spicy {

template<typename T>
using Stack = std::vector<T>;
    
class SpicyVM {
    Stack<Stack<SpicyObj>> stack;
    std::unordered_map<std::string, SpicyObj> globals;
    
    bool trace_execution;
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
    SpicyObj& peek(int distance);
    
    void printStack();
    void runtimeError(const std::string& msg, const Chunk& chunk);
    
};


}// namespace spicy

#endif // H_SPICYVM
