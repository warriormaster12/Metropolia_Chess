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
bool Position::check_collision(int row_now, int col_now, int row, int col, int player,vector<Move>& out) {
    if (m_board[row_now][col_now]==NA) {
        int start_pos[2] = {row, col};
        int end_pos[2] = {row_now, col_now};
        out.push_back(Move(start_pos, end_pos));
        return true;
    }
    if (get_chess_piece_color(m_board[row_now][col_now]) == player) {
        return false;
    }
    int start_pos[2] = {row, col};
    int end_pos[2] = {row_now, col_now};
    out.push_back(Move(start_pos, end_pos));
    return false;
}

vector<Move> Position::get_tower_raw_move(int row, int col, int player) {
    vector<Move> out;
    //UP
    int row_now = row;
    int col_now = col;
    while (true) {
        row_now--;
        if (row_now < 0) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    //DOWN
    row_now = row;
    col_now = col;
    while (true) {
        row_now++;
        if (row_now > 7) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    //LEFT
    row_now = row;
    col_now = col;
    while (true) {
        col_now--;
        if (col_now < 0) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    //RIGHT
    row_now = row;
    col_now = col;
    while (true) {
        col_now++;
        if (col_now > 7) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    return out;
}

vector<Move> Position::get_rook_raw_move(int row, int col, int player) {
    vector<Move> out;
    //UP
    int row_now = row;
    int col_now = col;
    while (true) {
        col_now--;
        row_now--;
        if (row_now < 0 || col_now < 0) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    //DOWN
    row_now = row;
    col_now = col;
    while (true) {
        row_now++;
        col_now++;
        if (row_now > 7 || col_now > 7) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    //LEFT
    row_now = row;
    col_now = col;
    while (true) {
        col_now--;
        row_now++;
        if (col_now < 0 || row_now > 7) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
        break;
    }
    //RIGHT
    row_now = row;
    col_now = col;
    while (true) {
        col_now++;
        row_now--;
        if (col_now > 7 || row_now < 0) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
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