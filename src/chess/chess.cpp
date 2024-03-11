#include "chess.h"

std::string chess_piece_to_string(int p_index) {
    std::string out = "";
    switch (p_index) {
      case wR: {
        out = "wR";
        break;
      }
      case wN: {
        out = "wN";
        break;
      }
      case wB: {
        out = "wB";
        break;
      }
      case wQ: {
        out = "wQ";
        break;
      }
      case wK: {
        out = "wK";
        break;
      }
      case wP: {
        out = "wP";
        break;
      }
      case bR: {
        out = "bR";
        break;
      }
      case bN: {
        out = "bN";
        break;
      }
      case bB: {
        out = "bB";
        break;
      }
      case bQ: {
        out = "bQ";
        break;
      }
      case bK: {
        out = "bK";
        break;
      }
      case bP: {
        out = "bP";
        break;
      }
    }
    return out;
}

int get_chess_piece_color(int p_index) {
  if (p_index < 6) {
    return WHITE;
  } else {
    return BLACK;
  }
}

bool is_promotable(int p_piece, int p_destination_row) {
  return p_piece == wP && p_destination_row == 0 || p_piece == bP && p_destination_row == 7;
}

const float WHITE_PAWN_SCORE[8][8] = {
    {0.0, 0.0, 0.0, 0.0, 0.0,0.0, 0.0, 0.0},
    {5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0, 5.0}, 
    {1.0, 1.0, 2.0, 3.0, 3.0, 2.0, 1.0, 1.0},
    {0.5, 0.5, 1.0, 2.5, 2.5, 1.0, 0.5, 0.5},
    {0.0, 0.0, 0.0, 2.0, 2.0, 0.0, 0.0, 0.0},
    {0.5, -0.5, -1.0, 0.0, 0.0, -1.0, -0.5, 0.5},
    {0.5, 1.0, 1.0, -2.0, -2.0, 1.0, 1.0, 0.5},
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, 
  };
const float WHITE_KNIGHT_SCORE[8][8] = {
    {-5.0, -4.0, -3.0, -3.0, -3.0,-3.0, -4.0, -5.0},
    {-4.0, -2.0, 0.0, 0.0, 0.0, 0.0, -2.0, -4.0}, 
    {-3.0, 0.0, 1.0, 1.5, 1.5, 1.0, 0.0, -3.0},
    {-3.0, 0.5, 1.5, 2.0, 2.0, 1.5, 0.5, -3.0},
    {-3.0, 0.0, 1.5, 2.0, 2.0, 1.5, 0.0, -3.0},
    {-3.0, 0.5, 1.0, 1.5, 1.5, 1.0, 0.5, -3.0},
    {-4.0, -2.0, 0.0, 0.5, 0.5, 0.0, -2.0, -4.0},
    {-5.0, -4.0, -3.0, -3.0, -3.0, -3.0, -4.0, -5.0}, 
  };
const float WHITE_BISHOP_SCORE[8][8] = {
    {-2.0, -1.0, -1.0, -1.0, -1.0,-1.0, -1.0, -2.0},
    {-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0}, 
    {-1.0, 0.0, 0.5, 1.0, 1.0, 0.5, 0.0, -1.0},
    {-1.0, 0.5, 0.5, 1.0, 1.0, 0.5, 0.5, -1.0},
    {-1.0, 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, -1.0},
    {-1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0},
    {-1.0, 0.5, 0.0, 0.0, 0.0, 0.0, 0.5, -1.0},
    {-2.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -2.0}, 
  };
const float WHITE_KING_SCORE[8][8] = {
    {-3.0,-4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0},
    {-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0}, 
    {-3.0, -4.0, -4.0, -5.0, -5.0, -4.0, -4.0, -3.0},
    {-3.0, -4.0, -4.0, -5.0, 5.0, -4.0, -4.0, -3.0},
    {-2.0, -3.0, -3.0, -4.0, -4.0,-3.0, -3.0, -2.0},
    {-1.0, -2.0, -2.0, -2.0, -2.0, -2.0, -2.0, -1.0},
    {2.0, 2.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0},
    {2.0, 3.0, 1.0, 0.0, 0.0, 1.0, 3.0, 2.0}, 
  };
const float WHITE_QUEEN_SCORE[8][8] = {
    {-2.0,-1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0},
    {-1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0}, 
    {-1.0, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -1.0},
    {-0.5, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -0.5},
    {0.0, 0.0, 0.5, 0.5, 0.5, 0.5, 0.0, -0.5},
    {-1.0, 0.5, 0.5, 0.5, 0.5, 0.5, 0.0, -1.0},
    {-1.0, 0.0, 0.5, 0.0, 0.0, 0.0, 0.0, -1.0},
    {-2.0, -1.0, -1.0, -0.5, -0.5, -1.0, -1.0, -2.0}, 
  };
const float WHITE_ROOK_SCORE[8][8] = {
    {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.5, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5},
    {-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5},
    {-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5},
    {-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5},
    {-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5},
    {-0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.5},
    {0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0},
  };

float get_square_score(std::array<int, 2> p_position, int p_chess_piece) {
  int sign = get_chess_piece_color(p_chess_piece) == WHITE ? 1 : -1;
  if (get_chess_piece_color(p_chess_piece) == BLACK) {
    p_position = {7-p_position[0], 7-p_position[1]};
  }
  float out_value = 0.0;
  switch (p_chess_piece) {
    case wP:
    case bP: {
      out_value = WHITE_PAWN_SCORE[p_position[0]][p_position[1]] * sign;
      break;
    }
    case wB:
    case bB: {
      out_value = WHITE_BISHOP_SCORE[p_position[0]][p_position[1]] * sign;
      break;
    }
    case wR:
    case bR: {
      out_value = WHITE_ROOK_SCORE[p_position[0]][p_position[1]] * sign;
      break;
    }
    case wN:
    case bN: {
      out_value = WHITE_KNIGHT_SCORE[p_position[0]][p_position[1]] * sign;
      break;
    }
    case wQ:
    case bQ: {
      out_value = WHITE_QUEEN_SCORE[p_position[0]][p_position[1]] * sign;
      break;
    }
    case wK:
    case bK: {
      out_value = WHITE_KING_SCORE[p_position[0]][p_position[1]] * sign;
      break;
    }
  }
  return out_value*0.1;
}