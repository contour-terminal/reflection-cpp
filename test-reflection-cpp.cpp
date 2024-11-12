// SPDX-License-Identifier: Apache-2.0
#include <reflection-cpp/reflection.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <string>
#include <string_view>

struct Person
{
    std::string_view name;
    std::string email;
    int age;
};

struct TestStruct
{
    int a;
    float b;
    double c;
    std::string d;
    Person e;
};

enum Color
{
    Red,
    Green,
    Blue
};

TEST_CASE("GetName", "[reflection]")
{
    auto const enumValue = Reflection::GetName<Color::Red>();
    CHECK(enumValue == "Red");

    auto const enumValue2 = Reflection::GetName<Color::Green>();
    CHECK(enumValue2 == "Green");

    auto const person = Person { "John Doe", "john@doe.com", 42 };
    auto const memberName1 = Reflection::GetName<&Person::email>();
    CHECK(memberName1 == "email");
}

TEST_CASE("core", "[reflection]")
{
    auto p = Person { "John Doe", "john@doe.com", 42 };
    auto const result = Reflection::Inspect(p);
    CHECK(result == R"(name="John Doe" email="john@doe.com" age=42)");
}

TEST_CASE("vector", "[reflection]")
{
    auto v = std::vector<Person> {};
    v.emplace_back("John Doe", "john@doe.com", 42);
    v.emplace_back("John Doe", "john@doe.com", 43);
    auto const result = Reflection::Inspect(v);
    CHECK(result == R"(name="John Doe" email="john@doe.com" age=42
name="John Doe" email="john@doe.com" age=43
)");
}

TEST_CASE("nested", "[reflection]")
{
    auto ts = TestStruct { 1, 2.0f, 3.0, "hello", { "John Doe", "john@doe.com", 42 } };
    auto const result = Reflection::Inspect(ts);
    CHECK(result == R"(a=1 b=2 c=3 d="hello" e={name="John Doe" email="john@doe.com" age=42})");
}

TEST_CASE("FoldMembers.type", "[reflection]")
{
    // clang-format off
    auto const result = Reflection::FoldMembers<TestStruct>(size_t{0}, []<size_t I, typename T>(auto&& result) {
        return result + I;
    });
    // clang-format on
    CHECK(result == 0 + 0 + 1 + 2 + 3 + 4);
}

struct S
{
    int a {};
    int b {};
    int c {};
};

TEST_CASE("FoldMembers.value", "[reflection]")
{
    auto const s = S { 1, 2, 3 };
    auto const result = Reflection::FoldMembers(
        s, 0, [](auto&& /*name*/, auto&& memberValue, auto&& accum) { return accum + memberValue; });

    CHECK(result == 6);
}

TEST_CASE("MemberTypeOf", "[reflection]")
{
    static_assert(std::same_as<Reflection::MemberTypeOf<0, TestStruct>, int>);
    static_assert(std::same_as<Reflection::MemberTypeOf<1, TestStruct>, float>);
    static_assert(std::same_as<Reflection::MemberTypeOf<2, TestStruct>, double>);
    static_assert(std::same_as<Reflection::MemberTypeOf<3, TestStruct>, std::string>);
    static_assert(std::same_as<Reflection::MemberTypeOf<4, TestStruct>, Person>);
}

struct Record
{
    int id;
    std::string name;
    int age;
};

TEST_CASE("Compare.simple", "[reflection]")
{
    auto const r1 = Record { .id = 1, .name = "John Doe", .age = 42 };
    auto const r2 = Record { .id = 1, .name = "John Doe", .age = 42 };
    auto const r3 = Record { .id = 2, .name = "Jane Doe", .age = 43 };

    std::string diff;
    auto differenceCallback = [&](std::string_view name, auto const& lhs, auto const& rhs) {
        diff += std::format("{}: {} != {}\n", name, lhs, rhs);
    };

    Reflection::CollectDifferences(r1, r2, differenceCallback);
    CHECK(diff.empty());
    Reflection::CollectDifferences(r1, r3, differenceCallback);
    CHECK(diff == "id: 1 != 2\nname: John Doe != Jane Doe\nage: 42 != 43\n");
}

struct Table
{
    Record first;
    Record second;
};

TEST_CASE("Compare.nested", "[reflection]")
{
    auto const t1 = Table { .first = { .id = 1, .name = "John Doe", .age = 42 },
                            .second = { .id = 2, .name = "Jane Doe", .age = 43 } };
    auto const t2 = Table { .first = { .id = 1, .name = "John Doe", .age = 42 },
                            .second = { .id = 2, .name = "Jane Doe", .age = 43 } };
    auto const t3 = Table { .first = { .id = 1, .name = "John Doe", .age = 42 },
                            .second = { .id = 3, .name = "Jane Doe", .age = 43 } };

    std::string diff;
    auto differenceCallback = [&](std::string_view name, auto const& lhs, auto const& rhs) {
        diff += std::format("{}: {} != {}\n", name, lhs, rhs);
    };

    Reflection::CollectDifferences(t1, t2, differenceCallback);
    CHECK(diff.empty());
    Reflection::CollectDifferences(t1, t3, differenceCallback);
    CHECK(diff == "id: 2 != 3\n");
}
