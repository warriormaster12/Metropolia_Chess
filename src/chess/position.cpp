#include "position.h"
#include <iostream>
#include <cmath>
#include <limits>
#include <map>
#include <future>
#include <algorithm>

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
        int chess_piece = m_board[move.get_start_pos()[0]][move.get_start_pos()[1]];
        if (chess_piece == wP) {
            if (move.get_start_pos()[0] - 1 == row && (move.get_start_pos()[1] + 1 == col || move.get_start_pos()[1] - 1 == col))
                return true;
        }
        else if (chess_piece == bP) {
            if (move.get_start_pos()[0] + 1 == row && (move.get_start_pos()[1] + 1 == col || move.get_start_pos()[1] - 1 == col))
                return true;
        }
        else if (move.get_end_pos()[0] == row && move.get_end_pos()[1] == col) {
            return true;
        }
    }
    return false;
}

vector<Move> Position::generate_legal_moves(const bool ai_legal_moves) const {
    int king = m_movingturn == WHITE ? wK : bK;
    int player = m_movingturn;
    int opponent = m_movingturn == WHITE ? BLACK : WHITE;
    std::vector<Move> raw_moves;
    std::vector<Move> castling_moves = get_castlings(player);
    raw_moves = get_all_raw_moves(player);
    raw_moves.insert(raw_moves.end(), castling_moves.begin(), castling_moves.end());
    std::vector<Move> legal_moves;
    std::array<int, 4> white_promotables = {wQ, wR, wB, wN};
    std::array<int, 4> black_promotables = {bQ, bR, bB, bN};
    for(Move& raw_move: raw_moves) {
        Position test_pos = *this;

        test_pos.move(raw_move);
        int row, col;
        test_pos.get_chess_piece(king, row, col);
        if (!test_pos.is_square_threatened(row, col, opponent)) {
            if (ai_legal_moves && test_pos.can_promote(raw_move)) {
                for (int i = 0; i < 4; i++) {
                    if (test_pos.get_moving_player() == WHITE) {
                        raw_move.set_promotable(white_promotables[i]);
                    } else {
                        raw_move.set_promotable(black_promotables[i]);
                    }
                    legal_moves.push_back(raw_move);
                }
            } else {
                legal_moves.push_back(raw_move);
            }
        }
        test_pos.end_turn();
    }

    return legal_moves;
}

float Position::score_end_result(const int p_depth) const {
    if (m_movingturn == WHITE) {
        int row, col;
        get_chess_piece(wK, row, col);

        if (is_square_threatened(row, col, BLACK)) {
            return -100000 - p_depth;
        }
    } else {
        int row, col;
        get_chess_piece(bK, row, col);

        if (is_square_threatened(row, col, WHITE)) {
            return 100000 + p_depth;
        }
    }
    return 0;
}
float Position::evaluate() const {
    return material();
}

float Position::material() const {
    static map<int, float> piece_values = {
        {wP, 1.0},{wN, 3.0}, {wB, 3.0}, {wR, 5.0}, {wQ, 9.0}, {wK, 90},
        {bP, -1.0},{bN, -3.0}, {bB, -3.0}, {bR, -5.0}, {bQ, -9.0},{bK, -90},
        {NA, 0.0}
    };

    float result = 0;
    for (int row = 0; row < 8; ++row) {
        for (int col = 0; col < 8; ++col) {
            int piece = m_board[row][col];
            float piece_value  = piece_values[piece];
            float square_score = get_square_score({row, col}, piece);
            result += piece_value + square_score;
        }
    }
    return result;
}

float Position::mobility() const {
    vector<Move> white_moves = get_all_raw_moves(WHITE); 
    vector<Move> black_moves = get_all_raw_moves(BLACK);

    return (float)white_moves.size() - (float)black_moves.size();
}

MinmaxValue Position::minmax(int depth) {
    vector<Move> legal_moves = this->generate_legal_moves(true);

    if (legal_moves.size() == 0) {
        return MinmaxValue(this->score_end_result(depth), Move());
    }

    if (depth == 0) {
        float value = this->evaluate();
        return MinmaxValue(value, Move());
    }

    float best_value = this->get_moving_player() == WHITE ? numeric_limits<float>::lowest() : numeric_limits<float>::max();
    Move best_move;
    for(Move &move : legal_moves) {
        Position new_pos = *this;
        new_pos.move(move);
        new_pos.end_turn();
        MinmaxValue minmaxval = new_pos.minmax(depth -1);
        if (this->get_moving_player() == WHITE && minmaxval.value > best_value) {
            best_value = minmaxval.value;
            best_move = move;
        } else if (this->get_moving_player() == BLACK && minmaxval.value < best_value) {
            best_value = minmaxval.value;
            best_move = move;
        }
    }
    return MinmaxValue(best_value, best_move);
}

