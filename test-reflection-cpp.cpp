// SPDX-License-Identifier: Apache-2.0
#include <reflection-cpp/Reflection.hpp>

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
