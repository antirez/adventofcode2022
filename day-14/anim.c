#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>
#include <unistd.h> /* usleep(). */

#if 1 /* small.txt */
#define XOFFSET 480         /* World appears to start at x >= 490. */
#define WORLDW 50
#define WORLDH 15
#else   /* input.txt needs bigger world. */
#define XOFFSET 280         /* Make more space for sand to rest. */
#define WORLDW 500
#define WORLDH 300
#endif

/* Our map. */
typedef struct map {
    int width;
    int height;
    char *buf;  /* Map data: width*height bytes of data. */
    char *aux;  /* Aux data, used for visualization in printMap() and more. */
    int sx, sy; /* Start position. */
} map;

/* Macros to set/get aux data. */
#define setAux(m,x,y,v) (m)->aux[(y)*m->width+(x)] = (v)
#define getAux(m,x,y) (m)->aux[(y)*m->width+(x)]

/* Allocate and initialize a map structure of the specified
 * width 'w' and height 'h'. Return NULL on OOM. */
map *createMap(int w, int h) {
    map *m = malloc(sizeof(*m));
    if (!m) return NULL;
    m->width = w;
    m->height = h;
    m->buf = malloc(w*h);
    m->aux = malloc(w*h);
    if (!m->buf || !m->aux) {
        free(m->buf);
        free(m->aux);
        return NULL;
    }
    m->sx = m->sy = 0;
    return m;
}

/* Relase map memory. */
void freeMap(map *m) {
    free(m->buf);
    free(m->aux);
    free(m);
}

/* Return the current map size, widht*height. */
int mapSize(map *m) {
    return m->width * m->height;
}

/* Print the current map and aux value. cx,cy is
 * the current position we want to highlight. */
void printMap(map *m, int cx, int cy) {
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            int h = m->buf[x+y*m->width]; /* Map elevation / value. */
            int d = m->aux[x+y*m->width]; /* Aux data. */
            if (h == 0) h = '.';
            if (x == cx && y == cy) {
                printf("\033[32;1m%c\033[0m", h);
            } else {
                printf("%c", d ? d : h);
            }
        }
        printf("\n");
    }
}

/* Return the byte at x,y. Performs bound checking returning -1
 * for out of range areas. */
char mapGet(map *m, int x, int y) {
    int idx = m->width*y+x;
    if (x < 0 || x >= m->width ||
        y < 0 || y >= m->height) return -1;
    return m->buf[idx];
}

/* Set the specific point on the map. Silently discards out of range
 * coordinates. */
void mapSet(map *m, int x, int y, int c) {
    int idx = m->width*y+x;
    if (x < 0 || x >= m->width ||
        y < 0 || y >= m->height) return;
    m->buf[idx] = c;
}

/* Draws a line from x1,y1 to x2,y2. The function is pretty trivial and
 * will not work if the line is not exactly horizontal, vertical or diagonal. */
void mapLine(map *m, int x1, int y1, int x2, int y2, int c) {
    int dx = x2-x1;
    int dy = y2-y1;

    /* Normalize retaining sign. */
    if (dx) dx /= abs(dx);
    if (dy) dy /= abs(dy);

    int x = x1, y = y1;
    while(1) {
        mapSet(m,x,y,c);
        if (x == x2 && y == y2) break;
        x += dx;
        y += dy;
    }
}

/* Find 'c' in the map. Return 1 if it was found, 0 otherwise.
 * The position is returned in *xp and *yp. */
int mapFind(map *m, int c, int *xp, int *yp) {
    /* Scan the map in a cache obvious way. However this function is not
     * speed critical for the search of the solution. */
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            if (mapGet(m,x,y) == c) {
                *xp = x;
                *yp = y;
                return 1;
            }
        }
    }
    return 0;
}

/* Read the map from the input file and return a map object, or NULL
 * on out of memory. */
