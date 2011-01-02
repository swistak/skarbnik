
#define _GNU_SOURCE

#include <malloc.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "../include/bitmap_index.h"
#include "../include/bit_helpers.h"

int part_length(int bit_length) {
    float new_part_length;
    new_part_length = floor((bit_length - 1) / sizeof (BitMapPart) / 8) + 1;
    new_part_length = pow(2, ceil(log2(new_part_length)));
    if (new_part_length < 2) new_part_length = 2;

    return ((int) new_part_length);
}

char* i2b(int number) {
    int width = sizeof (int) * 8;
    char* s = malloc(width + 1);

    s[width] = 0x00; // terminate string
    for (int i = 0; i < width; i++) {
        s[i] = ((1 << i) & number) > 0 ? '1' : '0';
    }

    return s;
}

/*
 * Initializes new BitMap for given bit_length, with given name.
 * There are two kinds of bit maps, persisted ones, and in memory ones.
 *
 * Persisted BitMaps are memory maped to files inside indices/ directory.
 * Once created, type of instance cannot be changed. If you need to save in
 * memory BitMap - copy it to new mmaped Bitmap.
 *
 * Size of bit map is aligned to nearest exponent of 2 * sizeof(BitMapPart).
 * So if you create bit map to hold 2 bits, you'll be using 16 bytes of memory
 * just for minimal 2 allocated parts. Becouse of that memory is allocated
 * first to hold 128 bits then 256, 512, 1024.
 *
 */
BitMap bm_initialize(int bit_length, char* name, int persist) {
    BitMap self;
    int new_part_length = part_length(bit_length);

    self = malloc(sizeof (struct BitMapStructure));
    self->part_length = new_part_length;
    self->bit_length = bit_length;
    self->name = name;
    self->persisted = persist;
    if (persist) {
        bm_attach_to_file(self);
    } else {
        self->parts = malloc(new_part_length * sizeof (BitMapPart));
    }

    return (self);
};

/*
 * Resizes bitmap to new bit length. If bit length falls into same part length,
 * then nothing is changed besides bit_length.
 *
 * realloc and mremap are used - both claim to copy data, but they might change
 * address of parts pointer. Becouse of that, client should not store pointers
 * to arbitrary parts, instead store index of part relative to first part.
 * 
 */
BitMap bm_resize(BitMap self, int new_bit_length) {
    int new_part_length = part_length(new_bit_length);
    int new_byte_size = new_part_length * sizeof (BitMapPart);

    if (new_part_length != self->part_length) {
        if (self->persisted) {
            try_io(lseek(self->fd, new_byte_size - 1, SEEK_SET));
            try_io(write(self->fd, "\0", 1));
            self->parts = mremap(
                    self->parts,
                    self->part_length * sizeof (BitMapPart),
                    new_byte_size,
                    MREMAP_MAYMOVE
                    );
        } else {
            self->parts = realloc(self->parts, new_byte_size);
        }
    }

    self->bit_length = new_bit_length;
    self->part_length = new_part_length;

    return (self);
}

/*
 * Attaches BitMap to memory mapped file that is created in indices/ directory.
 * file name is based on nmap name, so bit map name should be valid linux file
 * name.
 *
 * File is created immidietly and space is allocated to match requested bit map
 * size. Previously allocated bit map space is freed without copying it!
 */
BitMap bm_attach_to_file(BitMap self) {
    int fname_length = strlen(self->name) + 8 + 4 + 1; // length of 'indices/' + length of '.idx' + 1 for 0
    char* fname = malloc(fname_length);
    struct stat buffer;
    int fsize = self->part_length * sizeof (BitMapPart);
    sprintf(fname, "indices/%s.idx", self->name);

    if (self->parts) free(self->parts);

    try_io(self->fd = open(fname, O_RDWR | O_CREAT | O_TRUNC, 0600));
    try_io(fstat(self->fd, &buffer));
    if (buffer.st_size < fsize) {
        try_io(lseek(self->fd, fsize - 1, SEEK_SET));
        try_io(write(self->fd, "\0", 1));
    }
    try_io(self->parts = mmap(NULL, fsize, PROT_WRITE | PROT_READ, MAP_SHARED, self->fd, 0));

    free(fname);
    return (self);
}

/*
 * Releases memory allocated for bit map. If bit map is memory mapped, this will
 * ensure it's written to disk.
 */
