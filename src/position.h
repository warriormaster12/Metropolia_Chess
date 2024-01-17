#pragma once

#include "chess.h"
#include "move.h"

class Position {
public: 
  void clear();
  void move(const Move& p_move);

  //Homework
  //Print ascii board.
private:
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