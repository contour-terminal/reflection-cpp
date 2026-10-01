// SPDX-License-Identifier: Apache-2.0
#include <reflection-cpp/reflection.hpp>

#include <catch2/catch_test_macros.hpp>

#include <concepts>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

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

struct NoDefaultCtor
{
    NoDefaultCtor() = delete;
    constexpr NoDefaultCtor(int) {}
};

struct WithReq
{
    int a;
    NoDefaultCtor b;
};

struct WithReq2
{
    int a;
    int _a;
    NoDefaultCtor b;
};

namespace
{
struct InternalRecord
{
    int first;
    std::string second;
};
} // namespace

TEST_CASE("internal linkage types", "[reflection]")
{
    static_assert(Reflection::CountMembers<InternalRecord> == 2);
    static_assert(Reflection::MemberNameOf<0, InternalRecord> == "first");
    static_assert(Reflection::MemberNameOf<1, InternalRecord> == "second");
    static_assert(Reflection::NameOf<&InternalRecord::second> == "second");
    static_assert(Reflection::MemberIndexOf<&InternalRecord::second> == 1);

    CHECK(Reflection::Inspect(InternalRecord { .first = 1, .second = "two" }) == R"(first=1 second="two")");
}

TEST_CASE("function local types", "[reflection]")
{
    struct LocalRecord
    {
        int alpha;
        std::string beta;
    };

    static_assert(Reflection::CountMembers<LocalRecord> == 2);
    static_assert(Reflection::MemberNameOf<0, LocalRecord> == "alpha");
    static_assert(Reflection::MemberNameOf<1, LocalRecord> == "beta");

    CHECK(Reflection::Inspect(LocalRecord { .alpha = 1, .beta = "two" }) == R"(alpha=1 beta="two")");
}

TEST_CASE("MemberIndex", "[reflection]")
{
    static_assert(Reflection::MemberIndexOf<&Person::name> == 0);
    static_assert(Reflection::MemberIndexOf<&Person::email> == 1);
    static_assert(Reflection::MemberIndexOf<&Person::age> == 2);
}

TEST_CASE("TypeNameOf", "[reflection]")
{
    CHECK(Reflection::TypeNameOf<int> == "int");
    CHECK(Reflection::TypeNameOf<Person> == "Person");
    CHECK(Reflection::TypeNameOf<std::optional<float>> == "std::optional<float>");
}

TEST_CASE("NameOf", "[reflection]")
{
    auto const enumValue = Reflection::NameOf<Color::Red>;
    CHECK(enumValue == "Red");

    auto const enumValue2 = Reflection::NameOf<Color::Green>;
    CHECK(enumValue2 == "Green");

    auto const memberName1 = Reflection::NameOf<&Person::email>;
    CHECK(memberName1 == "email");

    auto const singleValueField = Reflection::NameOf<&SingleValueRecord::value>;
    CHECK(singleValueField == "value");
}

struct NoDefaultCtorFirst
{
    NoDefaultCtor a;
    int b;
};

struct NoDefaultCtorMiddle
{
    int a;
    NoDefaultCtor b;
    int c;
    std::string d;
};

struct NoDefaultCtorOnly
{
    NoDefaultCtor a;
    NoDefaultCtor b;
};

TEST_CASE("CountMembers.NoDefaultCtor", "[reflection]")
{
    static_assert(Reflection::CountMembers<WithReq> == 2);
    static_assert(Reflection::CountMembers<WithReq2> == 3);
    static_assert(Reflection::CountMembers<NoDefaultCtorFirst> == 2);
    static_assert(Reflection::CountMembers<NoDefaultCtorMiddle> == 4);
    static_assert(Reflection::CountMembers<NoDefaultCtorOnly> == 2);

    auto record = NoDefaultCtorMiddle { .a = 1, .b = NoDefaultCtor { 2 }, .c = 3, .d = "four" };
    auto names = std::vector<std::string_view> {};
    Reflection::EnumerateMembers(
        record, [&]<size_t I, typename T>(T&&) { names.push_back(Reflection::MemberNameOf<I, NoDefaultCtorMiddle>); });
    CHECK(names == std::vector<std::string_view> { "a", "b", "c", "d" });
}

struct ComparableNoDefaultCtor
{
    ComparableNoDefaultCtor() = delete;
    constexpr ComparableNoDefaultCtor(int value):
        value { value }
    {
    }
    int value;
    bool operator==(ComparableNoDefaultCtor const&) const = default;
};

struct RecordWithNoDefaultCtor
{
    int id;
    ComparableNoDefaultCtor payload;
    std::string name;
};

