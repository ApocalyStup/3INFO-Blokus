#pragma once

#include "node.hpp"
#include <iostream>
#include <limits>
#include <iomanip>

template <typename Move>
class display_node
{
public:
  static void node_to_ascii(std::ostream& os, const node<Move>* n, int depth = std::numeric_limits<int>::max(), int prunning = 0);
  static void node_to_dot(std::ostream& os, const node<Move>* n, int depth = std::numeric_limits<int>::max(), int prunning = 0);
private:
  static void node_to_ascii(std::ostream& os, std::string prefix, const node<Move>* n, int depth, int prunning);
  static void children_to_ascii(std::ostream& os, std::string prefix, unsigned int nb_children,
                                const Move* moves, const node<Move>* children, int depth, int prunning);
  static int node_to_dot(std::ostream& os, int id, const Move* m, const node<Move>* n, int depth, int prunning);
};

template <typename Move>
void display_node<Move>::node_to_ascii(std::ostream& os, const node<Move>* n, int depth, int prunning)
{
  node_to_ascii(os, "", n, depth, prunning);
  os << std::endl;
}

template <typename Move>
void display_node<Move>::node_to_dot(std::ostream& os, const node<Move>* n, int depth, int prunning)
{
  using namespace std;
  os << "digraph {" << endl;
  node_to_dot(os, 0, n, depth, prunning);
  os << "}" << endl;
}

template <typename Move>
int display_node<Move>::node_to_dot(std::ostream& os, int id, const Move* m, const node<Move>* n, int depth, int prunning)
{
  using namespace std;
  stringbuf buffer;
  ostream o(&buffer);
  if (n->is_won()) o << "\"won\"";
  else if (n->is_lost()) o << "\"lost\"";
  else
    {
      o << "\"";
      if (m != nullptr) o << *m << "\\n";
      o << n->get_statistics().count << "\\n" << setprecision(2) << ((double)n->get_statistics().value / n->get_statistics().count) << "\"";
    }
  string label = buffer.str();
  os << id << "[label=" << label << "]" << endl;
  if (depth == 0 || n->get_statistics().count < prunning || n->get_number_of_children() == 0) return id + 1;
  int cpt = id + 1;
  for (int i = 0; i < n->get_number_of_children(); ++i)
    {
      os << id << "->" << cpt << endl;
      cpt = node_to_dot(os, cpt, n->get_moves() + i, n->get_children() + i, depth - 1, prunning);
    }
  return cpt + 1;
}

template <typename Move>
void display_node<Move>::node_to_ascii(std::ostream& os, std::string prefix, const node<Move>* n, int depth, int prunning)
{
  using namespace std;
  string s;
  s = n->get_statistics().to_string();
  if (n->is_won()) s += "won";
  else if (n->is_lost()) s += "lost";
  else s = n->get_statistics().to_string();
  os << s;
  unsigned int w = s.size();
  string new_prefix = prefix + string(w + 1, ' ');
  if (depth == 0 || n->get_statistics().count < prunning || n->get_number_of_children() == 0) return;
  if (n->get_number_of_children() == 1)
    {
      os << "--" << n->get_moves()[0] << ": ";
      node_to_ascii(os, new_prefix + "  ", n->get_children(), depth - 1, prunning);
      return;
    }
  os << "-";
  children_to_ascii(os, new_prefix, n->get_number_of_children(), n->get_moves(), n->get_children(), depth, prunning);
}

template <typename Move>
void display_node<Move>::children_to_ascii(std::ostream& os, std::string prefix, unsigned int nb_children,
                                           const Move* moves, const node<Move>* children, int depth, int prunning)
{
  using namespace std;
  os << "+-" << moves[0] << ": ";
  node_to_ascii(os, prefix + "| ", children, depth - 1, prunning);
  os << endl;
  os << prefix;
  for (unsigned int i = 1; i < nb_children - 1; ++i)
    {
      os << "|-" << moves[i] << ": ";
      node_to_ascii(os, prefix + "| ", children + i, depth - 1, prunning);
      os << endl;
      os << prefix;
    }
  os << "`-" << moves[nb_children - 1] << ": ";
  node_to_ascii(os, prefix + "  ", children + nb_children - 1, depth - 1, prunning);
}
