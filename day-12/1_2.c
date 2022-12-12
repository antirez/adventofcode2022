#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

#define ELFDIR_NAME_MAX 64
#define ELFDIR_SUB_MAX 16

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

/* Initialize a map structure. */
void initMap(map *m) {
    m->width = 0;
    m->height = 0;
    m->buf = NULL;
    m->aux = NULL;
    m->sx = m->sy = 0;
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
    m->aux = malloc(mapSize(m));
    if (!m->aux) {
        freeMap(m);
        return NULL;
    }
    mapFind(m,'S',&m->sx,&m->sy); /* Set the start location. */
    return m;
}

/* Print the current map and aux value. cx,cy is
 * the current position we want to highlight. */
void printMap(map *m, int cx, int cy) {
    for (int y = 0; y < m->height; y++) {
        for (int x = 0; x < m->width; x++) {
            int h = m->buf[x+y*m->width]; /* Map elevation. */
            int d = m->aux[x+y*m->width]; /* Current path direction. */
            if (x == cx && y == cy) {
                printf("\033[32;1m%c\033[0m", h);
            } else {
                printf("%c", d ? d : h);
            }
        }
        printf("\n");
    }
}

/* Find the shortest path to get from 'S' to 'E' in map 'map'.
 * x,y represent the current position we are at. The pointer 'p'
 * is the current path, and 'shortest' is updated every time we find
 * a path to 'E' that is shortest than the current value.
 *
 * If the goal wasn't found, -1 is returned. Otherwise the number
 * of steps required to reach it. */
int findShortestPath(map *m, int target, int invert) {
    static struct { /* Use static since this is constant AND we use
                       deep recursion here. No good to put all this
                       in the stack every time. */
        int d, dx, dy;
    } dir[4] = {
        {'>', 1, 0},
        {'<', -1, 0},
        {'^', 0, -1},
        {'v', 0, 1}
    };

    /* Reset "visited" */
    memset(m->aux,0,mapSize(m));

    /* Poor's man queue... Very poor. */
    struct {int x; int y; int depth;} *queue = malloc(sizeof(queue[0])*10000);
    int qlen = 0;

    /* Add the starting point in the queue and mark it as already
     * visited. */
    queue[qlen].x = m->sx;
    queue[qlen].y = m->sy;
    queue[qlen].depth = 0;
    qlen++;
    setAux(m,m->sx,m->sy,'.');

    while(qlen) {
        qlen--;
        int x = queue[0].x;
        int y = queue[0].y;
        int depth = queue[0].depth;

        /* God, forgive me for what I'm doing: */
        memmove(queue,queue+1,qlen*sizeof(queue[0]));

        /* Check if we are already at 'E'. */
        int curh = mapGet(m,x,y); /* Current elevation. */
        if (curh == target) {
            printf("Found a path of len %d\n", depth);
            /* The following map printing is nice with small maps, but
             * terrible with the actual input. */
            // printMap(m,x,y);
            free(queue);
            return depth;
        }
        if (curh == 'S') curh = 'a'; /* Normalize special elevation. */

        /* Queue all the possible next steps starting from here. */
        for (int j = 0; j < 4; j++) {
            int xx = x+dir[j].dx;
            int yy = y+dir[j].dy;
            int h = mapGet(m,xx,yy);
            if (h == 'E') h = 'z';
            if (h == -1) continue; /* Out of our world boundaries. */
            /* Check for height jump. However if 'invert' is true
             * instead of following the rule of going back just by 1
             * and going lower as much as we wish, we reverse it, and
             * we go lower only 1 step, and high as much as we want.
             * This is useful for Puzzle 2. */
            if (!invert) {
                if ((h - curh) > 1) continue;
            } else {
                if ((h - curh) < -1) continue;
            }
            if (getAux(m,xx,yy) != 0) continue; /* Already visited. */

            /* Remember to explore this path. */
            queue[qlen].x = xx;
            queue[qlen].y = yy;
            queue[qlen].depth = depth+1;
            qlen++;

            /* Mark as visited. */
            setAux(m,xx,yy,'.');
        }
    }
    free(queue);
    return -1;
}

int main(void) {
    /* Read map in memory. */
    map *m = readMapFromFile(stdin);
    if (m == NULL) {
        printf("Out of memory reading map\n");
        exit(1);
    }

    printf("Map loaded. Size: %dx%d\n", m->width, m->height);
    printf("Start is at %d,%d\n", m->sx, m->sy);
    findShortestPath(m,'E',0);

    /* To find the shortest a -> E, we do a trick, start from E and
     * search for the first 'a', however we have to fix those 'S'
     * and 'E' special chars that will create issues otherwise. */
    m->buf[m->sx+m->sy*m->width] = 'a';
    mapFind(m,'E',&m->sx,&m->sy); /* Set the start location. */
    m->buf[m->sx+m->sy*m->width] = 'z';
    findShortestPath(m,'a',1);

    /* Cleanup. */
    freeMap(m);
    return 0;
}
