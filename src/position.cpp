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

vector<Move> Position::give_tower_raw_move(int row, int col, int player) {
    int row_now = row;
    int col_now = col;
    vector<Move> out;
    while (true) {
        row_now--;
        if (row_now < 0) {
            break;
        }
        if (m_board[row_now][col_now]==NA) {
            int start_pos[2] = {row, col};
            int end_pos[2] = {row_now, col_now};
            out.push_back(Move(start_pos, end_pos));
            continue;
        }
    }
}

void Position::render_board() {
    int board_size = 8;
    for (int row = 0; row < board_size; row++) {
        std::string map = " ";
        if (row == 0) {
            map += "\n ";
            const char *letters = "ABCDEFG";
            for (int col= 0; col < board_size; col ++){
                map += "  ";
                map += letters[col];
                map += " ";
            }
            map += "\n ";
        }
        for (int col= 0; col < board_size; col ++) {
            if (col == board_size - 1) {
                map += "+\n";
            } else {
                map += "+---";
            }
            
        }
        for (int col= 0; col < board_size; col ++) {
            if (col == board_size - 1) {
                map += "|";
            }
            else if (col == 0) {
                map += std::to_string(board_size - row) + "|   ";
            } else {
                map += "|   ";
            }
        }
        if (row == board_size-1) {
            map += "\n ";
            for (int col= 0; col < board_size; col ++) {
                if (col == board_size - 1) {
                    map += "+";
                } else {
                    map += "+---";
                }
                
            }
        }
        std::cout<<map;
        std::cout<<std::endl;
    }
}