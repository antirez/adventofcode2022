#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <ctype.h>

/* This describes our elf object type. It can be used to represent
 * nested lists of lists and/or integers. */
#define ELFOBJ_TYPE_INT  0
#define ELFOBJ_TYPE_LIST 1
typedef struct elfobj {
    int type;       /* ELFOBJ_TYPE_... */
    union {
        int i;      /* Integer value. */
        struct {    /* List value. */
            struct elfobj **ele;
            size_t len;
        } l;
    } val;
} elfobj;

/* Life is too short to handle OOM for Advent of Code. */
void *elfalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        fprintf(stderr,"Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return p;
}

void *elfrealloc(void *ptr, size_t size) {
    void *p = realloc(ptr,size);
    if (!p) {
        fprintf(stderr,"Out of memory allocating %zu bytes\n", size);
        exit(1);
    }
    return p;
}

/* Recursively free an elf object. */
void freeElfObj(elfobj *obj) {
    switch(obj->type) {
    case ELFOBJ_TYPE_INT: break; /* Nothing nested to free. */
    case ELFOBJ_TYPE_LIST:
        for (size_t j = 0; j < obj->val.l.len; j++)
            freeElfObj(obj->val.l.ele[j]);
        free(obj->val.l.ele);
        break;
    }
    free(obj);
}

/* Given the string 's' return the elfobj representing the list or
 * NULL on syntax error. '*next' is set to the next byte to parse, after
 * the current value was completely parsed. */
elfobj *parseList(const char *s, const char **next) {
    elfobj *obj = elfalloc(sizeof(*obj));
    while(isspace(s[0])) s++;
    if (s[0] == '-' || isdigit(s[0])) {
        char buf[64];
        size_t len = 0;
        while((*s == '-' || isdigit(*s)) && len < sizeof(buf)-1)
            buf[len++] = *s++;
        buf[len] = 0;
        obj->type = ELFOBJ_TYPE_INT;
        obj->val.i = atoi(buf);
        if (next) *next = s;
        return obj;
    } else if (s[0] == '[') {
        obj->type = ELFOBJ_TYPE_LIST;
        obj->val.l.len = 0;
        obj->val.l.ele = NULL;
        s++;
        /* Parse comma separated elements. */
        while(1) {
            /* The list may be empty, so we need to parse for "]"
             * ASAP. */
            while(isspace(s[0])) s++;
            if (s[0] == ']') {
                if (next) *next = s+1;
                return obj;
            }

            /* Parse the current sub-element recursively. */
            const char *nextptr;
            elfobj *element = parseList(s,&nextptr);
            if (element == NULL) {
                freeElfObj(obj);
                return NULL;
            }
            obj->val.l.ele = elfrealloc(obj->val.l.ele,
                                        sizeof(elfobj*)*(obj->val.l.len+1));
            obj->val.l.ele[obj->val.l.len++] = element;
            s = nextptr; /* Continue from first byte not parsed. */

            while(isspace(s[0])) s++;
            if (s[0] == ']') continue; /* Will be handled by the loop. */
            if (s[0] == ',') {
                s++;
                continue; /* Parse next element. */
            }

            /* Syntax error. */
            freeElfObj(obj);
            return NULL;
        }
        /* Syntax error (list not closed). */
        freeElfObj(obj);
        return NULL;
    } else {
        /* In a serious program you don't printf() in the middle of
         * a function. Just return NULL. */
        fprintf(stderr,"Syntax error parsing '%s'\n", s);
        return NULL;
    }
    return obj;
}

/* Output an object human readable representation .*/
void printElfObj(elfobj *obj) {
    switch(obj->type) {
    case ELFOBJ_TYPE_INT:
        printf("%d",obj->val.i);
        break;
    case ELFOBJ_TYPE_LIST:
        printf("[");
        for (size_t j = 0; j < obj->val.l.len; j++) {
            printElfObj(obj->val.l.ele[j]);
            if (j != obj->val.l.len-1) printf(", ");
        }
        printf("]");
        break;
    }
}

