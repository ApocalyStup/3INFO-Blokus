#include "allocator.hpp"
#include <iostream>
#include <cstring>

allocator::allocator(unsigned int size)
{
  std::cout << "allocator initialization..." << std::endl;
  arena = new char[size];
  std::cout << "allocator initialization done" << std::endl;
  limit = arena + size;
  free_pointer = arena;
}

allocator::~allocator()
{
  delete[] arena;
}

char* allocator::allocate(unsigned int size)
{
  char* mem;
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma omp atomic capture
  {
    mem = free_pointer;
    free_pointer += size;
  }
  memset(mem, 0, size);
  if (mem + size >= limit) throw "allocate: not enough memory";
  return mem;
}

unsigned int allocator::size() const
{
  return limit - arena;
}

unsigned int allocator::free_space() const
{
  return limit - free_pointer;
}

void allocator::clear()
{
  free_pointer = arena;
}

char* allocator::allocate_unsafe(unsigned int size)
{
  char* n = free_pointer;
  free_pointer += size;
  memset(n, 0, size);
  if (n + size > limit) throw "allocate_unsafe: not enough memory";
  return n;
}
