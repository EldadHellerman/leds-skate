#include "osapi.h"
#include "ets_sys.h"
#include "gpio.h"
#include "user_interface.h"
#include "stdbool.h"
#include "espconn.h"
#include <mem.h>
//#include "uart.h"
//#include "espconn.h"
//#include "eagle_soc.h"
//#include "uart.h"

#define PORT 80

uint32 ICACHE_FLASH_ATTR user_rf_cal_sector_set(void){
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch(size_map){
        case FLASH_SIZE_4M_MAP_256_256: rf_cal_sec = 128 - 5; break;
        case FLASH_SIZE_8M_MAP_512_512: rf_cal_sec = 256 - 5; break; //this is the one used.
        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024: rf_cal_sec = 512 - 5; break;
        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024: rf_cal_sec = 1024 - 5; break;
        case FLASH_SIZE_64M_MAP_1024_1024: rf_cal_sec = 2048 - 5; break;
        case FLASH_SIZE_128M_MAP_1024_1024: rf_cal_sec = 4096 - 5; break;
        default: rf_cal_sec = 0; break;
    }
    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR user_rf_pre_init(void){}

os_timer_t timer;
bool b = true;


void my_timer_func(void *arg){
	os_printf("new printing from timer!\n");
	gpio_output_set(b ? 0 : 0x04,b ? 0x04 : 0,0,0);
	b = !b;
}

void sdk_init_done_cb(void){
	//gpio_output_set(set_mask, clear_mask, enable_mask, disable_mask)
	os_printf("done!\n");
	os_printf("SDK version:%s\n", system_get_sdk_version());
	os_printf("setting timer!\n");
	os_timer_disarm(&timer);
	os_timer_setfn(&timer, my_timer_func, NULL);
	os_timer_arm(&timer,5000,1);
	os_printf("timer set!\n");
	//char *apn = "trial";
	//bool suc = wifi_station_set_hostname(apn);
	//os_printf("set! result %d\n",b);

	os_printf("setting wifi \n");
	wifi_set_opmode(SOFTAP_MODE); os_printf(".");
	wifi_set_phy_mode(PHY_MODE_11B); os_printf(".");
	struct softap_config config = {":) :) :) e-ma! :) :) :)","87654321",0,1,AUTH_WPA_WPA2_PSK,0,4,100};  os_printf(".");
	//wifi_softap_get_config(&config); os_printf(".");
	bool r = wifi_softap_set_config(&config); os_printf(".");
	os_printf("set! result %d\n",r);
	/*
	wifi_set_phy_mode(PHY_MODE_11B); os_printf(".");
	wifi_set_opmode(SOFTAP_MODE); os_printf(".");
	struct softap_config config = (struct softap_config*)os_malloc(sizeof(config)); os_printf(".");// = {"normal ssid","87654321",12,1,AUTH_WPA_WPA2_PSK,0,4,100};
	wifi_softap_get_config(&config); os_printf(".");
	os_memcpy(config.ssid,"ofrikosh",9); os_printf(".");
	os_memcpy(config.password,"87654321",8); os_printf(".");
	config.ssid_len = 0; os_printf(".");
	bool r = wifi_softap_set_config(&config); os_printf(".");
	os_printf("set! result %d\n",r);
	*/
	/*
	char *ssid = "!^^%^*&^%$(())!!:0";
	uint8 ssidlen = os_strlen(ssid);
	uint8 ch = 6;
	char *pass = "87654321";
	AUTH_MODE am = AUTH_WPA2_PSK;
	os_printf("length: %d\n",ssidlen);
	os_printf("copying memory ");
	os_memcpy(config->ssid,ssid,ssidlen); os_printf(".");
	config->ssid_len = ssidlen; os_printf(".");
	config->channel = ch; os_printf(".");
	os_memcpy(config->password,pass,os_strlen(pass)); os_printf(".");
	config->authmode = am; os_printf(". done!\n");
	os_printf("about to set configuration:\n");
	wifi_set_opmode(2); os_printf(".");
	bool r = wifi_softap_set_config_current(config); os_printf(".");
	os_printf("set! result %d\n",r);
	*/
}

void wifi_event(System_Event_t *evt);

//LOCAL void ICACHE_FLASH_ATTR tcp_server_sent_cb(void *arg);
//LOCAL struct espconn connection;
//LOCAL esp_tcp esptcp;

void user_init(void){
	/*
	 * wifi_set_opmode(STATIONAP_MODE);
char sofap_mac[6] = {0x16, 0x34, 0x56, 0x78, 0x90, 0xab};
char sta_mac[6] = {0x12, 0x34, 0x56, 0x78, 0x90, 0xab};
wifi_set_macaddr(SOFTAP_IF, sofap_mac);
wifi_set_macaddr(STATION_IF, sta_mac);
	 */
	uint8 mac[6] = {0,0xe,1,0xd,0xa,0xd};
	wifi_set_macaddr(SOFTAP_IF,mac);
	gpio_init();
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U,FUNC_GPIO1);
	gpio_output_set(0,0,0x04,0);
	uart_div_modify(0,UART_CLK_FREQ / 115200);
	os_delay_us(1000);
	os_printf("\r\n\r\n""initiating...");
	//wifi_set_event_handler_cb(wifi_event);
	system_init_done_cb(sdk_init_done_cb);
}

