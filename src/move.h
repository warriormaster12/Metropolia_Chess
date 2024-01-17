#pragma once

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
private: 
    // rows and cols
    int m_start_pos[2];
    int m_end_pos[2];

    friend class Position;
};