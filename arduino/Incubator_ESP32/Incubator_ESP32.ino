#include <WiFi.h>
#include <Preferences.h>
#define NTC_SAMPLES 30

volatile int cycleTime = 10000; // 10 secondi
volatile int minOnTime = 3000;  // 2 secondi
volatile int customminOnTime = 0;
volatile int maxOnTime = 9000;  // 9 secondi

volatile double deltasetpoint = 0;
volatile double deltaTemperature = 0;
volatile long TimeUpdateMQTT = 60000;

bool DebugMutex = false;

SemaphoreHandle_t xMutex;  // Mutex per proteggere l'accesso
String ssid;
String ssidpassword;
String BOTtoken;
String AdminTelegram;
char stringWifi[200];
bool APMode = false;
char ChipID[50];

const char* WIFI_NAME = "WIFI_NAME";
const char* WIFI_PASSWORD = "WIFI_PASSWORD";
const char* TELEGRAM_BOT = "TELEGRAM_BOT";
const char* TELEGRAM_ADMIN = "TELEGRAM_ADMIN";
const char* URL_MQTT = "URL_MQTT";
const char* USERNAME_MQTT = "USERNAME_MQTT";
const char* PASSWORD_MQTT = "PASSWORD_MQTT";

int waitTemp = 0;
volatile int RebootRequested = -1;
int autoresetDisplay = 360;
int lastCheckReboot;
int animaleint = 0;
char Animale[200];


char StartIncubata[200];

bool AlarmsManagement = true;
int StepIncubata = 0;
int giornipassatiint = 0;
int giornipassatiintold = 0;
int giornitotaliint = 0;
char GiorniPassati[200];
char humstr[200];

char StartDay[200];
char settingsstr[200];
unsigned long lastreadntc;
char lcdStringTemp1[200];
char lcdStringTemp2[200];
int SecondsSetupWifi = 30;
bool first_read_time = true;
bool ErrorHumidity = true;
double Arraytemp[NTC_SAMPLES];
int indexarratemp = 0;
bool Temp1Ready = false;

float desiredH = 70;
double tempext = 0, tempextt = 0, desiredT = 37.0;
double histT = 0.1, histH = 10;

bool Reset = false;
Preferences preferences;
Preferences preferences_command;

volatile float humidity;
unsigned long lastCheckLocalTime = 0;
unsigned long lastTemperatureMQTT = 0;
unsigned long lastCheckAlarms = 0;
unsigned int testpercentage = 0;
long mainLightsForce = 0;
struct tm timeinfo;
struct tm startTime;
bool heater = false, humidifier = false;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;
// Latitude and Longitude for Piove di Sacco, Italy
float latitude = 45.2991;
float longitude = 12.0468;
float PercentageGiorniPassati = 0;

// Function to check if Daylight Saving Time (DST) is in effect
bool isDST(int day, int month, int dow) {
  if (month < 3 || month > 10) return false;
  if (month > 3 && month < 10) return true;

  int previousSunday = day - dow;

  if (month == 3) return previousSunday >= 25;
  if (month == 10) return previousSunday < 25;

  return false;
}

void StartAPMode() {
  Cian(true);
  delay(2000);
  WiFi.mode(WIFI_AP);
  sprintf(stringWifi, "%s", ChipID);
  WiFi.softAP(stringWifi, "12345679");
  sprintf(lcdStringTemp2, "%s", ChipID);
  TFT_log("WIFI", lcdStringTemp2, 3);
  TFT_log("PASSWORD", "12345679", 3);
  delay(5000);
  APMode = true;
  preferences.putUInt("APMode", 0);
  setupAP();
}

