/* 
 * File:   bit_counter.h
 * Author: swistak
 *
 * Created on 1 styczeń 2011, 18:32
 */

#ifndef _BIT_COUNTER_H
#define	_BIT_COUNTER_H

#ifdef	__cplusplus
extern "C" {
#endif
    // Table that holds number of bits set for given key (Int).
    // Alternative ways to bitcount see
    //   http://graphics.stanford.edu/~seander/bithacks.html
    //   http://aggregate.org/MAGIC/#Population%20Count%20%28Ones%20Count%29
    unsigned char* table_bits_in;
    typedef unsigned long long int ull;

    void prepare_bit_table(int bit_part_size) {
        table_bits_in = malloc(bit_part_size * sizeof (unsigned char));
        table_bits_in[0] = 0;
        for (int i = 0; i < bit_part_size; i++) {
            table_bits_in[i] = (i & 1) + table_bits_in[i / 2];
        }
    }

    int bit(ull source, int x) {
        ull mask = (1 << x);
        return ((source & mask) > 0 ? 1 : 0);
    }

    inline int bits_in(ull part) {
        return table_bits_in[part];
    }

#ifdef	__cplusplus
}
#endif

#endif	/* _BIT_COUNTER_H */

