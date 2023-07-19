char *main_server_error_response = 
	"HTTP/1.1 404 GO TO HELL\r\n"
	"Content-Length: 25\r\n"
	"Content-Type: text/html\r\n"
	"Connection: Closed\r\n"
	"\r\n"
	"<html>nothing here</html>";

char *main_server_main_response =
"HTTP/1.1 200 OK\r\n"
"Content-Length: 563\r\n"
"Content-Type: text/html\r\n"
"Connection: Closed\r\n"
"\r\n"
"<html><head>\
	<title>ledSkate</title>\
	<h1 style=\"color:blue; text-align:center;\">ledSkate<h1>\
	<script>\
	function run(){\
		var x = new XMLHttpRequest();\
		x.open(\"POST\", \"/leds\", false);\
		x.send(document.getElementById(\"text\").value);\
	}\
	</script>\
</head><body>\
	<div style=\"text-align:center;\">\
	<textarea id=\"text\" style=\"resize:none; width:75%;\" rows=\"20\" placeholder=\"script here! For example:\
set([{255,140,20},#ff],50);\
delay(5000);\
setAll(#ff00FF);\"></textarea><br><br><br>\
	<button onclick=\"run();\">Parse!</button></div>\
</body></html>";
