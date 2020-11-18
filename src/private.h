#ifndef _PRIVATE_H_
#define _PRIVATE_H_

#define EXPORT __attribute__ ((visibility("default")))
#define UNUSED(x) ((void)x)

#define CHK_ZERO(x) (((x) < 0) ? (0) : (x))

#define min_t(type, a, b) __extension__ ({ \
	type _a = (a); \
	type _b = (b); \
	_a < _b ? _a : _b; })

#define max_t(type, a, b) __extension__ ({ \
	type _a = (a); \
	type _b = (b); \
	_a > _b ? _a : _b; })

#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*x))

#define DIV_ROUND_UP(x, d) (((x) + (d) - 1) / (d))

#endif /* _PRIVATE_H_ */