// SPDX-License-Identifier: Apache-2.0
#include <reflection-cpp/reflection.hpp>

#include <catch2/catch_test_macros.hpp>

#include <string>
#include <string_view>
#include <utility>

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

enum Color : std::uint8_t
{
    Red,
    Green,
    Blue
};

struct SingleValueRecord
{
    int value;
};

TEST_CASE("GetName", "[reflection]")
{
    auto const enumValue = Reflection::GetName<Color::Red>();
    CHECK(enumValue == "Red");

    auto const enumValue2 = Reflection::GetName<Color::Green>();
    CHECK(enumValue2 == "Green");

    auto const memberName1 = Reflection::GetName<&Person::email>();
    CHECK(memberName1 == "email");

    auto const singleValueField = Reflection::GetName<&SingleValueRecord::value>();
    CHECK(singleValueField == "value");
}

TEST_CASE("single value record", "[reflection]")
{
    static_assert(Reflection::CountMembers<SingleValueRecord> == 1);

    auto const s = SingleValueRecord { 42 };
    auto const t = Reflection::ToTuple(s);

    CHECK(std::get<0>(t) == 42);
    CHECK(Reflection::GetMemberAt<0>(s) == 42);

    Reflection::CallOnMembers(s, [](auto&& name, auto&& value) {
        CHECK(name == "value");
        CHECK(value == 42);
    });
}

TEST_CASE("core", "[reflection]")
{
    auto s = SingleValueRecord { 42 };
    CHECK(Reflection::Inspect(s) == "value=42");

    auto p = Person { .name = "John Doe", .email = "john@doe.com", .age = 42 };
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
    auto ts = TestStruct {
        .a = 1,
        .b = 2.0f,
        .c = 3.0,
        .d = "hello",
        .e = { .name = "John Doe", .email = "john@doe.com", .age = 42 },
    };
    auto const result = Reflection::Inspect(ts);
    CHECK(result == R"(a=1 b=2 c=3 d="hello" e={name="John Doe" email="john@doe.com" age=42})");
}

TEST_CASE("EnumerateMembers.index_and_value", "[reflection]")
{
    auto ps = Person { .name = "John Doe", .email = "john@doe.com", .age = 42 };
    Reflection::EnumerateMembers(ps, []<size_t I>(auto&& value) {
        if constexpr (I == 0)
        {
            CHECK(value == "John Doe");
        }
        else if constexpr (I == 1)
        {
            CHECK(value == "john@doe.com");
        }
        else if constexpr (I == 2)
        {
            CHECK(value == 42);
        }
    });
}

TEST_CASE("EnumerateMembers.index_and_type", "[reflection]")
{
    Reflection::EnumerateMembers<Person>([]<auto I, typename T>() {
        if constexpr (I == 0)
        {
            static_assert(std::same_as<T, std::string_view>);
        }
        if constexpr (I == 1)
        {
            static_assert(std::same_as<T, std::string>);
        }
        if constexpr (I == 2)
        {
            static_assert(std::same_as<T, int>);
        }
    });
}

TEST_CASE("EnumerateMembers.partial", "[reflection]")
{
    Reflection::EnumerateMembers<std::integer_sequence<size_t, 0, 2>, Person>([]<auto I, typename T>() {
        if constexpr (I == 0)
        {
            static_assert(std::same_as<T, std::string_view>);
        }
        if constexpr (I == 1)
        {
            static_assert(false);
        }
        if constexpr (I == 2)
        {
            static_assert(std::same_as<T, int>);
        }
    });
}


TEST_CASE("CallOnMembers", "[reflection]")
{
    auto ps = Person { .name = "John Doe", .email = "john@doe.com", .age = 42 };
    std::string result;
    Reflection::CallOnMembers(ps, [&result](auto&& name, auto&& value) {
        result += name;
        result += "=";
        result += std::format("{}", value);
        result += " ";
    });
    CHECK(result == R"(name=John Doe email=john@doe.com age=42 )");
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
    auto const s = S { .a = 1, .b = 2, .c = 3 };
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

TEST_CASE("Compare.simple_with_indexing", "[reflection]")
{
    auto const r1 = Record { .id = 1, .name = "John Doe", .age = 42 };
    auto const r2 = Record { .id = 2, .name = "John Doe", .age = 42 };

    auto check = static_cast<size_t>(-1);
    auto differenceCallback = [&](size_t index, auto const& /*lhs*/, auto const& /*rhs*/) {
        check = index;
    };

    Reflection::CollectDifferences(r1, r2, differenceCallback);
    CHECK(check == 0);
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

TEST_CASE("TemplateFor over sequence", "[refleciton]")
{
    std::string result {};
    Reflection::template_for<std::integer_sequence<size_t, 3, 2, 1>>([&]<size_t I>(){result += std::to_string(I);});
    CHECK(result == "321");
}
