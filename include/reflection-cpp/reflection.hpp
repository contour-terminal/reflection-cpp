// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <algorithm>
#include <array>
#include <format>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if __has_include(<source_location>)
    #include <source_location>
    #define REFLECTION_PRETTY_FUNCTION (std::source_location::current().function_name())
#elif defined(__clang__) || defined(__GNUC__)
    #define REFLECTION_PRETTY_FUNCTION __PRETTY_FUNCTION__
#elif defined(_MSC_VER)
    #define REFLECTION_PRETTY_FUNCTION __FUNCSIG__
#endif

namespace Reflection
{

template <size_t N>
struct StringLiteral
{
    static constexpr size_t length = (N > 0) ? (N - 1) : 0;

    [[nodiscard]] constexpr size_t size() const noexcept
    {
        return length;
    }

    constexpr StringLiteral() noexcept = default;
    constexpr StringLiteral(StringLiteral const&) noexcept = default;
    constexpr StringLiteral(StringLiteral&&) noexcept = default;
    constexpr StringLiteral& operator=(StringLiteral const&) noexcept = default;
    constexpr StringLiteral& operator=(StringLiteral&&) noexcept = default;

    constexpr StringLiteral(char const (&str)[N]) noexcept
    {
        std::copy_n(str, N, value);
    }

    char value[N] {};

    [[nodiscard]] constexpr char const* begin() const noexcept
    {
        return value;
    }
    [[nodiscard]] constexpr char const* end() const noexcept
    {
        return value + length;
    }

    [[nodiscard]] constexpr auto operator<=>(const StringLiteral&) const = default;

    [[nodiscard]] constexpr std::string_view sv() const noexcept
    {
        return { value, length };
    }

    [[nodiscard]] constexpr operator std::string_view() const noexcept
    {
        return { value, length };
    }
};

namespace detail
{
    template <std::array V>
    struct make_static
    {
        static constexpr auto value = V;
    };

    template <const std::string_view&... Strs>
    constexpr std::string_view join()
    {
        constexpr auto joined_arr = []() {
            constexpr size_t len = (Strs.size() + ... + 0);
            std::array<char, len + 1> arr {};
            auto append = [i = 0u, &arr](const auto& s) mutable {
                for (auto c: s)
                    arr[i++] = c;
            };
            (append(Strs), ...);
            arr[len] = 0;
            return arr;
        }();
        auto& static_arr = make_static<joined_arr>::value;
        return { static_arr.data(), static_arr.size() - 1 };
    }
} // namespace detail

// Helper to get the value out
template <std::string_view const&... Strs>
inline constexpr auto JoinStringLiterals = detail::join<Strs...>();

namespace detail
{
    // This helper-struct is only used by CountMembers to count the number of members in an aggregate type
    struct AnyType final
    {
        template <class T>
        [[maybe_unused]] constexpr operator T() const;
    };

    template <auto Ptr>
    [[nodiscard]] consteval auto MangledName()
    {
        // return std::source_location::current().function_name();
        return REFLECTION_PRETTY_FUNCTION;
    }

    template <class T>
    [[nodiscard]] consteval auto MangledName()
    {
        // return std::source_location::current().function_name();
        return REFLECTION_PRETTY_FUNCTION;
    }

