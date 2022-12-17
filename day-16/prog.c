#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>
#include <time.h>
#include "aoc.h"

/* Node (valve) representation. */
typedef struct node {
    char name[3];           /* Name is always two letters, plus null term. */
    int flow;               /* Flow rate. */
    struct node *next[32];  /* Connected nodes. */
    int numnext;            /* How many next nodes it has. */

    /* We have a "next" variant where we enlist all the other nodes
     * with the number of steps needed to reach them. Otherwise the program
     * takes quite a while to run (hours). This is a way to cut the problem
     * dimension, since we are getting rid of all the useless nodes (flow 0) */

    int open;               /* Node already open? */
    struct node *nn;        /* Next Node in the linked list of nodes.
                               This is used for lookup. */
} node;

/* Create a new node, and take all the nodes in a linked list
 * so we can lookup them even as long as they are not reachable
 * from the graph. */
node *NodesHead = NULL;
node *newNode(void) {
    node *n = xalloc(sizeof(*n));
    n->name[0] = n->name[1] = '?';
    n->name[2] = 0;
    n->numnext = 0;
    n->open = 0;
    n->flow = 0;
    n->nn = NodesHead;
    NodesHead = n;
    return n;
}

/* Return the node named 'name' */
node *lookupNode(char *name) {
    node *n = NodesHead;
    while(n) {
        if (!memcmp(n->name,name,2)) return n;
        n = n->nn;
    }
    return NULL;
}

/* Let's remember the path, so that we can print the best path found so far. */
char BestPath[64];
char CurPath[64];
int CurPathLen = 0;
int bestflow[30] = {0}; // Best flow so far in a given minute
int DEBUG = 0;

/* Scan the graph lookign for the best total flow.
 * This scan would take a big amount of time, even in C, so to speedup
 * it I use a TERRIBLE heuristic that may not work reliably.
 * The search memorize the best flow at every minute, and will
 * avoid taking certain paths if the current flow is worse of what
 * we seen. This in practice work if you run the program a few times but
 * is technically not correct. However allows to substitute all the
 * graph preprocessing (shortest path and so forth) with a trivial IF. */
void searchGraph(node *root, int curflow, int leftminutes, int *maxflow) {
    if (DEBUG) printf("Scanning %s\n", CurPath);
    /* Ok, so this is the gist of it. The search space is big and it takes
     * (even with a C implementation) like one hour to find the solution.
     * However solutions that seem to suck compared to what already saw
     * in this same minute, are not likely to cut it. However there are times
     * where this heuristic would remove trees that are the best so we
     * apply it just with a probability. This is not "exact" but works very
     * well in practice, without doing precomputation of shortest paths. */
    if (bestflow[leftminutes] < curflow) {
        bestflow[leftminutes] = curflow;
    }
    if (random() % 100 > 25 && curflow < bestflow[leftminutes]) return;

    /* No time left nor valves to open, let's check our total flow. */
    if (leftminutes <= 1) {
        if (curflow > *maxflow) {
            printf("Best %d: %s\n", curflow, CurPath);
            *maxflow = curflow;
            memcpy(BestPath,CurPath,sizeof(BestPath));
        }
        return;
    }

    /* For every next step we can do, consider both the possibility
     * of opening this valve and of not opening it. */
    for (int j = 0; j < root->numnext; j++) {

        /* PATH A: "Open it" path. */
        if (root->open == 0 &&  // We can open it only if not already open.
            root->flow > 0)     // We don't open broken valves. */
        {
            root->open = 1;
            CurPath[CurPathLen++] = root->name[0];
            CurPath[CurPathLen++] = root->name[1];
            CurPath[CurPathLen] = 0;

            int flow = root->flow*(leftminutes-1);
            searchGraph(root->next[j],flow+curflow,leftminutes-2,maxflow);

            CurPathLen -= 2;
            CurPath[CurPathLen] = 0;
            root->open = 0; /* Undo so next paths will find it closed. */
        }

        /* PATH B: "Don't open it" path. */
        CurPath[CurPathLen++] = tolower(root->name[0]);
        CurPath[CurPathLen++] = tolower(root->name[1]);
        CurPath[CurPathLen] = 0;

        searchGraph(root->next[j],curflow,leftminutes-1,maxflow);
        CurPathLen -= 2;
        CurPath[CurPathLen] = 0;
    }
}

/* Given that we do things randomly, we need more randomness...
 * Shuffle all the children nodes of every node. */
void shuffle(node *n) {
    for (int j = 0; j < n->numnext; j++) {
        int r = rand() % n->numnext;
        if (j == r) continue;
        void *tmp = n->next[r];
        n->next[r] = n->next[j];
        n->next[j] = tmp;
    }
}

/* More broken generalizatino of searchGraph(). Needs to run several times
 * to find the *actual* best result. */
