#include "chess/position.h"
#include "chess/move.h"
#include "renderer/renderer.h"
#include <imgui.h>

int main() {
  Renderer renderer = Renderer(1280, 720);
  Position position;
  vector<Move> moves;
  moves = position.generate_legal_moves();
  position.render_legal_moves(moves);
  position.render_board();
  char coords[5] = "";
  while (renderer.is_active() && moves.size() > 0) {
    FrameInfo info = renderer.prepare_frame(position);
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(640, 480));
    ImGui::Begin("Chess");
    ImGui::Text(position.get_moving_player() == WHITE ? "White's turn" : "Black's turn");
    ImGui::SetKeyboardFocusHere(); // keeps the cursor in focus after hitting enter
    ImGui::InputText(
      "where to move", 
      coords, 
      5, 
      ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_AlwaysOverwrite
    );
    size_t char_count = strlen(coords);
    Move player_move = Move({-1,-1}, {-1, -1});
    if (char_count == 2) {
      
    }
    if (char_count == 4 && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
      bool valid_coords = false;
      for (Move& move : moves) {
        if (move.get_coords() == coords) {
          valid_coords = true; 
          break;
        }
      }
      if (valid_coords){
        player_move = Move(coords);
        position.move(player_move);
        position.can_promote(player_move);
        moves.clear();
        moves = position.generate_legal_moves();
        position.render_legal_moves(moves);
        position.render_board();
        memset(coords, 0, 5*sizeof(*coords));
      }
    }
    if (ImGui::Button("Undo Move")) {
      // if pressed do undo
    }
    ImGui::End();
    renderer.render_board(info, player_move);
  }
  renderer.destroy();
  return 0;
}