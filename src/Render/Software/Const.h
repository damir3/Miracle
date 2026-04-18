#ifndef _CONST_H
#define _CONST_H

extern double			chop_temp;
#define BIG_NUM			((float) (1 << 26) * (1 << 26) * 1.5)
#define float_to_int(x)	((chop_temp = (x) + BIG_NUM), *(int*)(&chop_temp))
#define float_to_fix(x)	((chop_temp = (x) + BIG_NUM/65536.0), *(int*)(&chop_temp))
						// or int((x)*65536.0)
#define fix_to_int(x)	(((x)+65535) >> 16)
#define fix_int(x)		((x) >> 16)
#define fix_make(a,b)	(((a) << 16) + (b))

#endif