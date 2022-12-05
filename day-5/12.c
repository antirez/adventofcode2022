#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "adlist.h"

#define MAXSTACKS 9

/* Cratemover model (part 1 and part 2 of the puzzle) */
#define CRATEMOVER9000 0
#define CRATEMOVER9001 1

/* Show the current stacks content. */
void printStacks(list **stacks, int count) {
    for (int j = 0; j < count; j++) {
        printf("%d: ", j+1);
        listIter *li = listGetIterator(stacks[j],AL_START_HEAD);
        listNode *node;
        while ((node = listNext(li)) != NULL) {
            printf("[%c]", (char)(unsigned long)(void*)listNodeValue(node));
        }
        listReleaseIterator(li);
        printf("\n");
    }
}

/* Given MAXSTACKS stacks and the string describing the move operation
 * to do in the form move <stack> from <src> to <dst> perform the
 * operation. From/to are 1-based. */
void makeMove(list **stacks, char *buf, int model) {
    int count, from, to;
    size_t len = strlen(buf);
    char *p;

    if (len < 4) return; /* Trivial sanity check. */
    if (buf[len-1] == '\n') buf[len-1] = '\0';
    /* Seek "to" */
    p = strchr(buf,'t'); if (!p) return;
    p[-1] = 0; /* nullterm "from" */
    to = atoi(p+3); /* Parse "to" */
    /* Seek "from" */
    p = strchr(buf,'f'); if (!p) return;
    p[-1] = 0; /* nullterm "move" */
    from = atoi(p+5); /* Parse "from" */
    count = atoi(buf+5); /* Parse "move" */

    /* Bound checking. */
    if (to   < 1 ||  to > 9 ||
        from < 1  || from > 9 ||
        count < 1) return;

    from--, to--;
    list *lfrom = stacks[from];
    list *lto = stacks[to];
    list *ltmp = listCreate(); /* Used only for 9001 model. */

    /* Common code between 9000 and 9001 model. */
    for (int j = 0; j < count; j++) {
        listNode *ln = listIndex(lfrom,-1);
        if (ln == NULL) {
            printf("Not enough items to move. Exiting.\n");
            exit(1);
            return;
        }
        void *value = listNodeValue(ln);
        listDelNode(lfrom,ln);
        listAddNodeTail(model == CRATEMOVER9000 ? lto : ltmp,value);
    }

    /* For cratemover9001, we simuate the behavior putting the items
     * in a temp list, and then moving back to the destination. This
     * way the order is retained. */
    if (model == CRATEMOVER9001) {
        for (int j = 0; j < count; j++) {
            listNode *ln = listIndex(ltmp,-1);
            void *value = listNodeValue(ln);
            listDelNode(ltmp,ln);
            listAddNodeTail(lto,value);
        }
    }

    listRelease(ltmp);
}

int main(int argc, char **argv) {
    char buf[128];
    int model = CRATEMOVER9000;

    /* Parse arguments to understand what model to use. */
    if (argc == 2) {
        if (!strcmp(argv[1],"--9000")) {
            model = CRATEMOVER9000;
        } else if (!strcmp(argv[1],"--9001")) {
            model = CRATEMOVER9001;
        } else {
            printf("Usage: %s [--9000 | --9001]\n", argv[0]);
            exit(1);
        }
    }

    FILE *fp = fopen("input.txt","r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    list *stacks[MAXSTACKS];
    for (int j = 0; j < MAXSTACKS; j++)
        stacks[j] = listCreate();

    int loading_stage = 1; /* When 0 we are at moving stage. */
    printf("Loading...\n");
    while(fgets(buf,sizeof(buf),fp) != NULL) {

        /* Switch from loading to moving stage when the line is
         * " 1   2   3 ..." */
        if (loading_stage && buf[1] == '1') {
            loading_stage = 0;
            printf("Switching to moving stage...\n");
            continue;
        }

        if (loading_stage) {
            for (int j = 0; j < MAXSTACKS; j++) {
                int c = buf[1+j*4];
                printf("%c",c);
                if (c >= 'A' && c <= 'Z')
                    listAddNodeHead(stacks[j],(void*)(unsigned long)c);
            }
            printf("\n");
        } else {
            /* Ignore lines not starting with "move" */
            if (buf[0] != 'm') continue;
            makeMove(stacks,buf,model);
        }
    }
    printStacks(stacks,MAXSTACKS);

    /* Cleanup. */
    for (int j = 0; j < MAXSTACKS; j++)
        listRelease(stacks[j]);
    fclose(fp);
    return 0;
}
