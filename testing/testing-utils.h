#ifndef TESTING_UTILS_H
#define TESTING_UTILS_H

#include <string>
#include <vector>
#include <random>
#include <memory>

// Maximum number of don't care bits to brute force
constexpr uint32_t MAX_BRUTE_DONT_CARE = 20;

// Number of bits in an instruction
constexpr uint32_t NUM_INSTR_BITS = 32;

class Field {
  public:
    Field(const std::string& str);

    Field& operator=(const std::string& str);

    static bool isBit(char c);

    static bool isBitmask(const std::string& str);

    std::string bitmask;
};

class Instruction {
  public:
    /*
    * The following together allow range-based for loops over the Instruction bitmask:
    * - begin()
    * - end()
    * - InstructionIterator
    * - InstructionIterator::operator++()
    * - InstructionIterator::operator*()
    * - InstructionIterator::operator!=()
    */
    class InstructionIterator {
      public:
        InstructionIterator(const Instruction& instruction, bool end = false);

        InstructionIterator& operator++();

        uint32_t operator*() const;

        bool operator!=(const InstructionIterator& other) const;

      private:
        const Instruction& instruction;

        std::string bitmask{""};

        // Current iteration number (iterates from 0 to numIters - 1)
        uint32_t currIter;

        // Total number of iterations based on the number of don't cares 
        uint32_t numIters;

        // Flag to indicate if the iterator is a brute force iterator
        // Brute force iterators iterate over all possible values of the don't cares
        // Non-brute force iterators randomly choose the values of the don't cares
        bool brute;

        // Random number generator for non-brute force iterators
        mutable std::mt19937 rand_eng{std::random_device{}()};
        mutable std::uniform_int_distribution<uint32_t> dist{0, 1};
    };

    // Variadic template allows combination of Field references and string literals when constructing an Instruction
    template<typename... Args>
    Instruction(Args&&... args);

    std::vector<std::shared_ptr<Field>> fields;

    InstructionIterator begin() const;

    InstructionIterator end() const;

  private:
    template<typename First, typename... Rest>
    void addFields(First&& first, Rest&&... rest);

    void addFields();

    void addField(Field& field);

    void addField(const std::string& str);
};

template<typename... Args>
Instruction::Instruction(Args&&... args) {
  addFields(std::forward<Args>(args)...);
}

template<typename First, typename... Rest>
void Instruction::addFields(First&& first, Rest&&... rest) {
  addField(std::forward<First>(first));
  addFields(std::forward<Rest>(rest)...);
}

#endif // TESTING_UTILS_H
