#include <string>
#include <cstdlib>
#include <ncurses.h>
#include <unistd.h>
#include <random>

typedef char pixel;
const static std::string GAME = "POOP TETRIS";
const pixel BLANK = '.';
const pixel BLOCK = 'Z';
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

    void rotate(int direction)
    {
        this->rotation_angle += direction;
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

std::string Piece::SQ = "_____ZZ__ZZ_____";
std::string Piece::I = "__Z___Z___Z___Z_";
std::string Piece::L = "__Z___Z__ZZ_____";
std::string Piece::BL = "__Z___Z___ZZ____";
std::string Piece::S = "_Z___ZZ___Z_____";
std::string Piece::BS = "__Z__ZZ__Z______";
std::string Piece::T = "__Z__ZZ___Z_____";
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

    void write(int x, int y, pixel p)
    {
        const int index = this->index(x, y);
        this->grid[index] = p;
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
                    this->write(x, y, FIXED_BLOCK);
                }
            }
        }
    }

    int empty_lines(Piece piece)
    {
        int cleared_lines = 0;
        int py = piece.get_y();
        int width = this->width;
        int last_line = this->height - 1;

        for (int y = py + 3; y >= py; y--)
        {
            if (y < last_line && y > 0 && this->is_line_full(y))
            {
                this->clear_line(y);
                cleared_lines++;
            }
        }

        return cleared_lines;
    }

    bool is_line_full(int line)
    {
        const int columns = this->width - 1;
        for (int x = 1; x < columns; x++)
        {
            if (this->read(x, line) == BLANK)
            {
                return false;
            }
        }
        return true;
    }

    void clear_line(int line)
    {
        const int columns = this->width;
        for (int y = line; y > 1; y--)
        {
            for (int x = 0; x < columns; x++)
            {
                const pixel above = this->read(x, y - 1);
                this->write(x, y, above);
            }
        }

        for (int x = 2; x < columns; x++)
        {
            this->write(x - 1, 1, BLANK);
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
                if (piece.get_pixel(px, py) == BLOCK)
                {
                    x = piece.get_x() + px + next_delta.x;
                    y = piece.get_y() + py + next_delta.y;
                    index = this->index(x, y);
                    next_pixel = this->grid[index];
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

    bool is_piece_overlapping(Piece piece)
    {
        int x, y;
        for (int py = 0; py < Piece::SIDE; ++py)
        {
            for (int px = 0; px < Piece::SIDE; ++px)
            {
                x = px + piece.get_x();
                y = py + piece.get_y();
                pixel piece_pixel = piece.get_pixel(px, py);
                pixel grid_pixel = this->read(x, y);
                if (piece_pixel == BLOCK && grid_pixel != BLANK)
                {
                    return true;
                }
            }
        }
        return false;
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
        case FIXED_BLOCK:
            return 'Z';
        case BLOCK:
            return 'B';
        case WALL:
            return '#';
        default:
        case BLANK:
            return '.';
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
            this->rotate_piece(1);
            break;
        case 'z':
            this->rotate_piece(3);
            break;
        case LEFT:
        case KEY_LEFT:
            this->move_piece(LEFT);
            break;
        case KEY_RIGHT:
        case RIGHT:
            this->move_piece(RIGHT);
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

    void rotate_piece(int direction)
    {
        this->current_piece.rotate(direction);
        while (this->grid.is_piece_overlapping(this->current_piece))
        {
            this->current_piece.rotate(direction);
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

    void push_down()
    {
        if (this->grid.is_colliding(this->current_piece, DOWN))
        {
            this->grid.fix_piece(this->current_piece);
            int cleared_lines = this->grid.empty_lines(this->current_piece);
            this->current_piece = Piece::random();
            this->check_game_over();
            this->update_score(cleared_lines);
        }
        else
        {
            this->current_piece.move(DOWN);
        }
    }

    void update_score(int cleared_lines)
    {
        this->score += cleared_lines * 100;
        if (speed > 10)
        {
            this->speed--;
        }
    }

    void check_game_over()
    {
        this->game_over = this->grid.is_piece_overlapping(this->current_piece);
    }

    void end_game()
    {
        getchar();
        endwin();
    }

public:
    Game() : grid(12, 22), current_piece(Piece::random())
    {
        this->game_over = false;
        this->speed = 40;
        this->score = 0;
        this->init();
    }

    void play()
    {
        int speed_counter = 0;
        while (!this->game_over)
        {
            usleep(1000 * 25);
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