    template <class AggregateType, class... Args>
        requires(std::is_aggregate_v<AggregateType>)
    constexpr inline auto CountMembers = []() constexpr {
        // NOLINTNEXTLINE(modernize-use-designated-initializers)
        if constexpr (requires { AggregateType { Args {}..., AnyType {} }; })
            return CountMembers<AggregateType, Args..., AnyType>;
        else
            return sizeof...(Args);
    }();

} // namespace detail

// Count the number of members in an aggregate type.
template <class T>
    requires(std::is_aggregate_v<std::remove_cvref_t<T>>)
constexpr inline auto CountMembers = detail::CountMembers<std::remove_cvref_t<T>>;

constexpr size_t MaxReflectionMemerCount = 50; // should go as high as 256, I guess? for now 30 is enough

/**

elisp functions to fill the ToTuple function

(defun create_params (n)
       (let ((res "") (counter 0))
           (while (< counter n)
               (setq res (concat res "p" (number-to-string counter)))
               (if (< counter (- n 1))
                   (setq res (concat res ", "))
                )
               (setq counter (1+ counter)))
       res)
)

(defun for_one_value (n)
       (setq params (create_params n))
       (concat "    else if constexpr (N == " (number-to-string n) ")\n"
           "    {\n"
           "        auto&& [" params "] = std::forward<T>(t);\n"
           "        return std::tie(" params ");\n"
           "    }\n"
       )
)

(defun fill_ToTuple()
    (interactive)
  (let ((total 1))
    (while (<= total 50)
        (insert (for_one_value total))
      (setq total (+ total 1))
    total))
)

**/

template <class T, size_t N = CountMembers<T>>
    requires(N <= MaxReflectionMemerCount)
constexpr decltype(auto) ToTuple(T&& t) noexcept
{
    if constexpr (N == 0)
        return std::tuple {};
    // clang-format off
    else if constexpr (N == 1)
    {
        auto&& [p0] = std::forward<T>(t);
        return std::tie(p0);
    }
    else if constexpr (N == 2)
    {
        auto&& [p0, p1] = std::forward<T>(t);
        return std::tie(p0, p1);
    }
    else if constexpr (N == 3)
    {
        auto&& [p0, p1, p2] = std::forward<T>(t);
        return std::tie(p0, p1, p2);
    }
    else if constexpr (N == 4)
    {
        auto&& [p0, p1, p2, p3] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3);
    }
    else if constexpr (N == 5)
    {
        auto&& [p0, p1, p2, p3, p4] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4);
    }
    else if constexpr (N == 6)
    {
        auto&& [p0, p1, p2, p3, p4, p5] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5);
    }
    else if constexpr (N == 7)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6);
    }
    else if constexpr (N == 8)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7);
    }
    else if constexpr (N == 9)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8);
    }
    else if constexpr (N == 10)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9);
    }
    else if constexpr (N == 11)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
    }
    else if constexpr (N == 12)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
    }
    else if constexpr (N == 13)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
    }
    else if constexpr (N == 14)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
    }
    else if constexpr (N == 15)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
    }
    else if constexpr (N == 16)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
    }
    else if constexpr (N == 17)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
    }
    else if constexpr (N == 18)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17);
    }
    else if constexpr (N == 19)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
    }
    else if constexpr (N == 20)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19);
    }
    else if constexpr (N == 21)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20);
    }
    else if constexpr (N == 22)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21);
    }
    else if constexpr (N == 23)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22);
    }
    else if constexpr (N == 24)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23);
    }
    else if constexpr (N == 25)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24);
    }
    else if constexpr (N == 26)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25);
    }
    else if constexpr (N == 27)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26);
    }
    else if constexpr (N == 28)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27);
    }
    else if constexpr (N == 29)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28);
    }
    else if constexpr (N == 30)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29);
    }
    else if constexpr (N == 31)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30);
    }
    else if constexpr (N == 32)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31);
    }
    else if constexpr (N == 33)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32);
    }
    else if constexpr (N == 34)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33);
    }
    else if constexpr (N == 35)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34);
    }
    else if constexpr (N == 36)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35);
    }
    else if constexpr (N == 37)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36);
    }
    else if constexpr (N == 38)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37);
    }
    else if constexpr (N == 39)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38);
    }
    else if constexpr (N == 40)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39);
    }
    else if constexpr (N == 41)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40);
    }
    else if constexpr (N == 42)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41);
    }
    else if constexpr (N == 43)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42);
    }
    else if constexpr (N == 44)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43);
    }
    else if constexpr (N == 45)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44);
    }
    else if constexpr (N == 46)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45);
    }
    else if constexpr (N == 47)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46);
    }
    else if constexpr (N == 48)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47);
    }
    else if constexpr (N == 49)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48);
    }
    else if constexpr (N == 50)
    {
        auto&& [p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49] = std::forward<T>(t);
        return std::tie(p0, p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49);
    }
    // clang-format on
}

