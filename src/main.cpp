#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <iostream>
#include "blokus.hpp"
#include "shape.hpp"
#include "mcts.hpp"

using namespace std;

////  Scale  ////
const double scale_background = 0.90; 
static const double size_board = 64.1*scale_background; 
const double scale = 1.39*scale_background;

////  Pieces   ////
sf::Sprite v[30];
bool fixationPiece[31] = {false}; 
bool debut = true;

void loadPosition()
{
  for (int j = 0; j < 3; j++)
  {
    for (int i = 0; i < 5; i++)
    {
      v[i + 5 * j].setTextureRect(sf::IntRect(i * 141, j * 141, 141, 141));
      v[i + 5 * j].setPosition(j * 220 + 850, i * 180);
      v[i + 5 * j].setScale(scale, scale);
      v[(i + 5 * j) + 15].setTextureRect(sf::IntRect(i * 141, j * 141, 141, 141));
      v[(i + 5 * j) + 15].setPosition(1500,50);
      v[(i + 5 * j) + 15].setScale(scale, scale);
      v[(i + 5 * j) + 15].setColor(sf::Color::Black);
    }
  }
}

template <typename Game>
int human_run(sf::RenderWindow& window, sf::Sprite background, Game& g)
{
 
  bool rotate = false, flip = false, isMove = false, affiche = false, valide=false, play=true;
  float dx = 0, dy = 0, oldX = 0, oldY = 0;
  int n = 0, nb_rot = 0, nb_flip = 0, x, y, pred = 30, res = -1;
  
  sf::Vector2f newPos;

  ////  PASS  ////
  std::vector<blokus::Move> m = g.get_moves();
  if(m[0]==blokus::PASS)
  {
    g.make_move(m[0]);
    play = false;
    res = 0;
  }

  while(play)
  {
    ////  Event  ////
    sf::Vector2i pos = sf::Mouse::getPosition(window);
    sf::Event e;
    while (window.pollEvent(e))
    {  
      if (e.type == sf::Event::Closed)
      {
          window.close();
      }

      if (e.type == sf::Event::MouseButtonPressed)
      {
        if (e.mouseButton.button == sf::Mouse::Left)
        {
          for (int i = 0; i < 15; i++)
          {
            if (v[i].getGlobalBounds().contains(pos.x, pos.y)  && !fixationPiece[i])
            {
              if (!fixationPiece[pred] && i!=pred)
              {
                v[pred].setRotation(0);
                v[pred].setScale(scale, scale);
                nb_rot = 0;
                nb_flip = 0;
              }
              isMove = true;
              n = i;
              dx = pos.x - v[i].getPosition().x;
              dy = pos.y - v[i].getPosition().y;
              oldX = v[i].getPosition().x;
              oldY = v[i].getPosition().y;
              pred = i;
            }
          }
        }
        
        if (e.mouseButton.button == sf::Mouse::Right)
        {  
          for (int i = 0; i < 15; i++)
          {
            if (v[i].getGlobalBounds().contains(pos.x, pos.y)  && !fixationPiece[i])
            {
              if (!fixationPiece[pred] && i!=pred)
              {
                v[pred].setRotation(0);
                v[pred].setScale(scale, scale);
                nb_rot = 0;
                nb_flip = 0;
              }
              n = i;
              dx = pos.x - v[i].getPosition().x;
              dy = pos.y - v[i].getPosition().y;
              pred = i;
              isMove = true;
            }
          }
        }
      }

      if (e.type == sf::Event::MouseButtonReleased && isMove)
      {  
        if (e.mouseButton.button == sf::Mouse::Left)
        {
          sf::Vector2f p = v[n].getPosition() + sf::Vector2f(size_board / 2, size_board / 2);
          newPos = sf::Vector2f(int(size_board * int(p.x / size_board)), int(size_board * int(p.y / size_board)));
          v[n].setPosition(newPos);
          isMove = false;
          affiche = true;
        }
        if (e.mouseButton.button == sf::Mouse::Right)
        {
          isMove = false;
        }
      }

      if (e.type == sf::Event::KeyPressed && !fixationPiece[n])
      {
        if (e.key.code == sf::Keyboard::Up)
        {
          rotate = true;
        }

        if (e.key.code == sf::Keyboard::Down)
        {
          flip = true;
        }
      }
    }

    ////  Rotate  ////
    if (rotate)
    {
      v[n].setRotation(v[n].getRotation() + 90);
      rotate = false;
      nb_rot = (nb_rot + 1) % 4;
    }

    ////  Flip  ////
    if (flip)
    {
      if (v[n].getScale().x > 0)
      {
        v[n].setScale(-scale, scale);
      }
      else
      {
        v[n].setScale(scale, scale);
      }
      nb_flip = (nb_flip + 1) % 2;
      flip = false;
    }

    ////  Move  ////
    if (isMove)
    {
      v[n].setPosition(pos.x - dx, pos.y - dy);
    }

    ////  Board  ////
    x = (newPos.x) / int(size_board);
    y = (newPos.y) / int(size_board);
    if (affiche)
    {
      ////  MAJ XY  ////
      g.maj_XY(&x,&y,nb_rot,nb_flip);
      bool b = true;
      for(int i=0; i<SIZE; i++)
      {
        for(int j=0; j<SIZE; j++)
        {
          if(PIECES[n][nb_rot+4*nb_flip][i][j]==1 && b)
          {
            y += i;
            b = false;
          }
        }
      }

      b = true;
      for(int i=0; i<SIZE; i++)
      {
        for(int j=0; j<SIZE; j++)
        {
          if(PIECES[n][nb_rot+4*nb_flip][j][i]==1 && b)
          {
            x += i;
            b = false;
          }
        }
      }

      for(unsigned i=0; i<g.get_moves().size(); i++)
      {
        if(blokus::patterns[m[i].pattern].piece==n && blokus::patterns[m[i].pattern].rotation==nb_rot+4*nb_flip && m[i].square==y*14+x)
        {
          g.make_move(m[i]);
          play = false;
          valide = true;
          res = i;
          fixationPiece[n] = true;
        }
      }

      if(!valide)
      {
          v[n].setPosition(oldX, oldY);
          v[n].setRotation(0);
          v[n].setScale(scale, scale);
          nb_rot = 0;
          nb_flip = 0;
      }
      affiche = false;
    }

    ////  Draw  ////
    window.clear(sf::Color::White);
    background.setScale(scale_background,scale_background);
    window.draw(background);
    for (int i = 0; i < 15; i++)
    {
      window.draw(v[i]);
    }
    for(int i = 15; i<30; i++)
    {
      if(fixationPiece[i])
      {
        window.draw(v[i]);
      }
    }
    window.display();
  }
  return res;
}