MinmaxValue Position::threaded_alpha_beta(std::vector<Move> p_legal_moves, int depth, MinmaxValue alpha, MinmaxValue beta) {
    float best_value = this->get_moving_player() == WHITE ? numeric_limits<float>::lowest() : numeric_limits<float>::max();
    bool maximizingPlayer = this->get_moving_player() == WHITE ? true : false;
    Move best_move;
    for (Move& move : p_legal_moves) {
        Position new_pos = *this;
        new_pos.move(move);
        if (new_pos.can_promote(move)) {
            new_pos.promote(move.get_end_pos(), move.get_promotable());
        }
        new_pos.end_turn();
        MinmaxValue current_move = new_pos.minmax_alphabeta(depth - 1, alpha, beta, false);
        if (maximizingPlayer) {
            if (current_move.value > best_value) {
                best_value = current_move.value;
                best_move = move;
            }
            if (current_move.value > alpha.value) {
                alpha.value = current_move.value;
                alpha.move = move;
            }
        } else {
            if (current_move.value < best_value) {
                best_value = current_move.value;
                best_move = move;
            }
            if (current_move.value < beta.value) {
                beta.value = current_move.value;
                beta.move = move;
            }
        }
        if (beta.value <= alpha.value) {
            break;
        }
    }
    return MinmaxValue(best_value, best_move);
}

std::vector<std::future<MinmaxValue>> threads;

MinmaxValue Position::minmax_alphabeta(int depth, MinmaxValue alpha, MinmaxValue beta, const bool threaded) {
    vector<Move> legal_moves = this->generate_legal_moves(true);
    if (legal_moves.size() == 0) {
        return MinmaxValue(this->score_end_result(depth), Move());
    }
    if (depth == 0) {
        float value = this->evaluate();
        return MinmaxValue(value, Move());
    }

    bool maximizingPlayer = this->get_moving_player() == WHITE ? true : false;
    float best_value = this->get_moving_player() == WHITE ? numeric_limits<float>::lowest() : numeric_limits<float>::max();
    MinmaxValue best_move;
    if (threaded) {
        if (threads.size() == 0) {
            // we want n-2 due to in the main function we are using one thread already + main thread
            unsigned int nthreads = std::thread::hardware_concurrency() - 2;
            threads.resize(nthreads);
        }
        int split_size = std::floor(legal_moves.size() / threads.size());
        std::vector<std::vector<Move>> split_moves;
        split_moves.resize(threads.size()); 
        int current_thread = split_size != 0 ? -1 : 0;
        for (int i = 0; i < legal_moves.size(); ++i) {
            if (split_size > 0) {
                if (current_thread != threads.size() - 1 && i % split_size == 0) {
                    current_thread += 1;
                }
            }
            split_moves[current_thread].push_back(legal_moves[i]);
        }
        std::vector<MinmaxValue> results;
        results.resize(threads.size());
        for (int i = 0; i < threads.size(); i++) {
            threads[i] = std::async(&Position::threaded_alpha_beta, this, split_moves[i], depth,alpha, beta);
            if (threads[i].valid()) {
                threads[i].wait();
                results[i] = threads[i].get();
            }
        }
        std::vector<float> values;
        for (int i = 0; i < results.size(); i++) {
            values.push_back(results[i].value);
        }
        std::vector<float>::iterator result = maximizingPlayer ? std::max_element(values.begin(), values.end()) : std::min_element(values.begin(), values.end());
        int index = std::distance(values.begin(), result);
        best_move = results[index];
    } else {
        best_move = threaded_alpha_beta(legal_moves, depth, alpha, beta);
    }

    return best_move;
}


