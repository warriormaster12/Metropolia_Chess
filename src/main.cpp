#include "position.h"
#include "move.h"
#include <iostream>

int main() {
  Position position;
  int s[2] = {6,4};
  int e[2] =  {4,4};
  Move m("c8d5");
  position.move(m);
  position.render_board();
  vector<Move> hello = position.get_rook_raw_move(3, 3, BLACK);
  return 0;
}