//void user_rf_pre_init (void)
//uint32 user_rf_cal_sector_set(void)

void wifi_event(System_Event_t *evt){
	os_printf("event %x\n", evt->event);
	switch(evt->event){
		case EVENT_STAMODE_CONNECTED:
			os_printf("connect to ssid %s, channel %d\n", evt->event_info.connected.ssid, evt->event_info.connected.channel);
			break;
		case EVENT_STAMODE_DISCONNECTED:
			os_printf("disconnect from ssid %s, reason %d\n", evt->event_info.disconnected.ssid, evt->event_info.disconnected.reason);
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			os_printf("mode: %d -> %d\n", evt->event_info.auth_change.old_mode, evt->event_info.auth_change.new_mode);
			break;
		case EVENT_STAMODE_GOT_IP:
			os_printf("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR, IP2STR(&evt->event_info.got_ip.ip), IP2STR(&evt->event_info.got_ip.mask), IP2STR(&evt->event_info.got_ip.gw));
			os_printf("\n");
			break;
		case EVENT_SOFTAPMODE_STACONNECTED:
			os_printf("station: " MACSTR "join, AID = %d\n", MAC2STR(evt->event_info.sta_connected.mac), evt->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			os_printf("station: " MACSTR "leave, AID = %d\n", MAC2STR(evt->event_info.sta_disconnected.mac), evt->event_info.sta_disconnected.aid);
			break;
		default: break;
	}
}

/*
LOCAL void ICACHE_FLASH_ATTR tcp_server_sent_cb(void *arg){
    os_printf("tcp sent cb \r\n");
}

LOCAL void ICACHE_FLASH_ATTR tcp_server_recv_cb(void *arg, char *pusrdata, unsigned short length){
   struct espconn *pespconn = arg;
   os_printf("tcp recv : %s \r\n", pusrdata);
   espconn_sent(pespconn, pusrdata, length);
}

LOCAL void ICACHE_FLASH_ATTR tcp_server_discon_cb(void *arg){
    os_printf("tcp disconnected successfully!!! \r\n");
}

LOCAL void ICACHE_FLASH_ATTR tcp_server_recon_cb(void *arg, sint8 err){
    os_printf("reconnect callback, error code %d !!! \r\n",err);
}


LOCAL void tcp_server_multi_send(void){
   struct espconn *pesp_conn = &esp_conn;

   remot_info *premot = NULL;
   uint8 count = 0;
   sint8 value = ESPCONN_OK;
   if (espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK){
      char *pbuf = "tcp_server_multi_send\n";
      for (count = 0; count < pesp_conn->link_cnt; count ++){
         pesp_conn->proto.tcp->remote_port = premot[count].remote_port;

         pesp_conn->proto.tcp->remote_ip[0] = premot[count].remote_ip[0];
         pesp_conn->proto.tcp->remote_ip[1] = premot[count].remote_ip[1];
         pesp_conn->proto.tcp->remote_ip[2] = premot[count].remote_ip[2];
         pesp_conn->proto.tcp->remote_ip[3] = premot[count].remote_ip[3];

         espconn_sent(pesp_conn, pbuf, os_strlen(pbuf));
      }
   }
}


LOCAL void ICACHE_FLASH_ATTR tcp_server_listen(void *arg){
    struct espconn *pesp_conn = arg;
    os_printf("tcp_server_listen !!! \r\n");

    espconn_regist_recvcb(pesp_conn, tcp_server_recv_cb);
    espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
    espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);

    espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
   tcp_server_multi_send();
}

void ICACHE_FLASH_ATTR user_tcpserver_init(uint32 port){
    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, tcp_server_listen);

    sint8 ret = espconn_accept(&esp_conn);

    os_printf("espconn_accept [%d] !!! \r\n", ret);

}
*/