void searchGraph2(node *root1, node *root2, int curflow, int leftminutes1, int leftminutes2, int *maxflow) {
    if (DEBUG) printf("Scanning %s\n", CurPath);
    /* Ok, so this is the gist of it. The search space is big and it takes
     * (even with a C implementation) like one hour to find the solution.
     * However solutions that seem to suck compared to what already saw
     * in this same minute, are not likely to cut it. However there are times
     * where this heuristic would remove trees that are the best so we
     * apply it just with a probability. This is not "exact" but works very
     * well in practice, without doing precomputation of shortest paths. */
    if (leftminutes1 > 0) {
        if (bestflow[leftminutes1] < curflow) {
            bestflow[leftminutes1] = curflow;
        }
        if (random() % 100 > 10 && curflow < bestflow[leftminutes1]) return;
    }

    if (leftminutes2 > 0) {
        if (bestflow[leftminutes2] < curflow) {
            bestflow[leftminutes2] = curflow;
        }
        if (random() % 100 > 10 && curflow < bestflow[leftminutes2]) return;
    }

    /* No time left nor valves to open, let's check our total flow. */
    if (leftminutes1 <= 1 && leftminutes2 <= 1) {
        if (curflow > *maxflow) {
            //printf("Best (%d %d) %d\n", leftminutes1, leftminutes2, curflow);
            *maxflow = curflow;
        }
        return;
    }

    /* For every next step we can do, consider both the possibility
     * of opening this valve and of not opening it. */
    for (int j = 0; j < root1->numnext; j++) {
        for (int i = 0; i < root2->numnext; i++) {
            for (int open1 = 0; open1 < 2; open1++) {
                for (int open2 = 0; open2 < 2; open2++) {
                    if (root1 == root2 && open1 && open2) {
                        /* Both actors can't open the same valve
                         * at the same time. */
                        continue;
                    }

                    if (open1 && root1->open) continue;
                    if (open2 && root2->open) continue;

                    int flow1 = 0, flow2 = 0;
                    int o1 = open1, o2 = open2;

                    if (open1 && root1->open == 0 && root1->flow != 0 &&
                        leftminutes1 >= 1)
                    {
                        root1->open = 1;
                        flow1 = root1->flow*(leftminutes1-1);
                    } else {
                        o1 = 0;
                        flow1 = 0;
                    }
                    if (open2 && root2->open == 0 && root2->flow != 0 &&
                        leftminutes2 >= 1)
                    {
                        root2->open = 1;
                        flow2 = root2->flow*(leftminutes2-1);
                    } else {
                        o2 = 0;
                        flow2 = 0;
                    }

                    searchGraph2(root1->next[j],root2->next[i],flow1+flow2+curflow,leftminutes1-1-o1,leftminutes2-1-o2,maxflow);
                    if (o1) root1->open = 0;
                    if (o2) root2->open = 0;
                }
            }
        }
    }
}



/* Only for debugging: I wanted to see if everything loaded correctly. */
void printGraph(node *root) {
    printf("From %s [%p] you can go to (%d):\n", root->name, (void*)root,
        root->numnext);
    for (int j = 0; j < root->numnext; j++) {
        printf("%d) %s [%p]\n", j, root->next[j]->name, (void*)root->next[j]);
    }
    root->open = 1;
    for (int j = 0; j < root->numnext; j++) {
        if (root->next[j]->open == 0)
            printGraph(root->next[j]);
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));

    FILE *fp = argc == 2 ? fopen(argv[1],"r") : stdin;
    if (!fp) {
        perror("fopen");
        exit(1);
    }

    /* Input parsing. Boring but needed. */
    char buf[256];
    node *root = NULL;
    int numvalves = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        size_t l = strlen(buf);
        if (l <= 1) continue;
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }

        char name[3];
        int flow;

        /* Note: next line is quick and dirty and unsafe. */
	sscanf(buf,
	    "Valve %s has flow rate=%d; tunnels lead to valves",
	    name, &flow);

        node *this = NULL;
        this = lookupNode(name);
        if (!this) {
            this = newNode(); /* Happens for AA. */
            if (!root) root = this;
        }
        memcpy(this->name,name,3);
        this->flow = flow;
        this->open = 0;

        /* Populate list of connected nodes. */
        char *p = strstr(buf,"valve");
        assert(p != NULL);
        p += p[5] == 's' ? 7 : 6;
        while(1) {
            node *new = lookupNode(p);
            if (!new) {
                new = newNode();
                memcpy(new->name,p,2);
                new->name[2] = 0;
            }

            /* Add the node as child of the parent. */
            this->next[this->numnext++] = new;

            if (p[2] != ',') break;
            p += 4;
        }

        printf("Valve %s at %p, flow:%d, numnext:%d\n",
            this->name, (void*)this, this->flow, this->numnext);
        numvalves++;
    }
    if (argc == 2) fclose(fp);

    /* Part 1. */
    int maxflow = 0;
    root = lookupNode("AA");
    searchGraph(root,0,30,&maxflow);
    printf("Part 1 %d following this path:\n", maxflow);
    printf("%s\n", BestPath);

    /* Part 2.
     *
     * Let's find the best path using 26 minutes. */
    int mmflow = 0; /* max maxflow. */
    printf("Keep part 2 running for some time (a few minutes). It will\n"
           "converge to the actual solution.\n");
    while(1) {
        memset(bestflow,0,sizeof(bestflow));
        CurPathLen = 0;
        maxflow = 0;
        searchGraph2(root,root,0,26,26,&maxflow);
        if (mmflow < maxflow) mmflow = maxflow;
        printf("Part 2 %d (highet found: %d)\n", maxflow, mmflow);

        /* Rearrange nodes to explore other paths */
        node *n = NodesHead;
        while(n) {
            shuffle(n);
            n = n->nn;
        }
    }
}
