#include "chess/position.h"
#include "chess/move.h"
#include "renderer/renderer.h"
#include <imgui.h>
#include <future>

const int MAX_HISTORY_SIZE = 10;
vector<Position> history;

bool blackAI = false;
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

bool begin_game = false;

int main() {
    Renderer renderer = Renderer(1280, 720);
    Position position;

    vector<Move> moves;
    moves = position.generate_legal_moves();
    position.render_legal_moves(moves);
    position.render_board();
    char coords[5] = "";
    while (renderer.is_active() && moves.size() > 0) {
        FrameInfo info = renderer.prepare_frame(position, begin_game && moved);
        ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(640, 480));
        ImGui::Begin("Chess");
        if (!begin_game) {
            ImGui::Text("Choose one option");
            if (ImGui::Button("Play against another human player")) {
                begin_game = true;
            }
            if (ImGui::Button("Play as WHITE against BLACK AI")) {
                blackAI = true;
                begin_game = true;
            }
            if (ImGui::Button("Play as BLACK against WHITE AI playing as WHITE")) {
                whiteAI = true;
                begin_game = true;
            }
            if (ImGui::Button("Put two AI players against each other")) {
                whiteAI = true;
                blackAI = true;
                begin_game = true;
            }
        }
        else {
            if (moved) {
                moved = false;
            }
            ImGui::Text(position.get_moving_player() == WHITE ? "White's turn" : "Black's turn");
            //ImGui::SetKeyboardFocusHere(); // keeps the cursor in focus after hitting enter

            if (position.get_moving_player() == BLACK && blackAI || position.get_moving_player() == WHITE && whiteAI) {
                MinmaxValue alpha = MinmaxValue(numeric_limits<float>::lowest(), Move({ 0,0 }, { 0,0 }));
                MinmaxValue beta = MinmaxValue(numeric_limits<float>::max(), Move({ 0,0 }, { 0,0 }));
                if (!minmax_result.valid()) {
                    update_history(position);
                    minmax_result = std::async(&Position::minmax_alphabeta, &position, 3, alpha, beta);
                }
                else if (is_ready(minmax_result)) {
                    Move move = minmax_result.get().move;
                    position.move(move);
                    position.can_promote(move);
                    moves.clear();
                    moves = position.generate_legal_moves();
                    position.render_board();
                    moved = true;
                }

            }
            else {
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
                    if (valid_coords) {
                        Move move = Move(coords);
                        update_history(position);
                        position.move(move);
                        position.can_promote(move);
                        moves.clear();
                        moves = position.generate_legal_moves();
                        position.render_legal_moves(moves);
                        position.render_board();
                        memset(coords, 0, 5 * sizeof(*coords));
                        moved = true;
                    }
                }
                if (ImGui::Button("Undo Move") && history.size() > 0) {
                    // if pressed do undo
                    if (blackAI || whiteAI && history.size() > 1) {
                        //undo two steps instead of one to get to the last move made by the player (no point in undoing only what the AI does)
                        position = history[std::max<size_t>(history.size() - 2, 0)];
                        history.erase(history.end() - 2, history.end());
                    }
                    if (!blackAI && !whiteAI) {
                        //undo one step
                        position = history[std::max<size_t>(history.size() - 1, 0)];
                        history.pop_back();
                    }
                    moves.clear();
                    moves = position.generate_legal_moves();
                    position.render_board();
                    moved = true;
                }
            }
        }
        ImGui::End();
        renderer.render_board(info);
    }
    if (minmax_result.valid() && !is_ready(minmax_result)) {
        minmax_result.wait();
    }
    renderer.destroy();
    return 0;
}