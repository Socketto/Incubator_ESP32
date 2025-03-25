#include <WebServer.h>

WebServer server(80);

String index_html =
"<!DOCTYPE HTML><html><head>"
"<center><h1>WIFI-INCUBATOR</h1></center>"
"<title>INCUBATOR settings:</title>"
"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
"</head><body>"
"<form action=\"/get\">"
"  TELEGRAM_BOT: <input type=\"text\" name=\"TELEGRAM_BOT\">"
"  <input type=\"submit\" class=btn value=\"Set telegram BOT\">"
"</form><br>"
"<form action=\"/get\">"
"  TELEGRAM_ADMIN: <input type=\"text\" name=\"TELEGRAM_ADMIN\">"
"  <input type=\"submit\" class=btn value=\"Set TELEGRAM_ADMIN\">"
"</form><br>"
"<form action=\"/get\">"
"  WIFI_NAME: <input type=\"text\" name=\"WIFI_NAME\">"
"  <input type=\"submit\" class=btn value=\"Set wifi name\">"
"</form><br>"
"<form action=\"/get\">"
"  WIFI_PASSWORD: <input type=\"text\" name=\"WIFI_PASSWORD\">"
"  <input type=\"submit\" class=btn value=\"Set wifi password\">"
"</form><br>"
"<form action=\"/get\">"
"  URL_MQTT: <input type=\"text\" name=\"URL_MQTT\">"
"  <input type=\"submit\" class=btn value=\"Set URL of MQTT\">"
"</form><br>"
"<form action=\"/get\">"
"  USERNAME_MQTT: <input type=\"text\" name=\"USERNAME_MQTT\">"
"  <input type=\"submit\" class=btn value=\"Set username of MQTT\">"
"</form><br>"
"<form action=\"/get\">"
"  PASSWORD_MQTT: <input type=\"text\" name=\"PASSWORD_MQTT\">"
"  <input type=\"submit\" class=btn value=\"Set passowrd of MQTT\">"
"</form><br>"
"<form action=\"/reset\">"
"  <input type=\"submit\" class=btn value=\"RESTART\">"
"</form>"
"</body></html>";


void setupAP() {
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](){
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", index_html);
  });

  server.on("/reset", HTTP_GET, [] () {
      ESP.restart();
  });
  server.on("/get", HTTP_GET, [] () {
    String inputMessage;
    String inputParam;
    
    preferences.begin("my-app", false);
    if (server.hasArg(WIFI_NAME)) {
      inputMessage = server.arg(WIFI_NAME);
      inputParam = WIFI_NAME;
      preferences.putString("SSID", inputMessage);
      inputMessage.toCharArray(lcdStringTemp1,inputMessage.length()+1);
	    TFT_log("WIFI_NAME:",lcdStringTemp1,0);
    }
    else if (server.hasArg(WIFI_PASSWORD)) {
      inputMessage = server.arg(WIFI_PASSWORD);
      inputParam = WIFI_PASSWORD;
      preferences.putString("SSIDPSW", inputMessage);
	    TFT_log("WIFI_PASSWORD:","*****",0);
    }
    else if (server.hasArg(TELEGRAM_BOT)) {
      inputMessage = server.arg(TELEGRAM_BOT);
      inputParam = TELEGRAM_BOT;
      preferences.putString("TELEGRAM_BOT", inputMessage);
	    TFT_log("TELEGRAM_BOT:","******",0);
    }
    else if (server.hasArg(TELEGRAM_ADMIN)) {
      inputMessage = server.arg(TELEGRAM_ADMIN);
      inputParam = TELEGRAM_ADMIN;
      preferences.putString("TELEGRAM_ADMIN", inputMessage);
	    TFT_log("TELEGRAM_ADMIN:","******",0);
    }
    else if (server.hasArg(URL_MQTT)) {
      inputMessage = server.arg(URL_MQTT);
      inputParam = URL_MQTT;
      preferences.putString("URL_MQTT", inputMessage);
      inputMessage.toCharArray(lcdStringTemp1,inputMessage.length()+1);
	    TFT_log("URL_MQTT:",lcdStringTemp1,0);
    }
    else if (server.hasArg(USERNAME_MQTT)) {
      inputMessage = server.arg(USERNAME_MQTT);
      inputParam = USERNAME_MQTT;
      preferences.putString("USERNAME_MQTT", inputMessage);
      inputMessage.toCharArray(lcdStringTemp1,inputMessage.length()+1);
	    TFT_log("USERNAME_MQTT:",lcdStringTemp1,0);
    }
    else if (server.hasArg(PASSWORD_MQTT)) {
      inputMessage = server.arg(PASSWORD_MQTT);
      inputParam = PASSWORD_MQTT;
      preferences.putString("PASSWORD_MQTT", inputMessage);
	   TFT_log("PASSWORD_MQTT:","******",0);
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    preferences.end();
    Serial.println(inputMessage);
    server.send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });

  server.begin();
}

void checkAP()
{
    server.handleClient();
}