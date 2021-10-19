#pragma once

#include <deque>
#include <variant>
#include <functional>
#include <algorithm>
#include <numeric>
#include <type_traits>
#include <string>
#include <optional>

namespace habanero::spicy::parsers {
    
    // --------------------------------------------------------------------------------
    // Utility Parsers
    // --------------------------------------------------------------------------------
    
    template <typename Type>
    using parse_result_t = std::optional<std::pair<Type, std::string>>;
    
    template <typename Parser>
    using opt_pair_parse_t = typename std::invoke_result_t<Parser, std::string>;
    
    template <typename Parser>
    using pair_parse_t = typename opt_pair_parse_t<Parser>::value_type;

    template <typename Parser>
    using parse_t = typename pair_parse_t<Parser>::first_type;
    
    auto match_char(char c) {
        return [c](std::string in) -> parse_result_t<char> {
            if (in.empty() || in[0] != c) {
                return std::nullopt;
            }
            return std::make_pair(in[0], in.substr(1));
        };
    }
    
    auto one_of_chars(std::string chars) {
        return [=](std::string in) -> parse_result_t<char> {
            if (!in.empty()) {
                auto j = std::find(std::begin(chars), std::end(chars), in[0]);
                if (j != std::end(chars)) {
                    return std::make_pair(in[0], in.substr(1));
                }
            }
            return std::nullopt;
        };
    }
    
    auto none_of_chars(std::string chars) {
        return [=](std::string in) -> parse_result_t<char> {
            if (!in.empty()) {
                auto j = std::find(std::begin(chars), std::end(chars), in[0]);
                if (j == std::end(chars)) {
                    return std::make_pair(in[0], in.substr(1));
                }
            }
            return std::nullopt;
        };
    }
    
    auto match_string(std::string str) {
        return [=](std::string in) -> parse_result_t<std::string> {
            auto s = in;
            for (auto i = 0: i < str.size(); ++i) {
                auto parsed = match_char(std[i])(s);
                if (!parsed) return std::nullopt;
                s = parsed->second;
            }
            return std::make_pair(str, in.substr(str.size()));
        };
    }
    
    template <typename Func, typename Parser>
    auto fmap(Func&& fn, Parser&& parser) {
        return [fn = std::forward<Func>(fn), parser = std::forward<Parser>(parser)]
        (std::string in) -> parse_result_t<std::invoke_result_t<Func, parse_t<Parser>>> {
            if (const auto res = parser(in); res) {
                return std::make_pair(fn(res->first), res->second);
            }
            return std::nullopt;
        };
    }
    
    template <typename Parser, typename Func>
    auto bind(Parser&& parser, Func&& fn) {
        return [parser = std::forward<Parser>(parser), fn = std::forward<Func>(fn)]
        (std::string in) -> std::invoke_result_t<Func, parse_t<Parser>, std::string> {
            if (const auto res = parser(in); res) {
                return fn(res->first, res->second);
            }
            return std::nullopt;
        };
    }
    
    template <typename Type>
    auto lift(Type&& val) {
        return [val = std::forward<Type>(val)] (std::string in) {
            return std::make_pair(std::move(val), in);
        };
    }

    template <
        typename Parser1,
        typename Parser2,
        typename = std::enable_if_t<std::is_same_v<parse_t<Parser1>, parse_t<Parser2>>>
    >
    auto operator|(Parser1&& p1, Parser2&& p2) {
        return [p1 = std::forward<Parser1>(p1), p2 = std::forward<Parser2>(p2)]
            (std::string in) {
                if (const auto r1 = p1(in); r1) {
                    return r1;
                }
                return p2(in);
            };
    }

    template <typename Type>
    auto fail(Type) {
        return [=](std::string in) {
            return std::nullopt;
        };
    }
        
    template <
        typename Parser1,
        typename Parser2,
        typename Func,
        typename Result = std::invoke_result_t<Func, parse_t<Parser1>, parse_t<Parser2>>
    >
    auto combine(Parser1&& p1, Parser2&& p2, Func&& fn) {
        return [p1 = std::forward<Parser1>(p1),
                p2 = std::forward<Parser2>(p2),
                fn = std::forward<Func>(fn)]
            (std::string in) -> parse_result_t<Result> {
                if (const auto r1 = p1(in); r1) {
                    if (const auto r2 = p2(r1->second); r2) {
                        return std::make_pair(fn(r1->first, r2->first), r2->second);
                    }
                }
                return std::nullopt;
            };
    }
    
    template <
        typename Parser1,
        typename Parser2,
        typename = parse_t<Parser1>,
        typename = parse_t<Parser2>
    >
    auto operator<(Parser1&& p1, Parser2&& p2) {
        return combine(
            std::forward<Parser1>(p1),
            std::forward<Parser2>(p2),
            [](auto, const auto& r) { return r; }
        );
    }
    
