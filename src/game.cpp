#include <game.h>

/////////////////////
// Shapes and Coor //
/////////////////////

// Coor (Coordinate) class
Coor::Coor() : x(0), y(0) {}
Coor::Coor(int x_, int y_) : x(x_), y(y_) {}
Coor& Coor::operator=(const Coor& in){
    if(this == &in)
        return *this; 
    x = in.x; y = in.y;
    return *this;
}
Coor Coor::operator+(const Coor& in){
    Coor res;
    res.x = x + in.x;
    res.y = y + in.y;
    return res;
}
bool Coor::operator==(const Coor& in){
    return in.x == x && in.y == y;
}

// Shape Class
Shape::Shape(){
    idx = 0;
    origin.x = 0; origin.y = 0;
    squares.push_back(origin);
}

Shape::Shape(const Shape &in){
    idx = in.idx;
    origin = in.origin;
    for(auto e=in.squares.begin();e!=in.squares.end(); ++e)
        squares.push_back(*(e));
}

void Shape::rotate(bool dir){
    std::vector<Coor> new_squares;
    for(auto e=squares.begin();e!=squares.end();++e){
        if(dir)
            new_squares.push_back(Coor(-e->y, e->x));
        else
            new_squares.push_back(Coor(e->y, -e->x));
    }
    squares = new_squares;
}

void Shape::up(){
    origin.y -= 1;
}

void Shape::down(){
    origin.y += 1;
}

void Shape::left(){
    origin.x -= 1;
}

void Shape::right(){
    origin.x += 1;
}

bool Shape::check_collision(std::vector<Coor> board){
    for(auto eb=board.begin();eb!=board.end();++eb){
        for(auto es=squares.begin();es!=squares.end();++es){
            if((*es + origin) == *eb)
                return true;
        }
    }
    return false;
}

////////////
// Render //
////////////

Relement::Relement(int x, int y, std::string str){
    this->x = x; this->y = y; this->str = str;
}

Rvector::Rvector(){
    full   = "\u2588\u2588";
    shaded = "\u2593\u2593";
    light  = "\u2591\u2591";
    blank  = "  ";
}

void Rvector::push_back(Relement e){
    do { } while (atomic_flag_test_and_set(&aflag));
    vec.push_back(e);
    atomic_flag_clear(&aflag);
}

std::string Rvector::to_string(){
    do { } while (atomic_flag_test_and_set(&aflag));
    std::string tmp;
    std::string pstring;
    for(int i = 0; i < vec.size(); i++){
        tmp = "\033["+std::to_string(vec[i].y)+";"+
                      std::to_string(vec[i].x)+"H"+
                      vec[i].str;
        pstring.append(tmp);
    }
    vec.clear();
    atomic_flag_clear(&aflag);
    return pstring;
}

////////////////
// Game Class //
////////////////

// Management functions
Game::Game(unsigned int WIDTH_, unsigned int HEIGHT_){

    // Set the Seed
    srand(time(NULL));

    // Check window size
    WIDTH = WIDTH_; HEIGHT = HEIGHT_;
    if(WIDTH_ < 39 || HEIGHT_ < 25){
        printf("Screen dimensions are too small... Please resize and try again.\n");
        exit(1);
    }

    // Initialize gameplay variables 
    current      = 0;
    projection = 0;
    next         = 0;
    saved        = 0;

    create_shapes();
    reset_game();

    // Start game loop
    running = true; 
    t_game = std::thread(&Game::loop, this); 

    // Drawing 
    colors.push_back("\033[37m"); // white
    colors.push_back("\033[31m"); // red
    colors.push_back("\033[32m"); // green
    colors.push_back("\033[33m"); // brown
    colors.push_back("\033[34m"); // blue
    colors.push_back("\033[35m"); // magenta
    colors.push_back("\033[36m"); // cyan

}

Game::~Game(){
    delete current;
    delete next;
    delete saved;

    t_game.join();
}

bool Game::is_running(){
    return running;
}