TEST_CASE("NoDefaultCtor.all_apis", "[reflection]")
{
    static_assert(std::same_as<Reflection::MemberTypeOf<0, RecordWithNoDefaultCtor>, int>);
    static_assert(std::same_as<Reflection::MemberTypeOf<1, RecordWithNoDefaultCtor>, ComparableNoDefaultCtor>);
    static_assert(std::same_as<Reflection::MemberTypeOf<2, RecordWithNoDefaultCtor>, std::string>);

    auto const lhs = RecordWithNoDefaultCtor { .id = 1, .payload = ComparableNoDefaultCtor { 2 }, .name = "three" };
    auto const rhs = RecordWithNoDefaultCtor { .id = 1, .payload = ComparableNoDefaultCtor { 4 }, .name = "three" };

    std::string names;
    Reflection::CallOnMembers(lhs, [&](std::string_view name, auto const& /*value*/) { names += name; });
    CHECK(names == "idpayloadname");

    size_t typeCount = 0;
    Reflection::EnumerateMembers<RecordWithNoDefaultCtor>([&]<auto I, typename T>() { ++typeCount; });
    CHECK(typeCount == 3);

    auto const memberCount = Reflection::FoldMembers(
        lhs, size_t { 0 }, [](auto&& /*name*/, auto&& /*value*/, auto&& accum) { return accum + 1; });
    CHECK(memberCount == 3);

    std::string differences;
    Reflection::CollectDifferences(
        lhs, rhs, [&](std::string_view name, auto const& /*lhs*/, auto const& /*rhs*/) { differences += name; });
    CHECK(differences == "payload");
}

template <typename T>
struct RequiredField
{
    constexpr RequiredField() = delete;
    constexpr RequiredField(RequiredField const&) = default;
    constexpr RequiredField(RequiredField&&) noexcept = default;
    constexpr RequiredField& operator=(RequiredField const&) = default;
    constexpr RequiredField& operator=(RequiredField&&) noexcept = default;
    constexpr ~RequiredField() = default;

    template <typename... S>
        requires(sizeof...(S) != 0) && std::constructible_from<T, S...>
    constexpr RequiredField(S&&... args):
        value { std::forward<S>(args)... }
    {
    }

    bool operator==(RequiredField const&) const = default;

    T value;
};

struct RecordWithRequiredFields
{
    RequiredField<int> id;
    std::optional<int> age;
    RequiredField<std::string> name;
    int visits;
};

TEST_CASE("NoDefaultCtor.required_fields", "[reflection]")
{
    static_assert(Reflection::CountMembers<RecordWithRequiredFields> == 4);
    static_assert(std::same_as<Reflection::MemberTypeOf<0, RecordWithRequiredFields>, RequiredField<int>>);
    static_assert(std::same_as<Reflection::MemberTypeOf<1, RecordWithRequiredFields>, std::optional<int>>);
    static_assert(std::same_as<Reflection::MemberTypeOf<2, RecordWithRequiredFields>, RequiredField<std::string>>);
    static_assert(std::same_as<Reflection::MemberTypeOf<3, RecordWithRequiredFields>, int>);

    static_assert(Reflection::MemberNameOf<0, RecordWithRequiredFields> == "id");
    static_assert(Reflection::MemberNameOf<2, RecordWithRequiredFields> == "name");
    static_assert(Reflection::MemberNames<RecordWithRequiredFields>.size() == 4);
    static_assert(Reflection::NameOf<&RecordWithRequiredFields::name> == "name");
    static_assert(Reflection::MemberIndexOf<&RecordWithRequiredFields::name> == 2);
    static_assert(Reflection::MemberIndexOf<&RecordWithRequiredFields::visits> == 3);

    auto record = RecordWithRequiredFields { .id = 7, .age = std::nullopt, .name = "Jane", .visits = 3 };
    CHECK(Reflection::GetMemberAt<0>(record).value == 7);
    CHECK(Reflection::GetMemberAt<2>(record).value == "Jane");

    // members are handed out by reference, also for writing
    Reflection::GetMemberAt<3>(record) = 4;
    CHECK(record.visits == 4);

    std::string names;
    Reflection::EnumerateMembers<std::integer_sequence<size_t, 0, 2>>(record, [&]<size_t I>(auto const& /*value*/) {
        names += Reflection::MemberNameOf<I, RecordWithRequiredFields>;
    });
    CHECK(names == "idname");

    auto other = record;
    other.name = RequiredField<std::string> { "John" };
    std::vector<size_t> differences;
    Reflection::CollectDifferences(
        record, other, [&](size_t index, auto const& /*lhs*/, auto const& /*rhs*/) { differences.push_back(index); });
    CHECK(differences == std::vector<size_t> { 2 });
}

