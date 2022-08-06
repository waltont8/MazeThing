// g++ maze.cpp -omaze -Wall -Werror -fsanitize=address -g3 --std=c++20
#include <functional> 
#include <iostream>
#include <vector>
#include <stdio.h>
#include <time.h>

using namespace std;

const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 20;
const int CELL_WIDTH = 20;
const int CELL_HEIGHT = 20;

typedef enum { Vampire, Ghost, Bats, Empty } Entity;
typedef enum { Up, Down, Left, Right, None } Dir;

class Cell {
    public:
    Cell() {
        this->visited = false;
        this->up = false;
        this->down = false;
        this->left = false;
        this->right = false;
        this->e = Empty;
    }

    int *bitmap() {
        int *ret = (int *) calloc(CELL_WIDTH*CELL_HEIGHT, sizeof(int));
        if (this->up == false) {
            for (int n=0;n<CELL_WIDTH;n++) ret[n+(0*CELL_WIDTH)] = 255;
        }
        if (this->down == false) {
            for (int n=0;n<CELL_WIDTH;n++) ret[n+((CELL_HEIGHT-1)*CELL_WIDTH)] = 255;
        }
        if (this->left == false) {
            for (int n=0;n<CELL_HEIGHT;n++) ret[0+(n*CELL_WIDTH)] = 255;
        }
        if (this->right == false) {
            for (int n=0;n<CELL_HEIGHT;n++) ret[(CELL_WIDTH-1)+(n*CELL_WIDTH)] = 255;
        }

        return ret;
    }

    Entity e;
    bool visited;
    bool up, down, left, right;
};

int mvx(int x, Dir d) {
    switch(d) {
        case Left:;return x-1;
        case Right:return x+1;
        default:return x;
    }
    return x;
}

int mvy(int y, Dir d) {
    switch(d) {
        case Up:return y-1;
        case Down:return y+1;
        default:return y;
    }
    return y;
}

class Maze {
    public:
    Maze(int w, int h) {
        this->cells = vector<vector<Cell>> (h, vector<Cell>(w)); 
        this->width = w;
        this->height = h;
    }

    bool isFree(Dir d, int x, int y) {
        int tx=mvx(x,d);
        int ty=mvy(y,d);

        if (tx<0 || ty <0 || tx >= this->width || ty >=height) return false;
        return !(this->cells[ty][tx]).visited;
    }

    void makeExit(int x, int y, Dir d) {
        switch(d) {
            case Up: (this->cells[y][x]).up = true; (this->cells[y-1][x]).down = true; break;
            case Down: (this->cells[y][x]).down = true; (this->cells[y+1][x]).up = true; break;
            case Left: (this->cells[y][x]).left = true; (this->cells[y][x-1]).right = true; break;
            case Right: (this->cells[y][x]).right = true; (this->cells[y][x+1]).left = true; break;
            default:break; // ASSERT
        }
    }

    int *bitmap() {
        int *pixels = (int *) malloc(sizeof(int)*this->width*this->height*CELL_HEIGHT*CELL_WIDTH);

        for(int x=0; x<this->width ; x++) {
            for (int y=0; y<this->height; y++) {
                int *freeMe = (this->cells[y][x]).bitmap();
                for(int ix=0; ix<CELL_WIDTH; ix++) {
                    for (int iy=0; iy<CELL_HEIGHT; iy++) {
                        pixels[(x*this->width +ix) + (y*this->height+iy) * CELL_WIDTH*this->width ] = freeMe[ix+iy*CELL_WIDTH];
                    }
                }
                free(freeMe);
            }
        }
        return pixels;
    }

    void apply (const std::function <Entity (Entity)>& f)
    {
        for(int x=0; x<this->width ; x++) {
            for (int y=0; y<this->height; y++) {
                (this->cells[y][x]).e = f((this->cells[y][x]).e);
            }
        }
    }

    int width;
    int height;
    vector<vector<Cell>> cells;
};

void generate(Maze &m, int x, int y) {
    int randomDir = rand()%4;
    (m.cells[y][x]).visited = true;
    printf("Visit (%d,%d)\n", x, y);
    
    Dir d = None;
    do {
        d = None;
        int count = 4;
        switch (randomDir) {
            do {
            case 0:count--;
                if (m.isFree(Up,x,y)) {d = Up; break;}
            case 1:count--;
                if (m.isFree(Down,x,y)) {d = Down; break;}
            case 2:count--;
                if (m.isFree(Left,x,y)) {d = Left; break;}
            case 3:count--;
                if (m.isFree(Right,x,y)) {d = Right; break;}
            } while (count>0);
        }
        if (d!=None) {
            m.makeExit(x,y,d);
            generate(m,mvx(x,d),mvy(y,d));
        }
    } while (d != None);
}

void ppmg8(const char *fileName, int *bitmap, int width, int height) {
        FILE *fptr = fopen(fileName, "w");
        fprintf(fptr, "P3 %d %d 255\n", width, height);
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                fprintf(fptr, "%d %d %d  ",bitmap[x+y*width],bitmap[x+y*width],bitmap[x+y*width]);
            }
            fprintf(fptr, "\n");
        }
        fclose(fptr);
}

int main() {
    srand(time(NULL));
    Maze m = Maze(MAP_WIDTH,MAP_HEIGHT);
    generate(m,0,0);
    // Mark all dead ends as vampires
    m.apply([](Entity e){return e;});
    // Generate an image of the maze
    int *bitmap = m.bitmap();
    ppmg8("output.ppm", bitmap, MAP_WIDTH*CELL_WIDTH, MAP_HEIGHT*CELL_HEIGHT);
}
