#include "chess/position.h"
#include "chess/move.h"
#include "renderer/renderer.h"
#include <imgui.h>
#include <future>

const int MAX_HISTORY_SIZE = 10;
struct HistoryInfo {
    Position position;
    Move move;
    HistoryInfo(Position p_position, Move p_move) {
        position = p_position;
        move = p_move;
    }
};
vector<HistoryInfo> history;

bool blackAI = false;
bool whiteAI = false;

void update_history(const Position p_position, const Move p_move) {
    if (history.size() + 1 > MAX_HISTORY_SIZE) {
        history.erase(history.begin());
    }
    history.push_back(HistoryInfo(p_position, p_move));
}

// This helper function could eventually be moved to some better place
template<typename T>
bool is_ready(std::future<T> const& f)
{
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

std::future<MinmaxValue> minmax_result;

bool moved = false;

bool begin_game = false;

std::array<int, 2> promotable_coords = {-1, -1};

float black_score = 0.0;
float white_score = 0.0;

//Benchmark
bool show_fps = false;
bool show_ai_process_time = false;
double delta_time = 0.0;
double ai_delta_time = 0.0;
double ai_time = 0.0;
int ai_move_count = 0;
int current_frame = 0;


int main() {
    Renderer renderer = Renderer(1280, 720);
    Position position;

    vector<Move> moves;
    moves = position.generate_legal_moves(whiteAI);
    position.render_legal_moves(moves);
    position.render_board();
    char coords[5] = "";
    
    std::chrono::time_point<std::chrono::system_clock> ai_time_start, ai_time_end;

    while (renderer.is_active() && moves.size() > 0) {
        std::chrono::time_point<std::chrono::system_clock> frame_start, frame_end;
        frame_start = std::chrono::system_clock::now();
        FrameInfo info = renderer.prepare_frame(position, begin_game && moved);
        ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(640, 480));
        ImGui::Begin("Chess");
        if (!begin_game) {
            ImGui::Text("Choose one option");
            if (ImGui::Button("Play against another human player")) {
                begin_game = true;
                moved = true;
            }
            if (ImGui::Button("Play as WHITE against BLACK AI")) {
                blackAI = true;
                begin_game = true;
                moved = true;
            }
            if (ImGui::Button("Play as BLACK against WHITE AI playing as WHITE")) {
                whiteAI = true;
                begin_game = true;
                moved = true;
            }
            if (ImGui::Button("Put two AI players against each other")) {
                whiteAI = true;
                blackAI = true;
                begin_game = true;
                moved = true;
            }
        } else if (promotable_coords[0] != -1 && promotable_coords[1] != -1 && ((position.get_moving_player() == WHITE && !whiteAI) || (position.get_moving_player() == BLACK && !blackAI)) ) {
            ImGui::Text("Promote your pawn");
            if (ImGui::Button("Queen")) {
                position.promote(promotable_coords, position.get_moving_player() == WHITE ? wQ : bQ);
                promotable_coords[0] = -1;
                promotable_coords[1] = -1;
                position.end_turn();
                moves.clear();
                moves = position.generate_legal_moves();
                position.render_legal_moves(moves);
                position.render_board();
            }

            if (ImGui::Button("Rook")) {
                position.promote(promotable_coords, position.get_moving_player() == WHITE ? wR : bR);
                promotable_coords[0] = -1;
                promotable_coords[1] = -1;
                position.end_turn();
                moves.clear();
                moves = position.generate_legal_moves();
                position.render_legal_moves(moves);
                position.render_board();
            }

            if (ImGui::Button("Bishop")) {
                position.promote(promotable_coords, position.get_moving_player() == WHITE ? wB : bB);
                promotable_coords[0] = -1;
                promotable_coords[1] = -1;
                position.end_turn();
                moves.clear();
                moves = position.generate_legal_moves();
                position.render_legal_moves(moves);
                position.render_board();
            }

            if (ImGui::Button("Knight")) {
                position.promote(promotable_coords, position.get_moving_player() == WHITE ? wN : bN);
                promotable_coords[0] = -1;
                promotable_coords[1] = -1;
                position.end_turn();
                moves.clear();
                moves = position.generate_legal_moves();
                position.render_legal_moves(moves);
                position.render_board();
                moved = true;
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
                    ai_time_start = std::chrono::system_clock::now();
                    minmax_result = std::async(&Position::minmax_alphabeta, &position, 4, alpha, beta, true);
                }
                else if (is_ready(minmax_result)) {
                    MinmaxValue minmax_val = minmax_result.get();
                    ai_time_end = std::chrono::system_clock::now();
                    std::chrono::duration<double> elapsed_seconds = ai_time_end - ai_time_start;
                    ai_delta_time = elapsed_seconds.count();
                    ai_time += ai_delta_time;
                    ai_move_count += 1;
                    if (position.get_moving_player() == WHITE) {
                        white_score = minmax_val.value;
                    } else {
                        black_score = minmax_val.value;
                    }
                    update_history(position, minmax_val.move);
                    std::cout<< (position.get_moving_player() == WHITE ? "White " : "Black ")<<"moved from to: "<<minmax_val.move.get_coords()<<std::endl;
                    position.move(minmax_val.move);
                    if (position.can_promote(minmax_val.move)) {
                        position.promote(minmax_val.move.get_end_pos(), minmax_val.move.get_promotable());
                    }
                    position.end_turn();
                    moves.clear();
                    moves = position.generate_legal_moves(true);
                    position.render_legal_moves(moves);
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
                        std::cout<< (position.get_moving_player() == WHITE ? "White " : "Black ")<<"moved from to: "<<move.get_coords()<<std::endl;
                        update_history(position, move);
                        position.move(move);
                        if (position.can_promote(move)) {
                            promotable_coords = move.get_end_pos();
                        } else {
                            position.end_turn();
                            moves.clear();
                            moves = position.generate_legal_moves();
                            position.render_legal_moves(moves);
                        }

                        position.render_board();
                        memset(coords, 0, 5 * sizeof(*coords));
                        moved = true;
                    }
                }
                if (ImGui::Button("Undo Move") && history.size() > 0) {
                    // if pressed do undo
                    if (blackAI || whiteAI && history.size() > 1) {
                        //undo two steps instead of one to get to the last move made by the player (no point in undoing only what the AI does)
                        position = history[std::max<size_t>(history.size() - 2, 0)].position;
                        history.erase(history.end() - 2, history.end());
                    }
                    if (!blackAI && !whiteAI) {
                        //undo one step
                        position = history[std::max<size_t>(history.size() - 1, 0)].position;
                        history.pop_back();
                    }
                    moves.clear();
                    moves = position.generate_legal_moves();
                    position.render_board();
                    moved = true;
                }
            }
            if (whiteAI) {
                ImGui::Text("White AI's absolute score: %f", std::abs(white_score));
            }
            if (blackAI) {
                ImGui::Text("Black AI's absolute score: %f", std::abs(black_score));
            }
            if (ImGui::CollapsingHeader("History", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen)) {
                for (int i=history.size() - 1; i > -1; --i) {
                    const std::string player = history[i].position.get_moving_player() == WHITE ? "white" : "black";
                    const std::string result = player + " move: " + history[i].move.get_coords();
                    ImGui::BulletText("%s",result.c_str());
                }
            }
        }
        if (ImGui::CollapsingHeader("Benchmark")) {
            ImGui::Checkbox("Show Fps", &show_fps);
            ImGui::Checkbox("Show AI Process Time", &show_ai_process_time);
            
            if (show_fps) {
                ImGui::BulletText("Fps: %i", (int)std::round(1/delta_time));
                ImGui::BulletText("Frame Time: %f millisec", std::round(delta_time * 100000) / 1000);
            }

            if (show_ai_process_time) {
                ImGui::BulletText("AI Process Time: %f sec", ai_delta_time);
                ImGui::BulletText("AI Average Time: %f sec", ai_time/ai_move_count);
            }
        }
        ImGui::End();
        renderer.render_board(info);

        frame_end = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = frame_end - frame_start;
        if (current_frame % 100 == 0 ) {
            delta_time = elapsed_seconds.count();
        }
        current_frame++;
        if (current_frame > 9999999){
            current_frame = 0;
        }
    }
    if (minmax_result.valid() && !is_ready(minmax_result)) {
        minmax_result.wait();
    }
    renderer.destroy();
    return 0;
}