void Game::input_handler(char c){
    Shape current_prev(*current);
    if (c == 'w'){ 
        current->rotate(true);
        bool valid_rotation = !current->check_collision(board);

        // up
        if(!valid_rotation){
            current->up();
            valid_rotation = !current->check_collision(board);
            if(!valid_rotation)
                current->down();
        }

        // up-up
        if(!valid_rotation){
            current->up();
            current->up();
            valid_rotation = !current->check_collision(board);
            if(!valid_rotation){
                current->down();
                current->down();
            }
        }

        // left
        if(!valid_rotation){
            current->left();
            valid_rotation = !current->check_collision(board);
            if(!valid_rotation)
                current->right();
        }

        // right
        if(!valid_rotation){
            current->right();
            valid_rotation = !current->check_collision(board);
            if(!valid_rotation)
                current->left();
        }

        if(valid_rotation)
            draw_current_shape(&current_prev);
        else
            current->rotate(false);
    }
    else if (c == 'd'){ 
        current->right();
        if(current->check_collision(board))
            current->left();
        else
            draw_current_shape(&current_prev);
    }
    else if (c == 'a'){
        current->left();
        if(current->check_collision(board))
            current->right();
        else
            draw_current_shape(&current_prev);
    }
    else if (c == 's'){ 
        current->down();
        if(current->check_collision(board))
            current->up();
        else
            draw_current_shape(&current_prev);
    }
    else if (c == 'q'){ 
        int current_shape_idx = current->idx;

        Shape tmp(*current);
        delete current;
        if( saved != 0 ){
            // Swap current and saved
            current = new Shape(*saved);
            current->origin = tmp.origin;

            if(current->check_collision(board)){
                delete current;
                current = new Shape(tmp);
                return;
            }

            delete saved;
            saved = new Shape(shapes[current_shape_idx]);
        } else {
            // Save for the first time
            current = new Shape(*next);
            current->origin = Coor(4,0);

            if(current->check_collision(board)){
                delete current;
                current = new Shape(tmp);
                return;
            }

            saved = new Shape(shapes[current_shape_idx]);
            shape_idx += 1;
            if(shape_idx >= 7){
                shape_idx = 0;
                shuffle_shape_idx_vec();
            }
            delete next;
            next = new Shape(shapes[shape_idx_vec[shape_idx]]);
            draw_next_box();
        }

        draw_saved_box();
        draw_current_shape(&tmp);
    }
    else if (c == ' '){ 
        while(!current->check_collision(board))
            current->down();
        current->up();
        draw_current_shape(&current_prev);
    }

    Shape projection_prev(*projection);
    calculate_projection();
    draw_projection_shape(&projection_prev);
}

std::string Game::render(){
    return rvector.to_string();
}

// Drawing
void Game::draw_box(int left, int top, int width, int height){
    //Height >= 2; Width >= 2;

    // Top bottom
    std::string tb;
    for(int i = 0; i < width; i++){ tb.append("\u2550");}
    rvector.push_back(Relement(left, top, tb));
    rvector.push_back(Relement(left, top+height-1, tb));

    // Left right
    std::string lr("\u2551"); 
    for(int i = top; i < top+height; i++){ 
        rvector.push_back(Relement(left,         i, lr.c_str()));
        rvector.push_back(Relement(left+width-1, i, lr.c_str()));
    }

    // Corners
    rvector.push_back(Relement(left,         top,         std::string("\u2554")));
    rvector.push_back(Relement(left+width-1, top,         std::string("\u2557")));
    rvector.push_back(Relement(left,         top+height-1, std::string("\u255A")));
    rvector.push_back(Relement(left+width-1, top+height-1, std::string("\u255D")));
}

void Game::draw_blank_board(){

    // Board
    draw_box(16, 3, 20+2, 20+2);
    rvector.push_back(Relement(16, 2, std::string("==TETRIS==")));

    // Next box
    draw_box(4, 3, 8+2, 5);
    rvector.push_back(Relement(5, 2, std::string("Next")));

    // Saved box
    draw_box(4, 9, 8+2, 5);
    rvector.push_back(Relement(5, 8, std::string("Saved")));
    
    // Level box
    draw_box(5, 16, 6+2, 3);
    rvector.push_back(Relement(6, 16, std::string("Level")));

    // Score box
    draw_box(5, 19, 6+2, 3);
    rvector.push_back(Relement(6, 19, std::string("Score")));

    // Lines box
    draw_box(5, 22, 6+2, 3);
    rvector.push_back(Relement(6, 22, std::string("Lines")));

    // Border
    draw_box(1, 1, 39, 27);

    std::string txt("[Esc]xit [p]ause [h]elp");
    rvector.push_back(Relement(10, 26, txt));
}

