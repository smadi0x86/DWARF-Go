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
    // Dward form name
    char form[64];
    // # of times form appeared
    int count;
    // Handle to uthash
    UT_hash_handle hh;
} FormCount;

typedef struct KindStats {
    char kind[32];        // The Go kind (e.g., kindInt, kindMap).
    int count;            // The number of times this kind appeared.
    FormCount *forms;     // A hash table to store the forms associated with this kind.
    UT_hash_handle hh;    // Hash table handle for uthash.
} KindStats;

KindStats *kinds = NULL;  // A hash table for all Go kinds.


void addForm(KindStats *kstat, const char *form) {
    FormCount *fc;
    // Look for form
    HASH_FIND_STR(kstat->forms, form, fc);
    if (!fc) {
        // Alloc mem for form
        fc = (FormCount *)calloc(1, sizeof(FormCount));
        // Copy form name to the struct
        strncpy(fc->form, form, 63);
        // Add to hash table
        HASH_ADD_STR(kstat->forms, form, fc);  
    }
    fc->count++;  // Increment the count of the form.
}

void addKind(const char *kind, const char *form) {
    KindStats *kstat;

    // Look for the go kind
    HASH_FIND_STR(kinds, kind, kstat);

    if (!kstat) {
        kstat = (KindStats *)calloc(1, sizeof(KindStats));
        strncpy(kstat->kind, kind, 31);
        HASH_ADD_STR(kinds, kind, kstat);
    }
    kstat->count++;  // Increment the count of the Go kind.
    if (form)  // If a form is provided, add it to the Go kind's stats.
        addForm(kstat, form);
}

void parseFile(const char *fname) {
    FILE *fp = fopen(fname, "r");
    
    if (!fp) {
        perror("fopen");

        exit(EXIT_FAILURE);
    }

    // Buffer for each line
    char line[MAX_LINE];
    // Track parsing stage
    int state = 0;
    // Store dwarf tag
    char tag[64] = "";  

    while (fgets(line, MAX_LINE, fp)) {
        if (!state) {
            // Look for the "Compilation Unit" to start parsing DWARF attributes
            if (strstr(line, "Compilation Unit @ offset")) {
                state = 1;
            }
            continue;
        }

        // Extract the abbreviation number (DWARF tag)
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

        // Look for the custom specific go DWARF attribute.
        if (sscanf(line, " <%*x> Unknown AT value: %u: %d", &atVal, &kindVal) == 2) {
            if (atVal == 2900 && kindVal < KIND_NAME_COUNT) {
                // If it's a specific go attribute (DW_AT_go_kind), add it to the stats
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
        // print go kind and count of it
        printf("%s: %d\n", kstat->kind, kstat->count);
        HASH_ITER(hh, kstat->forms, fc, fctmp) {
            // print forms for each kind
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
