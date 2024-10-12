// SPDX-License-Identifier: Apache-2.0
#include <reflection-cpp/Reflection.hpp>

#include <catch2/catch_test_macros.hpp>

#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

struct Person
{
    std::string_view name;
    std::string email;
    int age;
};

// TODO: This could be a public API function that prints all object members to a string (in a single-line)
template <typename Object>
std::string inspect(Object const& object)
{
    auto const& tuple = Reflection::ToTuple(object);
    return [&]<size_t... I>(std::index_sequence<I...>) {
        std::stringstream str;
        ((str << "Member " << I << " (" << Reflection::MemberNameOf<I, Object> << "): " << std::get<I>(tuple) << '\n'),
         ...);
        return str.str();
    }(std::make_index_sequence<Reflection::CountMembers<Object>> {});
}

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
    std::cout << "Elements: " << Reflection::CountMembers<Person> << '\n';
    std::cout << inspect(p);

    // TODO: make this a real test
}
