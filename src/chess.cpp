#include "chess.h"

std::string Utils::chess_piece_to_string(int p_index) {
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