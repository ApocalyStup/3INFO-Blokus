#pragma once

class allocator
{
  char* arena;
  char* limit;
  char* free_pointer;

public:
  allocator(unsigned int size = 2000'000'000U);
  ~allocator();
  char* allocate(unsigned int size);
  char* allocate_unsafe(unsigned int size);
  void clear();
  unsigned int size() const;
  unsigned int free_space() const;
};
