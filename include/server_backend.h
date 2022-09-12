#ifndef JG_NETCALC_SRC_SERVER_SERVER_BACKEND_H_
#define JG_NETCALC_SRC_SERVER_SERVER_BACKEND_H_
#ifdef __cplusplus
extern "C" {
#endif //END __cplusplus
#include <utils.h>
#include <stdint.h>
void start_server(uint8_t thread_count, uint32_t port_num);

#ifdef __cplusplus
}
#endif // END __cplusplus
#endif //JG_NETCALC_SRC_SERVER_SERVER_BACKEND_H_
