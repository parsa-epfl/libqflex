#include "testing-utils.h"
#include <cassert>
#include <algorithm>
#include <numeric>

// Only smallcase x denotes a don't care bit (not uppercase X)
bool Field::isBit(char c) {
  return c == '0' || c == '1' || c == 'x';
}

bool Field::isBitmask(const std::string& str) {
  for (const auto& c : str) {
    if (!isBit(c)) {
      return false;
    }
  }
  return true;
}

Field::Field(const std::string& str) : bitmask(str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
}

Field& Field::operator=(const std::string& str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
  bitmask = str;
  return *this;
}

void Instruction::addFields() {}

void Instruction::addField(Field& field) {
  fields.push_back(std::shared_ptr<Field>(&field, [](Field*){}));
}

void Instruction::addField(const std::string& str) {
  assert(Field::isBitmask(str) && "Invalid bitmask string.");
  fields.push_back(std::make_shared<Field>(str));
}

Instruction::InstructionIterator Instruction::begin() const {
  return InstructionIterator(*this);
}

Instruction::InstructionIterator Instruction::end() const {
  return InstructionIterator(*this, true);
}

Instruction::InstructionIterator::InstructionIterator(const Instruction& instruction, bool end) : instruction(instruction) {
  bitmask = std::accumulate(
    instruction.fields.begin(),
    instruction.fields.end(),
    std::string(""),
    [](std::string acc, std::shared_ptr<Field> field) {
      return acc + field->bitmask;
    }
  );
  assert((bitmask.size() == NUM_INSTR_BITS) && "Bitmask size does not match the number of bits in an instruction (32 bits).");
  
  uint32_t numDontCares = std::count(bitmask.begin(), bitmask.end(), 'x');

  if (numDontCares <= MAX_BRUTE_DONT_CARE) {
    brute = true;
    numIters = 1 << numDontCares;
  } else {
    brute = false;
    numIters = 1 << MAX_BRUTE_DONT_CARE;
  }

  if (end) {
    currIter = numIters;
  } else {
    currIter = 0;
  }
}

Instruction::InstructionIterator& Instruction::InstructionIterator::operator++() {
  currIter++;
  return *this;
}

uint32_t Instruction::InstructionIterator::operator*() const {
  uint32_t result = 0;

  // Generate the instruction based on the bitmask
  for (uint32_t i = 0, j = 0; i < NUM_INSTR_BITS; ++i) {
    // If the bitmask bit is a don't care, ...
    if (bitmask[NUM_INSTR_BITS - 1 - i] == 'x') {
      // ... and we are brute forcing, ...
      if (brute) {
        // ... set the result bit according to the current iteration
        result |= ((currIter >> j) & 1) << i;
      } else {
        // ... else set the result bit randomly
        result |= dist(rand_eng) << i;
      }
      // Increment the don't care counter
      ++j;
    // If the bitmask bit is a 1, ...
    } else if (bitmask[NUM_INSTR_BITS - 1 - i] == '1') {
      // ... set the result bit to 1
      result |= 1 << i;
    }
  }

  return result;
}

bool Instruction::InstructionIterator::operator!=(const InstructionIterator& other) const {
  return currIter != other.currIter;
}
