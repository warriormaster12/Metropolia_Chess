#include "position.h"
#include <iostream>
#include <cmath>
#include <cctype>


void Position::clear() {
    for (int rows = 0; rows < 8; rows ++) {
        for (int cols = 0; cols < 8; cols ++) {
            m_board[rows][cols] = NA;
        }
    }
}

std::vector<Move> Position::get_all_raw_moves(int player) const {
    std::vector<Move> out;
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++)
		{
			int chess_piece = m_board[row][col];

			if (chess_piece == NA)
				continue;

			if (get_chess_piece_color(chess_piece) != player)
				continue;

            std::vector<Move> temp;
			switch (chess_piece)
			{
			case wR: case bR:
                temp = get_rook_raw_move(row, col, player);
				break;
			case wQ: case bQ:
				temp = get_queen_raw_move(row, col, player);
				break;
			case wN: case bN:
				temp = get_knight_raw_move(row, col, player);
				break;
			case wB: case bB:
				temp = get_bishop_raw_move(row, col, player);
				break;
			case wK: case bK:
				temp = get_king_raw_move(row, col, player);
				break;
			case wP: case bP:
				temp = get_pawn_raw_move(row, col, player);
				break;
			}
            if (temp.size() > 0) {
                out.insert(out.end(), temp.begin(), temp.end());
            }
		}
    }

    return out;
        
}

void Position::get_chess_piece(int chess_piece, int& row, int& col) const {
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (chess_piece == m_board[i][j]) {
                row = i; 
                col = j;
                return;
            }
        }
    }
}

bool Position::is_square_threatened(int row, int col, int threatening_player) const {
    vector<Move> moves = get_all_raw_moves(threatening_player);
    for (int i=0; i < moves.size(); ++i) {
        const Move& move = moves[i];
        if (move.m_end_pos[0] == row && move.m_end_pos[1] == col) {
            return true;
        }
    }
    return false;
}

vector<Move> Position::generate_legal_moves() const {
    int king = m_movingturn == WHITE ? wK : bK;
    int player = m_movingturn;
    int opponent = m_movingturn == WHITE ? BLACK : WHITE;
    std::vector<Move> raw_moves;
    std::vector<Move> castling_moves = get_castlings(player);
    raw_moves = get_all_raw_moves(player);
    raw_moves.insert(raw_moves.end(), castling_moves.begin(), castling_moves.end());
    std::vector<Move> legal_moves;
    for(const Move& raw_move: raw_moves) {
        Position test_pos = *this;

        test_pos.move(raw_move);
        int row, col;
        test_pos.get_chess_piece(king, row, col);
        if (!test_pos.is_square_threatened(row, col, opponent)) {
            legal_moves.push_back(raw_move);
        }
    }

    return legal_moves;
}

