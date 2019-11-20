#ifndef TERM_H
#define TERM_H

#include <unistd.h> 
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h> 
#include <thread>
#include <string>
#include <time.h>
#include <vector>

// Custom libraries
#include <game.h>

class Term{
public:
    struct termios default_termios;
    unsigned int WIDTH;
    unsigned int HEIGHT;
    std::thread t_listen;
    bool listening;

    // Constructor/Destructor
    Term();
    ~Term();
    
    // Terminal setup and error handling
    void enableRawMode();
    void disableRawMode();
    void die(const char *s);

    // stdin handler
    bool is_running();
    void listen();

    // Render
    void render();

    Game * game;
    // Cursor manip
    void clear();

};

#endif
