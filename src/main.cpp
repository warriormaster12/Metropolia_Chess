#include "position.h"
#include "move.h"
#include <iostream>

int main() {
  Position position;
  int s[2] = {6,4};
  int e[2] =  {4,4};
  Move m("b7b4");
  position.move(m);
  position.render_board();
  vector<Move> hello = position.get_pawn_raw_move(4, 1, BLACK);
  return 0;
}