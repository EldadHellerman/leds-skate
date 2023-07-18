#include "server_leds.h"

#define CLOSE_SERVER_TASK_PRIORITY 1
#define CLOSE_SERVER_TASK_SIG_ACTION 'a'
#define CLOSE_SERVER_TASK_PARAMETER_CLOSE 'c'

os_event_t *close_server_task;

struct espconn leds_esp_conn;
esp_tcp leds_tcp;

static void ICACHE_FLASH_ATTR close_tcp_leds_server(os_event_t *e){
	if(e->sig == CLOSE_SERVER_TASK_SIG_ACTION && e->par == CLOSE_SERVER_TASK_PARAMETER_CLOSE){
		espconn_disconnect(&leds_esp_conn);
	}
}

void ICACHE_FLASH_ATTR tcp_leds_server_recv_cb(void *arg, unsigned char *data, unsigned short length){
	struct espconn *conn = (struct espconn *)arg;
	espconn_recv_hold(conn);
//	os_printf("leds rx %d;\r\n", (int)length);
	execute_commands_from_string((char *)data, (int)length);
	espconn_recv_unhold(conn);
	//user_leds_server_disconnect();
}

void ICACHE_FLASH_ATTR tcp_leds_server_sent_cb(void *arg){ os_printf("leds tcp sent;\r\n"); /*close_server = true;*/}
void ICACHE_FLASH_ATTR tcp_leds_server_discon_cb(void *arg){ /*os_printf("leds discon;\r\n");*/}
void ICACHE_FLASH_ATTR tcp_leds_server_recon_cb(void *arg, sint8 err){ os_printf("leds recon cb, err %d;\r\n",err);}

void ICACHE_FLASH_ATTR tcp_leds_server_connected(void *arg){
	//struct espconn *esp_conn = arg;
	/*os_printf("leds server got connected!\r\n");*/
}

void ICACHE_FLASH_ATTR user_leds_server_disconnect(){
	system_os_post(CLOSE_SERVER_TASK_PRIORITY, CLOSE_SERVER_TASK_SIG_ACTION, CLOSE_SERVER_TASK_PARAMETER_CLOSE);
}

void ICACHE_FLASH_ATTR user_leds_server_close(){
	stop_executing_queue();
	espconn_delete(&leds_esp_conn);
}

void ICACHE_FLASH_ATTR user_leds_server_init(uint32 port){
	close_server_task = (os_event_t *)os_malloc(sizeof(os_event_t));
	system_os_task(close_tcp_leds_server, CLOSE_SERVER_TASK_PRIORITY, close_server_task, 1); //close connection

	leds_esp_conn.type = ESPCONN_TCP; leds_esp_conn.state = ESPCONN_NONE;
	leds_tcp.local_port = port;
	leds_esp_conn.proto.tcp = &leds_tcp;
	sint8 r = espconn_accept(&leds_esp_conn);
	if(r != 0) os_printf("failed to create leds server! error code: %d\r\n", r);
	else{
		espconn_regist_recvcb(&leds_esp_conn, (espconn_recv_callback)tcp_leds_server_recv_cb);
//		espconn_regist_connectcb(&leds_esp_conn, tcp_leds_server_connected);
//		espconn_regist_disconcb(&leds_esp_conn, (espconn_connect_callback)tcp_leds_server_discon_cb);
//		espconn_regist_reconcb(&leds_esp_conn, (espconn_reconnect_callback)tcp_leds_server_recon_cb);
//		espconn_regist_sentcb(&leds_esp_conn, (espconn_sent_callback)tcp_leds_server_sent_cb);
	}
}
