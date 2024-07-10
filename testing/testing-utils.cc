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

template <size_t N>
class Bitmask {
  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = uint32_t;
    using difference_type = std::ptrdiff_t;
    using pointer = uint32_t*;
    using reference = uint32_t&;

    std::bitset<N> bits{0};
    std::bitset<N> dontCares{0};

    Bitmask(const std::string& bitstring) {
      assert(bitstring.size() == N && "String argument size does not match the bitmask size.");

      for (int i = 0; i < N; ++i) {
        if (bitstring[i] == '1') {
          bits.set(N - 1 - i);
        } else if (bitstring[i] == '0') {
          bits.reset(N - 1 - i);
        } else if (bitstring[i] == 'x' || bitstring[i] == 'X') {
          dontCares.set(N - 1 - i);
        }
      }
    }

    template <size_t M>
    Bitmask<N + M> operator+(const Bitmask<M>& other) const {
      std::string combinedBits = bits.to_string() + other.bits.to_string();
      std::string combinedDontCares = dontCares.to_string() + other.dontCares.to_string();

      Bitmask<N + M> result(combinedBits);
      result.dontCares = std::bitset<N + M>(combinedDontCares);

      return result;
    }

    friend std::ostream& operator<<(std::ostream& os, const Bitmask& bitmask) {
      for (int i = 0; i < N; ++i) {
        if (bitmask.dontCares[N - 1 - i]) {
          os << 'x';
        } else {
          os << bitmask.bits[N - 1 - i];
        }
      }
      return os;
    }

    class BitmaskIterator {
      public:
        BitmaskIterator(const Bitmask& bitmask, bool end = false) : bitmask(bitmask) {
          if (bitmask.dontCares.count() <= 20) {
            brute = true;
            maxIter = 1 << bitmask.dontCares.count();
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

        BitmaskIterator& operator++() {
          currIter++;
          return *this;
        }

        uint32_t operator*() const {
          std::bitset<N> result = bitmask.bits;
          if (brute) {
            for (int i = 0, j = 0; i < N; ++i) {
              if (bitmask.dontCares[i]) {
                result.set(i, (currIter >> j) & 1);
                ++j;
              }
            }
          } else {
            for (int i = 0; i < N; ++i) {
              if (bitmask.dontCares[i]) {
                result.set(i, dist(rand_eng));
              }
            }
          }
          return (uint32_t)result.to_ulong();
        }

        bool operator!=(const BitmaskIterator& other) const {
            return currIter != other.currIter;
        }

      private:
        const Bitmask& bitmask;
        int currIter;
        int maxIter;
        bool brute;
        mutable std::mt19937 rand_eng{std::random_device{}()};
        mutable std::uniform_int_distribution<int> dist{0, 1};
    };

    BitmaskIterator begin() const {
      return BitmaskIterator(*this);
    }

    BitmaskIterator end() const {
      return BitmaskIterator(*this, true);
    }
};

int main() {
    Bitmask<4> b1("110x");
    Bitmask<6> b2("1x10x1");

    std::cout << "Bitmask 1: " << b1 << "\n";
    std::cout << "Bitmask 2: " << b2 << "\n";

    auto result = b1 + b2;

    std::cout << "\nConcatenated Bitmask:\n";
    std::cout << result << "\n";

    std::cout << "\nStream of randomized numbers for the combined Bitmask:\n";
    for (const auto& bm : result) {
        std::cout << bm << "\n";
    }

    return 0;
}
