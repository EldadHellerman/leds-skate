#ifndef SERVER_LEDS_H_
#define SERVER_LEDS_H_

#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "command_parser.h"
#include "command_executer.h"

//extern volatile bool close_server;
struct espconn main_esp_conn;
esp_tcp main_tcp, leds_tcp;

void ICACHE_FLASH_ATTR tcp_leds_server_recv_cb(void *arg, unsigned char *pusrdata, unsigned short length);

void ICACHE_FLASH_ATTR tcp_leds_server_sent_cb(void *arg);
void ICACHE_FLASH_ATTR tcp_leds_server_discon_cb(void *arg);
void ICACHE_FLASH_ATTR tcp_leds_server_recon_cb(void *arg, sint8 err);
void ICACHE_FLASH_ATTR tcp_leds_server_connected(void *arg);

void ICACHE_FLASH_ATTR user_leds_server_close(void);
void ICACHE_FLASH_ATTR user_leds_server_disconnect(void);

void ICACHE_FLASH_ATTR tcp_leds_server_connected(void *arg);

void ICACHE_FLASH_ATTR user_leds_server_init(uint32 port);
#endif // SERVER_LEDS_H_