void Game::draw_next_box(){
    rvector.push_back(Relement(5, 4, std::string("        ")));
    rvector.push_back(Relement(5, 5, std::string("        ")));
    rvector.push_back(Relement(5, 6, std::string("        ")));
    if( next == 0 )
        return;

    //delete next;
    //next = new Shape(shapes[shape_idx]);

    Coor base(7,5);
    for(auto e=next->squares.begin(); e!=next->squares.end(); ++e){
        Coor c(*e + next->origin);
        if( (c.x + c.y) % 2 ) 
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.full));
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.shaded));
    }
    //draw_shape(next, Coor(7,5));
}

void Game::draw_saved_box(){
    rvector.push_back(Relement(5, 10, std::string("        ")));
    rvector.push_back(Relement(5, 11, std::string("        ")));
    rvector.push_back(Relement(5, 12, std::string("        ")));
    if( saved == 0 )
        return;
    
    Coor base(7,11);
    for(auto e=saved->squares.begin(); e!=saved->squares.end(); ++e){
        Coor c(*e + saved->origin);
        if( (c.x + c.y) % 2 ) 
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.full));
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.shaded));
    }
}

void Game::draw_board_box(){
    Coor base(17,4);

    // Clear playing area 
    std::string blank_row(20, ' ');
    for(int i = 0; i < 20; i++)
        rvector.push_back(Relement(base.x, base.y+i, blank_row));

    //draw_current_shape(current);

    // Draw board
    for(auto e=board.begin(); e!=board.end(); ++e){
        Coor c(*e);
        if(c.y >= 20 || c.y < 0 || c.x < 0 || c.x >= 10) // don't draw out of bounds
            continue;
        else if( (c.x + c.y) % 2 ) 
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.full));
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.shaded));
    }
}

void Game::draw_current_shape(Shape* current_prev){
    if( current == 0 ) 
        return;
    Coor base(17,4);

    // Clear old shape
    for(auto e=current_prev->squares.begin(); e!=current_prev->squares.end(); ++e){
        Coor c(*e + current_prev->origin);
        if(c.y >= 20 || c.y < 0) // don't draw out of bounds
            continue;
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.blank));
    }

    // Draw current shape (but not out of bounds pixels)
    for(auto e=current->squares.begin(); e!=current->squares.end(); ++e){
        Coor c(*e + current->origin);
        if(c.y >= 20 || c.y < 0) // don't draw out of bounds
            continue;
        else if( (c.x + c.y) % 2 ) 
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.full));
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.shaded));
    }
}

void Game::draw_projection_shape(Shape* prev){
    if( projection == 0 ) 
        return;
    Coor base(17,4);

    // Clear old shape
    for(auto e=prev->squares.begin(); e!=prev->squares.end(); ++e){
        Coor c(*e + prev->origin);
        if(c.y >= 20 || c.y < 0) // don't draw out of bounds
            continue;
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.blank));
    }

    // Draw projection shape (but not out of bounds pixels)
    for(auto e=projection->squares.begin(); e!=projection->squares.end(); ++e){
        Coor c(*e + projection->origin);
        if(c.y >= 20 || c.y < 0) // don't draw out of bounds
            continue;
        else
            rvector.push_back(Relement(c.x*2 + base.x, c.y + base.y, rvector.light));
    }

    draw_current_shape(current);
}

void Game::draw_counter_boxes(){
    char buf[16];
    int len_buf;

    // Level box
    len_buf = sprintf(buf, "%4d", level);
    rvector.push_back(Relement(7, 17, std::string(buf, len_buf)));

    // Score box
    len_buf = sprintf(buf, "%4d", score);
    rvector.push_back(Relement(7, 20, std::string(buf, len_buf)));

    // Lines box
    len_buf = sprintf(buf, "%4d", lines);
    rvector.push_back(Relement(7, 23, std::string(buf, len_buf)));
}

