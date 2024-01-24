#include "position.h"
#include <iostream>
#include <cmath>


void Position::clear() {
    for (int rows = 0; rows < 8; rows ++) {
        for (int cols = 0; cols < 8; cols ++) {
            m_board[rows][cols] = NA;
        }
    }
}

void Position::move(const Move& p_move) {
    int chess_piece = m_board[p_move.m_start_pos[0]][p_move.m_start_pos[1]];
    m_board[p_move.m_start_pos[0]][p_move.m_start_pos[1]] = NA;
    m_board[p_move.m_end_pos[0]][p_move.m_end_pos[1]] = chess_piece;
}
bool Position::check_collision(int row_now, int col_now, int row, int col, int player,vector<Move>& out) const {
    int chess_piece = m_board[row][col];
    bool can_hit = !(chess_piece == wP || chess_piece == bP);
    if (m_board[row_now][col_now]==NA) {
        out.push_back(Move({row, col}, {row_now, col_now}));
        return true;
    }
    if (get_chess_piece_color(m_board[row_now][col_now]) == player) {
        return false;
    }
    if (can_hit) {
        out.push_back(Move({row, col}, {row_now, col_now}));
    }
    return false;
}

vector<Move> Position::get_tower_raw_move(int row, int col, int player) const {
    vector<Move> out;
    vector<Move> up = get_directional_raw_move({row, col}, {0, -1}, player);
    vector<Move> down = get_directional_raw_move({row, col}, {0, 1}, player);
    vector<Move> left = get_directional_raw_move({row, col}, {-1, 0}, player);
    vector<Move> right = get_directional_raw_move({row, col}, {1, 0}, player);
    out.insert(out.end(), up.begin(), up.end());
    out.insert(out.end(), down.begin(), down.end());
    out.insert(out.end(), left.begin(), left.end());
    out.insert(out.end(), right.begin(), right.end());
    return out;
}

vector<Move> Position::get_rook_raw_move(int row, int col, int player) const {
    vector<Move> out;
    vector<Move> up_right = get_directional_raw_move({row, col}, {1, -1}, player);
    vector<Move> up_left = get_directional_raw_move({row, col}, {-1, -1}, player);
    vector<Move> down_left = get_directional_raw_move({row, col}, {-1, 1}, player);
    vector<Move> down_right = get_directional_raw_move({row, col}, {1, 1}, player);
    out.insert(out.end(), up_right.begin(), up_right.end());
    out.insert(out.end(), up_left.begin(), up_left.end());
    out.insert(out.end(), down_left.begin(), down_left.end());
    out.insert(out.end(), down_right.begin(), down_right.end());
    return out;
}

vector<Move> Position::get_directional_raw_move(std::array<int, 2> position, std::array<int, 2> direction, int player) const {
    vector<Move> out;
    int row_now = position[0];
    int col_now = position[1];
    int max_moves = 7;
    int chess_piece = m_board[position[0]][position[1]];
    int move = 0;
    if (chess_piece == NA) {
        std::cout<<"no chess piece found"<<std::endl;
        return out;
    }
    if (chess_piece == wK || chess_piece == bK) {
        max_moves = 1;
    }
    while (move < max_moves) {
        col_now += direction[0];
        row_now += direction[1];
        if (row_now < 0 || row_now > 7 || col_now < 0 || col_now > 7) {
            break;
        }
        if (check_collision(row_now, col_now, position[0], position[1], player, out)) {
            move++;
            continue;
        }
        break;
    }
    return out;
}

void Position::render_board() {
    int rowcount = 17;
    for (int rows = 0; rows < rowcount; rows ++) {
        for (int cols = 0; cols < 9; cols ++) {
            if (rows % 2 == 0) {
                if (cols == 8) {
                    std::cout<<"+";
                } else {
                    if (cols == 0) { 
                        std::cout<<" +----";
                    } else {
                        std::cout<<"+----";
                    }
                }
            } else {
                int row = floor(rows/2);
                if (cols == 0) { 
                    std::cout<<8-row<<"| ";
                } else {
                     std::cout<<"| ";
                }
                if (cols < 8) {
                    std::string name = chess_piece_to_string(m_board[row][cols]);
                    if (name.size() > 0) {
                        std::cout<<name<<" ";
                    } else {
                        std::cout<<"   ";
                    }
                }
            }
        }
        std::cout<<std::endl;
    }
}