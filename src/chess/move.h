#pragma once
#include <iostream>
#include <string>
#include <cctype>
#include <array>
using namespace std;

// Describes change in position.
class Move {
public: 
    Move() {};
    Move(array<int, 2> start_pos, array<int, 2> end_pos) {
        m_start_pos[0] = start_pos[0];
        m_start_pos[1] = start_pos[1];

        m_end_pos[0] = end_pos[0];
        m_end_pos[1] = end_pos[1];
    }
    Move(const string& s)
    {
        //changes the numbers in the string to int and subtracts from 8 to reverse the order
        m_start_pos[0] = 8 - stoi(string(1, s[1]));
        m_end_pos[0] = 8 - stoi(string(1, s[3]));

        //changes the letters in the string to integers that correspond with alphabetical order
        const string alph_order = "abcdefgh";
        if (isalnum(s[0]))
        {
            m_start_pos[1] = alph_order.find(tolower(s[0]));
        }
        if (isalnum(s[2]))
        {
            m_end_pos[1] = alph_order.find(tolower(s[2]));
        }
    }
    string get_coords() const;
    std::array<int, 2> get_start_pos() const {return {m_start_pos[0], m_start_pos[1]};};
    std::array<int, 2> get_end_pos() const {return {m_end_pos[0], m_end_pos[1]};};
private: 
    // rows and cols
    int m_start_pos[2];
    int m_end_pos[2];
};