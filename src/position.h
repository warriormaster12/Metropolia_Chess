#pragma once

#include "chess.h"
#include "move.h"
#include <vector>
#include <array>

class Position {
public: 
  void clear();
  void move(const Move& p_move);

  //Homework
  //Print ascii board.
  void render_board();
  std::vector<Move> get_all_raw_moves(int player) const;
  vector<Move> get_rook_raw_move(int row, int col, int player) const;
  vector<Move> get_bishop_raw_move(int row, int col, int player) const;
  vector<Move> get_queen_raw_move(int row, int col, int player) const;
  vector<Move> get_knight_raw_move(int row, int col, int player) const;
  vector<Move> get_king_raw_move(int row, int col, int player) const;
  vector<Move> get_pawn_raw_move(int row, int col, int player) const;
private:
  vector<Move> get_directional_raw_move(std::array<int, 2> position, std::array<int, 2> direction, int player) const;
  bool check_collision(int row_now, int col_now, int row, int col,int player,vector<Move>& out) const;
  // board pieces cols and rows. Example:
  //
  // [0][0] : left upper corner ("a8")
  // [7][0] : left lower corner ("a1")
  // [7][7] : right lower corner ("h1")
  //
  int m_board[8][8] = {
    {bR, bN, bB, bQ, bK, bB, bN, bR},
    {bP, bP, bP, bP, bP, bP, bP, bP}, 
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {wP, wP, wP, wP, wP, wP, wP, wP},
    {wR, wN, wB, wQ, wK, wB, wN, wR}, 
  };

  int m_movingturn = WHITE;
};