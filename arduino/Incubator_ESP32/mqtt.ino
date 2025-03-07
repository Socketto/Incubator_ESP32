#include <WiFiClient.h>
#include <PubSubClient.h>


WiFiClient espClient;
PubSubClient client(espClient);

void MQTT_loop() {
  client.loop();
}

void MQTT_setServer() {
  client.setServer("192.168.1.12", 1883);
  client.setBufferSize(2048);
}

bool MQTT_IsConnected() {
  return client.connected();
}


char tempchars[200];
void MQTT_Publish() {
  if (client.connected()) {
    if (Temp1Ready) {
      sprintf(tempchars, "%2.1f", tempext);
      client.publish("Incubator/temperature/temp", tempchars);
      sprintf(tempchars, "%2.1f", PercentageGiorniPassati);
      client.publish("Incubator/eggs/percentdays", tempchars);
      sprintf(tempchars, "%2.1f", humidity);
      client.publish("Incubator/humidity/hum", tempchars);
      sprintf(tempchars, "%2.1f", desiredT);
      client.publish("Incubator/temperature/temp_desired", tempchars);
      sprintf(tempchars, "%2.1f", desiredH);
      client.publish("Incubator/humidity/hum_desired", tempchars);
      sprintf(tempchars, "%2.1f", readtempEXT());
      client.publish("Incubator/temperature/temp_ext", tempchars);
	  switch (animaleint) {
		  case 0: strcpy(tempchars, "Chicken"); break;
		  case 1: strcpy(tempchars, "Quail"); break;
		  case 2: strcpy(tempchars, "none"); break;
		  default: sprintf(tempchars, "%u", animaleint); break;
	  }
      client.publish("Incubator/eggs/animal", tempchars);
      sprintf(tempchars, "%u", giornipassatiint);
      client.publish("Incubator/eggs/day", tempchars);
      sprintf(tempchars, "%u", giornitotaliint);
      client.publish("Incubator/eggs/day_total", tempchars);
	  switch (StepIncubata) {
		  case 1:  strcpy(tempchars, "incubation");  break;
		  case 2:  strcpy(tempchars, "hatching");  break;
		  case 3:  strcpy(tempchars, "hatching");  break;
		  default: sprintf(tempchars, "%u", StepIncubata); break;
	  }
      client.publish("Incubator/eggs/step", tempchars);
      sprintf(tempchars, "%u", testpercentage);
      client.publish("Incubator/wifi/wifi_signal", tempchars);
	  
      //publish
      UpdateServerStatus(2);
    }
  }
}

// Funzione per convertire l'RSSI in percentuale
int rssi_to_percentage(int rssi) {
  // Definisci i limiti di RSSI
  const int RSSI_MIN = -100;
  const int RSSI_MAX = -30;

  // Controlla che l'RSSI sia nel range previsto
  if (rssi < RSSI_MIN) {
    rssi = RSSI_MIN;
  } else if (rssi > RSSI_MAX) {
    rssi = RSSI_MAX;
  }

  // Calcola la percentuale
  int percentage = ((rssi - RSSI_MIN) * 100) / (RSSI_MAX - RSSI_MIN);
  return percentage;
}

void MQTT_Wifi_Status(long RSSI) {
  //https://copyprogramming.com/howto/at-csq-network-percentage-formula-from-rssi

  testpercentage = rssi_to_percentage((int)RSSI);
  if (testpercentage > 100) {
    testpercentage = 100;
  }

  if (testpercentage > 40) {
    if (testpercentage > 70) {
      if (testpercentage > 90) {
        UpdateWifiStatus(6);
      } else {
        UpdateWifiStatus(5);
      }
    } else {
      UpdateWifiStatus(4);
    }
  } else {
    UpdateWifiStatus(3);
  }
}

char ClientName[200];
bool MQTT_reconnect() {
  int o = 0;
  // Loop until we're reconnected
  if (!client.connected()) {
    UpdateServerStatus(0);
    Magenta(true);
    TFT_log("CONNECTING MQTT", "WAIT", 1);
    // Attempt to connect
    if (WiFi.status() != WL_CONNECTED) {
      Red(true);
      delay(200);
      TFT_log("CONNECTING WIFI", "WAIT", 1);
      return false;
    }

    sprintf(ClientName, "Incubator_ESP32");
    if (client.connect(ClientName, "mqtt_user", "1982")) {
      Serial.println("connected");
      client.setCallback(callback);
      TFT_log("MQTT CONNECTED", "OK", 0);
    } else {
      Red(true);
      switch (client.state()) {
        case -4: strcpy(lcdStringTemp2, "CONN.TIMEOUT"); break;
        case -3: strcpy(lcdStringTemp2, "CONN.LOST"); break;
        case -2: strcpy(lcdStringTemp2, "CONN.FAILED"); break;
        case -1: strcpy(lcdStringTemp2, "DISCONNECTED"); break;
        case 0: strcpy(lcdStringTemp2, "CONNECTED"); break;
        case 1: strcpy(lcdStringTemp2, "BAD_PROTOCOL"); break;
        case 2: strcpy(lcdStringTemp2, "BAD_CLIENT_ID"); break;
        case 3: strcpy(lcdStringTemp2, "UNAVAILABLE"); break;
        case 4: strcpy(lcdStringTemp2, "BAD_CREDENTIALS"); break;
        case 5: strcpy(lcdStringTemp2, "UNAUTHORIZED"); break;
      }
      TFT_log("CONNECTING MQTT", lcdStringTemp2, 2);
      // Wait 5 seconds before retrying
      delay(5000);
    }
    return false;
  }
  return true;
  UpdateServerStatus(1);
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
}
