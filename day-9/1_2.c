#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ELFDIR_NAME_MAX 64
#define ELFDIR_SUB_MAX 16

/* Our rope world. */
typedef struct world {
    int width;
    int height;
    int knots;  /* Number of simulated knots, head included. */
    int *kx;    /* Knots x position. kx[0] is the head of the rope. */
    int *ky;    /* Knots y position. ky[0] is the head of the rope. */
    char *visited;  /* Positions visited by the tail. */
} world;

/* Return the current map size, widht*height. */
int worldSize(world *w) {
    return w->width * w->height;
}

/* Mark the current tail (last knot) location as visited. */
void worldVisited(world *w) {
    int x = w->kx[w->knots-1];
    int y = w->ky[w->knots-1];
    if (x < 0 || x >= w->width ||
        y < 0 || y >= w->height)
    {
        printf("World too small for this simulation!\n");
        exit(1);
    }
    w->visited[x+y*w->width] = 1;
}

/* Return the number of places visited. */
int worldCountVisited(world *w) {
    int size = worldSize(w);
    int visited = 0;
    for (int j = 0; j < size; j++) visited += w->visited[j];
    return visited;
}

/* Initialize a world structure. */
world *createWorld(int width, int height, int knots) {
    world *w = malloc(sizeof(*w));
    if (!w) return NULL;
    w->width = width;
    w->height = height;
    w->knots = knots;
    w->visited = NULL;
    w->kx = w->ky = NULL;
    w->visited = malloc(worldSize(w));
    w->kx = malloc(w->knots*sizeof(int));
    w->ky = malloc(w->knots*sizeof(int));
    if (!w->visited || !w->kx || !w->ky) {
        free(w);
        free(w->visited);
        free(w->kx);
        free(w->ky);
        return NULL;
    }

    /* Let's put the rope at the center. Head and all its knots start
     * at the same positon. */
    for (int j = 0; j < knots; j++) {
        w->kx[j] = w->width/2;
        w->ky[j] = w->height/2;
    }
    memset(w->visited,0,worldSize(w));
    worldVisited(w); /* Starting position is visited. */
    return w;
}

/* Relase map memory. */
void freeWorld(world *w) {
    free(w->visited);
    free(w->kx);
    free(w->ky);
    free(w);
}

/* Print the current world. */
void printWorld(world *w) {
    for (int y = 0; y < w->height; y++) {
        for (int x = 0; x < w->width; x++) {
            int c = '.';
            /* Scan each rope knot. Terribly inefficient but enough
             * for printing. Loop is reversed since we want 'H' to
             * cover the lower ranking nots and so forth. */
            for (int j = w->knots-1; j >= 0; j--) {
                if (w->kx[j] == x && w->ky[j] == y) {
                    c = j == 0 ? 'H' : '0'+j;
                }
            }
            printf("%c",c);
        }
        printf("\n");
    }
}

/* Read the rope moves from file and simulate it. Return the world
 * state after the simulation took place. Return NULL on out of memory.
 * The 'size' parameters is used to select the world width and height: you
 * need enough space depending on the movements performed. */
world *simulateRope(FILE *fp, int size, int knots) {
    char buf[1024];
    world *w = createWorld(size,size,knots);
    if (w == NULL) return NULL;
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        /* Skip empty lines. */
        size_t l = strlen(buf);
        if (l <= 1) continue;

        /* Strip newline. */
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        /* Parse movement. */
        buf[1] = 0;
        int count = atoi(buf+2);
        int vx, vy;
        switch(buf[0]) {
            case 'R': vx = 1; vy = 0; break;
            case 'L': vx = -1; vy = 0; break;
            case 'U': vx = 0; vy = -1; break;
            case 'D': vx = 0; vy = 1; break;
            default: assert(1 != 1); break; /* Shold never happen. */
        }

        /* Simulate moving N times in the specified direction. */
        while(count--) {
            /* Move rope head. */
            w->kx[0] += vx;
            w->ky[0] += vy;

            /* Recusively propagate the movement to the other
             * knots. */
            for (int j = 0; j < knots-1; j++) {
                /* We call them Head X,Y and Tail X,Y (hx,hy,tx,ty).
                 * But actually hx,hy represents the head and tx,ty the
                 * first knot after the head in the first cycles, and then
                 * the next pair and so forth. */
                int *hx = w->kx+j;
                int *hy = w->ky+j;
                int *tx = w->kx+j+1;
                int *ty = w->ky+j+1;

                /* Calculate empty distance between head and tail. */
                int dx = *hx - *tx;
                int dy = *hy - *ty;

                /* Apply distance to tail to make it follow. Note that as
                 * long as delta is 1 or 0 we don't care: they are still
                 * touching or overlapping.
                 *
                 * Note that when we perform the move, we aligh the tail
                 * to the head in the other axis, as this is the rules
                 * described in the Day 9 puzzle. */
                if ((dx == 2 || dx == -2) && (dy == 2 || dy == -2)) {
                    /* This movement was not cover in the first case
                     * of two knots, puzzle 1. So apparently there is
                     * also the rule that one knot can move aways two
                     * positions both in x and y, as a result of the move
                     * it did to follow other knots. In this case we just
                     * move diagonally. Here the specification of the way
                     * the rope moved was less than perfect. */
                    dx /= 2;
                    dy /= 2;
                    (*tx) += dx;
                    (*ty) += dy;
                } else if (dx == 2) {
                    (*tx)++;
                    *ty = *hy;
                } else if (dx == -2) {
                    (*tx)--;
                    *ty = *hy;
                } else if (dy == 2) {
                    (*ty)++;
                    *tx = *hx;
                } else if (dy == -2) {
                    (*ty)--;
                    *tx = *hx;
                } else {
                    /* Optimization: if the current knot didn't move, then
                     * all the following will not move as well. */
                    break;
                }
            }
            worldVisited(w);

            # if 0
            /* Uncomment here if you want to actaully see what happens
             * step by step. However this does not make sense with the
             * default world size of 1000... */
            printWorld(w);
            printf("\n");
            #endif
        }
    }
    return w;
}

int main(int argc, char **argv) {
    const char *filename = (argc == 2) ? argv[1] : "input.txt";
    FILE *fp = fopen(filename,"r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    world *w = simulateRope(fp,1000,10);
    printf("The tail visited %d places\n", worldCountVisited(w));
    if (w == NULL) {
        printf("Out of memory reading map\n");
        exit(1);
    }
    fclose(fp);
    freeWorld(w);
    return 0;
}
