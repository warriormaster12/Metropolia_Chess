#include "chess/position.h"
#include "chess/move.h"
#include "renderer/renderer.h"
#include <imgui.h>
#include <future>

const int MAX_HISTORY_SIZE = 10;
vector<Position> history;

bool blackAI = true;
bool whiteAI = false;

void update_history(const Position p_position) {
  if (history.size() + 1 > MAX_HISTORY_SIZE) {
      history.erase(history.begin());
  }
  history.push_back(p_position);
}

// This helper function could eventually be moved to some better place
template<typename T>
bool is_ready(std::future<T> const& f)
{ 
  return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; 
}

std::future<MinmaxValue> minmax_result;

bool moved = true;

int main() {
  bool undo_lastround = false;

  Renderer renderer = Renderer(1280, 720);
  Position position;
  
  vector<Move> moves;
  moves = position.generate_legal_moves();
  position.render_legal_moves(moves);
  position.render_board();
  char coords[5] = "";
  while (renderer.is_active() && moves.size() > 0) {
    FrameInfo info = renderer.prepare_frame(position, moved);
    if (moved) {
      moved = false;
    }
    ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(640, 480));
    ImGui::Begin("Chess");
    ImGui::Text(position.get_moving_player() == WHITE ? "White's turn" : "Black's turn");
    //ImGui::SetKeyboardFocusHere(); // keeps the cursor in focus after hitting enter
    
    if (position.get_moving_player() == BLACK && blackAI || position.get_moving_player() == WHITE && whiteAI) {
      update_history(position);
      MinmaxValue alpha = MinmaxValue(numeric_limits<float>::lowest(), Move({ 0,0 }, { 0,0 }));
      MinmaxValue beta = MinmaxValue(numeric_limits<float>::max(), Move({ 0,0 }, { 0,0 }));
      if (!minmax_result.valid()) {
        minmax_result = std::async(&Position::minmax_alphabeta, &position, 3, alpha, beta);
      } else if (is_ready(minmax_result)) {
        Move move = minmax_result.get().move;
        position.move(move);
        position.can_promote(move);
        moves.clear();
        moves = position.generate_legal_moves();
        position.render_board();
        moved = true;
      }
      
    } else {
      ImGui::InputText(
        "where to move", 
        coords, 
        5, 
        ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_AlwaysOverwrite
      );
      size_t char_count = strlen(coords);
      // if (!undo_lastround) {
      //     if (history.size() + 1 > MAX_HISTORY_SIZE) {
      //         history.erase(history.begin());
      //     }
      //     history.push_back(position);
      // }
      // else {
      //     undo_lastround = false;
      // }
      //     if (char_count == 2) {
        
      // }
      if (char_count == 4 && ImGui::IsKeyPressed(ImGuiKey_Enter)) {
        bool valid_coords = false;
        for (Move& move : moves) {
          if (move.get_coords() == coords) {
            valid_coords = true; 
            break;
          }
        }
        if (valid_coords){
          Move move = Move(coords);
          update_history(position);
          position.move(move);
          position.can_promote(move);
          moves.clear();
          moves = position.generate_legal_moves();
          position.render_legal_moves(moves);
          position.render_board();
          memset(coords, 0, 5*sizeof(*coords));
          moved = true;
        }
      }
      if (ImGui::Button("Undo Move") && ((history.size() > 1 && !whiteAI) || history.size() > 2)) {
        // if pressed do undo
          if (blackAI || whiteAI) {
              //undo two steps instead of one to get to the last move made by the player (no point in undoing only what the AI does)
              position = history[history.size() - 3];
              history.pop_back();
          }
          else {
              //undo one step
              position = history[history.size() - 2];
          }
          //nää alla olevat jutut oli perässä viime historia koodin kaa, en oo varma miten tän uuden kans tulis toimii
          moves.clear();
          moves = position.generate_legal_moves();
          position.render_board();
          history.pop_back();
          undo_lastround = true;
      }
    }
    ImGui::End();
    renderer.render_board(info);
  }
  renderer.destroy();
  return 0;
}