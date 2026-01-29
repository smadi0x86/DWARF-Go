#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uthash.h"

#define MAX_LINE 2048
#define KIND_NAME_COUNT 28

const char *kindNames[KIND_NAME_COUNT] = {
    "kindZero", "kindBool", "kindInt", "kindInt8", "kindInt16", "kindInt32",
    "kindInt64", "kindUint", "kindUint8", "kindUint16", "kindUint32", "kindUint64",
    "kindUintptr", "kindFloat32", "kindFloat64", "kindComplex64", "kindComplex128",
    "kindArray", "kindChan", "kindFunc", "kindInterface", "kindMap", "kindPtr",
    "kindSlice", "kindString", "kindStruct", "kindUnsafePointer"
};

typedef struct FormCount {
    char form[64];
    int count;
    UT_hash_handle hh;
} FormCount;

typedef struct KindStats {
    char kind[32];        
    int count;
    FormCount *forms;
    UT_hash_handle hh;
} KindStats;

KindStats *kinds = NULL;


void addForm(KindStats *kstat, const char *form) {
    FormCount *fc;
    HASH_FIND_STR(kstat->forms, form, fc);

    if (!fc) {
        fc = (FormCount *)calloc(1, sizeof(FormCount));
        strncpy(fc->form, form, 63);
        HASH_ADD_STR(kstat->forms, form, fc);  
    }
    fc->count++;
}

void addKind(const char *kind, const char *form) {
    KindStats *kstat;

    HASH_FIND_STR(kinds, kind, kstat);

    if (!kstat) {
        kstat = (KindStats *)calloc(1, sizeof(KindStats));
        strncpy(kstat->kind, kind, 31);
        HASH_ADD_STR(kinds, kind, kstat);
    }
    kstat->count++;  
    if (form)
        addForm(kstat, form);
}

void parseFile(const char *fname) {
    FILE *fp = fopen(fname, "r");
    
    if (!fp) {
        perror("fopen");

        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE];
    int state = 0;
    char tag[64] = "";  

    while (fgets(line, MAX_LINE, fp)) {
        if (!state) {
            if (strstr(line, "Compilation Unit @ offset")) {
                state = 1;
            }
            continue;
        }

        if (strstr(line, ": Abbrev Number: ")) {
            char *start = strchr(line, '(');
            char *end = strchr(line, ')');

            if (start && end && end > start) {
                size_t len = end - start - 1;
                strncpy(tag, start + 1, len);
                tag[len] = '\0';
            }
            continue;
        }

        unsigned int atVal;
        int kindVal;

        if (sscanf(line, " <%*x> Unknown AT value: %u: %d", &atVal, &kindVal) == 2) {
            if (atVal == 2900 && kindVal < KIND_NAME_COUNT) {
                addKind(kindNames[kindVal], tag);
            }
        }
    }
    
    fclose(fp);
}

void printStats() {

    KindStats *kstat, *tmp;
    FormCount *fc, *fctmp;

    printf(" ---------------------- KINDS ----------------------\n");
    HASH_ITER(hh, kinds, kstat, tmp) {
        
        printf("%s: %d\n", kstat->kind, kstat->count);
        
        HASH_ITER(hh, kstat->forms, fc, fctmp) {
            printf("  %s: %d\n", fc->form, fc->count);  
        }
    }
}

int main(int argc, char **argv) {
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <output.txt>\n", argv[0]);
        
        return EXIT_FAILURE;
    }
    
    for (int i = 1; i < argc; i++) {
        parseFile(argv[i]);
    }

    printStats();
    
    return EXIT_SUCCESS;
}