template <typename Game>
void computer_run(sf::RenderWindow& window, sf::Sprite background, blokus::Move& move,Game& g)
{
  bool play = true;

  if (move==blokus::PASS)
  {
    play = false;
  }

  while(play)
  {
    sf::Event e;
    while (window.pollEvent(e))
    {
      if (e.type == sf::Event::Closed)
      {
          window.close();
      }
    }

    //// Move ////
    int n = blokus::patterns[move.pattern].piece + 15;
    int nb_rot = blokus::patterns[move.pattern].rotation % 4;
    int nb_flip = blokus::patterns[move.pattern].rotation / 4;
    int x = move.square % 14;
    int y = move.square / 14;
    fixationPiece[n] = true;

    ////  Rotate  ////
    while(nb_rot != 0)
    {
      v[n].setRotation(v[n].getRotation() + 90);
      nb_rot--;
    }

    ////  Flip  ////
    while(nb_flip != 0)
    {
      if (v[n].getScale().x > 0)
      {
        v[n].setScale(-scale, scale);
      }
      else
      {
        v[n].setScale(scale, scale);
      }
      nb_flip--;
    }

    //// MAJ XY ////
    nb_rot = blokus::patterns[move.pattern].rotation % 4;
    nb_flip = blokus::patterns[move.pattern].rotation / 4;
    g.maj_XY2(&x,&y,nb_rot,nb_flip);
    
    bool b = true;
    for(int i=0; i<SIZE; i++)
    {
      for(int j=0; j<SIZE; j++)
      {
        if(PIECES[n-15][nb_rot+4*nb_flip][i][j]==1 && b)
        {
          y -= i;
          b = false;
        }
      }
    }

    b = true;
    for(int i=0; i<SIZE; i++)
    {
      for(int j=0; j<SIZE; j++)
      {
        if(PIECES[n-15][nb_rot+4*nb_flip][j][i]==1 && b)
        {
          x -= i;
          b = false;
        }
      }
    }

    v[n].setPosition(int(x * size_board), int(y * size_board));
    play = false;
  }
}

