#pragma once
#ifndef H_SPICYUTIL
#define H_SPICYUTIL

namespace spicy::util {
    
// thanks aakshintala on github :)    
struct Uncopyable {
    explicit Uncopyable() = default;
    virtual ~Uncopyable() = default;
    Uncopyable(const Uncopyable&) = delete;
    auto operator=(const Uncopyable&) -> Uncopyable& = delete;
    Uncopyable(Uncopyable&&) = delete;
    auto operator=(Uncopyable&&) -> Uncopyable& = delete;
};

}

#endif
