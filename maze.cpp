// g++ maze.cpp -omaze -Wall -Werror -fsanitize=address -g3 --std=c++20
#include <functional> 
#include <algorithm>
#include <iostream>
#include <vector>
#include <queue>

#include <stdio.h>
#include <time.h>

using namespace std;

const int MAP_WIDTH = 20;
const int MAP_HEIGHT = 20;
const int CELL_WIDTH = 20;
const int CELL_HEIGHT = 20;
const int Infinity = 10000;

typedef enum { Treasure, Vampire, Ghost, Bats, Path, Empty, Wall } Entity;
typedef enum { Up=0, Down, Left, Right, None } Dir;

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

    Entity *bitmap() {
        Entity *ret = (Entity *) malloc(CELL_WIDTH*CELL_HEIGHT * sizeof(Entity));

        for (int n=0;n<CELL_WIDTH*CELL_HEIGHT;n++) ret[n] = this->e;
        

        if (this->up == false) {
            for (int n=0;n<CELL_WIDTH;n++) ret[n+(0*CELL_WIDTH)] = Wall;
        }
        if (this->down == false) {
            for (int n=0;n<CELL_WIDTH;n++) ret[n+((CELL_HEIGHT-1)*CELL_WIDTH)] = Wall;
        }
        if (this->left == false) {
            for (int n=0;n<CELL_HEIGHT;n++) ret[0+(n*CELL_WIDTH)] = Wall;
        }
        if (this->right == false) {
            for (int n=0;n<CELL_HEIGHT;n++) ret[(CELL_WIDTH-1)+(n*CELL_WIDTH)] = Wall;
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

    void generate(int x, int y) {
        int randomDir = rand()%4;
        (this->cells[y][x]).visited = true;
        
        Dir d;
        do {
            d = None;
            int count = 4;
            switch (randomDir) {
                do {
                case 0:count--;
                    if (this->isFree(Up,x,y)) {d = Up; break;}
                case 1:count--;
                    if (this->isFree(Down,x,y)) {d = Down; break;}
                case 2:count--;
                    if (this->isFree(Left,x,y)) {d = Left; break;}
                case 3:count--;
                    if (this->isFree(Right,x,y)) {d = Right; break;}
                } while (count>0);
            }
            if (d!=None) {
                this->makeExit(x,y,d);
                generate(mvx(x,d),mvy(y,d));
            }
        } while (d != None);
    }

    Entity *bitmap() {
        Entity *pixels = (Entity *) malloc(sizeof(Entity)*this->width*this->height*CELL_HEIGHT*CELL_WIDTH);

        for(int x=0; x<this->width ; x++) {
            for (int y=0; y<this->height; y++) {
                Entity *freeMe = (this->cells[y][x]).bitmap();
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

    // Lambda all maze cells
    void applyAll (const std::function <void (Cell&)>& f)
    {
        for(int x=0; x<this->width ; x++) {
            for (int y=0; y<this->height; y++) {
                f((this->cells[y][x]));
            }
        }
    }

    // Lambda one maze cell
    void applyOne(int x, int y, const std::function <void (Cell&)>& f)
    {
        f((this->cells[y][x]));
    }

    // Breadth first search and mark up a path from (sx,sy) to (fx,fy)
    void markPath(int sx,int sy,int fx,int fy) {
        queue<pair<int,int>> toVisit;
        vector<vector<int>> distances = vector<vector<int>> (this->height, vector<int>(this->width, Infinity)); 

        toVisit.push({sx,sy});
        distances[sy][sx] = 1;

        while (!toVisit.empty()) {
            auto [x,y] = toVisit.front(); toVisit.pop();
            if (x==fx && y==fy) continue; // We found the exit

            // Can we go up?
            if (y>0 && this->cells[y][x].up == true) {
                if (distances[y-1][x] == 0 || distances[y-1][x] > distances[y][x]+1) { // Go up
                    distances[y-1][x] = distances[y][x]+1;
                    toVisit.push({x,y-1});
                }
            }

            // Can we go down?
            if (y<(this->height-1) && this->cells[y][x].down == true) {
                if (distances[y+1][x] == 0 || distances[y+1][x] > distances[y][x]+1) { // Go up
                    distances[y+1][x] = distances[y][x]+1;
                    toVisit.push({x,y+1});
                }
            }

            // Can we go left?
            if (x>0 && this->cells[y][x].left == true) {
                if (distances[y][x-1] == 0 || distances[y][x-1] > distances[y][x]+1) { // Go up
                    distances[y][x-1] = distances[y][x]+1;
                    toVisit.push({x-1,y});
                }
            }
            // Can we go right?
            if (x<(this->width-1) && this->cells[y][x].right == true) {
                if (distances[y][x+1] == 0 || distances[y][x+1] > distances[y][x]+1) { // Go up
                    distances[y][x+1] = distances[y][x]+1;
                    toVisit.push({x+1,y});
                }
            }
        }

        // Unwind the stack painting the path
        int x=fx;
        int y=fy;
        while (!(x==sx && y==sy)) {
            this->cells[y][x].e = Path;
            // This code steps in the direction of the lowest value
            // Don't really like how it turned out.
            vector<int> v{
                (y>0&&this->cells[y][x].up==true)?distances[y-1][x]:Infinity,
                (y<this->height-1&&this->cells[y][x].down==true)?distances[y+1][x]:Infinity,
                (x>0&&this->cells[y][x].left==true)?distances[y][x-1]:Infinity,
                (x<this->width-1&&this->cells[y][x].right==true)?distances[y][x+1]:Infinity
            };

            int m = *min_element(v.begin(), v.end());
            if (m==v[Up]) y--;
            else if (m==v[Down]) y++;
            else if (m==v[Left]) x--;
            else if (m==v[Right]) x++;
        }
        this->cells[y][x].e = Path;      
    }

    int width;
    int height;
    vector<vector<Cell>> cells;
};

void mobSwaps(Maze &m) {
    // Check every cell
    for (int x=0;x<m.width;x++) {
        for (int y=0;y<m.height;y++) {
            // if the cell is a ghost or vampire
            if (m.cells[y][x].e == Vampire || m.cells[y][x].e == Ghost) {
                // Walk to the path

                // Try right
                for (int mx=x;mx<m.width;mx++) {
                    if (m.cells[y][mx].e == Path) {
                        m.cells[y][x].e = Treasure;
                        break;
                    }
                    if (m.cells[y][mx].right == false) break;
                }
                
                // Try left
                for (int mx=x;mx>=0;mx--) {
                    if (m.cells[y][mx].e == Path) {
                        m.cells[y][x].e = Treasure;
                        break;
                    }
                    if (m.cells[y][mx].left == false) break;
                }
                 
                // Try up
                for (int my=y;my>=0;my--) {
                    if (m.cells[my][x].e == Path) {
                        m.cells[y][x].e = Treasure;
                        break;
                    }
                    if (m.cells[my][x].up == false) break;
                }

                // Try down
                for (int my=y;my<m.height;my++) {
                    if (m.cells[my][x].e == Path) {
                        m.cells[y][x].e = Treasure;
                        break;
                    }
                    if (m.cells[my][x].down == false) break;
                }
            }
        }
    }
}

void ppmg8(const char *fileName, Entity *bitmap, int width, int height) {
        FILE *fptr = fopen(fileName, "w");
        fprintf(fptr, "P3 %d %d 255\n", width, height);
        for (int y=0; y<height; y++) {
            for (int x=0; x<width; x++) {
                switch(bitmap[x+y*width]) {
                    case Treasure:
                        fprintf(fptr, "255 253 0  ");
                    break;
                    case Vampire:
                        fprintf(fptr, "221 160 221  ");
                    break;
                    case Ghost:
                        fprintf(fptr, "136 206 235  ");
                    break;
                    case Path:
                        fprintf(fptr, "22 111 19  ");
                    break;
                    case Empty:
                        fprintf(fptr, "0 0 0  ");
                    break;
                    case Bats:
                        fprintf(fptr, "128 128 128  ");
                    break;
                    case Wall:
                        fprintf(fptr, "255 255 255  ");
                    break;
                    default:
                        printf("Error here %d\n", bitmap[x+y*width]);
                }
            }
            fprintf(fptr, "\n");
        }
        fclose(fptr);
}

struct vertex {
  float x,y,z;
};

struct face {
  int f1,f2,f3;
};

void dumpToObj(const char *fileName, Maze &m) {
    vector<vertex> v;
    vector<face> f;

    for (int x=0;x<m.width;x++) {
        for (int y=0;y<m.height;y++) {
            // 8 corners of the cube
                // Base
                v.push_back({0.1f+x,0.1f+y,0.1});
                v.push_back({0.1f+x,0.9f+y,0.1});
                v.push_back({0.9f+x,0.9f+y,0.1});
                v.push_back({0.9f+x,0.1f+y,0.1});
                // Top
                v.push_back({0.1f+x,0.1f+y,0.9});
                v.push_back({0.1f+x,0.9f+y,0.9});
                v.push_back({0.9f+x,0.9f+y,0.9});
                v.push_back({0.9f+x,0.1f+y,0.9});
            // 4 Exit corridors
                // +x
                    // Base
                    v.push_back({0.9f+x,0.1f+y,0.1});
                    v.push_back({0.9f+x,0.9f+y,0.1});
                    v.push_back({1.0f+x,0.9f+y,0.1});
                    v.push_back({1.0f+x,0.1f+y,0.1});
                    // Top
                    v.push_back({0.9f+x,0.1f+y,0.9});
                    v.push_back({0.9f+x,0.9f+y,0.9});
                    v.push_back({1.0f+x,0.9f+y,0.9});
                    v.push_back({1.0f+x,0.1f+y,0.9});
                // -x
                    // Base
                    v.push_back({0.0f+x,0.1f+y,0.1});
                    v.push_back({0.0f+x,0.9f+y,0.1});
                    v.push_back({0.1f+x,0.9f+y,0.1});
                    v.push_back({0.1f+x,0.1f+y,0.1});
                    // Top
                    v.push_back({0.0f+x,0.1f+y,0.9});
                    v.push_back({0.0f+x,0.9f+y,0.9});
                    v.push_back({0.1f+x,0.9f+y,0.9});
                    v.push_back({0.1f+x,0.1f+y,0.9});
                // +y
                    // Base
                    v.push_back({0.1f+x,0.9f+y,0.1});
                    v.push_back({0.1f+x,1.0f+y,0.1});
                    v.push_back({0.9f+x,1.0f+y,0.1});
                    v.push_back({0.9f+x,0.9f+y,0.1});
                    // Top
                    v.push_back({0.1f+x,0.9f+y,0.9});
                    v.push_back({0.1f+x,1.0f+y,0.9});
                    v.push_back({0.9f+x,1.0f+y,0.9});
                    v.push_back({0.9f+x,0.9f+y,0.9});
                // -y
                    // Base
                    v.push_back({0.1f+x,0.1f+y,0.1});
                    v.push_back({0.9f+x,0.1f+y,0.1});
                    v.push_back({0.9f+x,0.0f+y,0.1});
                    v.push_back({0.1f+x,0.0f+y,0.1});
                    // Top
                    v.push_back({0.9f+x,0.1f+y,0.9});
                    v.push_back({0.9f+x,0.0f+y,0.9});
                    v.push_back({0.1f+x,0.0f+y,0.9});
                    v.push_back({0.1f+x,0.1f+y,0.9});

            // 6 faces
                int n = v.size() - 8*(4+1);
                // 1 - base
                f.push_back({1+n,2+n,3+n});
                f.push_back({3+n,4+n,1+n});
                // 2 - top
                f.push_back({5+n,6+n,7+n});
                f.push_back({7+n,8+n,5+n});
                // 3 - left
                if (!(m.cells[y][x]).left) {
                f.push_back({1+n,5+n,2+n});
                f.push_back({2+n,5+n,6+n});
                }
                // 4 - back
                if (!(m.cells[y][x]).down) {
                f.push_back({2+n,6+n,3+n});
                f.push_back({3+n,6+n,7+n});
                }
                // 5 - right
                if (!(m.cells[y][x]).right) {
                f.push_back({4+n,3+n,7+n});
                f.push_back({7+n,8+n,4+n});
                }
                // 6 - front
                if (!(m.cells[y][x]).up) {
                f.push_back({1+n,5+n,4+n});
                f.push_back({4+n,5+n,8+n});
                }
            // Exit +x
                n = v.size() - 8*(4);
                if ((m.cells[y][x]).right) {
                // 1 - base
                f.push_back({1+n,2+n,3+n});
                f.push_back({3+n,4+n,1+n});
                // 2 - top
                f.push_back({5+n,6+n,7+n});
                f.push_back({7+n,8+n,5+n});
                // 3 - left
                f.push_back({2+n,6+n,3+n});
                f.push_back({3+n,6+n,7+n});
                // 4 - right
                f.push_back({1+n,5+n,8+n});
                f.push_back({8+n,4+n,1+n});
                }
            // Exit -x
                n = v.size() - 8*(3);
                if ((m.cells[y][x]).left) {
                // 1 - base
                f.push_back({1+n,2+n,3+n});
                f.push_back({3+n,4+n,1+n});
                // 2 - top
                f.push_back({5+n,6+n,7+n});
                f.push_back({7+n,8+n,5+n});
                // 3 - left
                f.push_back({2+n,6+n,3+n});
                f.push_back({3+n,6+n,7+n});
                // 4 - right
                f.push_back({1+n,5+n,8+n});
                f.push_back({8+n,4+n,1+n});
                }
            // Exit +y
                n = v.size() - 8*(2);
                if ((m.cells[y][x]).down) {
                // 1 - base
                f.push_back({1+n,2+n,3+n});
                f.push_back({3+n,4+n,1+n});
                // 2 - top
                f.push_back({5+n,6+n,7+n});
                f.push_back({7+n,8+n,5+n});
                // 3 - left
                f.push_back({1+n,7+n,2+n});
                f.push_back({7+n,6+n,2+n});
                // 4 - right
                f.push_back({4+n,7+n,6+n});
                f.push_back({4+n,3+n,7+n});
                }
            // Exit -y
                n = v.size() - 8*(1);
                if ((m.cells[y][x]).up) {
                // 1 - base
                f.push_back({1+n,2+n,3+n});
                f.push_back({3+n,4+n,1+n});
                // 2 - top
                f.push_back({5+n,6+n,7+n});
                f.push_back({7+n,8+n,5+n});
                // 3 - left
                f.push_back({1+n,7+n,2+n});
                f.push_back({7+n,6+n,2+n});
                // 4 - right
                f.push_back({4+n,7+n,6+n});
                f.push_back({4+n,3+n,7+n});
                }

        }
    }
    FILE *fptr = fopen(fileName, "w");
    for (auto vn : v) {
        fprintf(fptr, "v %f %f %f\n", vn.x,vn.y,vn.z);
    }
    for (auto fn : f) {
        fprintf(fptr, "f %d %d %d\n", fn.f1,fn.f2,fn.f3);
    }
    fclose(fptr);
}

int main() {
    // Random seed - comment out for debug
    srand(time(NULL));

    // New maze
    Maze m = Maze(MAP_WIDTH,MAP_HEIGHT);
    m.generate(0,0);

    // Mark all dead ends as vampires or Ghosts
    m.applyAll([](Cell &c){if ((int)c.up + (int)c.down + (int)c.left + (int)c.right == 1) c.e=(rand()%2)?Vampire:Ghost;});

    // Highlight a path through the maze
    m.markPath(0,0,MAP_WIDTH-1, MAP_HEIGHT-1);

    // Mobs that can see the path become treasure
    mobSwaps(m);

    // Generate an image of the maze
    Entity *bitmap = m.bitmap();
    ppmg8("output.ppm", bitmap, MAP_WIDTH*CELL_WIDTH, MAP_HEIGHT*CELL_HEIGHT);

    dumpToObj("test.obj", m);
}
