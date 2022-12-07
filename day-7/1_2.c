#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ELFDIR_NAME_MAX 64
#define ELFDIR_SUB_MAX 16

/* This describes every directory in our filesystem. */
typedef struct elfdir {
    char name[ELFDIR_NAME_MAX];
    size_t size;            /* Size excluding subdirs. */
    size_t totalsize;       /* Size including subdirs. */
    int subdirs;            /* Number of sub directories. */
    struct elfdir *sub[ELFDIR_SUB_MAX]; /* Subdirectories pointers. */
    struct elfdir *parent;              /* Containing directory. NULL for / */
} elfdir;

/* Allocate and initialize a new directory node. */
elfdir *makeElfdir(elfdir *parent, const char *name) {
    elfdir *ed = malloc(sizeof(*ed));
    if (ed == NULL) return NULL;
    strncpy(ed->name,name,ELFDIR_NAME_MAX);
    ed->size = 0;
    ed->totalsize = 0;
    ed->subdirs = 0;
    ed->parent = parent;
    return ed;
}

/* Scan the console output at 'filename' to reconstruct the directory
 * tree. Return the parent directory. We assume that the file starts
 * with "cd /". Return the top-most level directory found. */
elfdir *readTree(const char *filename) {
    elfdir *root = NULL; /* Top most dir. */
    elfdir *cd = NULL;  /* Current dir. */

    FILE *fp = fopen(filename,"r");
    if (fp == NULL) {
        perror("Opening input file");
        exit(1);
    }

    char buf[128];
    while(fgets(buf,sizeof(buf),fp) != NULL) {
        size_t len = strlen(buf);
        if (len && buf[len-1] == '\n') buf[len-1] = 0;

        if (buf[0] == '$') {
            char *p = strchr(buf+2,' ');
            if (p == NULL) continue;
            *p = 0;
            char *cmd = buf+2;
            char *arg = p+1;

            if (!strcmp(cmd,"ls")) continue; /* Just scan the files. */
            if (!strcmp(cmd,"cd")) {
                if (!strcmp(arg,"..")) {
                    /* Go upper. */
                    if (!cd) continue; /* We don't know where we are... */
                    if (cd->parent == NULL) {
                        /* Wait... we didn't knew there was a directory
                         * at higher level. Does not happen in input, anyway. */
                        cd->parent = makeElfdir(NULL,"?");
                        root = cd->parent;
                    }
                    /* Again: does not happen in the input, but if we
                     * believe this is the root dir, and we see "..", we
                     * need to update our view of the tree. */
                    if (cd == root) root = cd->parent;

                    cd->parent->totalsize += cd->totalsize;
                    cd = cd->parent; /* Update the current dir. */
                } else {
                    /* Entering a subdir. Assume we enter the subdir
                     * only once in the input. */
                    elfdir *newdir = makeElfdir(cd,arg);
                    if (!root) root = newdir;
                    /* Add this subdirectory to the current one. */
                    if (cd) cd->sub[cd->subdirs++] = newdir;
                    cd = newdir;
                }
            }
        } else {
            char *p = strchr(buf,' ');
            if (p == NULL) continue;
            *p = 0;
            char *attr = buf; /* Attribute part. */
            // char *name = p+1; /* Filename -- never used */
            if (!strcmp(attr,"dir")) continue; /* zero size. */

            /* If the input starts with $ ls and we don't know what
             * the current dir is, we don't handle that to avoid adding
             * too much complexity. Just discard such files. Does not
             * happen in the actual input. */
            if (!cd) continue;
            size_t filesize = atoi(attr);
            cd->size += filesize;
            cd->totalsize += filesize;
        }
    }

    /* The current directory after seeing all the commands will be
     * some random one. We need to navigate up to the root to fix
     * the total size computation. */
    while(cd != root) {
        cd->parent->totalsize += cd->totalsize;
        cd = cd->parent;
    }
    fclose(fp);
    return root;
}

/* Pretty print a given directory. */
void printDir(elfdir *ed, int level) {
    for (int k = 0; k < level*2; k++) printf(" ");
    printf("- %s (size %zu | total %zu)\n", ed->name, ed->size, ed->totalsize);
}

/* Pretty print the directory tree. */
void printTree(elfdir *root, int level) {
    if (level == 0) printDir(root,level);
    for (int j = 0; j < root->subdirs; j++) {
        printDir(root->sub[j], level+1);
        printTree(root->sub[j], level+1);
    }
}

/* Sum of total size of every dir <= 'upto' bytes. */
void findTotalUpTo(elfdir *root, size_t upto, size_t *sum) {
    for (int j = 0; j < root->subdirs; j++) {
        size_t size = root->sub[j]->totalsize;
        if (size <= upto) *sum += size;
        findTotalUpTo(root->sub[j], upto, sum);
    }
}

/* Find the size of the smallest directory that is at least 'minsize'
 * in size. */
void findSmallestUpTo(elfdir *root, size_t minsize, size_t *result) {
    for (int j = 0; j < root->subdirs; j++) {
        size_t size = root->sub[j]->totalsize;
        if (size >= minsize && (*result == 0 || *result > size))
            *result = size;
        findSmallestUpTo(root->sub[j], minsize, result);
    }
}

int main(void) {
    elfdir *root = readTree("input.txt");
    printTree(root,0);
    size_t part1 = 0;
    findTotalUpTo(root,100000,&part1);
    printf("part 1 answer: %zu\n", part1);

    size_t minsize = 30000000 - (70000000 - root->totalsize);
    size_t part2 = 0;
    findSmallestUpTo(root,minsize,&part2);
    printf("part 2 answer: %zu\n", part2);

    /* TODO reclaim memory from 'root' :D */
    return 0;
}
