#include <term.h> 

////////////////
// Term Class //
////////////////

Term::Term(){
   
    // Get terminal height and width 
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    WIDTH  = w.ws_col;
    HEIGHT = w.ws_row;

    game = new Game(WIDTH, HEIGHT);

    enableRawMode();
    printf("\033[?25l"); // Turns off cursor
    fflush(stdout);

    listening = true;
    t_listen = std::thread(&Term::listen, this); 

    //game = new Game(WIDTH, HEIGHT);
    render();
}

Term::~Term(){
    disableRawMode();
    t_listen.join();

    // Clear the screen
    clear();
    printf("\033[?25h"); 
    fflush(stdout);
}

void Term::render(){
    std::string rstring(game->render());
    ssize_t res = write(STDOUT_FILENO, rstring.c_str(), rstring.size());
}

bool Term::is_running(){
    return listening;
}

void Term::clear(){
    printf("\033[H\033[2J");
}

void Term::enableRawMode(){
    // Get default termios setting
    tcgetattr(STDIN_FILENO, &default_termios);

    if (tcgetattr(STDIN_FILENO, &default_termios) == -1) 
        die("tcgetattr");

    // Set terminal to Raw mode
    struct termios raw = default_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    //raw.c_oflag &= ~(OPOST);

    // Update temrinal with new setting
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcgetattr");
}

void Term::disableRawMode(){
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &default_termios) == -1)
        die("tscsetattr");
}

void Term::die(const char *s) {
    perror(s);
    exit(1);
}

void Term::listen(){
    char c;
    while (1) {
        //char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
        if (c == '\033') break;

        game->input_queue.push_back(c);
    }
    listening = false;
    game->end();
}