void Position::move(const Move& p_move) {
    int chess_piece = m_board[p_move.m_start_pos[0]][p_move.m_start_pos[1]];
    int player = get_chess_piece_color(chess_piece);
    m_board[p_move.m_start_pos[0]][p_move.m_start_pos[1]] = NA;
    if (is_promotable(chess_piece, p_move.m_end_pos[0])) {
        bool promoted = false;
        while (!promoted) {
            std::cout <<"Promote your pawn: \n";
            std::cout<< "1. Queen \n";
            std::cout<< "2. Rook \n";
            std::cout<< "3. Bishop \n";
            std::cout<< "4. Knight \n";
            std::string selected_character;
            std::cout<<"Input number: ";
            std::cin>>selected_character;
            if (selected_character.size() == 1 && isdigit(selected_character[0])) {
                int selected_num = stoi(selected_character);
                if (selected_num > 0 && selected_num < 5) {
                    switch (selected_num) {
                        case 1: {
                            chess_piece = player == WHITE ? wQ : bQ;
                            break;
                        }
                        case 2: {
                            chess_piece = player == WHITE ? wR : bR;
                            break;
                        }
                        case 3: {
                            chess_piece = player == WHITE ? wB : bB;
                            break;
                        }
                        case 4: {
                            chess_piece = player == WHITE ? wN : bN;
                            break;
                        }
                    
                    }
                    promoted = true;
                }
            } else {
                std::cout<<"Invalid input"<<std::endl;
            }
        }
    }
    if (chess_piece == wK && p_move.m_start_pos[0] == 7 && p_move.m_start_pos[1] == 4 && p_move.m_end_pos[0] == 7 && p_move.m_end_pos[1] == 6)
    {
        m_board[7][7] = NA;
        m_board[7][5] = wR;
    }
    else if (chess_piece == wK && p_move.m_start_pos[0] == 7 && p_move.m_start_pos[1] == 4 && p_move.m_end_pos[0] == 7 && p_move.m_end_pos[1] == 2)
    {
        m_board[7][0] = NA;
        m_board[7][3] = wR;
    }
    else if (chess_piece == bK && p_move.m_start_pos[0] == 0 && p_move.m_start_pos[1] == 4 && p_move.m_end_pos[0] == 0 && p_move.m_end_pos[1] == 6)
    {
        m_board[0][7] = NA;
        m_board[0][5] = bR;
    }
    else if (chess_piece == bK && p_move.m_start_pos[0] == 0 && p_move.m_start_pos[1] == 4 && p_move.m_end_pos[0] == 0 && p_move.m_end_pos[1] == 2)
    {
        m_board[0][0] = NA;
        m_board[0][3] = bR;
    }

    if (chess_piece == bK)
    {
        m_black_long_castling_allowed = false;
        m_black_short_castling_allowed = false;
    }
    else if (chess_piece == wK)
    {
        m_white_long_castling_allowed = false;
        m_white_short_castling_allowed = false;
    }
    if (chess_piece == bR && p_move.m_start_pos[1] == 0 || p_move.m_end_pos[0] == 0 && p_move.m_end_pos[1] == 0)
    {
        m_black_long_castling_allowed = false;
    }
    else if (chess_piece == bR && p_move.m_start_pos[1] == 7 || p_move.m_end_pos[0] == 0 && p_move.m_end_pos[1] == 7)
    {
        m_black_short_castling_allowed = false;
    }
    if (chess_piece == wR && p_move.m_start_pos[1] == 0 || p_move.m_end_pos[0] == 7 && p_move.m_end_pos[1] == 0)
    {
        m_white_long_castling_allowed = false;
    }
    else if (chess_piece == wR && p_move.m_start_pos[1] == 7 || p_move.m_end_pos[0] == 7 && p_move.m_end_pos[1] == 7)
    {
        m_white_short_castling_allowed = false;
    }

    m_board[p_move.m_end_pos[0]][p_move.m_end_pos[1]] = chess_piece;
    if (m_movingturn == WHITE) {
        m_movingturn = BLACK;
    } else if (m_movingturn == BLACK) {
        m_movingturn = WHITE;
    }
}
bool Position::check_collision(int row_now, int col_now, int row, int col, int player,vector<Move>& out) const {
    int chess_piece = m_board[row][col];
    bool is_pawn = chess_piece == wP || chess_piece == bP;
    bool can_hit = !(is_pawn && col_now == col);
    // Please god don't touch this. It works perfectly trust me bro.
    if (m_board[row_now][col_now]==NA) {
        if (is_pawn) {
            if (!can_hit) {
               out.push_back(Move({row, col}, {row_now, col_now})); 
            }
        } else {
            out.push_back(Move({row, col}, {row_now, col_now}));
        }
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

vector<Move> Position::get_rook_raw_move(int row, int col, int player) const {
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

vector<Move> Position::get_bishop_raw_move(int row, int col, int player) const {
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

vector<Move> Position::get_queen_raw_move(int row, int col, int player) const {
    vector<Move> out; 
    vector<Move> rook = get_bishop_raw_move(row, col, player);
    vector<Move> tower = get_rook_raw_move(row, col, player);
    out.insert(out.end(), rook.begin(), rook.end());
    out.insert(out.end(), tower.begin(), tower.end());
    return out;
}

vector<Move> Position::get_knight_raw_move(int row, int col, int player) const {
    vector<Move> out;
    vector<Move> up_right = get_directional_raw_move({row, col}, {1, -2}, player);
    vector<Move> right_up = get_directional_raw_move({row, col}, {2, -1}, player);
    vector<Move> right_down = get_directional_raw_move({row, col}, {2, 1}, player);
    vector<Move> down_right = get_directional_raw_move({row, col}, {1, 2}, player);
    vector<Move> down_left = get_directional_raw_move({row, col}, {-1, 2}, player);
    vector<Move> left_down = get_directional_raw_move({row, col}, {-2, 1}, player);
    vector<Move> left_up = get_directional_raw_move({row, col}, {-2, -1}, player);
    vector<Move> up_left = get_directional_raw_move({row, col}, {-1, -2}, player);
    out.insert(out.end(), up_right.begin(), up_right.end());
    out.insert(out.end(), right_up.begin(), right_up.end());
    out.insert(out.end(), right_down.begin(), right_down.end());
    out.insert(out.end(), down_right.begin(), down_right.end());
    out.insert(out.end(), down_left.begin(), down_left.end());
    out.insert(out.end(), left_down.begin(), left_down.end());
    out.insert(out.end(), left_up.begin(), left_up.end());
    out.insert(out.end(), up_left.begin(), up_left.end());
    return out;
}

vector<Move> Position::get_king_raw_move(int row, int col, int player) const {
    return get_queen_raw_move(row, col, player);
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
    if (chess_piece == wK || chess_piece == bK || chess_piece == wN || chess_piece == bN ) {
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

vector<Move> Position::get_pawn_raw_move(int row, int col, int player) const {
    vector<Move> out;
    int row_now = row;
    int col_now = col;
    int max_moves = 1;
    int chess_piece = m_board[row][col];
    int move = 0;
    if (chess_piece == NA) {
        std::cout<<"no chess piece found"<<std::endl;
        return out;
    }
    if ((chess_piece == bP && row == 1) || (chess_piece == wP && row == 6)) {
        max_moves = 2;
    }
    while (move < max_moves) {
        if (player == BLACK) {
            row_now += 1;
        } else {
            row_now -= 1;
        }
        if (row_now < 0 || row_now > 7 || col_now < 0 || col_now > 7) {
            break;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            move++;
            continue;
        }
        break;
    }
    // Eating detection
    row_now = row;
    col_now = col;
    for (int i = 0; i < 2; i++) {
        col_now = col + (i == 0 ? 1 : -1); 
        row_now = row + (player == BLACK ? 1 : -1);
        if (row_now < 0 || row_now > 7 || col_now < 0 || col_now > 7) {
            continue;
        }
        if (check_collision(row_now, col_now, row, col, player, out)) {
            continue;
        }
    }
    return out;
}

vector<Move> Position::get_castlings(int player) const {
    vector<Move> out;
    if (player == WHITE)
    {
        if (m_white_short_castling_allowed && m_board[7][5] == NA && m_board[7][6] == NA && !is_square_threatened(7, 4, BLACK) && !is_square_threatened(7, 5, BLACK))
        {
            out.push_back(Move({7, 4}, {7, 6}));
        }
        if (m_white_long_castling_allowed && m_board[7][3] == NA && m_board[7][2] == NA && m_board[7][1] == NA && !is_square_threatened(7, 4, BLACK) && !is_square_threatened(7, 3, BLACK))
        {
            out.push_back(Move({7, 4}, {7, 2}));
        }
    }
    else
    {
        if (m_black_short_castling_allowed && m_board[0][5] == NA && m_board[0][6] == NA && !is_square_threatened(0, 4, WHITE) && !is_square_threatened(0, 5, WHITE))
        {
            out.push_back(Move({0, 4}, {0, 6}));
        }
        if (m_black_long_castling_allowed && m_board[0][3] == NA && m_board[0][2] == NA && m_board[0][1] == NA && !is_square_threatened(0, 4, WHITE) && !is_square_threatened(0, 3, WHITE))
        {
            out.push_back(Move({0, 4}, {0, 2}));
        }
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

void Position::render_legal_moves(const vector<Move>& p_moves) {
    std::string letters = "abcdefgh";
    std::cout<<"valid moves:"<<std::endl;
    for (int i = 0; i < p_moves.size(); i++) {
        std::cout<<" "<<p_moves[i].get_coords()<<std::endl;
    }

    std::cout<<"Legal move count: "<<p_moves.size()<<std::endl;
}