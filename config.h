#ifndef __CONFIG_H__
#define __CONFIG_H__

#define SEARCH_DEPTH	CFG_DEPTH

#ifdef CFG_DEPTH_EXTENSION
 #define EXTRA_CHECK	1
 #define EXTRA_CAPTURE	5
 #define EXTRA_PROMOTION	99
#else
 #define EXTRA_CHECK	0
 #define EXTRA_CAPTURE	0
 #define EXTRA_PROMOTION	0
#endif

#ifdef CFG_SHUFFLE
 const int flag_shuffle = 1;
#else
 const int flag_shuffle = 0;
#endif

#ifdef CFG_RANDOMIZE
 const int flag_randomize = 1;
#else
 const int flag_randomize = 0;
#endif

#ifdef CFG_ALPHABETA
 const int alpha_beta = 1;
#else
 const int alpha_beta = 0;
#endif

#endif
