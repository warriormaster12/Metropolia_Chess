#pragma once
#include "string"

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