#include "ets_sys.h"
#include "osapi.h"
#include "user_interface.h"
#include "espconn.h"

#define SERVER_LOCAL_PORT 80

static struct espconn esp_conn;
static esp_tcp esptcp;
static os_timer_t test_timer;

static void ICACHE_FLASH_ATTR tcp_server_recv_cb(void *arg, unsigned char *pusrdata, unsigned short length){
	struct espconn *pespconn = arg;
	os_printf("tcp recv : %s \r\n", pusrdata);
	espconn_sent(pespconn, pusrdata, length);
}
static void ICACHE_FLASH_ATTR tcp_server_sent_cb(void *arg){ os_printf("tcp sent cb \r\n");}
static void ICACHE_FLASH_ATTR tcp_server_discon_cb(void *arg){ os_printf("tcp disconnect succeed !!! \r\n");}
static void ICACHE_FLASH_ATTR tcp_server_recon_cb(void *arg, sint8 err){ os_printf("reconnect callback, error code %d !!! \r\n",err);}

static void tcp_server_multi_send(void){
	struct espconn *pesp_conn = &esp_conn;
	remot_info *premot = NULL;
	//sint8 value = ESPCONN_OK;
	if(espconn_get_connection_info(pesp_conn,&premot,0) == ESPCONN_OK){
		char *pbuf = "tcp_server_multi_send\n";
		for(uint8 i=0; i < pesp_conn->link_cnt; i++){
			pesp_conn->proto.tcp->remote_port = premot[i].remote_port;
			pesp_conn->proto.tcp->remote_ip[0] = premot[i].remote_ip[0];
			pesp_conn->proto.tcp->remote_ip[1] = premot[i].remote_ip[1];
			pesp_conn->proto.tcp->remote_ip[2] = premot[i].remote_ip[2];
			pesp_conn->proto.tcp->remote_ip[3] = premot[i].remote_ip[3];
			espconn_sent(pesp_conn, (unsigned char*)pbuf, os_strlen(pbuf));
		}
	}
}

static void ICACHE_FLASH_ATTR tcp_server_listen(void *arg){
	struct espconn *pesp_conn = arg;
	os_printf("tcp_server_listen !!! \r\n");
	espconn_regist_recvcb(pesp_conn, (espconn_recv_callback)tcp_server_recv_cb);
	espconn_regist_reconcb(pesp_conn, tcp_server_recon_cb);
	espconn_regist_disconcb(pesp_conn, tcp_server_discon_cb);
	espconn_regist_sentcb(pesp_conn, tcp_server_sent_cb);
	tcp_server_multi_send();
}

/******************************************************************************
 * FunctionName : user_tcpserver_init
 * Description  : parameter initialize as a TCP server
 * Parameters   : port -- server port
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_tcpserver_init(uint32 port){
	esp_conn.type = ESPCONN_TCP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.tcp = &esptcp;
	esp_conn.proto.tcp->local_port = port;
	espconn_regist_connectcb(&esp_conn, tcp_server_listen);
	sint8 ret = espconn_accept(&esp_conn);
	os_printf("espconn_accept [%d] !!! \r\n", ret);
}

/******************************************************************************
 * FunctionName : user_esp_platform_check_ip
 * Description  : check whether get ip addr or not
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR user_esp_platform_check_ip(void){
	struct ip_info ipconfig;
	os_timer_disarm(&test_timer); //disarm timer first
	wifi_get_ip_info(STATION_IF, &ipconfig); //get ip info of ESP8266 station
	if(wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0){
		os_printf("got ip !!! \r\n");
		user_tcpserver_init(SERVER_LOCAL_PORT);
	}else{
		os_printf("#a\n");
		if ((wifi_station_get_connect_status() == STATION_WRONG_PASSWORD ||
				wifi_station_get_connect_status() == STATION_NO_AP_FOUND ||
				wifi_station_get_connect_status() == STATION_CONNECT_FAIL)){
			os_printf("connect fail !!! \r\n");
			os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);//re-arm timer to check ip
						os_timer_arm(&test_timer, 100, 0);
		}else{
			os_printf("#d\n");
			os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);//re-arm timer to check ip
			os_timer_arm(&test_timer, 100, 0);
		}
	}
}

void ICACHE_FLASH_ATTR user_set_station_config(void){
	// Wifi configuration
	char ssid[32] = "221b";
	char password[64] = "baker street";
	struct station_config stationConf;

	//need not mac address
	stationConf.bssid_set = 0;

	//Set ap settings
	os_memcpy(&stationConf.ssid, ssid, 32);
	os_memcpy(&stationConf.password, password, 64);
	wifi_station_set_config(&stationConf);

	//set a timer to check whether got ip from router succeed or not.
	os_timer_disarm(&test_timer);
	os_timer_setfn(&test_timer, (os_timer_func_t *)user_esp_platform_check_ip, NULL);
	os_timer_arm(&test_timer, 100, 0);
}

void user_init(void){
	uart_div_modify(0,UART_CLK_FREQ / 115200);
	os_delay_us(1000);
	wifi_set_opmode(STATIONAP_MODE);
	user_set_station_config();
}
