#pragma once
#ifndef H_SPICYAST_PRINTER
#define H_SPICYAST_PRINTER

#include <string>
#include <vector>

#include "spicyast.h"

namespace spicy::ast::printer {
    [[nodiscard]] auto toString(const std::vector<StmtPtrVariant>& stmts) -> std::vector<std::string>;
    [[nodiscard]] auto toString(const ExprPtrVariant& expr) -> std::string;
    [[nodiscard]] auto toString(const StmtPtrVariant& stmt) -> std::vector<std::string>;
}

#endif