// Gameplay functions
void Game::loop(){
    double twall = get_wall_time();
    double twall_new = twall;
    double dt = 0;
    double tick = 0;

    while(running){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // Timing
        twall_new = get_wall_time();
        dt = twall_new - twall;
        twall = twall_new;
        tick += dt;

        // Input handling
        while( !input_queue.empty() ){
            char in = input_queue.back();
            input_queue.pop_back();
            input_handler(in);
        }

        // Game tick 
        if(tick > game_speed){
            tick = 0;
            bool landed = down_tick();
            
            // Losing condition
            if(landed && current->check_collision(board)){
                reset_game();
            }
        }
    }
}

void Game::reset_game(){
    // Reset the board
    reset_board();

    // Reset shape selection
    shape_idx = 0;
    shuffle_shape_idx_vec();

    // Set the current shape
    if( current != 0 )
        delete current;
    current = new Shape(shapes[shape_idx_vec[shape_idx++]]);
    current->origin = current->origin + Coor(4,0);
    calculate_projection();

    // Next shape
    if( next != 0 )
        delete next;
    next = new Shape(shapes[shape_idx_vec[shape_idx++]]);

    // Saved shape
    if( saved != 0 )
        delete saved;
    saved = 0; // no saved shape
    
    level = 1;
    lines = 0;
    score = 0;
    game_speed = 1/(level*0.5 + 1);

    // Reset display and draw the board
    std::string reset_str = "\033[H\033[2J";
    rvector.push_back(Relement(1,1,reset_str));
    draw_blank_board();

    draw_current_shape(current);     // Draw the current shape
    draw_next_box();                 // Draw the next box
    draw_saved_box();                // Draw the saved box 
    draw_counter_boxes();            // Draw counter boxes
    draw_projection_shape(projection); // Draw the current projection shape
}

void Game::end(){
    running = false;
}

// Helper functions
double Game::get_wall_time(){
    struct timeval time;
    if (gettimeofday(&time,NULL)) return 0;
    return (double)time.tv_sec + (double)time.tv_usec * .000001;
}

void Game::create_shapes(){

    // Square shape
    shapes.push_back(Shape());                 // 0█▓▓
    shapes[0].squares.push_back(Coor( 0, 1));  // ▓▓██
    shapes[0].squares.push_back(Coor( 1, 0));
    shapes[0].squares.push_back(Coor( 1, 1));
    shapes[0].idx = 0;

    // Line shape
    shapes.push_back(Shape());                // ▓▓0█▓▓██
    shapes[1].squares.push_back(Coor(-1, 0));   
    shapes[1].squares.push_back(Coor( 1, 0));
    shapes[1].squares.push_back(Coor( 2, 0));
    shapes[1].idx = 1;

    // T-shape
    shapes.push_back(Shape());                 // ▓▓  
    shapes[2].squares.push_back(Coor( 0,-1));  // 0█▓▓
    shapes[2].squares.push_back(Coor( 0, 1));  // ▓▓  
    shapes[2].squares.push_back(Coor( 1, 0));
    shapes[2].idx = 2;

    // L-shape
    shapes.push_back(Shape());                // ▓▓  
    shapes[3].squares.push_back(Coor( 0,-1)); // 0█  
    shapes[3].squares.push_back(Coor( 0, 1)); // ▓▓██
    shapes[3].squares.push_back(Coor( 1, 1));
    shapes[3].idx = 3;

    // Li-shape                                  
    shapes.push_back(Shape());                 // ▓▓██
    shapes[4].squares.push_back(Coor( 0,-1));  // 0█  
    shapes[4].squares.push_back(Coor( 0, 1));  // ▓▓  
    shapes[4].squares.push_back(Coor( 1,-1));
    shapes[4].idx = 4;

    // S-shape                                  
    shapes.push_back(Shape());                 //   ██
    shapes[5].squares.push_back(Coor( 1, 0));  // 0█▓▓
    shapes[5].squares.push_back(Coor( 0, 1));  // ▓▓  
    shapes[5].squares.push_back(Coor( 1,-1));
    shapes[5].idx = 5;

    // Si-shape                                  
    shapes.push_back(Shape());                 // ▓▓  
    shapes[6].squares.push_back(Coor( 0,-1));  // 0█▓▓
    shapes[6].squares.push_back(Coor( 1, 0));  //   ██
    shapes[6].squares.push_back(Coor( 1, 1));
    shapes[6].idx = 6;


    // Index vector for picking the next shape 
    shape_idx = 0;
    for(int i = 0; i < 7; i++)
        shape_idx_vec.push_back(i);
    shuffle_shape_idx_vec();
}