void bm_free(BitMap self) {
    if (self->persisted) {
        if (self -> parts) munmap(self->parts, self->part_length * sizeof (BitMapPart));
        close(self->fd);
    } else {
        free(self->parts);
    }
    free(self);
};

/*
 * Copies data and bit_length from other array.
 * 
 * If another array won't fit current one, it's resized.
 * 
 */
BitMap bm_copy(BitMap self, BitMap other) {
    if (other->part_length > self->part_length) {
        bm_resize(self, other->bit_length);
    }

    self->bit_length = other->bit_length;
    for (int i = 0; i < other->part_length; i++) {
        self->parts[i] = other->parts[i];
    }

    return (self);
};

/*
 * Merges right BitMap performing logical _OR_ operation on bits.
 *
 * If bit_lengths are not equal NULL is returned.
 */
BitMap bm_or(BitMap self, BitMap right) {
    if (self->bit_length != right->bit_length) return (NULL);

    for (int i = 0; i < self->part_length; i++) {
        self->parts[i] |= right->parts[i];
    }

    return (self);
};

/*
 * Merges right BitMap performing logical _AND_ operation on bits.
 *
 * If bit_lengths are not equal NULL is returned.
 */
BitMap bm_and(BitMap self, BitMap right) {
    if (self->bit_length != right->bit_length) return (NULL);

    for (int i = 0; i < self->part_length; i++) {
        self->parts[i] &= right->parts[i];
    }

    return (self);
};

/*
 * Merges right BitMap performing logical _XOR_ operation on bits.
 *
 * If bit_lengths are not equal NULL is returned.
 */
BitMap bm_xor(BitMap self, BitMap right) {
    if (self->bit_length != right->bit_length) return (NULL);

    for (int i = 0; i < self->part_length; i++) {
        self->parts[i] ^= right->parts[i];
    }

    return (self);
};

/*
 * Merges right BitMap performing logical _NOT_ operation on right bits.
 *
 * If bit_lengths are not equal NULL is returned.
 */
BitMap bm_not(BitMap self, BitMap right) {
    if (self->bit_length != right->bit_length) return (NULL);

    for (int i = 0; i < self->part_length; i++) {
        self->parts[i] = ~right->parts[i];
    }

    return (self);
};

/*
 * Prints BitMap to stdout.
 */
BitMap bm_print(BitMap self) {
    printf("%-10s - Part length: %d  Bit Length: %d\n", self->name, self->part_length, self->bit_length);
    for (int i = 0; i < self->part_length; i += 2) {
        printf("%d-%d: %s %s \n", i, i + 1, i2b(self->parts[i]), i2b(self->parts[i + 1]));
    }
    return (self);
}

int main(int argc, char** argv) {
    int i;

    prepare_bit_table(sizeof (BitMapPart) * 8);

    BitMap left = bm_initialize(16 * 8, "left", 0);
    BitMap right = bm_initialize(16 * 8, "right", 0);
    BitMap or = bm_initialize(16 * 8, "OR", 1); 
    BitMap and = bm_initialize(16 * 8, "AND", 1); 
    BitMap xor = bm_initialize(16 * 8, "XOR", 1); 
    BitMap not = bm_initialize(16 * 8, "NOT", 1); 

    BitMap large = bm_initialize(16 * 8, "large", 1);
    bm_resize(large, 1024 * 1024 * 8);
    for (i = 0; i < large->part_length; i++) {
        large->parts[i] = rand();
    }
    for (i = 0; i < 2; i++) {
        left->parts[i] = rand();
        right->parts[i] = rand();
    }

    bm_or(bm_copy(or, left), right);
    bm_and(bm_copy(and, left), right);
    bm_xor(bm_copy(xor, left), right);
    bm_not(bm_copy(not, left), left);

    printf("| i/d | i/d | l | r |or |and|xor|not|\n");

    int d = sizeof (BitMapPart) * 8;
    for (i = 0; i < 16; i++) {
        printf("| %3d | %3d | %d | %d |%2d |%2d |%2d |%2d |\n",
                i / d, i % d,
                bit(left->parts[i / d], i % d),
                bit(right->parts[i / d], i % d),
                bit(or->parts[i / d], i % d),
                bit(and->parts[i / d], i % d),
                bit(xor->parts[i / d], i % d),
                bit(not->parts[i / d], i % d)
                );
    }

    bm_free(left);
    bm_free(right);
    bm_free(or);
    bm_free(and);
}