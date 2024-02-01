#include "position.h"
#include "move.h"
#include <iostream>

int main() {
  Position position;
  position.render_board();
  vector<Move> aaaww_yeah = position.get_all_raw_moves(BLACK);
  return 0;
}