#pragma once
#include <string>

struct statistics
{
  int count = 0;
  int value = 0;
  std::string to_string() const;
};