struct EmptyRecord
{
};

TEST_CASE("empty record", "[reflection]")
{
    static_assert(Reflection::CountMembers<EmptyRecord> == 0);

    auto const empty = EmptyRecord {};
    CHECK(Reflection::Inspect(empty).empty());

    size_t calls = 0;
    Reflection::CallOnMembers(empty, [&](auto&& /*name*/, auto&& /*value*/) { ++calls; });
    CHECK(calls == 0);

    auto const folded =
        Reflection::FoldMembers(empty, 42, [](auto&& /*name*/, auto&& /*value*/, auto&& accum) { return accum; });
    CHECK(folded == 42);
}

TEST_CASE("TypeNameOf.fundamental", "[reflection]")
{
    CHECK(Reflection::TypeNameOf<unsigned int> == "unsigned int");
    CHECK(Reflection::TypeNameOf<double> == "double");
}

TEST_CASE("TypeNameOf.array", "[reflection]")
{
    // The spelling of array types differs between compilers ("int[3]" vs "int [3]")
    CHECK(Reflection::TypeNameOf<int[3]>.starts_with("int"));
    CHECK(Reflection::TypeNameOf<int[3]>.ends_with("[3]"));
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

    std::string resultAnother;
    Reflection::CallOnMembersWithoutName(ps, [&resultAnother]<size_t I, typename FieldType>(FieldType const& value) {
        resultAnother += std::format("{}", value);
    });
    CHECK(resultAnother == R"(John Doejohn@doe.com42)");

    // The callable receives the plain member type, for const and non-const objects alike.
    auto const constPerson = ps;
    Reflection::CallOnMembersWithoutName(constPerson, []<size_t I, typename FieldType>(FieldType const& /*value*/) {
        static_assert(std::same_as<FieldType, Reflection::MemberTypeOf<I, Person>>);
    });
    Reflection::CallOnMembersWithoutName(ps, []<size_t I, typename FieldType>(FieldType& /*value*/) {
        static_assert(std::same_as<FieldType, Reflection::MemberTypeOf<I, Person>>);
    });
}

enum class Level : std::uint8_t
{
    Low = 1,
    High = 2
};

struct Opaque
{
    Opaque() = default;
    Opaque(Opaque const&) = delete;
    Opaque& operator=(Opaque const&) = delete;
    std::mutex mutex;
};

struct InspectedRecord
{
    Color color;
    Level level;
    std::optional<int> missing;
    std::optional<std::string> present;
    std::vector<int> numbers;
    std::vector<Person> people;
    char const* text;
    char const* nullText;
    Opaque opaque;
};

TEST_CASE("Inspect.member_kinds", "[reflection]")
{
    auto const record = InspectedRecord {
        .color = Color::Blue,
        .level = Level::High,
        .missing = std::nullopt,
        .present = "yes",
        .numbers = { 1, 2, 3 },
        .people = { Person { .name = "John Doe", .email = "john@doe.com", .age = 42 } },
        .text = "text",
        .nullText = nullptr,
        .opaque = {},
    };

    CHECK(Reflection::Inspect(record)
          == R"(color=2 level=2 missing=nullopt present="yes" numbers=[1, 2, 3] )"
             R"(people=[{name="John Doe" email="john@doe.com" age=42}] text="text" nullText=null opaque=<Opaque>)");
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

struct MoveOnlyAccumulator
{
    MoveOnlyAccumulator() = default;
    MoveOnlyAccumulator(MoveOnlyAccumulator const&) = delete;
    MoveOnlyAccumulator& operator=(MoveOnlyAccumulator const&) = delete;
    MoveOnlyAccumulator(MoveOnlyAccumulator&&) = default;
    MoveOnlyAccumulator& operator=(MoveOnlyAccumulator&&) = default;
    int count = 0;
};

TEST_CASE("FoldMembers.move_only_accumulator", "[reflection]")
{
    auto const s = S { .a = 1, .b = 2, .c = 3 };
    auto const result = Reflection::FoldMembers(
        s, MoveOnlyAccumulator {}, [](auto&& /*name*/, auto&& /*value*/, MoveOnlyAccumulator accum) {
            ++accum.count;
            return accum;
        });

    CHECK(result.count == 3);
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
    Reflection::template_for<std::integer_sequence<size_t, 3, 2, 1>>([&]<size_t I>() { result += std::to_string(I); });
    CHECK(result == "321");
}
