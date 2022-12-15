#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

typedef struct sensor {
    int x, y;   /* Sensor position. */
    int bx, by; /* Detected beacon position. */
    int dist;   /* Distance between sensor and detected beacon. */
} sensor;

#define SENSORS_MAX 300

/* Compute the Manhattan distance between two points. */
int mandistance(int x1, int y1, int x2, int y2) {
    return abs(x1-x2)+abs(y1-y2)+1;
}

/* Return the number of sensors that could potentially detect
 * a potential beacon at x,y. */
int coulddetect(sensor *sensors, int count, int x, int y) {
    int could_detect = 0;
    for (int j = 0; j < count; j++) {
        /* If this position is already the beam this sensor detected,
         * we can't count this position. */
        if (sensors[j].bx == x && sensors[j].by == y) {
            could_detect = 0;
            break;
        }
        /* Are we in range? */
        int d = mandistance(x,y,sensors[j].x,sensors[j].y);
        if (d <= sensors[j].dist) {
            could_detect++;
            continue;
        }
    }
    return could_detect;
}

int main(int argc, char **argv) {
    FILE *fp = argc == 2 ? fopen(argv[1],"r") : stdin;
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    int sid = 0;    /* Sensor ID. */
    sensor *sensors = malloc(sizeof(sensor)*SENSORS_MAX);
    if (!sensors) {
        perror("OOM");
        exit(1);
    }

    char buf[256];
    int minx = INT_MAX, maxx = INT_MIN, miny = INT_MAX, maxy = INT_MIN;
    int maxdist = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL && sid < SENSORS_MAX) {
        size_t l = strlen(buf);
        if (l <= 1) continue;
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        sensor *s = sensors+sid;
        sid++;

	sscanf(buf,
	    "Sensor at x=%d, y=%d: closest beacon is at x=%d, y=%d",
	    &s->x, &s->y, &s->bx, &s->by);
        s->dist = mandistance(s->x,s->y,s->bx,s->by);
        if (maxdist < s->dist) maxdist = s->dist;
        printf("Sensor at %d,%d distance %d\n", s->x, s->y, s->dist);

        /* Brutal if wall to store the min/max coordinates we
         * have seen. We want to have the boundaries of our world
         * in order to check only for positions that can be
         * within our ranges. */
        if (s->x < minx) minx = s->x;
        if (s->x > maxx) maxx = s->x;
        if (s->y < miny) miny = s->y;
        if (s->y > maxy) maxy = s->y;
        if (s->bx < minx) minx = s->bx;
        if (s->bx > maxx) maxx = s->bx;
        if (s->by < miny) miny = s->by;
        if (s->by > maxy) maxy = s->by;
    }
    if (argc == 2) fclose(fp);
    printf("%d sensors read from file\n", sid);
    printf("World encosing box %d,%d - %d,%d\n", minx,miny,maxx,maxy);

    /* Puzzle 1. */
    int y = 2000000; /* Use y = 10 for example small input. */
    int cantbe = 0; /* Places where the missing beacon can't be. */
    for (int x = minx-maxdist; x <= maxx+maxdist; x++) {
        /* For each x,y check if at least one beacon could detect
         * this point. */
        int could_detect = coulddetect(sensors,sid,x,y);
        cantbe += could_detect != 0;
    }
    printf("%d\n", cantbe);

    /* Puzzle 2 want us to determine in which position the beacon could
     * be in the range 0 - 4000000. No way to do this by brute force...
     * however to be a single position among this space, and since we CAN'T
     * detect it, it must be near the edges of one of our sensors limits.
     * So we check near the limits of our sensors to find such a point.
     *
     * To navigate around the edges of a sensor is not hard. Consider this
     * sensor at sx,sy with detection range=3. If we start at any of the
     * corner points, we just have, for 3 times to do sx++, sy++ (or --
     * depending on the direction), and we can navigate all the positions.
     *          
     *          #
     *         ###
     *        ##S##
     *         ###
     *          #
     */
    for (int j = 0; j < sid; j++) {
        int x = sensors[j].x;
        int y = sensors[j].y;
        int dist = sensors[j].dist;

        /* Start at right edge. */
        x += dist;
        struct {
            int dx, dy;
        } dir[4] = {
            {-1,-1},
            {-1,1},
            {1,1},
            {1,-1}
        };

        for (int k = 0; k < 4; k++) {
            for (int steps = 0; steps < dist; steps++) {
                if (coulddetect(sensors,sid,x,y) == 0 &&
                    x >= 0 && x <= 4000000 &&
                    y >= 0 && y <= 4000000)
                {
                    printf("Puzzle 2: %d,%d\n", x,y);
                    printf("Freq: %lld\n", (long long)4000000*x+y);
                    return 0;
                }
                if (steps != dist-1) {
                    x += dir[k].dx;
                    y += dir[k].dy;
                }
            }
        }
    }
    return 0;
}
