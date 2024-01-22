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