template <auto I, typename T>
constexpr decltype(auto) GetMemberAt(T&& t)
{
    return std::get<I>(ToTuple(std::forward<T>(t)));
}

/// Represents the type of the member at index I of type Object
template <auto I, typename Object>
using MemberTypeOf = std::remove_cvref_t<decltype(std::get<I>(ToTuple(Object {})))>;

template <class T>
struct WrappedPointer final
{
    T* pointer;
};

template <size_t N, class T>
constexpr auto GetElementPtrAt(T&& t) noexcept
{
    auto& p = GetMemberAt<N>(std::forward<T>(t));
    return WrappedPointer<std::remove_reference_t<decltype(p)>> { &p };
}

namespace detail
{
    template <class T>
    extern const T External;

#if defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Weverything"

    template <auto N, class T>
    constexpr std::string_view GetNameImpl = MangledName<GetElementPtrAt<N>(External<std::remove_volatile_t<T>>)>();

    #pragma clang diagnostic pop
#elif __GNUC__
    template <auto N, class T>
    constexpr std::string_view GetNameImpl = MangledName<GetElementPtrAt<N>(External<std::remove_volatile_t<T>>)>();
#else
    template <auto N, class T>
    constexpr std::string_view GetNameImpl = MangledName<GetElementPtrAt<N>(External<std::remove_volatile_t<T>>)>();
#endif

    struct REFLE_REFLECTOR
    {
        int REFLE_FIELD;
    };

    struct reflect_field
    {
        static constexpr auto name = GetNameImpl<0, REFLE_REFLECTOR>;
        static constexpr auto end = name.substr(name.find("REFLE_FIELD") + sizeof("REFLE_FIELD") - 1);
        static constexpr auto begin = name[name.find("REFLE_FIELD") - 1];
    };

    struct reflect_type
    {
        static constexpr std::string_view name = MangledName<REFLE_REFLECTOR>();
        static constexpr auto end = name.substr(name.find("REFLE_REFLECTOR") + sizeof("REFLE_REFLECTOR") - 1);
#if defined(__GNUC__) || defined(__clang__)
        static constexpr auto begin = std::string_view { "T = " };
#else
        static constexpr auto begin = std::string_view { "Reflection::detail::MangledName<" };
#endif
    };

    template <auto N, class T>
    struct MemberNameOfImpl
    {
        static constexpr auto name = detail::GetNameImpl<N, T>;
        static constexpr auto begin = name.find(reflect_field::end);
        static constexpr auto tmp = name.substr(0, begin);
        static constexpr auto stripped = tmp.substr(tmp.find_last_of(reflect_field::begin) + 1);
        // Making static memory to stripped to help the compiler optimize away prettified function signature
        static constexpr std::string_view stripped_literal = JoinStringLiterals<stripped>;
    };
} // namespace detail

template <auto N, class T>
inline constexpr auto MemberNameOf = []() constexpr {
    return detail::MemberNameOfImpl<N, T>::stripped_literal;
}();

template <class T>
constexpr auto TypeNameOf = [] {
    constexpr std::string_view name = detail::MangledName<T>();
    constexpr auto begin = name.find(detail::reflect_type::end);
    constexpr auto tmp = name.substr(0, begin);
#if defined(__GNUC__) || defined(__clang__)
    return tmp.substr(tmp.rfind(detail::reflect_type::begin) + detail::reflect_type::begin.size());
#else
    constexpr auto name_with_keyword =
        tmp.substr(tmp.rfind(detail::reflect_type::begin) + detail::reflect_type::begin.size());
    return name_with_keyword.substr(name_with_keyword.find(' ') + 1);
#endif
}();

namespace detail
{
    template <typename T>
    struct MemberClassTypeHelper;

