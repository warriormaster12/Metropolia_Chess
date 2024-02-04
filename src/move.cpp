#include "move.h"

string Move::get_coords() const {
    std::string out = "";
    const std::string letters = "abcdefgh";
    out += letters[m_start_pos[1]];
    out += to_string(7-m_start_pos[0] + 1);
    out += letters[m_end_pos[1]];
    out += to_string(7-m_end_pos[0] + 1);
    return out;
}