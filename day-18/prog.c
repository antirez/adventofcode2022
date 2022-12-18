#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include "aoc.h"

#define SPACE_SIZE 30
char Space[SPACE_SIZE][SPACE_SIZE][SPACE_SIZE] = {0};

/* All the possible direction of adjacent cubes. */
#define SPACE_DIRS 6
struct {
    int x,y,z;
} Dirs[6] = {
    /* Imagine to look at a cube in front of you. */
    {1,0,0},    /* Cube on the right. */
    {-1,0,0},   /* Cube on the left. */
    {0,-1,0},   /* Cub on the top. */
    {0,1,0},    /* Cube on the bottom. */
    {0,0,-1},   /* Cube immediately nearest to us. */
    {0,0,1},    /* Cube immediately after this cube. */
};

/* This will mark all the reachabel cubes of air in Space (not trapped
 * inside other cubes) with the value '2'. x,y,z is the starting position
 * of the flood. */
void flood(int x, int y, int z) {
    /* Non zero? Either already flood or solid. */
    if (Space[x][y][z]) return;

    Space[x][y][z] = 2;
    /* Recursively flood all the adjacent cubes. */
    for (int j = 0; j < SPACE_DIRS; j++) {
        int cx = x+Dirs[j].x;
        int cy = y+Dirs[j].y;
        int cz = z+Dirs[j].z;
        
        /* Skip out of bound coordinates. */
        if (cx < 0 || cy < 0 || cz < 0 ||
            cx >= SPACE_SIZE ||
            cy >= SPACE_SIZE ||
            cz >= SPACE_SIZE) continue;
        flood(cx,cy,cz);
    }
}

int main(int argc, char **argv) {
    FILE *fp = argc == 2 ? fopen(argv[1],"r") : stdin;
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    /* Input parsing. Boring but needed. */
    char buf[256];
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        size_t l = strlen(buf);
        if (l <= 1) continue;
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        int x,y,z;
	sscanf(buf,"%d,%d,%d",&x,&y,&z);
        Space[x][y][z] = 1;
    }
    if (argc == 2) fclose(fp);

    /* Part 1. */

    /* Scan the space looking for cubes. */
    int area = 0;
    for (int x = 0; x < SPACE_SIZE; x++) {
        for (int y = 0; y < SPACE_SIZE; y++) {
            for (int z = 0; z < SPACE_SIZE; z++) {
                if (Space[x][y][z]) {
                    /* For each face check the adjacent space. */
                    for (int j = 0; j < SPACE_DIRS; j++) {
                        int cx = x+Dirs[j].x;
                        int cy = y+Dirs[j].y;
                        int cz = z+Dirs[j].z;

                        if (cx < 0 || cy < 0 || cz < 0 ||
                            cx >= SPACE_SIZE ||
                            cy >= SPACE_SIZE ||
                            cz >= SPACE_SIZE ||
                            Space[cx][cy][cz] == 0)
                        {
                            /* The adjacent position is free only if
                             * it's outside our space coordinates or
                             * if there is no cube. */
                            area++;
                        }
                    }
                }
            }
        }
    }
    printf("Puzzle 1: %d\n", area);

    flood(0,0,0); /* Mark all the reachable cubes of air with '2'. */

    /* Count the area of all the faces touching unreachable by water. */
    int trapped_area = 0;
    for (int x = 0; x < SPACE_SIZE; x++) {
        for (int y = 0; y < SPACE_SIZE; y++) {
            for (int z = 0; z < SPACE_SIZE; z++) {
                if (Space[x][y][z]) {
                    /* For each face check the adjacent space. */
                    for (int j = 0; j < SPACE_DIRS; j++) {
                        int cx = x+Dirs[j].x;
                        int cy = y+Dirs[j].y;
                        int cz = z+Dirs[j].z;

                        if (cx < 0 || cy < 0 || cz < 0 ||
                            cx >= SPACE_SIZE ||
                            cy >= SPACE_SIZE ||
                            cz >= SPACE_SIZE) continue;

                        if (Space[cx][cy][cz] == 0) {
                            /* Free space marked by '2' is reachable by
                             * wanter, Space with value of '1' is solid. */
                            trapped_area++;
                        }
                    }
                }
            }
        }
    }
    printf("Puzzle 2: %d\n", area - trapped_area);
}