    template <typename M, typename T>
    struct MemberClassTypeHelper<M T::*>
    {
        using type = std::remove_cvref_t<T>;
    };
} // namespace detail

/// Represents the class type of a member pointer
///
/// @tparam P The member pointer, e.g. &MyClass::member
template <auto P>
using MemberClassType = typename detail::MemberClassTypeHelper<decltype(P)>::type;

namespace detail
{
    template <class T, size_t... I>
    [[nodiscard]] constexpr auto MemberNamesImpl(std::index_sequence<I...>)
    {
        if constexpr (sizeof...(I) == 0)
        {
            return std::array<std::string_view, 0> {};
        }
        else
        {
            return std::array { MemberNameOf<I, T>... };
        }
    }
} // namespace detail

template <class T>
inline constexpr auto MemberNames = [] {
    return detail::MemberNamesImpl<T>(std::make_index_sequence<CountMembers<T>> {});
}();

namespace detail
{

    template <class T>
    struct remove_member_pointer
    {
        using type = T;
    };

    template <class C, class T>
    struct remove_member_pointer<T C::*>
    {
        using type = C;
    };

    template <class C, class R, class... Args>
    struct remove_member_pointer<R (C::*)(Args...)>
    {
        using type = C;
    };

    template <class T, auto P>
    consteval std::string_view get_name_msvc()
    {
        std::string_view str = REFLECTION_PRETTY_FUNCTION;
        str = str.substr(str.find("->") + 2);
        return str.substr(0, str.find('>'));
    }

    template <class T, auto P>
    consteval std::string_view func_name_msvc()
    {
        std::string_view str = REFLECTION_PRETTY_FUNCTION;
        str = str.substr(str.rfind(TypeNameOf<T>) + TypeNameOf<T>.size());
        str = str.substr(str.find("::") + 2);
        return str.substr(0, str.find('('));
    }

#if defined(__clang__)
    inline constexpr auto pretty_function_tail = "]";
#elif defined(__GNUC__) || defined(__GNUG__)
    inline constexpr auto pretty_function_tail = ";";
#elif defined(_MSC_VER)
#endif

    template <typename... Ts, typename F>
    constexpr void enumerate_types(F&& f)
    {
        [&f]<auto... Is>(std::index_sequence<Is...>) {
            (f.template operator()<Ts, Is>(), ...);
        }(std::index_sequence_for<Ts...> {});
    }

    template <auto... Xs, typename F>
    constexpr void for_values(F&& f)
    {
        (f.template operator()<Xs>(), ...);
    }

    template <typename T>
    constexpr bool can_be_formatted = std::is_arithmetic_v<T> || std::is_convertible_v<T, std::string>;

} // namespace detail

template <auto B, auto E, typename F>
constexpr void template_for(F&& f)
{
    using t = std::common_type_t<decltype(B), decltype(E)>;

    [&f]<auto... Xs>(std::integer_sequence<t, Xs...>) {
        detail::for_values<(B + Xs)...>(f);
    }(std::make_integer_sequence<t, E - B> {});
}

template <typename ElementMask, typename F>
constexpr void template_for(F&& f)
{
    using t = typename ElementMask::value_type;
    [&f]<auto... Xs>(std::integer_sequence<t, Xs...>) {
        Reflection::detail::for_values<(Xs)...>(f);
    }(ElementMask {});
}

namespace detail
{
    template <auto P>
        requires(std::is_member_pointer_v<decltype(P)>)
    consteval std::string_view GetName()
    {
#if defined(_MSC_VER) && !defined(__clang__)
        if constexpr (std::is_member_object_pointer_v<decltype(P)>)
        {
            using T = detail::remove_member_pointer<std::decay_t<decltype(P)>>::type;
            constexpr auto p = P;
            return detail::get_name_msvc<T, &(detail::External<T>.*p)>();
        }
        else
        {
            using T = detail::remove_member_pointer<std::decay_t<decltype(P)>>::type;
            return detail::func_name_msvc<T, P>();
        }
#else
        // TODO: Use std::source_location when deprecating clang 14
        // std::string_view str = std::source_location::current().function_name();
        std::string_view str = REFLECTION_PRETTY_FUNCTION;
        str = str.substr(str.find('&') + 1);
        str = str.substr(0, str.find(detail::pretty_function_tail));
        return str.substr(str.rfind("::") + 2);
#endif
    }

