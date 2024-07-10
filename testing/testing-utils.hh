#ifndef TESTING_UTILS_HH
#define TESTING_UTILS_HH

#include <iostream>
#include <bitset>
#include <string>
#include <cstring>
#include <random>
#include <cassert>
#include <vector>
#include <functional>
#include <initializer_list>
#include <algorithm>

// Check if a character represents a bit
bool isBit(char c);

// Check if a string is a valid bitmask
bool isBitmask(const std::string& str);

class Field {
  public:
    // Constructor that initializes the field with a bitmask string
    Field(const std::string& str);

    // Assignment operator to set the bitmask
    Field& operator=(const std::string& str);

    // The bitmask string
    std::string bitmask;
};

struct Instruction {
  public:
    // Nested class for iterating over instructions
    class InstructionIterator {
      public:
        // Constructor for the iterator
        InstructionIterator(const Instruction& instruction, bool end = false);

        // Pre-increment operator
        InstructionIterator& operator++();

        // Dereference operator to get the current instruction value
        uint32_t operator*() const;

        // Inequality operator for iterators
        bool operator!=(const InstructionIterator& other) const;

      private:
        const Instruction& instruction;
        std::string bitmask{""};
        int currIter;
        int maxIter;
        bool brute;
        mutable std::mt19937 rand_eng{std::random_device{}()};
        mutable std::uniform_int_distribution<int> dist{0, 1};
    };

    // Constructor for the instruction with variadic template arguments
    template<typename... Args>
    Instruction(Args&&... args);

    // Vector to hold pointers to fields
    std::vector<Field*> fields;

    // Begin iterator for the instruction
    InstructionIterator begin() const;

    // End iterator for the instruction
    InstructionIterator end() const;

  private:
    // Template function to add fields to the instruction
    template<typename First, typename... Rest>
    void addFields(First&& first, Rest&&... rest);

    // Function to add a single field to the instruction
    void addFields();

    // Function to add a field by reference
    void addField(std::reference_wrapper<Field> field);

    // Function to add a field by string
    void addField(const std::string& str);
};

// Instruction class constructor
template<typename... Args>
Instruction::Instruction(Args&&... args) {
  addFields(std::forward<Args>(args)...);
}

// AddFields implementation for variadic templates
template<typename First, typename... Rest>
void Instruction::addFields(First&& first, Rest&&... rest) {
  addField(std::forward<First>(first));
  addFields(std::forward<Rest>(rest)...);
}

#endif // TESTING_UTILS_HH
