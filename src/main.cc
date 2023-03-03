#include <string>
#include <cstdlib>
#include <iostream>
#include <ncurses.h>

typedef char pixel;

const pixel BLANK = '.';
const pixel BLOCK = 'T';
const pixel WALL  = '#';

enum Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

class Piece {
    int x;
    int y;
    std::string piece_grid;
    int rotation_angle; 

public:
    static std::string SQ;
    static std::string I;
    static std::string L;
    static std::string BL;
    static std::string S;
    static std::string BS;
    static std::string T;

    Piece(int x, int y, std::string piece_grid) {
        this->x = x;
        this->y = y;
        this->piece_grid = piece_grid;
        this->rotation_angle = 0;
    }

    void rotate() {
        this->rotation_angle++;
        this->rotation_angle %= 4;
    }

    void move(Direction direction) {
        switch(direction) {
            case UP : this->x--;
            case DOWN : this->x++;
            case LEFT : this->y--;
            case RIGHT : this->y++;
        }
    }

    pixel get_block(int x, int y) {
        int i = this->index(x-this->x, y-this->y);
        return this->piece_grid[i];
    }

    std::string get_grid() {
        return this->piece_grid;
    }
    
    int index(int x, int y) {
        switch(this->rotation_angle) {
            case 0: return 4*y + x;
            case 1: return y - 4*x + 12;
            case 2: return 15 - x - 4*y; 
            case 3: return 3 - y + 4*x;
        }
        return 4*y + x;
    }
    
    bool is_piece(int x, int y) {
        return
            x >= this->x && x < (this->x + 4)
            && y >= this->y && y < (this->y + 4);
    }

};

std::string Piece::SQ = ".....TT..TT.....";
std::string Piece::I  = "..T...T...T...T.";
std::string Piece::L  = "..T...T..TT.....";
std::string Piece::BL = "..T...T...TT....";
std::string Piece::S  = ".T...TT...T.....";
std::string Piece::BS = "..T..TT..T......";
std::string Piece::T  = "..T..TT...T.....";


class Grid {
    int height;
    int width;
    pixel* grid;

    int index(int x, int y) {
        return this->width*y + x;
    }

    void init() {
        for(int i=0; i<this->height; ++i) {
            for(int j=0; j<this->width; ++j) {
                if (i == 0 || j == 0) {
                    int index = this->index(j,i);
                    this->grid[index] = WALL;
                } else if (i == this->height-1 || j == this->width-1) {
                    int index = this-> index(j, i);
                    this->grid[index] = WALL;
                } else {
                    int index = this-> index(j, i);
                    this->grid[index] = BLANK;   
                }
            }
        }
    }

public:
    Grid(int height, int width) {
        this->height = height;
        this->width = width;
        this->grid = new char[height*width];
        this->init();
    }

    int get_height() {
        return this->height;
    }
    
    int get_width() {
        return this->width;
    }

    void write(int x, int y, pixel p) {
        this->grid[this->index(x,y)] = p;
    }

    pixel read(int x, int y) {
        return this->grid[this->index(x, y)];
    }

    bool is_colliding(Piece piece, Direction direction) {
        return false;
    }
};

class Game  {
    bool game_over;
    double speed;
    unsigned int score;
    Grid grid;
    Piece current_piece;

    void draw() {
        const int height = this->grid.get_height();
        const int width = this->grid.get_width();

        for(int y=0; y < height; ++y) {
            for (int x=0; x < width; ++x) {
                if (this->current_piece.is_piece(x,y)) {
                    pixel p = this->current_piece.get_block(x, y);
                    this->draw_block(x, y, p);
                } else {
                    pixel p = this->grid.read(x,y);
                    this->draw_block(x, y, p);
                }
            }
        }
        this->draw_score();
    }

    void draw_block(int x, int y, pixel p) {
        char ch = this->block_to_char(p);
        mvaddch(x, y, ch);
    }

    char block_to_char(pixel block) {
        switch(block) {
            case '.' : return ' ';
            default : return block;
        }
    }

    void draw_score() {
        int x = this->grid.get_width() * 2;

        mvprintw(2, x, "score: %d", this->score);
    }

    void init() {
        initscr();
        clear();
        cbreak();
        noecho();
        curs_set(0);
    }

    void input() {
        keypad(stdscr, true);
        halfdelay(1);
        const int ESC = 27;
        int ch = getch();
        switch(ch) {
            case KEY_RIGHT:
                this->move_piece(RIGHT);
                break;
            case KEY_LEFT:
                this->move_piece(LEFT);
                break;
            case KEY_DOWN:
                this->tick();
                break;
            case KEY_UP:
                this->current_piece.rotate();
                break;
            case 'q':
            case ESC:
                this->game_over = true;
                break;
        }
    }

    void move_piece(Direction direction) {
        if(this->grid.is_colliding(this->current_piece, direction)) {
            return;
        }
        this->current_piece.move(direction);
    }

    void tick() {
        if (this->grid.is_colliding(this->current_piece, DOWN)) {
            // fix piece
            // completed lines
            // generate new random piece
            this->check_game_over();
        } else {
            this->current_piece.move(DOWN);
        }
    }

    void check_game_over() {
        int width = this->grid.get_width();
        for(int x=0; x<width; ++x) {
            pixel p = this->grid.read(x, 1);
            if(p == BLOCK) {
                this->game_over = true;
                return;
            }
        }
    }

public:
    Game(): grid(18,12), current_piece(2, 1, Piece::SQ) {
        this->game_over = false;
        this->speed = 1.00;
        this-> score = 0;
        std::cout << this->current_piece.get_grid() << std::endl;
        this->init();
        // generate first random piece
    }

    void play() {
        while(!this->game_over) {
            this->draw();
            this->input();
            this->tick();
            this->game_over = true;
        }
        this->end_game();
    }
    
    void end_game() {
        getchar();
        endwin();
    }

};

int main() {
    Game game = Game();
    game.play();
}