template <typename Game>
int move_to_index(const Game &g, const typename Game::Move &m)
{
  vector<typename Game::Move> moves = g.get_moves();
  int nb_moves = moves.size();
  for (int i = 0; i < nb_moves; ++i)
  {
    if (moves[i] == m)
      return i;
  }
  return -1;
}

template <typename Game>
int select_move(Game &g)
{
  cout << "your move: ";
  typename Game::Move m;
  cin >> m;
  int res = move_to_index(g, m);
  g.make_move(m);
  return res;
}

int select_move(blokus &g)
{
  g.print_possible_moves(cout);
  cout << "your move: ";
  int idx;
  cin >> idx;
  g.make_move(g.get_moves()[idx]);
  return idx;
}

template <typename Game>
void play_vs_machine(Game g)
{
  ////  Main window  ////
  sf::RenderWindow window(sf::VideoMode(1700, 900), "Blokus", sf::Style::Close);
  window.setVerticalSyncEnabled(true);
  window.setPosition(sf::Vector2i(50,50));
  sf::Texture t1, t2;
  t1.loadFromFile("images/pieces.png");
  t2.loadFromFile("images/board.png");
  sf::Sprite s(t1), background(t2);

  ////  Set texture pieces  ////
  for (int i = 0; i < 30; i++)
  {
    v[i].setTexture(t1);
  }

  ////  Load pieces  ////
  loadPosition();
  fixationPiece[30] = true;

  while (window.isOpen())
  {
    ////  Event  ////
    sf::Event e;
    while (window.pollEvent(e))
    {
      if (e.type == sf::Event::Closed)
      {
          window.close();
      }
    }

    ////  Draw  ////
    window.clear(sf::Color::White);
    background.setScale(scale_background,scale_background);
    window.draw(background);
    for (int i = 0; i < 15; i++)
    {
      window.draw(v[i]);
    }
    for(int i = 15; i<30; i++)
    {
      if(fixationPiece[i])
      {
        window.draw(v[i]);
      }
    }
    window.display();

    auto the_turk = make_mcts(g, 5000, 0.35, 8);
    cout << "play one game" << std::endl;
    string ans;

    if(debut)
    {
      ////  Second window  ////
      sf::RenderWindow choix(sf::VideoMode(700, 300), "choix", sf::Style::None);
      choix.setPosition(sf::Vector2i(200,200));
      sf::Texture question;
      question.loadFromFile("images/question.png");
      sf::Sprite s;
      sf::RectangleShape rectangle1(sf::Vector2f(200, 100));
      rectangle1.setFillColor(sf::Color::Cyan);
      rectangle1.setPosition(50,200);
      sf::RectangleShape rectangle2(sf::Vector2f(200, 100));
      rectangle2.setFillColor(sf::Color::Red);
      rectangle2.setPosition(450,200);
      s.setTexture(question);
      s.setTextureRect(sf::IntRect(0, 0, 500, 100));
      s.setPosition(100,10);
      
      while(choix.isOpen())
      {
        ////  Event  ////
        sf::Vector2i pos = sf::Mouse::getPosition(choix);
        sf::Event e;
        while (choix.pollEvent(e))
        {
          if (e.type == sf::Event::MouseButtonPressed)
          { 
            if (e.mouseButton.button == sf::Mouse::Left)
            {
              if (rectangle1.getGlobalBounds().contains(pos.x, pos.y))
              {
                ans = "h";
                choix.close();
              } else
              {
              if(rectangle2.getGlobalBounds().contains(pos.x, pos.y))
              {
                ans = "c";
                choix.close();
              }
              } 
            }
          }   
        }

        ////  Draw  ////
        choix.clear(sf::Color::White);
        choix.draw(rectangle1);
        choix.draw(rectangle2);
        choix.draw(s);
        choix.display();
      }
      debut = false;
    }

    cout << g << endl;
    int human_last_move = -1, computer_last_move = -1;
    while (!g.end_of_game())
    {
      if ((ans == "h" && g.current_player() == 0) || (ans == "c" && g.current_player() == 1))
      {
        human_last_move = human_run(window, background, g);
      }
      else
      {
        if (human_last_move != -1 && computer_last_move != -1)
        {
          the_turk.last_moves(computer_last_move, human_last_move);
        }
        typename Game::Move move = the_turk.select_move();
        computer_last_move = move_to_index(g, move);
        cout << "computer move: " << move << endl;
        g.make_move(move);
        computer_run(window, background, move, g);
      }
      cout << g << endl;
    }

    ////  End  ////
    if ((g.won(0) && ans == "c") || (g.won(1) && ans=="h")){
      window.close();
      sf::RenderWindow fin(sf::VideoMode(300, 200), "Game Over !", sf::Style::Close);
      fin.setPosition(sf::Vector2i(400,200));
      sf::Texture q;
      q.loadFromFile("images/computer_win.png");
      sf::Sprite s1;
      s1.setTexture(q);
      s.setTextureRect(sf::IntRect(0, 0, 300, 200));
      
      while(fin.isOpen())
      {
        sf::Event e2;
        while (fin.pollEvent(e2))
        {
          if (e2.type == sf::Event::Closed)
          { 
            fin.close();  
          } 
        }
        fin.draw(s1);
        fin.display();
      }   
    }
    else if ((g.won(1) && ans=="c") || (g.won(0) && ans=="h"))
    {
      window.close();
      sf::RenderWindow fin2(sf::VideoMode(300, 200), "You win !", sf::Style::Close);
      fin2.setPosition(sf::Vector2i(400,200));
      sf::Texture q;
      q.loadFromFile("images/win.png");
      sf::Sprite s1;
      s1.setTexture(q);
      s1.setTextureRect(sf::IntRect(0, 0, 300, 200));
      
      while(fin2.isOpen())
      {
        sf::Event e2;
        while (fin2.pollEvent(e2))
        {
          if (e2.type == sf::Event::Closed)
          { 
            fin2.close();
          } 
        }
        fin2.draw(s1);
        fin2.display();
      }
    }
    else
    {
      window.close();
      sf::RenderWindow fin2(sf::VideoMode(300, 200), "Draw", sf::Style::Close);
      fin2.setPosition(sf::Vector2i(400,200));
      sf::Texture q;
      q.loadFromFile("images/draw.png");
      sf::Sprite s1;
      s1.setTexture(q);
      s.setTextureRect(sf::IntRect(0, 0, 300, 200));

      while(fin2.isOpen())
      {
        sf::Event e2;
        while (fin2.pollEvent(e2))
        {
          if (e2.type == sf::Event::Closed)
          { 
            fin2.close();
          } 
        }
        fin2.draw(s1);
        fin2.display();
      }
    }
  }
}

template <typename Game>
void play_vs_human(Game g)
{
  cout << "play one game" << std::endl;
  while (!g.end_of_game())
  {
    cout << g << endl;
    select_move(g);
  }
  if (g.won(0))
    cout << "first player won";
  else if (g.won(1))
    cout << "second player won";
  else
    cout << "draw";
  cout << endl;
}

int main()
{
  try
  {
    play_vs_machine(blokus{});
  }
    catch (char const* e)
  {
    cout << e << endl;
  }
  return 0;
}