#ifndef __BITS_H
#define __BITS_H

static inline int fls_generic(u64 x) {
	return __builtin_ffsll(x);
}

#define fls fls_generic

#define mask_for_each(mask, temp, i)					\
	for ((temp) = (mask);						\
	     (temp) != 0 && ((i) = fls(temp) - 1, 1);			\
	     (temp) = (temp) & ~((u64)1 << (i)))

static inline u64 posbit(int r, int c) {
#ifdef FLIPBIT
	return ((u64)1 << (63 - r * 8 - c));
#else
	return ((u64)1 << (r * 8 + c));
#endif
}

static inline int bitrow(int i) {
#ifdef FLIPBIT
	return (63 - i) >> 3;
#else
	return i >> 3;
#endif
}

static inline int bitcol(int i) {
#ifdef FLIPBIT
	return (63 - i) & 7;
#else
	return i & 7;
#endif
}

#endif