void saveTimeInfo() {
  preferences.begin("time-info", false);
  preferences.putInt("tm_sec", timeinfo.tm_sec);
  preferences.putInt("tm_min", timeinfo.tm_min);
  preferences.putInt("tm_hour", timeinfo.tm_hour);
  preferences.putInt("tm_mday", timeinfo.tm_mday);
  preferences.putInt("tm_mon", timeinfo.tm_mon);
  preferences.putInt("tm_year", timeinfo.tm_year);
  preferences.putInt("tm_wday", timeinfo.tm_wday);
  preferences.putInt("tm_yday", timeinfo.tm_yday);
  preferences.putInt("tm_isdst", timeinfo.tm_isdst);
  preferences.end();
}
void readTimeInfo() {
  preferences.begin("time-info", true);
  startTime.tm_sec = preferences.getInt("tm_sec", 0);
  startTime.tm_min = preferences.getInt("tm_min", 0);
  startTime.tm_hour = preferences.getInt("tm_hour", 0);
  startTime.tm_mday = preferences.getInt("tm_mday", 1);
  startTime.tm_mon = preferences.getInt("tm_mon", 0);
  startTime.tm_year = preferences.getInt("tm_year", 70);
  startTime.tm_wday = preferences.getInt("tm_wday", 0);
  startTime.tm_yday = preferences.getInt("tm_yday", 0);
  startTime.tm_isdst = preferences.getInt("tm_isdst", -1);
  preferences.end();
}

