#include "testing-utils.hh"

// Function to check if a character represents a bit (including don't cares)
bool isBit(char c) {
  return c == '0' || c == '1' || c == 'x';
}

// Function to check if a string is a valid bitmask (i.e. only contains 0s, 1s, and x's)
bool isBitmask(const std::string& str) {
  for (const auto& c : str) {
    if (!isBit(c)) {
      return false;
    }
  }
  return true;
}

// Constructor that initializes a Field object with a bitmask string
Field::Field(const std::string& str) : bitmask(str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
}

// Assignment operator to set the bitmask of a Field object
Field& Field::operator=(const std::string& str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
  bitmask = str;
  return *this;
}

// Terminal case for the variadic template Instruction constructor
void Instruction::addFields() {}

// Allows Field objects in the variadic template Instruction constructor
void Instruction::addField(std::reference_wrapper<Field> field) {
  fields.push_back(&field.get());
}

// Allows string literals in the variadic template Instruction constructor
void Instruction::addField(const std::string& str) {
  assert(isBitmask(str) && "Invalid bitmask string.");
  fields.push_back(new Field(str));
}

// Begin iterator to allow range-based for loops
Instruction::InstructionIterator Instruction::begin() const {
  return InstructionIterator(*this);
}

// End iterator to allow range-based for loops
Instruction::InstructionIterator Instruction::end() const {
  return InstructionIterator(*this, true);
}

// An iterator for iterating over all possible values of an instruction based on its bitmask
Instruction::InstructionIterator::InstructionIterator(const Instruction& instruction, bool end) : instruction(instruction) {
  // Concatenate the bitmasks of all fields to get the instruction bitmask
  for (const auto& field : instruction.fields) {
    bitmask += field->bitmask;
  }
  // The instruction bitmask must be 32 bits
  assert(bitmask.size() == 32 && "Bitmask size must be 32.");

  // Count the number of don't cares in the instruction bitmask
  int numDontCares = std::count(bitmask.begin(), bitmask.end(), 'x');

  // If the number of don't cares is less than or equal to the maximum number of iterated don't cares, ...
  if (numDontCares <= MAX_ITER_DONT_CARE) {
    // ... brute force through all possible values of the don't cares
    brute = true;
    numIters = 1 << numDontCares;
  } else {
    // ... else randomly choose the values of the don't cares
    brute = false;
    numIters = 1 << MAX_ITER_DONT_CARE;
  }

  // If it's the end iterator, ...
  if (end) {
    // ... set the current iteration to the number of iterations
    currIter = numIters;
  } else {
    // ... else set the current iteration to 0
    currIter = 0;
  }
}

// Pre-increment operator for InstructionIterator to allow range-based for loops
Instruction::InstructionIterator& Instruction::InstructionIterator::operator++() {
  currIter++;
  return *this;
}

// Dereference operator to get the current instruction value during range-based for loops
uint32_t Instruction::InstructionIterator::operator*() const {
  uint32_t result = 0;

  // Generate the instruction based on the bitmask
  for (int i = 0, j = 0; i < 32; ++i) {
    // If the bitmask bit is a don't care, ...
    if (bitmask[31 - i] == 'x') {
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
    } else if (bitmask[31 - i] == '1') {
      // ... set the result bit to 1
      result |= 1 << i;
    }
  }

  return result;
}

// Inequality operator for InstructionIterator to allow range-based for loops
bool Instruction::InstructionIterator::operator!=(const InstructionIterator& other) const {
  return currIter != other.currIter;
}
