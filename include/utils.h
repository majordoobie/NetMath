#ifndef JG_NETCALC_SRC_UTILS_UTILS_H_
#define JG_NETCALC_SRC_UTILS_UTILS_H_

typedef enum util_verify_t
{
    UV_VALID_ALLOC,
    UV_INVALID_ALLOC
} util_verify_t;

util_verify_t verify_alloc(void * ptr);

#endif //JG_NETCALC_SRC_UTILS_UTILS_H_
