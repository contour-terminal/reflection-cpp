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

TEST_CASE("FoldType", "[reflection]")
{
    // clang-format off
    auto const result = Reflection::FoldType<TestStruct>(0, [](auto&& /*name*/, auto&& /*value*/, auto&& result) {
        return result + 1;
    });
    // clang-format on
    CHECK(result == 5);
}

struct S
{
    int a {};
    int b {};
    int c {};
};

TEST_CASE("FoldMembers", "[reflection]")
{
    auto const s = S { 1, 2, 3 };
    auto const result = Reflection::FoldMembers(
        s, 0, [](auto&& /*name*/, auto&& memberValue, auto&& accum) { return accum + memberValue; });

    CHECK(result == 6);
}
