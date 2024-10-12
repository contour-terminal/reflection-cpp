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

TEST_CASE("core", "[reflection]")
{
    auto p = Person { "John Doe", "jon@doe.com", 42 };
    std::cout << "Elements: " << Reflection::CountMembers<Person> << '\n';
    std::cout << inspect(p);

    // TODO: make this a real test
}
