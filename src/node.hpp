#pragma once

#include "statistics.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <limits>

template<typename Move>
class node
{
  statistics stats;
  bool flag = false;
  signed char won = 0;
  uint16_t number_of_children = 0;
  Move* moves          = nullptr;
  node<Move>* children = nullptr;

public:
  inline uint16_t get_winning_index() const;
  inline bool is_leaf() const;
  inline uint16_t get_number_of_children() const;
  uint32_t size() const;
  inline Move* get_moves() const;
  inline void set_moves(Move* moves);
  inline void set_moves(const std::vector<Move>& moves);
  inline node<Move>* get_children() const;
  inline void set_number_of_children(uint16_t n);
  inline void set_children(node<Move>* n);
  inline void set_won(signed char v);
  inline signed char get_won() const;
  inline void set_won();
  inline void set_lost();
  inline bool is_proven() const;
  inline bool is_won() const;
  inline bool is_lost() const;
  inline const statistics& get_statistics() const;
  inline statistics& get_statistics_ref();
  inline void set_statistics(const statistics& s);
  inline bool test_and_set();
  inline void add_virtual_loss(int n);
  inline void update(int value, int virtual_loss = 0);
  inline void update_count();
};

template<typename Move>
bool node<Move>::is_proven() const
{
  return is_won() || is_lost();
}

template<typename Move>
void node<Move>::set_won(signed char v)
{
  won = v;
}

template<typename Move>
signed char node<Move>::get_won() const
{
  return won;
}

template<typename Move>
void node<Move>::set_won()
{
  won = 1;
}

template<typename Move>
void node<Move>::set_lost()
{
  won = -1;
}

template<typename Move>
bool node<Move>::is_won() const
{
  return won == 1;
}

template<typename Move>
bool node<Move>::is_lost() const
{
  return won == -1;
}

template<typename Move>
bool node<Move>::is_leaf() const
{
  return get_number_of_children() == 0 || get_children() == nullptr;
}

template<typename Move>
uint16_t node<Move>::get_number_of_children() const
{
  uint16_t res;
  res = number_of_children;
  return res;
}

template<typename Move>
node<Move>* node<Move>::get_children() const
{
  node* res;
  res = children;
  return res;
}

template<typename Move>
Move* node<Move>::get_moves() const
{
  Move* res;
  res = moves;
  return res;
}

template<typename Move>
void node<Move>::set_moves(const std::vector<Move>& moves)
{
  for (unsigned int i = 0; i < moves.size(); ++i)
    {
      this->moves[i] = moves[i];
    }
}

template<typename Move>
void node<Move>::set_moves(Move* moves)
{
  this->moves = moves;
}

template<typename Move>
void node<Move>::set_number_of_children(uint16_t n)
{
  number_of_children = n;
}

template<typename Move>
void node<Move>::set_children(node<Move>* n)
{
  children = n;
}

template<typename Move>
const statistics& node<Move>::get_statistics() const
{
  return stats;
}

template<typename Move>
statistics& node<Move>::get_statistics_ref()
{
  return stats;
}

template<typename Move>
void node<Move>::set_statistics(const statistics& s)
{
  stats = s;
}

template<typename Move>
bool node<Move>::test_and_set()
{
  bool res;
#pragma GCC diagnostic ignored "-Wunused-value"
#pragma omp atomic capture
  {
    res = flag;
    flag = true;
  }
  return res;
}

template<typename Move>
void node<Move>::update_count()
{
#pragma omp atomic update
  ++stats.count;
}

template<typename Move>
void node<Move>::add_virtual_loss(int n)
{
#pragma omp atomic update
  stats.count += n;
#pragma omp atomic update
  stats.value -= n;
}

template<typename Move>
void node<Move>::update(int v, int virtual_loss)
{
#pragma omp atomic update
  stats.count += 1 - virtual_loss;
#pragma omp atomic update
  stats.value += v + virtual_loss;
}

template<typename Move>
uint32_t node<Move>::size() const
{
  uint32_t res = 1;
  for (uint16_t i = 0; i < number_of_children; ++i)
    {
      res += children[i].size();
    }
  return res;
}