    template <auto E>
        requires(std::is_enum_v<decltype(E)>)
    consteval auto GetName()
    {
#if defined(_MSC_VER) && !defined(__clang__)
        std::string_view str = REFLECTION_PRETTY_FUNCTION;
        str = str.substr(str.rfind("::") + 2);
        str = str.substr(0, str.find('>'));
        return str.substr(str.find('<') + 1);
#else
        constexpr auto MarkerStart = std::string_view { "E = " };
        std::string_view str = REFLECTION_PRETTY_FUNCTION;
        str = str.substr(str.rfind(MarkerStart) + MarkerStart.size());
        str = str.substr(0, str.find(']'));
        return str;
#endif
    }
} // namespace detail

/// Gets the name of a member or function pointer
template <auto V>
constexpr std::string_view NameOf = detail::GetName<V>();

namespace detail
{
    // private helper for implementing MemberIndexOf<P>
    template <auto P, size_t... I>
    consteval size_t MemberIndexHelperImpl(std::index_sequence<I...>)
    {
        return (((NameOf<P> == MemberNames<MemberClassType<P>>[I]) ? I : 0) + ...);
    }
} // namespace detail

/// Gets the index of a member pointer in the member list of its class
///
/// @tparam P The member pointer, e.g. &MyClass::member
template <auto P>
constexpr size_t MemberIndexOf =
    detail::MemberIndexHelperImpl<P>(std::make_index_sequence<CountMembers<MemberClassType<P>>> {});

/// Calls a callable on members of an object specified with ElementMask sequence with the index of the member as the
/// first argument. and the member's default-constructed value as the second argument.
template <typename ElementMask, typename Object, typename Callable>
constexpr void EnumerateMembers(Object& object, Callable&& callable)
{
    template_for<ElementMask>([&]<auto I>() { callable.template operator()<I>(GetMemberAt<I>(object)); });
}

/// Calls a callable on members of an object specified with ElementMask sequence with the index and member's type as
/// template arguments.
template <typename ElementMask, typename Object, typename Callable>
constexpr void EnumerateMembers(Callable&& callable)
{
    template_for<ElementMask>([&]<auto I>() { callable.template operator()<I, MemberTypeOf<I, Object>>(); });
}

/// Calls a callable on each member of an object with the index of the member as the first argument.
/// and the member's default-constructed value as the second argument.
template <typename Object, typename Callable>
constexpr void EnumerateMembers(Object& object, Callable&& callable)
{
    template_for<0, CountMembers<Object>>([&]<auto I>() { callable.template operator()<I>(GetMemberAt<I>(object)); });
}

/// Calls a callable on each member of an object with the index and member's type as template arguments.
template <typename Object, typename Callable>
constexpr void EnumerateMembers(Callable&& callable)
{
    // clang-format off
    template_for<0, CountMembers<Object>>(
        [&]<auto I>() {
            callable.template operator()<I, MemberTypeOf<I, Object>>();
        }
    );
    // clang-format on
}

template <typename Object, typename Callable>
    requires std::same_as<void, std::invoke_result_t<Callable, std::string, MemberTypeOf<0, Object>>>
void CallOnMembers(Object& object, Callable&& callable)
{
    EnumerateMembers(object, [&]<size_t I, typename T>(T&& value) { callable(MemberNameOf<I, Object>, value); });
}

/// Folds over the members of a type without an object of it.
///
/// @param initialValue The initial value to fold with
/// @param callable     The callable to fold with. The parameters are the member name,
///                     the member's default value and the current result of the fold.
///
/// @return The result of the fold
template <typename Object, typename Callable, typename ResultType>
constexpr ResultType FoldMembers(ResultType initialValue, Callable const& callable)
{
    // clang-format off
    ResultType result = initialValue;
    EnumerateMembers<Object>(
        [&]<size_t I, typename MemberType>() {
            result = callable.template operator()<I, MemberTypeOf<I, Object>>(result);
        }
    );
    // clang-format on
    return result;
}