void Position::move(const Move& p_move) {
    int chess_piece = m_board[p_move.get_start_pos()[0]][p_move.get_start_pos()[1]];
    m_board[p_move.get_start_pos()[0]][p_move.get_start_pos()[1]] = NA;
    // Castling
    if (chess_piece == wK && p_move.get_start_pos()[0] == 7 && p_move.get_start_pos()[1] == 4 && p_move.get_end_pos()[0] == 7 && p_move.get_end_pos()[1] == 6)
    {
        m_board[7][7] = NA;
        m_board[7][5] = wR;
    }
    else if (chess_piece == wK && p_move.get_start_pos()[0] == 7 && p_move.get_start_pos()[1] == 4 && p_move.get_end_pos()[0] == 7 && p_move.get_end_pos()[1] == 2)
    {
        m_board[7][0] = NA;
        m_board[7][3] = wR;
    }
    else if (chess_piece == bK && p_move.get_start_pos()[0] == 0 && p_move.get_start_pos()[1] == 4 && p_move.get_end_pos()[0] == 0 && p_move.get_end_pos()[1] == 6)
    {
        m_board[0][7] = NA;
        m_board[0][5] = bR;
    }
    else if (chess_piece == bK && p_move.get_start_pos()[0] == 0 && p_move.get_start_pos()[1] == 4 && p_move.get_end_pos()[0] == 0 && p_move.get_end_pos()[1] == 2)
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
    else if (chess_piece == bR && p_move.get_start_pos()[0] == 0 && p_move.get_start_pos()[1] == 0 || p_move.get_end_pos()[0] == 0 && p_move.get_end_pos()[1] == 0)
    {
        m_black_long_castling_allowed = false;
    }
    else if (chess_piece == bR && p_move.get_start_pos()[0] == 0 && p_move.get_start_pos()[1] == 7 || p_move.get_end_pos()[0] == 0 && p_move.get_end_pos()[1] == 7)
    {
        m_black_short_castling_allowed = false;
    }
    else if (chess_piece == wR && p_move.get_start_pos()[0] == 7 && p_move.get_start_pos()[1] == 0 || p_move.get_end_pos()[0] == 7 && p_move.get_end_pos()[1] == 0)
    {
        m_white_long_castling_allowed = false;
    }
    else if (chess_piece == wR && p_move.get_start_pos()[0] == 7 && p_move.get_start_pos()[1] == 7 || p_move.get_end_pos()[0] == 7 && p_move.get_end_pos()[1] == 7)
    {
        m_white_short_castling_allowed = false;
    }

    //en passant eating
    if (p_move.get_end_pos()[1] == m_en_passant_col[WHITE] && p_move.get_end_pos()[0] == 5)
    {
        m_board[4][m_en_passant_col[WHITE]] = NA;
    }
    else if (p_move.get_end_pos()[1] == m_en_passant_col[BLACK] && p_move.get_end_pos()[0] == 2)
    {
        m_board[3][m_en_passant_col[BLACK]] = NA;
    }

    //en passant check
    int moves = std::abs(p_move.get_end_pos()[0] - p_move.get_start_pos()[0]);
    if (chess_piece == wP && moves == 2)
    {
        m_en_passant_col[WHITE] = p_move.get_end_pos()[1];
        m_en_passant_col[BLACK] = -1;
    }
    else if (chess_piece == bP && moves == 2)
    {
        m_en_passant_col[BLACK] = p_move.get_end_pos()[1];
        m_en_passant_col[WHITE] = -1;
    }
    else
    {
        m_en_passant_col[BLACK] = -1;
        m_en_passant_col[WHITE] = -1;
    }

    m_board[p_move.get_end_pos()[0]][p_move.get_end_pos()[1]] = chess_piece;
}

void Position::end_turn() {
    if (m_movingturn == WHITE) {
        m_movingturn = BLACK;
    }
    else if (m_movingturn == BLACK) {
        m_movingturn = WHITE;
    }
}

bool Position::can_promote(const Move& p_move) {
    int chess_piece = m_board[p_move.get_end_pos()[0]][p_move.get_end_pos()[1]];
    int player = get_chess_piece_color(chess_piece);
    return is_promotable(chess_piece, p_move.get_end_pos()[0]);
}

void Position::promote(std::array<int, 2> end_pos, int chess_piece) {
    m_board[end_pos[0]][end_pos[1]] = chess_piece;
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

    row_now = row;
    col_now = col;
    if (m_en_passant_col[WHITE] != -1)
    {
        //blacks turn
        if (row_now == 4 && (col_now == m_en_passant_col[WHITE]-1 || col_now == m_en_passant_col[WHITE]+1))
        {
            out.push_back(Move({ row, col }, { 5, m_en_passant_col[WHITE] }));
        }
    }
    if (m_en_passant_col[BLACK] != -1)
    {
        //whites turn
        if (row_now == 3 && (col_now == m_en_passant_col[BLACK]-1 || col_now == m_en_passant_col[BLACK]+1))
        {
            out.push_back(Move({ row, col }, { 2, m_en_passant_col[BLACK] }));
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
    int board_size = 8;
    for (int row = 0; row < board_size; row++) {
        std::string map = " ";
        for (int col= 0; col < board_size + 1; col ++) {
            if (col == board_size) {
                map += "+\n";
            } else {
                map += "+----";
            }
            
        }
        for (int col= 0; col < board_size; col ++) {
            const std::string piece = chess_piece_to_string(m_board[row][col]);
            if (col == board_size) {
                map += "|";
            }
            else if (col == 0) {
                map += std::to_string(board_size - row) + (piece.size() > 0 ? "| " + piece + " " : "|    ");
            } else {
                map += piece.size() > 0 ? "| " + piece + " " : "|    ";
            }
        }
        if (row == board_size-1) {
            map += "\n ";
            for (int col= 0; col < board_size + 1; col ++) {
                if (col == board_size) {
                    map += "+";
                } else {
                    map += "+----";
                }
                
            }
        }
        std::cout<<map;
        std::cout<<std::endl;
    }
    std::cout<< "   A    B    C    D    E    F    G    H"<<std::endl;
}

void Position::render_legal_moves(const vector<Move>& p_moves) {
    std::cout<<"valid moves:"<<std::endl;
    for (int i = 0; i < p_moves.size(); i++) {
        std::cout<<" "<<p_moves[i].get_coords()<<std::endl;
    }

    std::cout<<"Legal move count: "<<p_moves.size()<<std::endl;
}