/* Read the lists contained in the file 'fp', parse them into an elfobj
 * type and populate v[...] with the values. The number of lists processed
 * is returned. */
int readLists(FILE *fp, elfobj **v, size_t vlen) {
    char buf[1024];
    size_t idx = 0;
    while(fgets(buf,sizeof(buf),fp) != NULL && idx < vlen) {
        size_t l = strlen(buf);
        if (l <= 1) continue;
        if (buf[l-1] == '\n') {
            buf[l-1] = 0;
            l--;
        }
        v[idx++] = parseList(buf,NULL);
    }
    return idx;
}

/* Compare the two objects 'a' and 'b' and return:
 * -1 if a<b; 0 if a==b; 1 if a>b. */
int compare(elfobj *a, elfobj *b) {
    if (a->type == ELFOBJ_TYPE_INT && b->type == ELFOBJ_TYPE_INT) {
        if (a->val.i < b->val.i) return -1;
        else if (a->val.i > b->val.i) return 1;
        return 0;
    }

    /* If one of the objects is not a list, promote it to a list.
     * Just use the stack to avoid allocating stuff for a single
     * element list. */
    elfobj list, listele, *ele[1];
    list.type = ELFOBJ_TYPE_LIST;
    list.val.l.len = 1;
    list.val.l.ele = ele;
    list.val.l.ele[0] = &listele;
    listele.type = ELFOBJ_TYPE_INT;

    /* Promote. */
    if (a->type == ELFOBJ_TYPE_INT) {
        listele.val.i = a->val.i;
        a = &list;
    } else if (b->type == ELFOBJ_TYPE_INT) {
        listele.val.i = b->val.i;
        b = &list;
    }

    /* Now we can handle the list to list comparison without
     * special cases. */
    size_t minlen = a->val.l.len < b->val.l.len ? a->val.l.len : b->val.l.len;
    for (size_t j = 0; j < minlen; j++) {
        int cmp = compare(a->val.l.ele[j],b->val.l.ele[j]);
        if (cmp != 0) return cmp;
    }

    /* First MIN(len_a,len_b) elements are the same? Longer list wins. */
    if (a->val.l.len < b->val.l.len) return -1;
    else if (a->val.l.len > b->val.l.len) return 1;
    return 0;
}

/* qsort() helper to sort arrays of elfobj pointers. */
int qsort_list_cmp(const void *a, const void *b) {
    elfobj **obja = (elfobj**)a, **objb = (elfobj**)b;
    return compare(obja[0],objb[0]);
}

#define LISTS_MAX 1024
int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n",argv[0]);
        exit(1);
    }
    elfobj *lists[LISTS_MAX];
    FILE *fp = fopen(argv[1],"r");
    if (!fp) {
        perror("Can't open file");
        exit(1);
    }
    size_t count = readLists(fp,lists,LISTS_MAX);
    fclose(fp);

    printf("%zu lists read from file\n", count);
    assert(count % 2 == 0); /* Must be all pairs. */

    /* Part 1: just compare all the pairs and do the sum. */
    int okpairs = 0;
    for (size_t j = 0; j < count; j += 2) {
        int cmp = compare(lists[j], lists[j+1]);
        if (cmp == -1) okpairs += (j/2)+1;
    }
    printf("Puzzle 1: %d\n", okpairs);

    /* Part 2: add the two additional lists. */
    elfobj *div1 = parseList("[[2]]",NULL);
    elfobj *div2 = parseList("[[6]]",NULL);
    lists[count++] = div1;
    lists[count++] = div2;
    qsort(lists,count,sizeof(elfobj*),qsort_list_cmp);

    /* Now scan the list to locate div1 and div2. */
    size_t div1idx, div2idx;
    for (size_t j = 0; j < count; j++) {
        if (lists[j] == div1) div1idx = j+1;
        else if (lists[j] == div2) div2idx = j+1;
    }
    printf("Puzzle 2: %zu\n", div1idx*div2idx);
    return 0;
}
