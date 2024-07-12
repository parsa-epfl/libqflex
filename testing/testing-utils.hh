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

// Maximum number of don't care bits to iterate over
constexpr int MAX_ITER_DONT_CARE = 20;

// Check if a character represents a bit
bool isBit(char c);

// Check if a string is a valid bitmask
bool isBitmask(const std::string& str);

// Field class that represents a field with a bitmask
class Field {
  public:
    // Constructor that initializes the field with a bitmask string
    Field(const std::string& str);

    // Assignment operator to set the bitmask
    Field& operator=(const std::string& str);

    // The bitmask string
    std::string bitmask;
};

// Instruction class that represents an instruction with multiple fields 
class Instruction {
  public:
    // An iterator for iterating over all possible values of an instruction based on its bitmask
    // This iterator is used to allow range-based for loops over the instruction bitmask
    class InstructionIterator {
      public:
        // Constructor for the iterator
        InstructionIterator(const Instruction& instruction, bool end = false);

        // Pre-increment operator to allow range-based for loops
        InstructionIterator& operator++();

        // Dereference operator to get the current instruction value during range-based for loops
        uint32_t operator*() const;

        // Inequality operator for iterators to allow range-based for loops
        bool operator!=(const InstructionIterator& other) const;

      private:
        // Reference to the Instruction the iterator is iterating over
        const Instruction& instruction;

        // Bitmask of the Instruction
        std::string bitmask{""};

        // Current iteration number (iterates from 0 to numIters - 1)
        int currIter;

        // Number of iterations based on the number of don't cares 
        int numIters;

        // Flag to indicate if the iterator is a brute force iterator
        // Brute force iterators iterate over all possible values of the don't cares
        // Non-brute force iterators randomly choose the values of the don't cares
        bool brute;

        // Random number generator for non-brute force iterators
        mutable std::mt19937 rand_eng{std::random_device{}()};
        mutable std::uniform_int_distribution<int> dist{0, 1};
    };

    // Variadic template constructor to allow combination of Field references and string literals
    template<typename... Args>
    Instruction(Args&&... args);

    // Vector to hold pointers to Field objects
    std::vector<Field*> fields;

    // Begin iterator for the Instruction to allow range-based for loops
    InstructionIterator begin() const;

    // End iterator for the Instruction to allow range-based for loops
    InstructionIterator end() const;

  private:
    // Variadic template function to add fields (Field objects or string literals) to the instruction
    template<typename First, typename... Rest>
    void addFields(First&& first, Rest&&... rest);

    // Terminal function for the variadic template addFields
    void addFields();

    // Function to add a Field object by reference
    void addField(std::reference_wrapper<Field> field);

    // Function to add a field by string literal
    void addField(const std::string& str);
};

// Variadic template constructor implementation
template<typename... Args>
Instruction::Instruction(Args&&... args) {
  addFields(std::forward<Args>(args)...);
}

// Variadic template addFields implementation
template<typename First, typename... Rest>
void Instruction::addFields(First&& first, Rest&&... rest) {
  addField(std::forward<First>(first));
  addFields(std::forward<Rest>(rest)...);
}

#endif // TESTING_UTILS_HH