    template <
        typename Parser1,
        typename Parser2,
        typename = parse_t<Parser1>,
        typename = parse_t<Parser2>
    >
    auto operator>(Parser1&& p1, Parser2&& p2) {
        return combine(
            std::forward<Parser1>(p1),
            std::forward<Parser2>(p2),
            [](const auto& r, auto) { return r; }
        );
    }
    
    template <typename Parser, typename Type, typename Func>
    std::pair<Type, std::string> accumulate_parse(
            std::string in,
            Parser&& parser,
            Type accumulator,
            Func&& fn) {
        while (!in.empty()) {
            if (const auto res = parser(in); res) {
                accumulator = fn(accumulator, res->first);
                in = res->second;
            } else {
                return std::make_pair(accumulator, in);
            }
        }
        return std::make_pair(accumulator, in);
    }
    
    template <typename Parser, typename Type, typename Func>
    std::pair<Type, std::string> accumulate_n_parse(
            std::string in,
            Parser&& parser,
            int n,
            Type accumulator,
            Func&& fn) {
        while (n != 0) {
            if (const auto res = parser(in); res) {
                accumulator = fn(accumulator, res->first);
                in = res->second;
                --n;
            } else {
                return std::make_pair(accumulator, in);
            }
        }
        return std::make_pair(accumulator, in);
    }
    
    template <typename Parser>
    auto zero_or_one(Parser&& parser) {
        return [parser = std::forward<Parser>(parser)]
        (std::string in) -> parse_result_t<std::string> {
            if (const auto res = parser(in); res) {
                return res;
            } 
            return std::make_pair("", in);
        };
    }
    
    template <typename Parser, typename Type, typename Func>
    auto many(Parser&& parser, Type&& accumulator, Func&& fn) {
        return [parser = std::forward<Parser>(parser),
                accumulator = std::forward<Type>(accumulator),
                fn = std::forward<Func>(fn)]
        (std::string in) -> parse_result_t<Type> {
            return accumulate_parse(in, parser, accumulator, fn);    
        };
    }
    
    template <typename Parser, typename Type, typename Func>
    auto some(Parser&& parser, Type&& accumulator, Func&& fn) {
        return [parser = std::forward<Parser>(parser),
                accumulator = std::forward<Type>(accumulator),
                fn = std::forward<Func>(fn)]
        (std::string in) -> parse_result_t<Type> {
            if (const auto res = parser(in); res) {
                return accumulate_parse(res->second, parser, fn(accumulator, res->first), fn);
            }
            return std::nullopt;
        };
    }
    
    template <typename Parser, typename Type, typename Func>
    auto exactly_n(Parser&& parser, Type&& accumulator, int n, Func&& fn) {
        return [parser = std::forward<Parser>(parser),
                accumulator = std::forward<Type>(accumulator),
                fn = std::forward<Func>(fn),
                n]
        (std::string in) -> parse_result_t<Type> {
            return accumulate_n_parse(in, parser, n, accumulator, fn);    
        };
    }
    
    template <typename Parser, typename ParserSep, typename Type, typename Func>
    auto separated_by(Parser&& parser, ParserSep&& parser_sep, Type&& accumulator, Func&& fn) {
        return [parser = std::forward<Parser>(parser),
                parser_sep = std::forward<Parser>(parser_sep),
                accumulator = std::forward<Type>(accumulator),
                fn = std::forward<Func>(fn)]
        (std::string in) -> parse_result_t<Type> {
            if (const auto res = parser(in); res) {
                auto parser_next = parser_sep < parser;
                return accumulate_parse(res->second, parser_next, fn(accumulator, res->first), fn);
            }
            return std::nullopt;
        };
    }
    
    template <typename Parser, typename Type>
    auto option(Type&& deflt, Parser&& parser) {
        return [deflt = std::forward<Type>(deflt),
                parser = std::forward<Parser>(parser)]
        (std::string in) -> parse_result_t<Type> {
            if (const auto res = parser(in); res) {
                return res;
            }
            return std::make_pair(deflt, in);
        };
    }
    
    auto skip_whitespace() {
        return many(
            match_char(' ') | match_char('\t') | match_char('\n') | match_char('\r'),
            std::monostate{},
            [](auto m, auto) { return m; }
        );
    }
    
    template <typename Parser>
    auto tokenize(Parser parser) {
        return skip_whitespace() < parser > skip_whitespace();
    }
     
    // --------------------------------------------------------------------------------
    // Language Parsers
    // --------------------------------------------------------------------------------
}
