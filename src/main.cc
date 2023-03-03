#include <string>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>
#include <random>

typedef char pixel;
const static std::string GAME = "POOP TETRIS";
const pixel BLANK = '.';
const pixel BLOCK = 'T';
const pixel FIXED_BLOCK = 'B';
const pixel WALL = '#';

const char UP = 'w';
const char DOWN = 's';
const char LEFT = 'a';
const char RIGHT = 'd';

struct Point
{
    int x;
    int y;
    Point(int x, int y) : x(x), y(y) {}
};
class Piece
{
    Point coord;
    std::string piece_grid;
    int rotation_angle;

public:
    static int SIDE;

    static std::string SQ;
    static std::string I;
    static std::string L;
    static std::string BL;
    static std::string S;
    static std::string BS;
    static std::string T;

    Piece(int x, int y, std::string piece_grid) : coord(x, y)
    {
        this->piece_grid = piece_grid;
        this->rotation_angle = 0;
    }

    static Piece random()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(0, 6);
        std::string pieces[] = {
            Piece::SQ,
            Piece::I,
            Piece::L,
            Piece::BL,
            Piece::S,
            Piece::BS,
            Piece::T,
        };
        return Piece(3, 1, pieces[distr(gen)]);
    }

    void rotate()
    {
        this->rotation_angle++;
        this->rotation_angle %= 4;
    }

    void move(char direction)
    {
        switch (direction)
        {
        case UP:
            this->coord.y--;
            break;
        case DOWN:
            this->coord.y++;
            break;
        case LEFT:
            this->coord.x--;
            break;
        case RIGHT:
            this->coord.x++;
            break;
        }
    }

    pixel get_pixel_by_coord(int x, int y)
    {
        int i = this->index(x - this->coord.x, y - this->coord.y);
        return this->piece_grid[i];
    }

    pixel get_pixel(int x, int y)
    {
        int i = this->index(x, y);
        return this->piece_grid[i];
    }

    int get_x()
    {
        return this->coord.x;
    }

    int get_y()
    {
        return this->coord.y;
    }

    int index(int x, int y)
    {
        switch (this->rotation_angle)
        {
        case 0:
            return SIDE * y + x;
        case 1:
            return y - SIDE * x + 12;
        case 2:
            return 15 - x - SIDE * y;
        case 3:
            return 3 - y + SIDE * x;
        }
        return 4 * y + x;
    }

    bool is_block(int x, int y)
    {
        int index = this->index(x - this->coord.x, y - this->coord.y);
        return x >= this->coord.x && x < (this->coord.x + 4)    //
               && y >= this->coord.y && y < (this->coord.y + 4) //
               && this->piece_grid[index] == BLOCK;
    }
};

std::string Piece::SQ = "_____TT__TT_____";
std::string Piece::I = "__T___T___T___T_";
std::string Piece::L = "__T___T__TT_____";
std::string Piece::BL = "__T___T___TT____";
std::string Piece::S = "_T___TT___T_____";
std::string Piece::BS = "__T__TT__T______";
std::string Piece::T = "__T__TT___T_____";
int Piece::SIDE = 4;

class Grid
{
    int height;
    int width;
    pixel *grid;

    int index(int x, int y)
    {
        return this->width * y + x;
    }

    void init()
    {
        for (int i = 0; i < this->height; ++i)
        {
            for (int j = 0; j < this->width; ++j)
            {
                if (i == 0 || j == 0)
                {
                    int index = this->index(j, i);
                    this->grid[index] = WALL;
                }
                else if (i == this->height - 1 || j == this->width - 1)
                {
                    int index = this->index(j, i);
                    this->grid[index] = WALL;
                }
                else
                {
                    int index = this->index(j, i);
                    this->grid[index] = BLANK;
                }
            }
        }
    }

public:
    Grid(int width, int height)
    {
        this->height = height;
        this->width = width;
        this->grid = new char[height * width];
        this->init();
    }

    int get_height()
    {
        return this->height;
    }

    int get_width()
    {
        return this->width;
    }

    void fix_piece(Piece piece)
    {
        int index, x, y;
        for (int py = 0; py < Piece::SIDE; py++)
        {
            for (int px = 0; px < Piece::SIDE; px++)
            {
                if (piece.get_pixel(px, py) == BLOCK)
                {
                    x = piece.get_x() + px;
                    y = piece.get_y() + py;
                    index = this->index(x, y);
                    this->grid[index] = FIXED_BLOCK;
                }
            }
        }
    }

