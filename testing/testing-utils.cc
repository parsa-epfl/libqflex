// #include "testing-utils.hh"
#include <iostream>
#include <bitset>
#include <string>
#include <cstring>
#include <random>
#include <cassert>
#include <vector>
#include <functional>
#include <initializer_list>
#include <numeric>

bool isBit(char c) {
  return c == '0' || c == '1' || c == 'x';
}

bool isBitmask(const std::string& str) {
  for (const auto& c : str) {
    if (!isBit(c)) {
      return false;
    }
  }
  return true;
}

class Field {
  public:
    Field(const std::string& str) : bitmask(str) {
      assert(isBitmask(str) && "Invalid bitmask string.");
    }

    Field& operator=(const std::string& bits) {
      bitmask = bits;
      return *this;
    }

    std::string bitmask;
};

struct Instruction {
  public:
    class InstructionIterator {
      public:
        InstructionIterator(const Instruction& instruction, bool end = false) : instruction(instruction) {
          for (const auto& field : instruction.fields) {
            bitmask += field->bitmask;
          }

          int numDontCares = std::count(bitmask.begin(), bitmask.end(), 'x');
          
          if (numDontCares <= 20) {
            brute = true;
            maxIter = 1 << numDontCares;
          } else {
            brute = false;
            maxIter = 1 << 20;
          }

          if (end) {
            currIter = maxIter;
          } else {
            currIter = 0;
          }
        }

        InstructionIterator& operator++() {
          currIter++;
          return *this;
        }

        uint32_t operator*() const {
          uint32_t result = 0;

          int N = bitmask.size();

          for (int i = 0, j = 0; i < N; ++i) {
            if (bitmask[N - 1 - i] == 'x') {
              if (brute) { 
                result |= ((currIter >> j) & 1) << i;
              } else {
                result |= dist(rand_eng) << i;
              }
              ++j;
            } else if (bitmask[N - 1 - i] == '1') {
              result |= 1 << i;
            }
          }

          return result;
        }

        bool operator!=(const InstructionIterator& other) const {
            return currIter != other.currIter;
        }

      private:
        const Instruction& instruction;
        std::string bitmask{""};
        int currIter;
        int maxIter;
        bool brute;
        mutable std::mt19937 rand_eng{std::random_device{}()};
        mutable std::uniform_int_distribution<int> dist{0, 1};
    };

  public:
    template<typename... Args>
    Instruction(Args&&... args) {
      addFields(std::forward<Args>(args)...);
    }

    std::vector<Field*> fields;

    InstructionIterator begin() const {
      return InstructionIterator(*this);
    }

    InstructionIterator end() const {
      return InstructionIterator(*this, true);
    }

  private:
    template<typename First, typename... Rest>
    void addFields(First&& first, Rest&&... rest) {
      addField(std::forward<First>(first));
      addFields(std::forward<Rest>(rest)...);
    }

    void addFields() {}

    void addField(std::reference_wrapper<Field> field) {
      fields.push_back(&field.get());
    }

    void addField(const std::string& str) {
      assert(isBitmask(str) && "Invalid bitmask string.");
      fields.push_back(new Field(str));
    }
};

int main() {
    Field b1("110x");
    Field b2("1x10x1");

    Instruction instr(b1, b2);

    for (const auto& i : instr) {
      std::cout << i << std::endl;
    }

    return 0;
}
