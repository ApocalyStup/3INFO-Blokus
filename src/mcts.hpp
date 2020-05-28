#pragma once

#include "allocator.hpp"
#include "fast_log.hpp"
#include "omp_util.hpp"
#include "node.hpp"
#include "display_node.hpp"
#include <fstream>
#include <vector>
#include <chrono>

template <typename Game>
class mcts
{
  using Move = typename Game::Move;
  Game& game;
  std::chrono::milliseconds thinking_time;
  std::vector<std::mt19937> generators;
  node<Move>* root;
  allocator memory_allocator;
  const fast_log log;
  const float exploration_factor;
  const unsigned int nb_visits_before_expansion;

  inline node<Move>* select(Game& game, std::mt19937& generator, node<Move>* parent);
  inline void expand(const Game& game, node<Move>* n);
  void think(Game game);

  void copy(node<Move>* n1, node<Move>* n2, int prunning = 0);
  node<Move>* move(node<Move>* root, int prunning = 0);
  node<Move>* allocate_node(unsigned int size);
  Move* allocate_move(unsigned int size);

public:
  mcts(Game& game, uint32_t milliseconds, float C, unsigned int nb_visits_before_expansion = 8);
  void reset();
  inline Move select_move();
  void last_move(int move);
  void last_moves(int computer, int other);
};

template <typename Game>
mcts<Game> make_mcts(Game& game, uint32_t milliseconds, float C, unsigned int nb_visits_before_expansion = 8)
{
  return mcts<Game>(game, milliseconds, C, nb_visits_before_expansion);
}

template <typename Game>
mcts<Game>::mcts(Game& game, uint32_t milliseconds, float C, unsigned int nb_visits_before_expansion)
  : game(game), thinking_time(milliseconds), exploration_factor(C), nb_visits_before_expansion(nb_visits_before_expansion)
{
  generators.assign(omp_util::get_num_threads(), std::mt19937());
  root = allocate_node(1);
}

template <typename Game>
void mcts<Game>::reset()
{
  memory_allocator.clear();
  root = allocate_node(1);
}

template <typename Game>
node<typename Game::Move>* mcts<Game>::select(Game& game, std::mt19937& generator, node<Move>* parent)
{
  using namespace std;
  const unsigned int N = parent->get_statistics().count;
  const float log_of_N = log.log(N);
  const uint16_t nb_children = parent->get_number_of_children();
  uniform_int_distribution<uint16_t> d(0, nb_children - 1);
  uint16_t k = d(generator);
  double best_value_so_far = numeric_limits<double>::lowest();
  int best_index_so_far = k;
  node<Move>* const children = parent->get_children();
  node<Move>* best_child_so_far = children + k;
  double v;
  for (uint16_t i = 0; i < nb_children; ++i)
    {
      node<Move>* const child = children + k;
      const unsigned int count = child->get_statistics().count;
      v = -((double)child->get_statistics().value / count) + exploration_factor * sqrt(log_of_N / count);
      if (v > best_value_so_far)
        {
          best_index_so_far = k;
          best_value_so_far = v;
          best_child_so_far = child;
        }
      ++k;
      k &= ~(-(k == nb_children));
    }
  if (best_child_so_far->is_proven())
    {
      if (best_child_so_far->is_lost()) parent->set_won();
      else
        {
          bool all_won = true;
          for (uint16_t i = 0; i < nb_children; ++i)
            {
              const node<Move>* const child = children + i;
              if (!child->is_won())
                {
                  all_won = false;
                  break;
                }
            }
          if (all_won) parent->set_lost();
        }
    }
  game.make_move(parent->get_moves()[best_index_so_far]);
  return best_child_so_far;
}

template <typename Game>
void mcts<Game>::expand(const Game& game, node<Move>* n)
{
  unsigned int count = n->get_statistics().count;
  if (count >= nb_visits_before_expansion && !n->test_and_set())
    {
      std::vector<Move> moves = game.get_moves();
      unsigned int nb_children = moves.size();
      node<Move>* children = allocate_node(nb_children);
      n->set_moves(allocate_move(nb_children));
      for (unsigned int i = 0; i < nb_children; ++i)
        {
          node<Move>* child = children + i;
          child->get_statistics_ref().count = 1;
          child->get_statistics_ref().value = 0;
        }
      n->set_moves(moves);
      n->set_children(children);
      n->set_number_of_children(nb_children);
    }
}

