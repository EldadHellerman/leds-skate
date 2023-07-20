#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "stdbool.h"
#include "string.h"
#include "pin_mux_register.h"
#include "mem.h"

#include "server_main.h"
#include "server_leds.h"
#include "command_parser.h"
#include "ws2812_i2s.h"

os_timer_t timer_battery;
os_timer_t timer_save_battery;
static uint8_t leds[20*3] = {0};

//void ICACHE_FLASH_ATTR user_rf_pre_init(void){}
//os_timer_t uart_adc_timer;

/*void ICACHE_FLASH_ATTR user_test_ip(void){
	os_printf("checking if i have ip\r\n");
//	os_timer_disarm(&ip_timer);
	struct ip_info ipconfig;
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if(wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0){
    	os_printf("----------got IP. ("IPSTR")\r\n", IP2STR(&ipconfig.ip.addr));
    	user_main_server_init(80);
    	user_leds_server_init(12345);
    }else{
        if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD) os_printf("----------connect fail! (wrong password) \r\n");
        if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND) os_printf("----------connect fail! (AP not found) \r\n");
        if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL) os_printf("----------connect fail! (connect fail) \r\n");
//        os_timer_setfn(&ip_timer, (os_timer_func_t *)user_test_ip, NULL);
//        os_timer_arm(&ip_timer, 3000, 0);
    }
}*/

bool battery_low = false;
bool shutdown = false;

void ICACHE_FLASH_ATTR check_battery(void){
	if(shutdown){
		stop_executing_queue();
		for(int i=0; i<20*3; i++) leds[i] = 0;
		ws2812_push(leds, 20*3);
		wifi_station_set_reconnect_policy(false);
		wifi_station_disconnect();
		wifi_set_opmode(NULL_MODE);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
		system_deep_sleep(0);
		return;
	}
	uint16 voltage = system_get_vdd33();
	os_printf("battery voltag is: %d\r\n",voltage);
	bool voltage_low = voltage < 3200;
	if(voltage_low && !battery_low){
		battery_low = true;
		os_timer_disarm(&timer_battery);
		os_timer_setfn(&timer_battery, (os_timer_func_t *)check_battery, NULL);
		os_timer_arm(&timer_battery, 2000, 1);
	}else if(voltage_low){
		os_timer_disarm(&timer_battery);
		user_main_server_close();
		user_leds_server_close();
		stop_executing_queue();
		for(int i=0; i<20*3; i++) leds[i] = 0;
		leds[1] = 0xff; leds[4] = 0xff; leds[18*3 + 1] = 0xff; leds[19*3 + 1] = 0xff;
		ws2812_push(leds, 20*3);
		shutdown = true;
		os_timer_setfn(&timer_battery, (os_timer_func_t *)check_battery, NULL);
		os_timer_arm(&timer_battery, 3*60*1000, 0);
		return;
	}else{
		battery_low = false;
	}
}

bool wifi_on = false;

void ICACHE_FLASH_ATTR save_battery(void){
	os_timer_disarm(&timer_save_battery);
	os_timer_setfn(&timer_save_battery, (os_timer_func_t *)save_battery, NULL);
	if(wifi_on){
		wifi_station_set_reconnect_policy(false);
		wifi_station_disconnect();
		wifi_set_opmode(NULL_MODE);
		//wifi_fpm_open();
		//wifi_fpm_do_sleep(55000);
		wifi_on = false;
	}else{
		//wifi_fpm_close();
		wifi_set_opmode(STATION_MODE);
		wifi_station_set_reconnect_policy(true);
		wifi_station_connect();
		wifi_on = true;
	}
	os_timer_arm(&timer_save_battery, wifi_on ? 5000 : 55000, 0);
	os_printf("wifi is ");
	if(wifi_on) os_printf("on\n"); else os_printf("off\n");
}

void ICACHE_FLASH_ATTR user_timer_save_battery_init(){
	wifi_on = false;
	os_timer_disarm(&timer_save_battery);
	os_timer_setfn(&timer_save_battery, (os_timer_func_t *)save_battery, NULL);
	os_timer_arm(&timer_save_battery, 1000, 1);
	wifi_fpm_set_sleep_type(MODEM_SLEEP_T);
}

void ICACHE_FLASH_ATTR user_timer_battery_init(){
	GPIO_DIS_OUTPUT(6);
	shutdown = false;
	battery_low = false;
	os_timer_disarm(&timer_battery);
	os_timer_setfn(&timer_battery, (os_timer_func_t *)check_battery, NULL);
	os_timer_arm(&timer_battery, 15000, 1);
	wifi_fpm_auto_sleep_set_in_null_mode(1);
	wifi_fpm_open();
}

void ICACHE_FLASH_ATTR user_wifi_event_cb(System_Event_t *e){
	if(e->event == EVENT_STAMODE_CONNECTED){
		os_timer_disarm(&timer_save_battery);
	}else if(e->event == EVENT_STAMODE_DISCONNECTED){
		wifi_on = true;
		os_timer_disarm(&timer_save_battery);
		os_timer_arm(&timer_save_battery,1000,0);
	}
}

void ICACHE_FLASH_ATTR user_set_station_config(void){
	char ssid[32] = "SKATELEDS";
	//char password[64] = "iceland2018LEDS";
	char password[64] = "iceeat2019LEDS";
	struct station_config stationConf;
	os_memset(stationConf.ssid, 0, 32); os_memset(stationConf.password, 0, 64);
	os_memcpy(&stationConf.ssid, ssid, 32); os_memcpy(&stationConf.password, password, 64);
	stationConf.bssid_set = 0; //doesnt check AP MAC
	/*char VMac[6] = {0x00,0x0e,0x2e,0xf8,0x4e,0xf4};
	char CHSMac[6] = {0x0e,0x8b,0xfd,0x17,0x3c,0xdf};
	stationConf.bssid_set = 1;
	os_memcpy(&stationConf.bssid, VMac, 6);
	//os_printf("will connect to: %-20s, "MACSTR"\r\n",ssid, MAC2STR(VMac));*/
	wifi_station_set_config(&stationConf);
	os_printf("will connect to: %-20s\r\n",ssid);
	wifi_set_event_handler_cb(user_wifi_event_cb);
	user_main_server_init(80);
	user_leds_server_init(12345);
}

void start_default_script(void){
	char *script = "delay(10000); script(3);";
	execute_commands_from_string(script,strlen(script));
}

//#include "upgrade.h"
void ICACHE_FLASH_ATTR user_pre_init(void){
}

void ICACHE_FLASH_ATTR user_init(void){
	setMaxCommandNameLength(20);
	uart_div_modify(0,UART_CLK_FREQ/115200); //uart_init(uart0_br, uart1_br); //why am i not using that?
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_I2SO_DATA);
	initalize_command_executor();
	os_delay_us(1000);
	ws2812_init();
	user_timer_battery_init();
	user_timer_save_battery_init();
	os_printf("\r\nstarted!\r\n");
	os_printf("setting wifi:");
	wifi_set_opmode(STATION_MODE);
	//wifi_station_disconnect();
	wifi_station_dhcpc_set_maxtry(4);
	wifi_station_set_hostname("esp8266!");
	wifi_station_set_reconnect_policy(true);
	os_printf("... set\r\n");
	user_set_station_config();
	start_default_script();
	//struct upgrade_server_info t;
}
