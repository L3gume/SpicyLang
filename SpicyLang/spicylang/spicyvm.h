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
    Stack<Stack<SpicyObj>> stack;
    unsigned long program_counter = 0l;
public:
    void disassemble(const Chunk& chunk);
    void execute(const Chunk& chunk);
};

}// namespace spicy

#endif // H_SPICYVM