void CheckLocalTime() {
  if (!getLocalTime(&timeinfo)) {
    TFT_log("Time", "Failed to obtain time", 2);
    Serial.println("Failed to obtain time");
    Red(true);
    return;
  } else {
    strftime(lcdStringTemp1, sizeof(lcdStringTemp1), "%m/%d %H:%M:%S", &timeinfo);
    if (Reset) {
      saveTimeInfo();
      Reset = false;
    }

    readTimeInfo();
    strftime(StartDay, sizeof(StartDay), "%m/%d %H:%M:%S", &startTime);

    if (first_read_time) {
      first_read_time = false;
      TFT_log("Time", lcdStringTemp1, 0);
    }

    UpdateTime(lcdStringTemp1);
  }
  //Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}


void setup() {
  Serial.begin(115200);
  xMutex = xSemaphoreCreateMutex();  // Creazione del mutex
  strcpy(ChipID, Network.macAddress().c_str());
  
  struct tm timeInfo;
  if(preferences_command.begin("incu1"))
  {
    animaleint = preferences_command.getInt("Animale", 0);
    if (animaleint == 2) {
      AlarmsManagement = false;
    }

    deltaTemperature = (double)preferences_command.getInt("deltaTe", 0.0)/100;
    Serial.println(deltaTemperature);
    deltasetpoint = (double)preferences_command.getInt("deltase", 0)/100;
    Serial.println(deltasetpoint);
    TimeUpdateMQTT = preferences_command.getLong("TimQTT", 60000);
    Serial.println(TimeUpdateMQTT);
    maxOnTime = preferences_command.getInt("maxOnTime", 9000);
    Serial.println(maxOnTime);
    minOnTime = preferences_command.getInt("minOnTime", 3000);
    Serial.println(minOnTime);
    cycleTime = preferences_command.getInt("cycleTime", 10000);
    Serial.println(cycleTime);
    desiredT = preferences_command.getFloat("desiredT", 37);
    Serial.println(desiredT);
    desiredH = preferences_command.getFloat("desiredH", 70);
    Serial.println(desiredH);
    customminOnTime = preferences_command.getInt("customm", 0);
    Serial.println(customminOnTime);
    preferences_command.end();
  }
  else
  {
    Serial.println("ERROR (preferences_command.begin)");
  }
  initHeaterTask();
  Serial.println("setupIO()");
  setupIO();
  Yellow(false);
  delay(200);
  Serial.println("SetupTFT()");
  SetupTFT();
  TFT_log_init();
  Serial.println("TFT_log_init");
  delay(200);
  WiFi.mode(WIFI_STA);
  Serial.println("WiFi.mode(WIFI_STA)");
  preferences.begin("my-app", true);
  ssid = preferences.getString("SSID", "");
  ssidpassword = preferences.getString("SSIDPSW", "");
    if (ssid == "" || ssidpassword == "") {
      TFT_log("ERROR WIFI", "NOT SETTED!", 2);
      ssid = "TEST";
      ssidpassword = "123456789";
      SecondsSetupWifi = 0;
      delay(5000);
    }
  WiFi.begin(ssid, ssidpassword);
  Serial.print("Connecting to WiFi: ");
  ssid.toCharArray(stringWifi, ssid.length() + 1);
  TFT_log("WIFI", stringWifi, 1);
  TFT_log_append("WAIT seconds ", 30, true);
  Serial.println(ssid);
  Red(true);
  while (WiFi.status() != WL_CONNECTED && SecondsSetupWifi > 0) {
    Serial.print('.');
    TFT_log_append(".", 65535, false);
    SecondsSetupWifi--;
    delay(1000);
  }
  if(SecondsSetupWifi == 0)
  {
    StartAPMode();
    return;
  }
  else
  {
    setupAP();
  }
  BOTtoken = preferences.getString("TELEGRAM_BOT", "");
  Serial.println(BOTtoken);
  AdminTelegram = preferences.getString("TELEGRAM_ADMIN", "");
  Serial.println(AdminTelegram);
  preferences.end();
  Green(true);
  Serial.println("TelegramSetup()");
  TelegramSetup();
  delay(200);
  MQTT_setServer();
  while (MQTT_reconnect() == false) {
    delay(1000);
  }
  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  CheckLocalTime();
  InitDashboard();
  Serial.println("TimeTask...");
  xTaskCreate(
    TelegramTask,   /* Task function. */
    "TelegramTask", /* String with name of task. */
    20000,          /* Stack size in bytes. */
    NULL,           /* Parameter passed as input of the task */
    0,              /* Priority of the task. */
    NULL);          /* Task handle. */

  setupDHT();
}

void TelegramTask(void* pvParameters) {
  while (true) {
    TelegramLoop();
    vTaskDelay(pdMS_TO_TICKS(200));  // Usa FreeRTOS per ritardare il task
  }
}

tm copyTMStruct(tm* source) {
  tm destination;
  destination.tm_sec = source->tm_sec;
  destination.tm_min = source->tm_min;
  destination.tm_hour = source->tm_hour;
  destination.tm_mday = source->tm_mday;
  destination.tm_mon = source->tm_mon;
  destination.tm_year = source->tm_year;
  destination.tm_wday = source->tm_wday;
  destination.tm_yday = source->tm_yday;
  destination.tm_isdst = source->tm_isdst;

  return destination;
}
// Funzione per convertire una struttura tm in epoch time
time_t tmToEpochTime(tm& date) {
  tm tempdatetime = copyTMStruct(&date);
  return mktime(&tempdatetime);
}

// Funzione per calcolare la differenza in giorni tra due date
int differenceInDays(tm& startDate, tm& endDate) {
  time_t startEpoch = tmToEpochTime(startDate);
  time_t endEpoch = tmToEpochTime(endDate);
  double differenceInSeconds = difftime(endEpoch, startEpoch);
  int differenceInDaysi = (int)(differenceInSeconds / (60 * 60 * 24));
  return differenceInDaysi;
}

void loop() {

  if(APMode)
  {
    checkAP();
    return;
  }
  else
  {
    checkAP();
  }

  if (millis() > lastCheckReboot + 1000) {  //1 minuto
    lastCheckReboot = millis();
    if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
      if (DebugMutex) {
        Serial.println("Mutex RebootRequested");
      }
      if (RebootRequested >= 0) {
        RebootRequested--;
      }
      if (RebootRequested == 0) {
        ESP.restart();
      }
      if (DebugMutex) {
        Serial.println("Mutex released");
      } 

      xSemaphoreGive(xMutex);
    }
  }



  if (WiFi.status() != WL_CONNECTED) {
    UpdateWifiStatus(0);
    UpdateServerStatus(0);
    Red(true);
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print('.');
      TFT_log_append(".", 65535, false);
      delay(1000);
      SecondsSetupWifi--;
      if (SecondsSetupWifi <= 0) {
        SecondsSetupWifi = 30;
        WiFi.disconnect();
        WiFi.reconnect();
      }
    }
    TFT_log_append("", 65535, true);
  } else {
    UpdateWifiStatus(2);
    Green(true);
    if (MQTT_reconnect()) {
      MQTT_loop();
	  if (xSemaphoreTake(xMutex, portMAX_DELAY)) {   
		if (DebugMutex) {
		  Serial.println("Mutex MQTT_Wifi_Status");
		}
		MQTT_Wifi_Status(WiFi.RSSI());
		xSemaphoreGive(xMutex); 
		if (DebugMutex) {
		  Serial.println("Mutex released");
		}
	  }
    }
  }

  if (xSemaphoreTake(xMutex, portMAX_DELAY)) {
    if (DebugMutex) {
      Serial.println("Mutex loop");
    }
    switch (animaleint) {
      case 0:
        giornitotaliint = 21;
        strcpy(Animale, "Animale: Gallina");
        if (giornipassatiint <= 18) {
          StepIncubata = 1;
        } else {
          if (StepIncubata < 2)
            StepIncubata = 2;
        }
        break;
      case 1:
        giornitotaliint = 17;
        strcpy(Animale, "Animale: Quaglia");
        if (giornipassatiint <= 15) {
          StepIncubata = 1;
        } else {
          if (StepIncubata < 2)
            StepIncubata = 2;
        }
        break;
      case 2:
        giornitotaliint = 365;
        strcpy(Animale, "Animale: NO");
        StepIncubata = 100;
        break;
    }
    switch (StepIncubata) {
      case 1:  //incubazione
        desiredH = 57;
        desiredT = 37.7;
        histT = 0.1;
        histH = 10;
        sprintf(settingsstr, "Temp:%2.1f Hum:%2.1f incub.", desiredT, desiredH);
        break;
      case 2:  //schiusa
        desiredH = 70;
        desiredT = 36.9;
        histT = 0.1;
        histH = 10;
        sendSchiusa();
        StepIncubata = 3;
        break;
      case 3:  //schiusa
        sprintf(settingsstr, "Temp:%2.1f Hum:%2.1f schiusa", desiredT, desiredH);
        break;
      default:
        sprintf(settingsstr, "Temp:%2.1f Hum:%2.1f ----", desiredT, desiredH);
        break;
    }

    giornipassatiint = differenceInDays(startTime, timeinfo);

    PercentageGiorniPassati = (float)((float)(giornipassatiint * 100) / (float)(giornitotaliint));
    sprintf(GiorniPassati, "Giorni: %d/%d (%2.1f%%)", giornipassatiint, giornitotaliint, PercentageGiorniPassati);

    if (millis() > lastTemperatureMQTT + TimeUpdateMQTT) {  //60 sec
      lastTemperatureMQTT = millis();
      MQTT_Publish();
      if(autoresetDisplay > 0) //
      {
        autoresetDisplay--;
        if(autoresetDisplay == 0)
        {
          autoresetDisplay = 30; //30 mim
          ResetTFT();
        }
      }
      if(customminOnTime == 0)
      {
          if(readtempEXT() <= 15)
          {
            minOnTime = 4000;
          }
          if(readtempEXT() > 15)
          {
            minOnTime = 3000;
          }
          if(readtempEXT() > 20)
          {
            minOnTime = 2000;
          }
          if(readtempEXT() > 25)
          {
            minOnTime = 1000;
          }
      }
    }

    if (millis() > lastreadntc + 100) {
      lastreadntc = millis();
      Arraytemp[indexarratemp] = readtemp();
      indexarratemp++;
      if (indexarratemp >= NTC_SAMPLES) {
        indexarratemp = 0;
        Temp1Ready = true;
      } else {
        waitTemp = (int)((double)((double)indexarratemp / (double)NTC_SAMPLES) * 100);
      }
      if (Temp1Ready) {
        tempextt = 0;
        for (int o = 0; o < NTC_SAMPLES; o++) {
          tempextt = tempextt + Arraytemp[o];
        }
        tempext = tempextt / NTC_SAMPLES;
      }
    }
    Updaterows();
    /*if (Temp1Ready) {
      if (tempext > desiredT + histT)
        heater = false;
      if (tempext < desiredT - histT)
        heater = true;
    }*/
    humidity = loopDHT();
    if (humidity >= 0) {
      ErrorHumidity = false;
      sprintf(humstr, " %2.1f%%rh", humidity);
      if (humidity > desiredH + histH)
        humidifier = false;
      if (humidity < desiredH - histH)
        humidifier = true;
    } else {
      ErrorHumidity = true;
    }
    CheckLocalTime();
    //Relay1(heater);
    Relay2(humidifier);
    if (DebugMutex) {
      Serial.println("Mutex released");
    }
    xSemaphoreGive(xMutex);
  }
}
