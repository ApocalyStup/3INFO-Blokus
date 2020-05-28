#ifndef BLOKUS_HPP
#define BLOKUS_HPP

#include <string>
#include <array>
#include <vector>
#include <random>
#include <iostream>
#include <bitset>
#include <array>

class blokus
{
public:
    struct Move {
        uint8_t pattern;
        uint8_t square;
        constexpr Move(uint8_t p = 255, uint8_t sq = 255) 
            : pattern(p), square(sq) {} 
    };

private:
    static constexpr int EMPTY = 0;
    static constexpr int FILLED = 1;
    static constexpr int BORDER = 2;
    static constexpr int CORNER = 3;
    static constexpr int WIDTH = 14;
    static constexpr int HEIGHT = 14;    
    static constexpr int SQUARES = 196;
    static constexpr int CELLS = 5;
    static constexpr int PIECES = 15;
    static constexpr uint32_t ALL_PIECES = (1 << PIECES) - 1;
    static constexpr int PATTERNS = 120;
    static constexpr int PLAYERS = 2;
    static constexpr int MOVES = 4096;
    static constexpr int INIT_SQUARE0 = 60;
    static constexpr int INIT_SQUARE1 = 135;

    static constexpr int square(int x, int y) {
        return y * WIDTH + x;
    }

    struct placement {
        std::bitset<SQUARES> filled;
        std::bitset<SQUARES> border;
        std::bitset<SQUARES> corner;
        bool valid;
    };
    struct pattern {
        int piece;
        int rotation;
        int width;
        int height;
        int cells;
        std::string name;
        int data[7][7];
    };
    static const std::vector<std::vector<std::vector<int>>> pattern_squares;
    static const std::vector<std::vector<placement>> placements;
    int player = 0;
    std::array<uint32_t, PLAYERS> pieces{};
    std::array<int, PLAYERS> score{};
    std::array<std::bitset<SQUARES>, PLAYERS> filled{};
    std::array<std::bitset<SQUARES>, PLAYERS> border{};
    std::array<std::bitset<SQUARES>, PLAYERS> corner{};
    static std::vector<uint8_t> get_bits(const std::bitset<SQUARES>&);
    Move random_move(std::mt19937& engine);
    int heuristic() const;
    static constexpr char x_to_letter(int x) {
        return "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[x];
    }
public:
    blokus();
    bool end_of_game() const;
    int current_player() const;
    int value(int player) const;
    int value_for_current_player() const;
    bool won(int player) const;
    void make_move(const Move& col);
    std::vector<Move> get_moves() const;
    void playout(std::mt19937& engine);
    std::array<bool, PLAYERS> passes{};
    static const Move PASS;
    std::ostream& print_possible_moves(std::ostream&, int number_by_row = 9) const;
    friend std::ostream& operator<<(std::ostream& os, const blokus& b);
    friend std::istream& operator>>(std::istream& is, Move& m);
    friend std::ostream& operator<<(std::ostream& os, const blokus::Move& m);
    static const std::vector<pattern> patterns;
    void maj_XY(int* x, int* y, int nb_rot, int nb_flip);
    void maj_XY2(int* x, int* y, int nb_rot, int nb_flip);
};

std::ostream& operator<<(std::ostream& os, const blokus& b);
std::istream& operator>>(std::istream& is, blokus::Move& m);
bool operator==(const blokus::Move& m1, const blokus::Move& m2);
std::ostream& operator<<(std::ostream& os, const blokus::Move& m);

#endif