map *readMapFromFile(FILE *fp, int *maxyp) {
    map *m = createMap(WORLDW,WORLDH);
    if (!m) return NULL;

    char buf[1024];
    int maxy = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        size_t l = strlen(buf);
        if (l <= 1) continue;
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        /* Lines are in the form x1,y1 -> x2,y2 -> x3,y2 -> ... */
        int x1, y1, x2, y2;
        int count = 0;
        char *p = buf;
        while(1) {
            while(isspace(*p)) p++;
            char *sep = strchr(p,'>'); /* Seek -> */
            if (sep) sep[-1] = 0;

            /* Now p points to the x coordinate, seek y. */
            char *yp = strchr(p,',');
            assert(yp != NULL);
            *yp = 0; yp++;  /* yp points to y coordinate. */
            x2 = atoi(p);
            y2 = atoi(yp);

            x2 -= XOFFSET; /* World seems to start at around 490. */
            assert(x2 > 0);

            if (count) { /* Nothing to draw if it's the first pair of
                            numbers seen. */
                if (x1 >= m->width || x2 >= m->width ||
                    y1 >= m->height || y2 >= m->height)
                {
                    printf("Map is too small for this world.\n");
                    exit(1);
                }
                mapLine(m,x1,y1,x2,y2,'#');
                if (y1 > maxy) maxy = y1;
                if (y2 > maxy) maxy = y2;
            }

            /* More data? */
            if (!sep) break;

            p = sep+1; /* Skip "->" */
            x1 = x2;
            y1 = y2;
            count++;
        }
    }
    *maxyp = maxy;
    return m;
}

/* Simulate sand falling from 500,0. Return the resting position
 * in *rx, *ry. If the sand fall out of the world limits they
 * will be set to -1,-1.
 *
 * If animate is non zero, the map is printed and the screen cleared at
 * every step. */
void simulSand(map *m, int *rx, int *ry, int animate) {
    int x = 500-XOFFSET;
    int y = 0;

    struct {
        int dx, dy;
    } nextpos[3] = {
        {0,1},       /* Down. */
        {-1,1},      /* Down left. */
        {1,1}        /* Down right. */
    };

    while(1) {
        int j;
        for (j = 0; j < 3; j++) {
            int nx = x + nextpos[j].dx;
            int ny = y + nextpos[j].dy;
            int cur = mapGet(m,nx,ny);
            if (cur == -1) {
                /* Out of range. */
                *rx = -1;
                *ry = -1;
                return;
            }
            if (cur != 0) continue; /* Try next direction. */
            x = nx;
            y = ny;
            break;
        }

        if (animate) {
            mapSet(m,x,y,'+');
            printf("\x1b[H\x1b[2J");
            printMap(m,x,y);
            mapSet(m,x,y,0);
            fflush(stdout);
            usleep(30000);
        }

        if (j == 3) {
            /* This sand can't move more. */
            *rx = x;
            *ry = y;
            mapSet(m,x,y,'o');
            return;
        }
    }
}

int main(int argc, char **argv) {
    FILE *fp;
    if (argc != 2) {
        fp = stdin;
    } else {
        fp = fopen(argv[1],"r");
        if (!fp) {
            perror("Opening file");
            exit(1);
        }
    }

    /* Read map in memory. */
    int maxy; /* Used for part 2. */
    map *m = readMapFromFile(fp,&maxy);
    if (argc == 2) fclose(fp);
    if (m == NULL) {
        printf("Out of memory reading map\n");
        exit(1);
    }

    /* Draw the floor. */
    mapLine(m,0,maxy+2,m->width-1,maxy+2,'_');
    int count = 0;

    /* Show the animation. */
    while(1) {
        int restx, resty; /* Where the sand rested. */
        simulSand(m,&restx,&resty,1);
        if (restx == 500-XOFFSET && resty == 0) break;
        count++;
    }

    /* Cleanup. */
    freeMap(m);
    return 0;
}