template <typename Game>
void mcts<Game>::think(Game game)
{
  using namespace std;
  const int VIRTUAL_LOSS = 4;
  const chrono::steady_clock::time_point start = chrono::steady_clock::now();
  chrono::steady_clock::time_point now;
  mt19937& generator = generators[omp_util::get_thread_num()];
  Game game_backup = game;
  vector<node<Move>*> visited(200);
  unsigned int nb_iter = 0;
  do
    {
      int size = 1;
      node<Move>* current = root;
      visited[0] = current;
      current->add_virtual_loss(VIRTUAL_LOSS);
      while (!game.end_of_game() && !current->is_leaf() && !current->is_proven())
        {
          current = select(game, generator, current);
          current->add_virtual_loss(VIRTUAL_LOSS);
          visited[size++] = current;
        }
      int game_value = 0;
      if (current->is_proven())
        {
          if (current->is_won()) game_value = 1;
          else game_value = -1;
        }
      else if (game.end_of_game())
        {
          game_value = game.value_for_current_player();
          if (game_value > 0)
            {
              game_value = 1;
              current->set_won();
            }
          else if (game_value < 0)
            {
              game_value = -1;
              current->set_lost();
              if (size > 1) visited[size - 2]->set_won();
            }
        }
      else
        {
          uint8_t player = game.current_player();
          expand(game, current);
          game.playout(generator);
          int v = game.value(player);
          if (v > 0) game_value = 1;
          else if (v < 0) game_value = -1;
        }
      for (int i = size - 1; i >= 0; --i)
        {
          visited[i]->update(game_value, VIRTUAL_LOSS);
          game_value = -game_value;
        }
      game = game_backup;
      ++nb_iter;
      if ((nb_iter & 0xFF) == 0) now = chrono::steady_clock::now();
    }
  while ((nb_iter & 0xFF) != 0 || now < start + this->thinking_time);
}

template <typename Game>
typename Game::Move mcts<Game>::select_move()
{
  using namespace std;
  if (!root->is_proven())
    {
#pragma omp parallel
      think(game);
    }
  //std::ofstream ofs ("graph.gv", ofstream::out);
  //    util::display_node::node_to_dot(ofs, root, 2, 1000);
  display_node<Move>::node_to_ascii(cout, root, 1);
  cout << root->size() << endl;
  unsigned int best_count_so_far = 0;
  uint16_t nb_children = root->get_number_of_children();
  uniform_int_distribution<uint16_t> d(0, nb_children - 1);
  uint16_t k = d(generators[0]);
  uint16_t best_move_so_far = k;
  node<Move>* children = root->get_children();
  unsigned int c;
  for (uint16_t i = 0; i < nb_children; ++i)
    {
      node<Move> *child = children + k;
      if (child->is_lost())
        {
          best_move_so_far = k;
          break;
        }
      c = children[k].get_statistics().count;
      if (c > best_count_so_far)
        {
          best_count_so_far = c;
          best_move_so_far = k;
        }
      ++k;
      k &= ~(-(k == nb_children));
    }
  return root->get_moves()[best_move_so_far];
}

const int PRUNNING = 20;

template <typename Game>
void mcts<Game>::last_moves(int computer, int other)
{
  if (root->is_leaf() || root->get_children()[computer].is_leaf())
    {
      memory_allocator.clear();
      root = allocate_node(1);
    }
  else
    {
      root = move(&root->get_children()[computer].get_children()[other], PRUNNING);
    }
}

template <typename Game>
void mcts<Game>::last_move(int m)
{
  if (root->is_leaf())
    {
      memory_allocator.clear();
      root = allocate_node(1);
    }
  else
    {
      root = move(&root->get_children()[m], PRUNNING);
    }
}

template <typename Game>
node<typename Game::Move>* mcts<Game>::allocate_node(unsigned int size)
{
  return (node<Move>*)memory_allocator.allocate(size * sizeof(node<Move>));
}

template <typename Game>
typename Game::Move* mcts<Game>::allocate_move(unsigned int size)
{
  return (Move*)memory_allocator.allocate(size * sizeof(Move));
}

template <typename Game>
void mcts<Game>::copy(node<Move>* n1, node<Move>* n2, int prunning)
{
  n2->set_statistics(n1->get_statistics());
  if (n1->get_statistics().count < prunning && n1->get_won() == 0) return;
  n2->set_won(n1->get_won());
  unsigned int nb_children = n1->get_number_of_children();
  n2->set_number_of_children(nb_children);
  if (nb_children == 0) return;
  n2->set_moves((Move*)memory_allocator.allocate_unsafe(nb_children * sizeof(Move)));
  n2->set_children((node<Move>*)memory_allocator.allocate_unsafe(nb_children * sizeof(node<Move>)));
  node<Move>* children1 = n1->get_children();
  node<Move>* children2 = n2->get_children();
  for (unsigned int i = 0; i < nb_children; ++i)
    {
      n2->get_moves()[i] = n1->get_moves()[i];
      copy(children1 + i, children2 + i, prunning);
    }
}

template <typename Game>
node<typename Game::Move>* mcts<Game>::move(node<Move>* root, int prunning)
{
  //  std::cout << root->size() << std::endl;
  node<Move>* r = allocate_node(1);
  copy(root, r, prunning);
  memory_allocator.clear();
  node<Move>* res = allocate_node(1);
  copy(r, res);
  //  std::cout << res->size() << std::endl;
  return res;
}
