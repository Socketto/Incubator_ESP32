#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>  // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <WiFi.h>

UniversalTelegramBot* bot;  // Pointer declaration
// Initialize Telegram BOT
char MessageAuthomatic[400];

bool userAuthorized(String value) {
  if (AdminTelegram == value) {
    return true;  // value found
  }
  return false;  // value not found
}


float extractFloat(String input) {
  bool foundDigit = false;
  String numStr = "";

  for (int i = 0; i < input.length(); i++) {
    char c = input[i];


    if (isdigit(c) || c == '.') {
      numStr += c;
      foundDigit = true;
    }

    else if (c == '-' && !foundDigit && i + 1 < input.length() && isdigit(input[i + 1])) {
      numStr += c;
    }

    else if (foundDigit) {
      break;
    }
  }

  return numStr.length() > 0 ? numStr.toFloat() : 0.0;
}

char messaget[400];
WiFiClientSecure clientt;

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot->messages[i].chat_id);
    if (userAuthorized(chat_id) == false) {
      bot->sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }

    // Print the received message
    String text = bot->messages[i].text;
    Serial.println(text);

    String from_name = bot->messages[i].from_name;

    if (text == "/start") {
      String welcome = "Welcome";
      bot->sendMessage(chat_id, welcome, "");
    }

    if (text == "/status") {
      if (Temp1Ready) {
        sprintf(messaget, "Wifi signal: %d%%\n%s\n%s\nTemperature: %2.1f°C\nHumidity: %2.1f%%\nStart: %s\nIP: http://%s", testpercentage, Animale, GiorniPassati, tempext, humidity, StartIncubata, WiFi.localIP().toString());
        bot->sendMessage(chat_id, messaget, "");
        sprintf(messaget, "cycle-time %i\nmin-on-time %i\ncustomminOnTime %i\nmax-on-time %i\nset-delta-set-point %2.2f\nupdate-mqtt %lu\nset-delta-temp %2.2f", cycleTime, minOnTime, customminOnTime, maxOnTime, deltasetpoint, TimeUpdateMQTT, deltaTemperature);
        bot->sendMessage(chat_id, messaget, "");
        MQTT_Publish();
      } else {
        sprintf(messaget, "Wifi signal: %d%%", testpercentage);
        bot->sendMessage(chat_id, messaget, "");
      }
    }

    if (text == "/gallina") {
      preferences_command.begin("incu1", false);
      preferences_command.putInt("Animale", 0);
      animaleint = 0;
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
      AlarmsManagement = true;
    }

    if (text == "/quaglia") {
      preferences_command.begin("incu1", false);
      preferences_command.putInt("Animale", 1);
      animaleint = 1;
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
      AlarmsManagement = true;
    }

    if (text == "/noanimal") {
      preferences_command.begin("incu1", false);
      preferences_command.putInt("Animale", 2);
      animaleint = 2;
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
      AlarmsManagement = false;
    }

    if (text == "reset-display") {
      ResetTFT();
      bot->sendMessage(chat_id, "Ok", "");
    }

    if (text == "soft-reset-tft") {
      SoftResetTFT();
      bot->sendMessage(chat_id, "Ok", "");
    }


    if (text == "/reset") {
      Reset = true;
      bot->sendMessage(chat_id, "Ok", "");
    }

    if (text == "/reboot") {
      bot->sendMessage(chat_id, "Ok", "");
      RebootRequested = 7;
    }

    if (text == "comandi") {
      bot->sendMessage(chat_id, "reset-display\nsoft-reset-tft\nset-temp 60\ncycle-time 10000\nmin-on-time 3000\ndefault-min-on-time\nmax-on-time 9000\nupdate-mqtt 60000\nset-delta-set-point 1.5\nset-delta-temp 1.5\nset-hum 50", "");
    }

    if (text.startsWith("set-temp ")) {
      desiredT = extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putFloat("desiredT", desiredT);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("cycle-time ")) {
      cycleTime = (int)extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putInt("cycleTime", cycleTime);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("min-on-time ")) {
      minOnTime = (int)extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putInt("minOnTime", minOnTime);
      preferences_command.end();
      customminOnTime = minOnTime;
      preferences_command.begin("incu1", false);
      preferences_command.putInt("customminOnTime", customminOnTime);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text == "default-min-on-time") {
      customminOnTime = 0;
      preferences_command.begin("incu1", false);
      preferences_command.putInt("customm", customminOnTime);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("max-on-time ")) {
      maxOnTime = (int)extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putInt("maxOnTime", maxOnTime);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("update-mqtt ")) {
      TimeUpdateMQTT = (long)extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putLong("TimQTT", TimeUpdateMQTT);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("set-delta-set-point ")) {
      deltasetpoint = extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putInt("deltase", (int)(deltasetpoint * 100));
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }
    if (text.startsWith("set-delta-temp ")) {
      deltaTemperature = extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putInt("deltaTe", (int)(deltaTemperature * 100));
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
    }



    if (text.startsWith("set-hum ")) {
      desiredH = extractFloat(text);
      preferences_command.begin("incu1", false);
      preferences_command.putFloat("desiredH", desiredH);
      preferences_command.end();
      bot->sendMessage(chat_id, "Ok", "");
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
    bot->sendMessage("134947143", "Build: " __DATE__ " " __TIME__, "");
  }
}

void SendDailyBot() {
  sprintf(MessageAuthomatic, "Giorno %d", giornipassatiintold);
  bot->sendMessage("134947143", MessageAuthomatic, "");
}

void sendAlarms() {
  if (AlarmsManagement == false) {
    return;
  }

  if (humidity >= 0) {
    if (humidity < desiredH - (histH + 20)) {
      sprintf(MessageAuthomatic, "Umidità troppo bassa! desiderata %2.1f misurata %2.1f", desiredH, humidity);
      bot->sendMessage("134947143", MessageAuthomatic, "");
    }
  }

  if (Temp1Ready) {
    if (tempext > desiredT + (histT + 3)) {
      sprintf(MessageAuthomatic, "Temperatura troppo alta! desiderata %2.1f misurata %2.1f", desiredT, tempext);
      bot->sendMessage("134947143", MessageAuthomatic, "");
    }
    if (tempext < desiredT - (histT + 3)) {
      sprintf(MessageAuthomatic, "Temperatura troppo bassa! desiderata %2.1f misurata %2.1f", desiredT, tempext);
      bot->sendMessage("134947143", MessageAuthomatic, "");
    }
  }
}

void TelegramSetup() {
  bot = new UniversalTelegramBot(BOTtoken.c_str(), clientt);
  clientt.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org
  bot->longPoll = 60;
  bot->sendMessage("134947143", "The board has been powered on.", "");
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
          bot->sendMessage("134947143", "Togliere il girauova!", "");
        }
        if (DebugMutex) {
          Serial.println("Mutex released");
        }
        xSemaphoreGive(xMutex);
      }
      Serial.println(bot->last_message_received);
      int numNewMessages = bot->getUpdates(bot->last_message_received + 1);
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
          xSemaphoreGive(xMutex);
        }
        Serial.println(bot->last_message_received);
        numNewMessages = bot->getUpdates(bot->last_message_received + 1);
        Serial.println(bot->last_message_received);
        Serial.println(numNewMessages);
        vTaskDelay(pdMS_TO_TICKS(200));
      }
      lastTimeBotRan = millis();
    }
  }
}
