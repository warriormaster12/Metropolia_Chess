#include "position.h"
#include "move.h"
#include <iostream>

int main() {
  Position position;
  vector<Move> moves;
  moves = position.generate_legal_moves();
  while (moves.size() > 0) {
    position.render_board();
    position.render_legal_moves(moves);
    std::string move_coords;
    bool valid_coords = false;
    while (!valid_coords) {
      std::cout<<"Where do you want to move: ";
      std::cin>>move_coords;
      for (int i = 0; i < moves.size(); i++) {
        Move &current = moves[i];
        if (current.get_coords() == move_coords) {
          valid_coords = true;
          break;
        }
      }
      if (!valid_coords) {
        std::cout<<"no valid coords: "<<move_coords<<std::endl;
      }
    }
    position.move(Move(move_coords));
    moves.clear();
    moves = position.generate_legal_moves();
  }
  return 0;
}