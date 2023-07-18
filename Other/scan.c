#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"
#include "stdbool.h"

static os_timer_t scan_timer;

void ICACHE_FLASH_ATTR scan_wifi(void);

static void ICACHE_FLASH_ATTR scan_done(void *arg, STATUS status){
	if(status != OK){
		os_printf("Scan failed!\r\n");
		return;
	}
	uint8 ssid[33];
	struct bss_info *bss_link = (struct bss_info*)arg;
	while(bss_link != NULL){
		os_memset(ssid, 0, 33);
		os_memcpy(ssid, bss_link->ssid,((os_strlen((char *)bss_link->ssid) < 32) ? os_strlen((char *)bss_link->ssid) : 32));
		os_printf("%d,%2d,%d, %-20s , "MACSTR"\r\n", bss_link->authmode,bss_link->channel,bss_link->rssi, ssid, MAC2STR(bss_link->bssid));
		bss_link = bss_link->next.stqe_next;
	}
	os_timer_disarm(&scan_timer);
	os_timer_setfn(&scan_timer,(os_timer_func_t *)scan_wifi,NULL);
	os_timer_arm(&scan_timer,5000,0);
}

void ICACHE_FLASH_ATTR scan_wifi(void){
	wifi_station_scan(NULL,scan_done);
}

static void ICACHE_FLASH_ATTR user_main(void){
	wifi_station_disconnect();
	scan_wifi();
}

void user_init(void){
	uart_div_modify(0,UART_CLK_FREQ / 115200);
	os_delay_us(1000);
	os_timer_disarm(&scan_timer);
	wifi_set_opmode(STATIONAP_MODE);
	system_init_done_cb(user_main);
}
