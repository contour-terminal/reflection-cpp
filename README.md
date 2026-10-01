# Reflection C++

This is a C++ static reflection library.

## Goals

- Trivial to integrate into existing codebases (no code generation, no macros, no build system changes)
- Minimal to zero runtime overhead
- Works with C++20 and later
- Prepared to integrate C++26 reflections when they are available

## Requirements

A compiler with C++20 support. The library is tested with GCC 13 and later, Clang 17 and later,
Apple Clang, MSVC and clang-cl. It is a single header without dependencies beyond the standard library.

Reflection works on aggregates: types without user-declared constructors, private members or base classes.
Members may be of any type, including types without a default constructor. An aggregate may have up to
150 members.

With MSVC, member names are only available for types with external linkage, that is, not for types declared
in an anonymous namespace or inside a function.

## Usage

```cpp
#include <reflection-cpp/reflection.hpp>

struct Person
{
    std::string name;
    std::string email;
    int age;
};

auto person = Person { .name = "John Doe", .email = "john@doe.com", .age = 42 };
```

Everything about the type itself is available at compile time:

```cpp
static_assert(Reflection::CountMembers<Person> == 3);
static_assert(Reflection::MemberNameOf<0, Person> == "name");
static_assert(Reflection::MemberNames<Person>[2] == "age");
static_assert(std::same_as<Reflection::MemberTypeOf<2, Person>, int>);
static_assert(Reflection::TypeNameOf<Person> == "Person");
static_assert(Reflection::NameOf<&Person::email> == "email");
static_assert(Reflection::MemberIndexOf<&Person::email> == 1);
```

Members of an object are accessed by reference, and never copied:

```cpp
Reflection::GetMemberAt<2>(person) = 43;

// name and value of each member
Reflection::CallOnMembers(person, [](std::string_view name, auto const& value) {
    std::println("{}: {}", name, value);
});

// index of each member as a template argument, e.g. to look up something else by it
Reflection::EnumerateMembers(person, []<size_t I>(auto& value) {
    std::println("{}: {}", Reflection::MemberNameOf<I, Person>, value);
});

// only the members at index 0 and 2
Reflection::EnumerateMembers<std::integer_sequence<size_t, 0, 2>>(person, []<size_t I>(auto& value) { /* ... */ });
```

A type can be traversed without having an object of it:

```cpp
Reflection::EnumerateMembers<Person>([]<size_t I, typename MemberType>() {
    std::println("{} is a {}", Reflection::MemberNameOf<I, Person>, Reflection::TypeNameOf<MemberType>);
});

// counting the members of type std::string
auto const stringCount = Reflection::FoldMembers<Person>(size_t { 0 }, []<size_t I, typename MemberType>(size_t count) {
    return count + (std::same_as<MemberType, std::string> ? 1 : 0);
});
```

Folding over the members of an object:

```cpp
auto const totalLength = Reflection::FoldMembers(person, size_t { 0 }, [](std::string_view name, auto const& /*value*/, size_t length) {
    return length + name.size();
});
```

For debugging, objects can be printed and compared:

```cpp
Reflection::Inspect(person); // name="John Doe" email="john@doe.com" age=43

auto other = person;
other.age = 50;
Reflection::CollectDifferences(person, other, [](std::string_view name, auto const& lhs, auto const& rhs) {
    std::println("{}: {} != {}", name, lhs, rhs); // age: 43 != 50
});
```