    pixel read(int x, int y)
    {
        return this->grid[this->index(x, y)];
    }

    bool is_colliding(Piece piece, char direction)
    {
        int x, y, index;
        Point next_delta = this->next_delta(direction);
        pixel next_pixel;

        for (int py = 0; py < Piece::SIDE; py++)
        {
            for (int px = 0; px < Piece::SIDE; px++)
            {
                mvprintw(4, 24, "pixel (%d, %d) ' '", piece.get_x() + px, piece.get_y());
                if (piece.get_pixel(px, py) == BLOCK)
                {
                    x = piece.get_x() + px + next_delta.x;
                    y = piece.get_y() + py + next_delta.y;
                    index = this->index(x, y);
                    next_pixel = this->grid[index];
                    mvprintw(5, 24, "pixel (%d, %d) has next_piece(%d, %d) '%c'", piece.get_x() + px, piece.get_y() + py, x, y, next_pixel);
                    if (next_pixel == WALL || next_pixel == FIXED_BLOCK)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    Point next_delta(char direction)
    {
        Point next = Point(0, 0);
        switch (direction)
        {
        case UP:
            next.y = -1;
            break;
        case DOWN:
            next.y = +1;
            break;
        case RIGHT:
            next.x = +1;
            break;
        case LEFT:
            next.x = -1;
            break;
        }
        return next;
    }
};

class Game
{
    bool game_over;
    int speed;
    unsigned int score;
    Grid grid;
    Piece current_piece;

    void init()
    {
        initscr();
        clear();
        cbreak();
        noecho();
        curs_set(0);
    }

    void draw()
    {
        const int height = this->grid.get_height();
        const int width = this->grid.get_width();

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                if (this->current_piece.is_block(x, y))
                {
                    pixel p = this->current_piece.get_pixel_by_coord(x, y);
                    this->draw_block(x, y, p);
                }
                else
                {
                    pixel p = this->grid.read(x, y);
                    this->draw_block(x, y, p);
                }
            }
        }
        this->draw_score();
    }

    void draw_block(int x, int y, pixel p)
    {
        char ch = this->block_to_char(p);
        mvaddch(y, x, ch);
    }

    char block_to_char(pixel block)
    {
        switch (block)
        {
        case '.':
            return ' ';
        default:
            return block;
        }
    }

    void draw_score()
    {
        int x = this->grid.get_width() * 2;

        mvprintw(2, x, "score: %d", this->score);
    }

    void input()
    {
        keypad(stdscr, true);
        halfdelay(1);
        const int ESC = 27;
        int ch = getch();

        switch (ch)
        {
        case KEY_DOWN:
        case DOWN:
            this->push_down();
            break;
        case KEY_UP:
        case UP:
            this->current_piece.rotate();
            break;
        case LEFT:
        case RIGHT:
            this->move_piece(ch);
            break;
        case 'q':
        case ESC:
            this->game_over = true;
            break;
        }
    }

    void move_piece(char direction)
    {
        if (!this->grid.is_colliding(this->current_piece, direction))
        {
            this->current_piece.move(direction);
        }
    }

    void push_down()
    {
        if (this->grid.is_colliding(this->current_piece, DOWN))
        {
            this->grid.fix_piece(this->current_piece);
            // completed lines
            this->current_piece = Piece::random();
            this->check_game_over();
        }
        else
        {
            this->current_piece.move(DOWN);
        }
    }

    void tick()
    {
        static int speed_counter = 0;
        if (speed_counter == speed)
        {
            speed_counter = 0;
            this->push_down();
        }
        else
        {
            speed_counter++;
        }
    }

    void check_game_over()
    {
        int width = this->grid.get_width();
        for (int x = 0; x < width; ++x)
        {
            pixel p = this->grid.read(x, 1);
            if (p == BLOCK)
            {
                this->game_over = true;
                return;
            }
        }
    }

    void end_game()
    {
        mvprintw(this->grid.get_height() / 2, this->grid.get_width() / 2, "GAME OVER");
        getchar();
        endwin();
    }

public:
    Game() : grid(20, 14), current_piece(Piece::random())
    {
        this->game_over = false;
        this->speed = 50;
        this->score = 0;
        this->init();
    }

    void play()
    {
        int speed_counter = 0;
        while (!this->game_over)
        {
            usleep(1000 * 20);
            this->draw();
            this->input();
            this->tick();
        }
        this->end_game();
    }
};

int main()
{
    Game game = Game();
    game.play();
}