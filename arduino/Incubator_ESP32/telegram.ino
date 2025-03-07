#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Preferences.h>
#include <HTTPClient.h>

// Initialize Telegram BOT
char MessageAuthomatic[400];

bool userAuthorized(String value) {
    if (AdminTelegram == value) {
      return true;  // Valore trovato
    }
  return false;  // Valore non trovato
}

// Funzione per estrarre il valore float da una stringa
float extractFloat(String input) {
    String tempString = "";
    for (int i = 0; i < input.length(); i++) {
        if (isDigit(input[i]) || input[i] == '.') {
            tempString += input[i];  // Costruisce il numero in formato stringa
        }
    }
    return tempString.toFloat();  // Converte la stringa in float
}

char messaget[400];
WiFiClientSecure clientt;
UniversalTelegramBot bot(BOTtoken, clientt);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (userAuthorized(chat_id) == false) {
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/status") {
      if (Temp1Ready) {
        sprintf(messaget, "Wifi signal: %d%%\n%s\n%s\nTemperature: %2.1f°C\nHumidity: %2.1f%%\nStart: %s\n", testpercentage, Animale, GiorniPassati, tempext, humidity,StartIncubata);
        bot.sendMessage(chat_id, messaget, "");
        MQTT_Publish();
      } else {
        sprintf(messaget, "Wifi signal: %d%%", testpercentage);
        bot.sendMessage(chat_id, messaget, "");
      }
    }

    if (text == "/gallina") {
      preferences.begin("time-info", false);
      preferences.putInt("Animale", 0);
      animaleint = 0;
      preferences.end();
      bot.sendMessage(chat_id, "Ok", "");
      AlarmsManagement = true;
    }

    if (text == "/quaglia") {
      preferences.begin("time-info", false);
      preferences.putInt("Animale", 1);
      animaleint = 1;
      preferences.end();
      bot.sendMessage(chat_id, "Ok", "");
      AlarmsManagement = true;
    }

    if (text == "/noanimal") {
      preferences.begin("time-info", false);
      preferences.putInt("Animale", 2);
      animaleint = 2;
      preferences.end();
      bot.sendMessage(chat_id, "Ok", "");
      AlarmsManagement = false;
    }
    
    if (text == "reset-display") {
      ResetTFT();
      bot.sendMessage(chat_id, "Ok", "");
    }

    if (text == "soft-reset-tft"){
      SoftResetTFT();
      bot.sendMessage(chat_id, "Ok", "");
    }
    
     
    if (text == "/reset") {
      Reset = true;
      bot.sendMessage(chat_id, "Ok", "");
    }

    if (text == "/reboot") {
      bot.sendMessage(chat_id, "Ok", "");
      RebootRequested = 7;
    }

    if (text.startsWith("set-temp "))
    {
      desiredT = extractFloat(text);
      bot.sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("set-hum "))
    {
      desiredH = extractFloat(text);
      bot.sendMessage(chat_id, "Ok", "");
    }

  }
}


int sendSchiusaFlag = 0;
void sendSchiusa() {
  sendSchiusaFlag = 1;
}
bool messageStart = false;
void SendBOTStart() {
  if (messageStart == false) {
    messageStart = true;
    bot.sendMessage("134947143", "Build: " __DATE__ " " __TIME__, "");
  }
}

void SendDailyBot() {
  sprintf(MessageAuthomatic, "Giorno %d", giornipassatiintold);
  bot.sendMessage("134947143", MessageAuthomatic, "");
}

void sendAlarms() {
  /*
	desiredH = 70;
					desiredT = 36.9;
					histT = 0.1;
					histH = 10;	
	*/
  if (AlarmsManagement == false) {
    return;
  }

  if (humidity >= 0) {
    if (humidity < desiredH - (histH + 20)) {
      sprintf(MessageAuthomatic, "Umidità troppo bassa! desiderata %2.1f misurata %2.1f", desiredH, humidity);
      bot.sendMessage("134947143", MessageAuthomatic, "");
    }
  }

  if (Temp1Ready) {
    if (tempext > desiredT + (histT + 3)) {
      sprintf(MessageAuthomatic, "Temperatura troppo alta! desiderata %2.1f misurata %2.1f", desiredT, tempext);
      bot.sendMessage("134947143", MessageAuthomatic, "");
    }
    if (tempext < desiredT - (histT + 3)) {
      sprintf(MessageAuthomatic, "Temperatura troppo bassa! desiderata %2.1f misurata %2.1f", desiredT, tempext);
      bot.sendMessage("134947143", MessageAuthomatic, "");
    }
  }
}

void TelegramSetup() {
  clientt.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  bot.longPoll = 60;
  bot.sendMessage("134947143", "The board has been powered on.", "");
}

void TelegramLoop() {
  if (WiFi.status() == WL_CONNECTED) {

    if (millis() > lastTimeBotRan + botRequestDelay) {
      if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
        if (DebugMutex) {
          Serial.println("Mutex TelegramLoop");
        }
        SendBOTStart();
        if (giornipassatiint > giornipassatiintold) {
          giornipassatiintold = giornipassatiint;
          SendDailyBot();
        }
        if (millis() > lastCheckAlarms + 120000) {  //2 min
          lastCheckAlarms = millis();
          sendAlarms();
        }
        if (sendSchiusaFlag == 1) {
          sendSchiusaFlag = 2;
          bot.sendMessage("134947143", "Togliere il girauova!", "");
        }
        if (DebugMutex) {
          Serial.println("Mutex released");
        }
        xSemaphoreGive(xMutex);  // Rilascia il mutex
      }
      Serial.println(bot.last_message_received);
      int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
      Serial.println(numNewMessages);
      while (numNewMessages) {
        Serial.println("MESSAGE:");
        Serial.println(numNewMessages);
        Serial.println("got response");
        if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
          if (DebugMutex) {
            Serial.println("Mutex TelegramLoop message");
          }
          handleNewMessages(numNewMessages);

          if (DebugMutex) {
            Serial.println("Mutex released");
          }
          xSemaphoreGive(xMutex);  // Rilascia il mutex
        }
        Serial.println(bot.last_message_received);
        numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        Serial.println(bot.last_message_received);
        Serial.println(numNewMessages);
        vTaskDelay(pdMS_TO_TICKS(200));  // Usa FreeRTOS per ritardare il task
      }
      lastTimeBotRan = millis();
    }
  }
}
