#include "position.h"
#include "move.h"
#include <iostream>

int main() {
  Position position;
  int s[2] = {6,4};
  int e[2] =  {4,4};
  Move m("e2e4");
  position.move(m);
  position.render_board();
  return 0;
}