/// Folds over the members of an object
///
/// @param object       The object to fold over
/// @param initialValue The initial value to fold with
/// @param callable     The callable to fold with. The parameters are the member name,
///                     the member value and the current result of the fold.
///
/// @return The result of the fold
template <typename Object, typename Callable, typename ResultType>
    requires std::same_as<ResultType, std::invoke_result_t<Callable, std::string, MemberTypeOf<0, Object>, ResultType>>
constexpr ResultType FoldMembers(Object& object, ResultType initialValue, Callable const& callable)
{
    // clang-format off
    ResultType result = initialValue;
    EnumerateMembers(
        object,
        [&]<size_t I, typename MemberType>(MemberType&& value) {
            result = callable(MemberNameOf<I, Object>, value, result);
        }
    );
    return result;
    // clang-format on
}

template <typename Object>
std::string Inspect(Object const& object)
{
    std::string str;
    auto const onMember = [&str]<typename Name, typename Value>(Name&& name, Value&& value) {
        auto const InspectValue = [&str]<typename T>(T&& arg) {
            // clang-format off
            if constexpr (std::is_convertible_v<T, std::string>
                       || std::is_convertible_v<T, std::string_view>
                       || std::is_convertible_v<T, char const*>) // clang-format on
            {
                str += std::format("\"{}\"", arg);
            }
            else if constexpr (std::is_convertible_v<T, int>) // use std::formattable when available
            {
                str += std::format("{}", arg);
            }
            else
            {
                str += '{';
                str += Inspect(arg);
                str += '}';
            }
        };
        if (!str.empty())
            str += ' ';
        str += name;
        str += '=';
        InspectValue(value);
    };

    CallOnMembers(object, onMember);
    return str;
}

template <typename Object>
std::string Inspect(std::vector<Object> const& objects)
{
    std::string str;
    for (auto const& object: objects)
    {
        str += Inspect(object);
        str += '\n';
    }
    return str;
}

template <typename Object, typename Callback>
void CollectDifferences(const Object& lhs, const Object& rhs, Callback const& callback)
{
    template_for<0, CountMembers<Object>>([&]<auto I>() {
        if constexpr (std::equality_comparable<MemberTypeOf<I, Object>>)
        {
            if (GetMemberAt<I>(lhs) != GetMemberAt<I>(rhs))
            {
                callback(MemberNameOf<I, Object>, GetMemberAt<I>(lhs), GetMemberAt<I>(rhs));
            }
        }
        else
        {
            CollectDifferences(GetMemberAt<I>(lhs), GetMemberAt<I>(rhs), callback);
        }
    });
}

template <typename Object, typename Callback>
    requires std::same_as<void,
                          std::invoke_result_t<Callback, size_t, MemberTypeOf<0, Object>, MemberTypeOf<0, Object>>>
void CollectDifferences(const Object& lhs, const Object& rhs, Callback const& callback)
{
    template_for<0, CountMembers<Object>>([&]<auto I>() {
        if constexpr (std::equality_comparable<MemberTypeOf<I, Object>>)
        {
            if (GetMemberAt<I>(lhs) != GetMemberAt<I>(rhs))
            {
                callback(I, GetMemberAt<I>(lhs), GetMemberAt<I>(rhs));
            }
        }
        else
        {
            CollectDifferences(GetMemberAt<I>(lhs), GetMemberAt<I>(rhs), callback);
        }
    });
}

} // namespace Reflection

template <std::size_t N>
struct std::formatter<Reflection::StringLiteral<N>>: std::formatter<std::string_view>
{
    auto format(Reflection::StringLiteral<N> const& value, auto& ctx) const
    {
        return formatter<std::string_view>::format(value.sv(), ctx);
    }
};
