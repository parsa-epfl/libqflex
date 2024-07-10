#include "testing-utils.hh"

// Function to check if a character represents a bit
bool isBit(char c) {
  return c == '0' || c == '1' || c == 'x';
}

// Function to check if a string is a valid bitmask
bool isBitmask(const std::string& str) {
  for (const auto& c : str) {
    if (!isBit(c)) {
      return false;
    }
  }
  return true;
}

// Constructor that initializes the field with a bitmask string
Field::Field(const std::string& str) : bitmask(str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
}

// Assignment operator to set the bitmask
Field& Field::operator=(const std::string& str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
  bitmask = str;
  return *this;
}

// Terminal case for the variadic template
void Instruction::addFields() {}

// Add a field by reference
void Instruction::addField(std::reference_wrapper<Field> field) {
  fields.push_back(&field.get());
}

// Add a field by string
void Instruction::addField(const std::string& str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
  fields.push_back(new Field(str));
}

// Begin iterator for the instruction
Instruction::InstructionIterator Instruction::begin() const {
  return InstructionIterator(*this);
}

// End iterator for the instruction
Instruction::InstructionIterator Instruction::end() const {
  return InstructionIterator(*this, true);
}

// Constructor for the iterator
Instruction::InstructionIterator::InstructionIterator(const Instruction& instruction, bool end) : instruction(instruction) {
  for (const auto& field : instruction.fields) {
    bitmask += field->bitmask;
  }
  assert(bitmask.size() == 32 && "Bitmask size must be 32.");

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

// Pre-increment operator
Instruction::InstructionIterator& Instruction::InstructionIterator::operator++() {
  currIter++;
  return *this;
}

// Dereference operator to get the current instruction value
uint32_t Instruction::InstructionIterator::operator*() const {
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

// Inequality operator for iterators
bool Instruction::InstructionIterator::operator!=(const InstructionIterator& other) const {
  return currIter != other.currIter;
}
