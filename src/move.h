#pragma once
#include <iostream>
#include <string>
#include <cctype>
using namespace std;

// Describes change in position.
class Move {
public: 
    Move(int start_pos[2], int end_pos[2]) {
        m_start_pos[0] = start_pos[0];
        m_start_pos[1] = start_pos[1];

        m_end_pos[0] = end_pos[0];
        m_end_pos[1] = end_pos[1];
    }
    //Homework
    // Create a constructor which converts chess coordinates to array indexes
    Move(const string& s)
    {
        //changes the letters in the string to integers that correspond with alphabetical order
        const string alph_order = "abcdefgh";
        if (std::isalnum(s[0]))
        {
            m_start_pos[0] = alph_order.find(std::tolower(s[0]));
        }
        if (std::isalnum(s[2]))
        {
            m_end_pos[0] = alph_order.find(std::tolower(s[2]));
        }
        
        //changes the numbers in the string to int and subtracts 1 to make it an index
        m_start_pos[1] = stoi(string(1, s[1])) - 1;
        m_end_pos[1] = stoi(string(1, s[3])) - 1;
    }
    
private: 
    // rows and cols
    int m_start_pos[2];
    int m_end_pos[2];

    friend class Position;
};