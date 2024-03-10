#pragma once

#include "chess.h"
#include "move.h"
#include <vector>
#include <array>

struct MinmaxValue {
  float value;
  Move move;

  MinmaxValue(float p_value, Move p_move) {
    value = p_value;
    move = p_move;
  }
};

class Position {
public: 
  void clear();
  void move(const Move& p_move);
  void end_turn();
  bool can_promote(const Move& p_move);
  void promote(int *end_pos, int chess_piece);
  void render_board();
  std::array<std::array<int, 8>, 8> get_board() const { return m_board;}
  void render_legal_moves(const vector<Move>& p_moves);
  std::vector<Move> get_all_raw_moves(int player) const;
  void get_chess_piece(int chess_piece, int& row, int& col) const;
  bool is_square_threatened(int row, int col, int threatening_player) const;
  vector<Move> get_rook_raw_move(int row, int col, int player) const;
  vector<Move> get_bishop_raw_move(int row, int col, int player) const;
  vector<Move> get_queen_raw_move(int row, int col, int player) const;
  vector<Move> get_knight_raw_move(int row, int col, int player) const;
  vector<Move> get_king_raw_move(int row, int col, int player) const;
  vector<Move> get_pawn_raw_move(int row, int col, int player) const;
  vector<Move> get_castlings(int player) const;
  vector<Move> generate_legal_moves() const;
  int get_moving_player() const {return m_movingturn;}

  float score_end_result(const int p_depth) const; 

  float evaluate() const;

  float material() const;

  float mobility() const;

  MinmaxValue minmax(int depth);

  MinmaxValue minmax_alphabeta(int depth, MinmaxValue alpha, MinmaxValue beta);

private:
  vector<Move> get_directional_raw_move(std::array<int, 2> position, std::array<int, 2> direction, int player) const;
  bool check_collision(int row_now, int col_now, int row, int col,int player,vector<Move>& out) const;
  // board pieces cols and rows. Example:
  //
  // [0][0] : left upper corner ("a8")
  // [7][0] : left lower corner ("a1")
  // [7][7] : right lower corner ("h1")
  //
  std::array<std::array<int, 8>, 8> m_board = {{
    {bR, bN, bB, bQ, bK, bB, bN, bR},
    {bP, bP, bP, bP, bP, bP, bP, bP}, 
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {NA, NA, NA, NA, NA, NA, NA, NA},
    {wP, wP, wP, wP, wP, wP, wP, wP},
    {wR, wN, wB, wQ, wK, wB, wN, wR}, 
  }};

  int m_movingturn = WHITE;

  bool m_white_short_castling_allowed = true;
  bool m_white_long_castling_allowed = true;
  bool m_black_short_castling_allowed = true;
  bool m_black_long_castling_allowed = true;

  int m_doublestep_on_row = -1;

  int m_en_passant_col[2] = { -1, -1 };
};