void Game::reset_board(){
    board.clear();

    for(int i=0; i < 10; i++) 
        board.push_back(Coor(i, 20));

    for(int i=0; i < 20; i++) {
        board.push_back(Coor(-1, i));
        board.push_back(Coor(10, i));
    }
}

int Game::erase_full_rows(){
    int rows[20];
    for(int i=0;i<20;i++)
        rows[i] = 0;

    for(auto e=board.begin();e!=board.end();++e){
        // Don't consider board border
        if(e->x >= 0 && e->x < 10 && e->y < 20)
            rows[e->y] += 1;
    }

    // Delete all squares in full rows
    int count = 0;
    for(int i = 0; i < 20; i++){
        if(rows[i] == 10){
            count += 1;
            // Remove row
            auto e = board.begin();
            while(e != board.end()){
                if((e->x >= 0 && e->x < 10 && e->y < 20) && e->y == i)
                    board.erase(e);
                else
                    ++e;
            }
        }
    }

    // Shift squares above full rows 'down'
    for(int i=0;i<20;i++){
        if(rows[i] == 10){
            for(auto e=board.begin();e!=board.end();++e){
                // Don't consider board border
                if((e->x >= 0 && e->x < 10 && e->y < 20) && e->y < i)
                    e->y += 1;
            }
        }
    }

    //        e = board.begin();
    //        while(e != board.end()){
    //            if((e->x >= 0 && e->x < 10 && e->y < 20) && e->y < i)
    //                e->y += 1;
    //            ++e;
    //        }
    //    }
    //}

    return count;
}

void Game::shuffle_shape_idx_vec(){
    int idx; 
    std::vector<int> new_shape_idx_vec;
    for(int i = 0; i < 7; i++){
        idx = rand() % shape_idx_vec.size();
        new_shape_idx_vec.push_back(shape_idx_vec[idx]);
        shape_idx_vec.erase(shape_idx_vec.begin()+idx);
    }
    shape_idx_vec = new_shape_idx_vec; 
    //return new_shape_idx_vec;
    //return shape_idx_vec;
}

void Game::calculate_projection(){
    if(projection != 0)
        delete projection;

    projection = new Shape(*current);
    while(!projection->check_collision(board)){
        projection->down();
    }
    projection->up();
}

bool Game::down_tick(){
    // Move current down
    Shape current_prev(*current);
    current->down();

    if(current->check_collision(board)){
        current->up();

        // Add current shape squares to board
        for(auto e=current->squares.begin();e!=current->squares.end();++e)
            board.push_back(Coor(*e + current->origin));

        // Remove full rows
        int count = erase_full_rows();
        if(count != 0)
            draw_board_box();
        
        // Update counters
        lines += count;
        score += count*count;
        level = lines / 10 + 1;
        game_speed = 1/(level*0.5 + 1);

        // Get new current and projection
        delete current;
        current = new Shape(*next);
        current->origin = current->origin + Coor(4, 0);
        current_prev = *current;
        calculate_projection();
        

        // Get new 'next'
        shape_idx += 1;
        if(shape_idx >= 7){
            shape_idx = 0;
            shuffle_shape_idx_vec();
        }
        delete next;
        next = new Shape(shapes[shape_idx_vec[shape_idx]]);


        // Draw
        draw_counter_boxes();
        draw_projection_shape(projection);
        draw_next_box();
        return true;
    }
    else{ // No collision
        draw_current_shape(&current_prev);
        return false;
    }
}
