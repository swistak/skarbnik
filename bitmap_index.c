#define _GNU_SOURCE

#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "bit_helpers.h"

#define INITIAL_LENGTH 16
#define ALINGMENT 16
// Maybe adjust to page size?
//  size_t page_size = (size_t) sysconf (_SC_PAGESIZE);


typedef unsigned long long int BitPart;
typedef unsigned long long int Int;

#define try_io(s) if ((void*)(s) == (void*)-1) { fprintf(stderr, "%s:%d (%s): ", __FILE__, __LINE__, __func__); perror("IO Error"); exit(EXIT_FAILURE); }

struct BitMapStructure {
    BitPart* parts;
    Int bit_length;
    Int part_length;

    char* name;
    int persisted;
    int fd;
};

typedef struct BitMapStructure* BitMap;

Int part_length(Int bit_length) {
    float new_part_length;
    new_part_length = floor((bit_length - 1) / sizeof (BitPart) / 8) + 1;
    new_part_length = pow(2, ceil(log2(new_part_length)));
    if (new_part_length < 2) new_part_length = 2;

    return ((Int) new_part_length);
}

BitMap attach_to_file(BitMap self);
void free_bitmap(BitMap self);

BitMap initialize_bitmap(Int bit_length, char* name) {
    BitMap self;
    Int new_part_length = part_length(bit_length);

    self = malloc(sizeof (struct BitMapStructure));
    self->part_length = new_part_length;
    self->bit_length = bit_length;
    if (name) {
        self->name = name;
        self->persisted = 1;
        attach_to_file(self);
    } else {
        self->parts = malloc(new_part_length * sizeof (BitPart));
    }

    return (self);
};

BitMap clone(BitMap right) {
    BitMap self = initialize_bitmap(right->bit_length, NULL);

    for (Int i = 0; i < self->part_length; i++) {
        self->parts[i] = right->parts[i];
    }

    return (self);
};

BitMap resize_bitmap(BitMap self, Int new_bit_length) {
    Int new_part_length = part_length(new_bit_length);

    if (new_part_length != self->part_length) {
        if (self->persisted) {
            int fsize = new_part_length * sizeof (BitPart);
            try_io(lseek(self->fd, fsize - 1, SEEK_SET));
            try_io(write(self->fd, "\0", 1));
            self->parts = mremap(
                    self->parts,
                    self->part_length * sizeof (BitPart),
                    fsize,
                    MREMAP_MAYMOVE
                    );
        } else {
            self->parts = realloc(self->parts, new_part_length * sizeof (BitPart));
        }
    }

    self->bit_length = new_bit_length;
    self->part_length = new_part_length;

    return (self);
}

BitMap bm_or(BitMap self, BitMap right) {
    if (self->bit_length != right->bit_length) return (NULL);

    for (Int i = 0; i < self->part_length; i++) {
        self->parts[i] |= right->parts[i];
    }

    return (self);
};

BitMap bm_and(BitMap self, BitMap right) {
    if (self->bit_length != right->bit_length) return (NULL);

    for (Int i = 0; i < self->part_length; i++) {
        self->parts[i] &= right->parts[i];
    }

    return (self);
};

char* i2b(Int number) {
    int width = sizeof (Int) * 8;
    char* s = malloc(width + 1);

    s[width] = 0x00; // terminate string
    for (int i = 0; i < width; i++) {
        s[i] = ((1 << i) & number) > 0 ? '1' : '0';
    }

    return s;
}

BitMap print(BitMap self) {
    printf("%-10s - Part length: %lld  Bit Length: %lld\n", self->name, self->part_length, self->bit_length);
    for (int i = 0; i < self->part_length; i += 2) {
        printf("%s %s \n", i2b(self->parts[i]), i2b(self->parts[i + 1]));
    }
    return (self);
}

BitMap attach_to_file(BitMap self) {
    int fname_length = strlen(self->name) + 8 + 4 + 1; // length of 'indices/' + length of '.idx' + 1 for 0
    char* fname = malloc(fname_length);
    struct stat buffer;
    int fsize = self->part_length * sizeof (BitPart);

    sprintf(fname, "indices/%s.idx", self->name);

    try_io(self->fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0600));

    try_io(fstat(self->fd, &buffer));
    if (buffer.st_size < fsize) {
        try_io(lseek(self->fd, fsize - 1, SEEK_SET));
        try_io(write(self->fd, "\0", 1));
    }

    try_io(self->parts = mmap(NULL, fsize, PROT_WRITE | PROT_READ, MAP_SHARED, self->fd, 0));

    return (self);
}

void free_bitmap(BitMap self) {
    if (self->persisted) {
        if (self -> parts) munmap(self->parts, self->part_length * sizeof (BitPart));
        close(self->fd);
    } else {
        free(self->parts);
    }
    free(self);
};

int main(int argc, char** argv) {
    int i;

    prepare_bit_table(sizeof (BitPart) * 8);

    BitMap left = initialize_bitmap(16 * 8, "left");
    BitMap right = initialize_bitmap(16 * 8, "right");

    BitMap large = initialize_bitmap(16 * 8, "large");
    resize_bitmap(large, 1024 * 1024 * 8);
    for (i = 0; i < large->part_length; i++) {
        large->parts[i] = rand();
    }
    //print(large);

    for (i = 0; i < 2; i++) {
        left->parts[i] = rand();
        right->parts[i] = rand();
    }

    print(left);
    print(right);

    BitMap or = bm_or(clone(left), right); //initialize_bitmap(16 * 8);
    BitMap and = bm_and(clone(left), right); //initialize_bitmap(16 * 8);

    or->name = "OR";
    and->name = "AND";

    print(or);
    print(and);

    printf(" i/d | i/d | x | y |or |and\n");

    int d = sizeof (BitPart) * 8;
    for (i = 0; i < 16; i++) {
        printf(" %3d | %3d | %d | %d |%2d |%2d \n",
                i / d, i % d,
                bit(left->parts[i / d], i % d),
                bit(right->parts[i / d], i % d),
                bit(or->parts[i / d], i % d),
                bit(and->parts[i / d], i % d)
                );
    }

    free_bitmap(left);
    free_bitmap(right);
    free_bitmap(or);
    free_bitmap(and);
}