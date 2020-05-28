#pragma once
#include <omp.h>

struct omp_util
{
  static int get_num_threads();
  static int get_thread_num();
};
