#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define ELFDIR_NAME_MAX 64
#define ELFDIR_SUB_MAX 16

/* Our map. */
typedef struct map {
    int width;
    int height;
    char *buf;  /* Map data: width*height bytes of data. */
    char *aux;  /* Aux data used for running algorithms on the three. */
} map;

/* Initialize a map structure. */
void initMap(map *m) {
    m->width = 0;
    m->height = 0;
    m->buf = NULL;
    m->aux = NULL;
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

/* Return the byte at x,y. Performs bound checking returning -1
 * for out of range areas. */
char mapGet(map *m, int x, int y) {
    int idx = m->width*y+x;
    if (x < 0 || x >= m->width ||
        y < 0 || y >= m->height) return -1;
    return m->buf[idx];
}

/* Read the map from the input file and return a map object, or NULL
 * on out of memory. */
map *readMapFromFile(FILE *fp) {
    map *m = malloc(sizeof(*m));
    if (!m) return NULL;
    initMap(m);

    char buf[1024];
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        size_t l = strlen(buf);
        if (l <= 1) continue;
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        /* The first line we see we can set the width of the
         * map. */
        if (m->width == 0) {
            m->width = l;
        } else {
            /* We want the other lines be equally sized. */
            assert(l == (size_t)m->width);
        }

        /* Make space and concat this new line. */
        m->height++;
        char *newbuf = realloc(m->buf,mapSize(m));
        if (newbuf == NULL) {
            freeMap(m);
            return NULL; /* Out of memory. */
        }
        m->buf = newbuf;
        memcpy(m->buf+mapSize(m)-m->width,buf,l);
    }
    return m;
}

/* Print the current map and aux value. */
void mapPrint(map *m) {
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            printf("%c[%d]", m->buf[x+y*m->width],
                             m->aux ? m->aux[x+y*m->width] : 0);
        }
        printf("\n");
    }
}

/* countVisibleTrees() helper:
 *
 * This function scans the map 'm' for visible trees in a specific direction.
 * The scan starts at px, py, and visits consecutive map cells moving every
 * time by x += vx, y+= vy. When the scan of the current line ends, it starts
 * again from where it was before scanning this line, but moving to the next
 * line doing x+= nx, y+= ny.
 *
 * So for instance starting at px = 0, py = 0, with velocity vx = 1, vy = 0,
 * means that each line scanned we move towards right. Then when this line
 * was completely scanned, we restore the current x,y position we had before
 * the line scan, and update them using nx, ny. In this case we would use
 * nx = 0 and ny = 1 to start again our left-to-right scan in the next row.
 *
 * The algorith used is simple: we mark each point in the map as visible as
 * at each new line scanned the current point is the highest visited so far.
 *
 * As visible trees are found, they are marked in m->aux, that must be
 * already allocated. */
void markVisibleTrees(map *m, int px, int py, /* Position x,y */
                              int vx, int vy, /* Velocity x,y (direction) */
                              int nx, int ny) /* Change line movement. */
{
    int x = px, y = py; /* Our starting points. */
    int bx, by; /* We save the starting point at the start of each line. */
    int steps = mapSize(m);
    while(steps--) {
        bx = x, by = y; /* Save starting point. */
        int max = 0; /* Max height seen so far. */
        int c; /* Current value on the map. */
        while((c = mapGet(m,x,y)) != -1) {
            if (c > max) {
                m->aux[y*m->width+x] = 1;
                max = c;
            }
            /* Go to next point of current line. */
            x += vx;
            y += vy;
        }
        /* Go to next line. */
        x = bx+nx;
        y = by+ny;
    }
}

/* Count all the threes that are not protected from one of the borders
 * by a tree that is higher or the same length. On out of memory
 * zero is returned. */
int countVisibleTrees(map *m) {
    /* We don't want to use an O(N*N) algorithm that casts from every
     * point to all the four directions. There is a simple four pass O(N)
     * algorithm that will do:
     *
     * 1. From each direction, mark all the trees that have an height
     *    greater than any tree seen so far visiting trees in that
     *    direction.
     * 2  At the end of the four scans all the trees that are marked
     *    are all visible.
     *
     * To do so we use a markVisibleTrees() function. it takes a starting
     * point and a direction vector. This way we can generalize the marking
     * stage needed for all the four sides. */

    /* To run this algorithm, we use an auxiliary map data where we mark
     * the trees without destroying our view of the height map. */
    if (m->aux == NULL) {
        m->aux = malloc(mapSize(m));
        if (!m->aux) return 0;
    }
    memset(m->aux,0,mapSize(m));

    /* The following starting points, directions, and next line calls
     * correspond to:
     * 1. From upper-left corner, toward east, one step south each line.
     * 2. From upper-right corner, toward west, one step south each line.
     * 3. From upper-left corner, towards south, one step east each line.
     * 3. From bottom-left corner, towards north, one step east each line. */
    markVisibleTrees(m,0,0,1,0,0,1);
    markVisibleTrees(m,m->width-1,0,-1,0,0,1);
    markVisibleTrees(m,0,0,0,1,1,0);
    markVisibleTrees(m,0,m->height-1,0,-1,1,0);

    int visible = 0, size = mapSize(m);
    for (int j = 0; j < size; j++) visible += m->aux[j];
    return visible;
}

/* computeScenicScore() helper:
 *
 * This function starts from x, y and goes toward vx, vy, counting
 * the number of visible trees: we walk as soon as we reach the border
 * or a tree of height equal or higher than the current one. */
int viewDistance(map *m, int x, int y, /* Position x,y */
                         int vx, int vy) /* Velocity x,y (direction) */
{
    int distance = 0;
    int height = mapGet(m,x,y);
    while(1) {
        x += vx;
        y += vy;
        int this = mapGet(m,x,y);
        if (this == -1) break; /* Border reached. */
        distance++;
        if (this >= height) break; /* Tall tree reached. */
    }
    return distance;
}

/* Compute the scenic score for point x,y. */
int computeScenicScore(map *m, int x, int y) {
    return viewDistance(m,x,y,1,0) *
           viewDistance(m,x,y,-1,0) *
           viewDistance(m,x,y,0,1) *
           viewDistance(m,x,y,0,-1);
}

int main(int argc, char **argv) {
    /* Read map in memory. */
    const char *filename = (argc == 2) ? argv[1] : "input.txt";
    FILE *fp = fopen(filename,"r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }
    map *m = readMapFromFile(fp);
    if (m == NULL) {
        printf("Out of memory reading map\n");
        exit(1);
    }
    fclose(fp);

    printf("Map loaded. Size: %dx%d\n", m->width, m->height);

    /* Puzzle 1. */
    int visible = countVisibleTrees(m);
    printf("Visble trees: %d\n", visible);

    /* Puzzle 2.
     *
     * Here we resort to a O(N^2) algorithm even if it's likely
     * we can do better (for instance marking trees that can't be
     * better by already computed trees exploiting the fact scores of
     * nearby trees are not independent). */
    int best_score = 0;
    for (int x = 0; x < m->width; x++) {
        for (int y = 0; y < m->height; y++) {
            int score = computeScenicScore(m,x,y);
            if (score > best_score) best_score = score;
        }
    }
    printf("Best scenic score: %d\n", best_score);

    /* Cleanup. */
    freeMap(m);

    return 0;
}
