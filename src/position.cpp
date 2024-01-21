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
                    std::string name = Utils::chess_piece_to_string(m_board[row][cols]);
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