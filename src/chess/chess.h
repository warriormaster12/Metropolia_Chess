#pragma once
#include <string>
#include <array>

// Players.
enum { WHITE, BLACK };

// constants for different chess pieces
enum {
  // white
  wR,
  wN,
  wB,
  wQ,
  wK,
  wP,
  // black
  bR,
  bN,
  bB,
  bQ,
  bK,
  bP,
  NA // Not available (empty cell)
};

std::string chess_piece_to_string(int p_index);
int get_chess_piece_color(int p_index);
bool is_promotable(int p_piece, int p_destination_row);

float get_square_score(std::array<int, 2> p_position, int p_chess_piece);