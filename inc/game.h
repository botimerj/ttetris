#ifndef GAME_H
#define GAME_H

#include <atomic>
#include <thread>
#include <string>
#include <time.h>
#include <vector>
#include <sys/time.h>

// Render classes 
class Relement{
public:
    std::string str;
    int x, y;
    Relement(int x, int y, std::string str);
};

class Rvector{
public:
    std::atomic_flag aflag = ATOMIC_FLAG_INIT;
    std::vector <Relement> vec;

    Rvector();

    std::string full;
    std::string shaded;
    std::string light;
    std::string blank;

    void push_back(Relement e);
    std::string to_string();
    //void clear();
};

// Shape classes 
class Coor{
public:
    int x;
    int y;

    Coor();
    Coor(int, int);

    Coor& operator=(const Coor& in);
    Coor operator+(const Coor& in);
    bool operator==(const Coor& in);
};

class Shape{
public:
    int idx;
    Coor origin;
    std::vector<Coor> squares;

    Shape();
    Shape(const Shape&);

    void rotate(bool dir);

    void up();
    void down();
    void left();
    void right();

    bool check_collision(std::vector<Coor> board);
};

// Game class 
class Game{
public:

    // Game parameters 
    std::thread t_game;
    unsigned int WIDTH;
    unsigned int HEIGHT;
    bool running;

    // Rendering
    Rvector rvector;
    std::vector<std::string> colors;

    void draw_blank_board();
    void draw_box(int, int, int, int);
    void draw_current_shape(Shape* current_prev);
    void draw_projection_shape(Shape* prev);
    void draw_board_box();
    void draw_next_box();
    void draw_saved_box();
    void draw_counter_boxes();
    std::string render();


    // Management functions 
    Game(unsigned int w, unsigned int h);
    ~Game();
    bool is_running();

    // Input 
    std::vector<char> input_queue;
    void input_handler(char c);

    // Gameplay variables
    std::vector<Shape> shapes;
    int shape_idx;
    std::vector<int> shape_idx_vec;

    Shape *current;
    Shape *projection;
    Shape *next;
    Shape *saved;

    std::vector<Coor> board;

    int level;
    int lines;
    int score;

    double game_speed;

    // Gameplay functions
    void reset_game();
    void loop();
    void end();
    void update();

    // Helper functions
    double get_wall_time();
    void reset_board();
    void create_shapes();
    int erase_full_rows();
    void shuffle_shape_idx_vec();
    void calculate_projection();
    bool down_tick();
};

#endif
