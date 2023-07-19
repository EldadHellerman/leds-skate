#ifndef SERVER_MAIN_H_
#define SERVER_MAIN_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "stdbool.h"
#include "mem.h"
#include "command_executer.h"
//extern volatile bool close_server;

void ICACHE_FLASH_ATTR tcp_main_server_recv_cb(void *arg, unsigned char *pusrdata, unsigned short length);
void ICACHE_FLASH_ATTR tcp_main_server_sent_cb(void *arg);
void ICACHE_FLASH_ATTR tcp_main_server_discon_cb(void *arg);
void ICACHE_FLASH_ATTR tcp_main_server_recon_cb(void *arg, sint8 err);
void ICACHE_FLASH_ATTR tcp_main_server_connected(void *arg);

void ICACHE_FLASH_ATTR tcp_server_main_page(void *arg);
void ICACHE_FLASH_ATTR tcp_server_404_page(void *arg);
void user_main_server_disconnect(void);
void user_main_server_close(void);

void ICACHE_FLASH_ATTR user_main_server_init(uint32 port);

#endif // SERVER_MAIN_H_
