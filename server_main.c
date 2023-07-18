#include "server_main.h"
#include "webpages_strings.h"

#define CLOSE_SERVER_TASK_PRIORITY 0
#define CLOSE_SERVER_TASK_SIG_ACTION 'a'
#define CLOSE_SERVER_TASK_PARAMETER_CLOSE 'c'

/*struct postRequest{
	union{
		uint8_t ip[4];
		struct{ uint8_t ip0, ip1, ip2, ip3;};
	};
	int port; //espconn's tcp uses int, dont want conversion issues.
};*/

os_event_t *close_server_task;
//struct postRequest recentPost;
struct espconn main_esp_conn;
esp_tcp main_tcp;

//XXX crashes when post data comes in a different tcp connection

static void ICACHE_FLASH_ATTR close_tcp_main_server(os_event_t *e){
	if(e->sig == CLOSE_SERVER_TASK_SIG_ACTION && e->par == CLOSE_SERVER_TASK_PARAMETER_CLOSE){
		espconn_disconnect(&main_esp_conn);
	}
}

static bool ICACHE_FLASH_ATTR string_starts_with(char *string, char* start){
	return(os_strncmp(string,start,os_strlen(start)) == 0);
}

void ICACHE_FLASH_ATTR tcp_main_server_recv_cb(void *arg, unsigned char *rawdata, unsigned short length){
	struct espconn *conn = (struct espconn *)arg;
	os_printf("tcp recv data of length %d, data:\r\n\"%s\"\r\n", (int)length,rawdata);
	bool disconnect = true;
	int index = 0;
	char *data = (char *)rawdata;
	/*if(recentPost.port != 0 && recentPost.port == main_tcp.remote_port && os_memcmp(recentPost.ip, main_tcp.remote_ip, 4) == 0){ //waiting for post data
		os_printf("data of length %d from post:\r\n\"%s\"\r\n",(int)length,pusrdata);
		recentPost.port = 0;
		espconn_recv_hold(conn);
		execute_command_from_string((char *)data, (int)length);
		espconn_recv_unhold(conn);
	}else*/
	if(string_starts_with(data, "GET ")){
		index += 4;
		data += 4;
		if(string_starts_with(data, "/ ")){
			tcp_server_main_page(arg);
		}else if(string_starts_with(data, "/bat ")){
			tcp_server_main_page(arg);
		}else{
			os_printf("get request to an non-existing path\r\n");
			tcp_server_404_page(arg);
		}
	}else if(string_starts_with(data, "POST ")){
		index += 5;
		data += 5;
		if(string_starts_with(data, "/leds ")){
			char *term = "\r\n\r\n";
			while(os_memcmp(term,data,4) != 0 && index < length){
				index++;
				data++;
			}
			index += 4;
			data += 4;
			//os_memcpy(recentPost.ip, main_tcp.remote_ip, 4);
			//recentPost.port = main_tcp.remote_port;
			//disconnect = false;
			os_printf("data of length %d from post:\r\n\"%s\"\r\n",(int)length-index,data);
			//recentPost.port = 0;
			espconn_recv_hold(conn);
			execute_commands_from_string((char *)data, (int)length-index);
			espconn_recv_unhold(conn);
		}else{
			os_printf("post request to an non-existing path\r\n");
			tcp_server_404_page(arg);
		}
	}else{
		os_printf("invalid request (not a GET nor a POST request)\r\n");
		tcp_server_404_page(arg);
	}
	if(disconnect) user_main_server_disconnect();
}

void ICACHE_FLASH_ATTR tcp_main_server_sent_cb(void *arg){ os_printf("tcp sent;\r\n");}
void ICACHE_FLASH_ATTR tcp_main_server_discon_cb(void *arg){ os_printf("tcp discon;\r\n");}
void ICACHE_FLASH_ATTR tcp_main_server_recon_cb(void *arg, sint8 err){ os_printf("recon cb, err %d;\r\n",err);}

void ICACHE_FLASH_ATTR tcp_main_server_connected(void *arg){
	//struct espconn *esp_conn = arg;
	os_printf("main server got connected!\r\n");
}

void ICACHE_FLASH_ATTR tcp_server_404_page(void *arg){
	struct espconn *pespconn = (struct espconn *)arg;
	os_printf("sending 404\r\n");
	sint8 r = espconn_send(pespconn, (uint8 *)main_server_error_response, os_strlen(main_server_error_response));
	if(r != 0) os_printf("sending response failed! error code: %d",r);
}

void ICACHE_FLASH_ATTR tcp_server_main_page(void *arg){
	struct espconn *pespconn = (struct espconn *)arg;
	os_printf("sending main...\r\n");
	sint8 r = espconn_send(pespconn, (uint8 *)main_server_main_response, os_strlen(main_server_main_response));
	if(r != 0) os_printf("sending response failed! error code: %d",r);
}

void ICACHE_FLASH_ATTR user_main_server_init(uint32 port){
	close_server_task = (os_event_t *)os_malloc(sizeof(os_event_t));
	system_os_task(close_tcp_main_server, CLOSE_SERVER_TASK_PRIORITY, close_server_task, 1); //close connection

	main_esp_conn.type = ESPCONN_TCP; main_esp_conn.state = ESPCONN_NONE;
	main_tcp.local_port = port;
	main_esp_conn.proto.tcp = &main_tcp;
	sint8 r = espconn_accept(&main_esp_conn);
	if(r != 0) os_printf("failed to create main server! error code: %d\r\n", r);
	else{
		espconn_regist_connectcb(&main_esp_conn, tcp_main_server_connected);
		espconn_regist_recvcb(&main_esp_conn, (espconn_recv_callback)tcp_main_server_recv_cb);
		espconn_regist_reconcb(&main_esp_conn, (espconn_reconnect_callback)tcp_main_server_recon_cb);
		espconn_regist_disconcb(&main_esp_conn, (espconn_connect_callback)tcp_main_server_discon_cb);
		espconn_regist_sentcb(&main_esp_conn, (espconn_sent_callback)tcp_main_server_sent_cb);
	}
}

void ICACHE_FLASH_ATTR user_main_server_disconnect(){
	system_os_post(CLOSE_SERVER_TASK_PRIORITY, CLOSE_SERVER_TASK_SIG_ACTION, CLOSE_SERVER_TASK_PARAMETER_CLOSE);
}

void ICACHE_FLASH_ATTR user_main_server_close(){
	stop_executing_queue();
	espconn_delete(&main_esp_conn);
}
