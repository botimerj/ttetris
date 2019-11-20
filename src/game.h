#ifndef GAME_H
#define GAME_H

#include <atomic>
#include <thread>
#include <string>
#include <time.h>
#include <vector>
#include <sys/time.h>


class Game{
public:

    // Game parameters 
    std::thread t_game;
    unsigned int WIDTH;
    unsigned int HEIGHT;
    int xmax, ymax, xmin, ymin;
    bool running;

    // Game variables
    double gtime;
    unsigned int speed;
    unsigned int length;
    unsigned int heading;
    unsigned int heading_next;
    Coor * head;
    Coor * tail;
    Coor * food;
    std::vector<Coor> snake;

    std::vector<std::string> colors;
    int cidx;
    int r,g,b;

    // Screen coords to update
    Uvector uvector;

    // Management functions 
    Game(unsigned int w, unsigned int h);
    ~Game();
    void input_handler(char c);
    std::string render();
    void border();

    // State functions
    void loop();
    void reset();
    void end();

    // Gameplay functions 
    void update_speed(bool up);
    void update_length(bool up);
    void update_clock();
    void update_movement();
    void spawn_food();

    // Helper functions
    double get_wall_time();
    bool   coor_in_snake(Coor test_coor);
};

#endif
