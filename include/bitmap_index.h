/* 
 * File:   bitmap_index.h
 * Author: swistak
 *
 * Created on 2 styczeń 2011, 20:43
 */

#ifndef _BITMAP_INDEX_H
#define	_BITMAP_INDEX_H

#ifdef	__cplusplus
extern "C" {
#endif

#define BITMAP_INITIAL_LENGTH 16
#define BITMAP_PART_ALINGMENT 16
// Maybe adjust to page size?
//  size_t page_size = (size_t) sysconf (_SC_PAGESIZE);

typedef unsigned long long int BitMapPart;

#define try_io(s) if ((void*)(s) == (void*)-1) { fprintf(stderr, "%s:%d (%s): ", __FILE__, __LINE__, __func__); perror("IO Error"); exit(EXIT_FAILURE); }

struct BitMapStructure;
typedef struct BitMapStructure* BitMap;

struct BitMapStructure {
    BitMapPart* parts;
    int bit_length;
    int part_length;

    char* name;
    int persisted;
    int fd;
};

/*
 * Bitmap initalization, resizing, persistance, and freeing.
 */
BitMap bm_initialize(int bit_length, char* name, int persist);
BitMap bm_resize(BitMap self, int new_bit_length);
void bm_free(BitMap self);

/*
 * Copying and logical operations on two bitmaps
 */
BitMap bm_copy(BitMap self, BitMap other);
BitMap bm_or(BitMap self, BitMap right);
BitMap bm_and(BitMap self, BitMap right);
BitMap bm_xor(BitMap self, BitMap right);
BitMap bm_not(BitMap self, BitMap right);

/*
 * Printing of bit map.
 */
BitMap bm_print(BitMap self);


#ifdef	__cplusplus
}
#endif

#endif	/* _BITMAP_INDEX_H */

