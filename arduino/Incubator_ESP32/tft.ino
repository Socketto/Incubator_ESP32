#include <Adafruit_GFX.h>     // Core graphics library
#include <Adafruit_ST7735.h>  // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h>  // Hardware-specific library for ST7789
#include <SPI.h>

#define TFT_CS 15
#define TFT_RST 14  // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC 12

// OPTION 2 lets you interface the display using ANY TWO or THREE PINS,
// tradeoff being that performance is not as fast as hardware SPI above.
//#define TFT_MOSI 14  // Data out
//#define TFT_SCLK 15  // Clock out

bool TFT_Initialized = false;
char oldtempexts[200];
char tempexts[200];
char strTime[200];
char row1[200];
char row2[200];
char row3[200];
char row4[200];
char row5[200];
bool UpdateRow1 = false;
bool UpdateRow2 = false;
bool UpdateRow3 = false;
bool UpdateRow4 = false;
bool UpdateRow5 = false;
bool updateTime = false;
bool Blink_TFT = false;
unsigned char WifiStatus = 0;
unsigned char prevWifiStatus = 0;
unsigned char ServerStatus = 0;
unsigned char ServerStatusPrec = 255;
unsigned char StatHPrec = 255;
unsigned char StatH = 255;
unsigned char StatHTPrec = 255;
unsigned char StatHT = 255;

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

float p = 3.1415926;
bool StartDashboard = false;


bool SetupTFT() {
  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);  // Init ST7735S chip, black tab
  tft.setRotation(3);
  // large block of text
  tft.fillScreen(ST77XX_BLACK);
  TFT_Initialized = true;
  return true;
}

bool ResetSoft = false;

bool SoftResetTFT() {
  ResetSoft = true;
  return true;
}

bool HardReset = false;
bool ResetTFT() {
  HardReset = true;
  return true;
}

void TFT_log_init() {
  if (StartDashboard || !TFT_Initialized) return;
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);
  tft.setCursor(0, 0);
}


unsigned char counterLine = 0;
void TFT_log(char* text, char* value, uint16_t color) {
  if (StartDashboard || !TFT_Initialized) return;
  counterLine++;
  if (counterLine == 16) {
    tft.fillScreen(ST77XX_BLACK);
    tft.setCursor(0, 0);
    counterLine = 1;
  }
  tft.setTextColor(ST77XX_WHITE);
  tft.print(text);
  tft.print(" - ");
  TFT_setColor(color);
  tft.println(value);
}

void TFT_log_append(char* text, uint16_t color, bool nl) {
  if (StartDashboard || !TFT_Initialized) return;

  TFT_setColor(color);

  if (nl) {
    tft.println(text);
  } else {
    tft.print(text);
  }
}

void TFT_setColor(uint16_t color) {
  tft.setTextColor(ST77XX_WHITE);
  switch (color) {
    case 0: tft.setTextColor(ST77XX_GREEN); break;
    case 1: tft.setTextColor(ST77XX_YELLOW); break;
    case 2: tft.setTextColor(ST77XX_RED); break;
    case 3: tft.setTextColor(ST77XX_CYAN); break;
  }
}


void InitDashboard() {
  if (!TFT_Initialized) return;
  StartDashboard = true;
  Serial.println("SET TASKTFT");

  tft.fillRect(0, 0, 160, 128, ST77XX_BLACK);
  DrawLogo();
  delay(5000);
  tft.fillRect(0, 0, 160, 128, ST77XX_BLACK);

  xTaskCreate(
    LCDTask,    /* Task function. */
    "LCD_TASK", /* String with name of task. */
    40000,      /* Stack size in bytes. */
    NULL,       /* Parameter passed as input of the task */
    1,          /* Priority of the task. */
    NULL);      /* Task handle. */
  Serial.println("SETTED TASKTFT");
}


void UpdateTime(char* t) {
  if (strcmp(strTime, t) != 0) {
    updateTime = true;
    sprintf(strTime, "%s", t);
  }
}

int colotTemp = 1;

void Updaterows() {
  if (strcmp(row1, StartDay) != 0) {
    sprintf(row1, "%s", StartDay);
    sprintf(StartIncubata, "%s", StartDay);
    UpdateRow1 = true;
  }
  if (strcmp(row3, Animale) != 0) {
    sprintf(row3, "%s", Animale);
    UpdateRow3 = true;
  }
  if (strcmp(row2, GiorniPassati) != 0) {
    sprintf(row2, "%s", GiorniPassati);
    UpdateRow2 = true;
  }

  if (Temp1Ready) {
    sprintf(tempexts, "  %2.1f\367C", tempext);
    if (strcmp(tempexts, oldtempexts) != 0) {
      if (tempext < 35)
        colotTemp = 0;
      else if (tempext < 39)
        colotTemp = 1;
      else
        colotTemp = 2;
      strcpy(oldtempexts, tempexts);
      sprintf(row4, "%s", tempexts);
      UpdateRow4 = true;
    }
  } else {
    sprintf(row4, "wait %d%%", waitTemp);
    UpdateRow4 = true;
  }
  if (strcmp(row5, humstr) != 0) {
    sprintf(row5, "%s", humstr);
    UpdateRow5 = true;
  }
}

void UpdateWifiStatus(unsigned char status) {
  if (WifiStatus > 2 && status == 2)
    return;
  WifiStatus = status;
}

void UpdateServerStatus(unsigned char status) {
  if (ServerStatus > 1) return;
  ServerStatus = status;
}

//https://www.instructables.com/Converting-Images-to-Flash-Memory-Iconsimages-for-/

void LCDTask(void* pvParameters) {
  tft.fillScreen(ST77XX_BLACK);
  while (true) {
    if (xSemaphoreTake(xMutex, portMAX_DELAY)) {  // Prendi il mutex
      if (ResetSoft || HardReset)
      {
        StatHTPrec = 100;
        StatHPrec = 100;
        prevWifiStatus = 123;
        ServerStatusPrec = 123;
      }
      if (ResetSoft) {
        ResetSoft = false;
        tft.initR(INITR_BLACKTAB);  // Init ST7735S chip, black tab
        tft.setRotation(3);
        // large block of text
        tft.fillScreen(ST77XX_BLACK);
        TFT_Initialized = true;
        UpdateRow1 = true;
        UpdateRow2 = true;
        UpdateRow3 = true;
        UpdateRow4 = true;
        UpdateRow5 = true;
        updateTime = true;
      }
      if (HardReset) {
        HardReset = false;
        digitalWrite(TFT_RST, LOW);   // Metti il reset a livello basso
        delay(100);                   // Aspetta un po'
        digitalWrite(TFT_RST, HIGH);  // Riporta il reset a livello alto
        delay(100);                   // Attendi la stabilizzazione
        // Use this initializer if using a 1.8" TFT screen:
        tft.initR(INITR_BLACKTAB);  // Init ST7735S chip, black tab
        tft.setRotation(3);
        // large block of text
        tft.fillScreen(ST77XX_BLACK);
        TFT_Initialized = true;
        UpdateRow1 = true;
        UpdateRow2 = true;
        UpdateRow3 = true;
        UpdateRow4 = true;
        UpdateRow5 = true;
        updateTime = true;
      }
      if (DebugMutex) {
        Serial.println("Mutex LCDTask");
      }
      if (colotTemp == 2)  //for blinking
      {
        UpdateRow3 = true;
      }

      if (updateTime) {
        tft.setTextColor(ST77XX_WHITE);
        tft.setCursor(0, 0);
        tft.setTextSize(1);
        tft.fillRect(0, 0, 115, 8, ST77XX_BLACK);
        tft.println(strTime);
        updateTime = false;
      }
      if (UpdateRow1) {
        UpdateRow1 = false;
        tft.setTextColor(ST77XX_WHITE);
        tft.fillRect(0, 8, 120, 8, ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println(row1);
      }
      if (UpdateRow2) {
        UpdateRow2 = false;
        tft.setTextColor(ST77XX_WHITE);
        tft.fillRect(0, 16, 160, 8, ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println("");
        tft.println(row2);
      }
      if (UpdateRow3) {
        UpdateRow3 = false;
        tft.setTextColor(ST77XX_WHITE);
        tft.fillRect(0, 24, 160, 8, ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println("");
        tft.println(row3);
      }
      if (UpdateRow4) {
        UpdateRow4 = false;
        tft.setTextColor(ST77XX_WHITE);
        tft.fillRect(0, 32, 160, 40, ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println("");
        tft.println("");
        tft.println("");
        tft.setTextSize(1);
        tft.println(settingsstr);
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(3);
        switch (colotTemp) {
          case 0: tft.setTextColor(ST77XX_CYAN); break;
          case 1: tft.setTextColor(ST77XX_WHITE); break;
          case 2:
            if (Blink_TFT) tft.setTextColor(ST77XX_YELLOW);
            else tft.setTextColor(ST77XX_ORANGE);
            break;
        }
        tft.println(row4);
        tft.setTextColor(ST77XX_WHITE);
      }
      if (UpdateRow5) {
        UpdateRow5 = false;
        tft.setTextColor(ST77XX_WHITE);
        tft.fillRect(0, 72, 160, 40, ST77XX_BLACK);
        tft.setCursor(0, 0);
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println("");
        tft.setTextSize(1);
        tft.println("");
        tft.println("");
        tft.setTextSize(3);
        tft.println("");
        tft.println("");
        tft.println(row5);
      }


      Blink_TFT = !Blink_TFT;

      if (WifiStatus != prevWifiStatus) {
        tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, 0x8410);  //grey
        prevWifiStatus = WifiStatus;
        switch (WifiStatus) {
          case 0: tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, ST77XX_RED); break;
          case 1:
            if (Blink_TFT) tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, ST77XX_CYAN);
            else tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, ST77XX_YELLOW);
            break;
          case 2:
            if (Blink_TFT) tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, ST77XX_GREEN);
            else tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, ST77XX_YELLOW);
            break;

          case 3: tft.drawBitmap(150, 0, image_data_WIFI_WHITE_0, 10, 10, ST77XX_GREEN); break;
          case 4: tft.drawBitmap(150, 0, image_data_WIFI_WHITE_1, 10, 10, ST77XX_GREEN); break;
          case 5: tft.drawBitmap(150, 0, image_data_WIFI_WHITE_2, 10, 10, ST77XX_GREEN); break;
          case 6: tft.drawBitmap(150, 0, image_data_WIFI_WHITE, 10, 10, ST77XX_GREEN); break;
        }
      }
      
      if(ServerStatusPrec != ServerStatus)
      {
        ServerStatusPrec = ServerStatus;
        switch (ServerStatus) {
          case 0: tft.drawBitmap(139, 0, image_data_WORLD_WHITE, 10, 10, ST77XX_RED); break;
          case 1: tft.drawBitmap(139, 0, image_data_WORLD_WHITE, 10, 10, ST77XX_GREEN); break;
          case 2:
          case 3:
          case 4:
          case 5:
          case 6:
          case 7:
          case 8:
          case 9:
            if (Blink_TFT) tft.drawBitmap(139, 0, image_data_WORLD_WHITE, 10, 10, ST77XX_MAGENTA);
            else tft.drawBitmap(139, 0, image_data_WORLD_WHITE, 10, 10, ST77XX_ORANGE);
            ServerStatus++;
            break;
          case 12:
          case 13:
          case 14:
          case 15:
          case 16:
          case 17:
          case 18:
          case 19:
            if (Blink_TFT) tft.drawBitmap(139, 0, image_data_WORLD_WHITE, 10, 10, ST77XX_CYAN);
            else tft.drawBitmap(139, 0, image_data_WORLD_WHITE, 10, 10, ST77XX_GREEN);
            ServerStatus++;
            break;
          default: ServerStatus = 1; break;
        }
      }

      if (humidifier) {
        if (ErrorHumidity && Blink_TFT)
          StatH = 0;
        else
        StatH = 1;
      } else {
        if (ErrorHumidity && Blink_TFT)
          StatH = 0;
        else
          StatH = 2;
      }

      if(StatH != StatHPrec) {
        StatHPrec = StatH;
        switch(StatH)
        {
          case 0:
            tft.drawBitmap(128, 0, image_data_humidifier, 10, 10, ST77XX_RED);
          break;
          case 1:
            tft.drawBitmap(128, 0, image_data_humidifier, 10, 10, ST77XX_CYAN);
          break;
          case 2:
            tft.drawBitmap(128, 0, image_data_humidifier, 10, 10, 0x8410);
          break;
        }
      }

      if (heater) {
        if (Blink_TFT) 
        StatHT = 0;
        else 
        StatHT = 1;
      } else {
        
        StatHT = 2;
      }

      if(StatHT != StatHTPrec) {
        StatHTPrec = StatHT;
        switch(StatHT)
        {
          case 0:
            tft.drawBitmap(117, 0, image_data_heater, 10, 10, ST77XX_ORANGE);
          break;
          case 1:
            tft.drawBitmap(117, 0, image_data_heater, 10, 10, ST77XX_RED);
          break;
          case 2:
            tft.drawBitmap(117, 0, image_data_heater, 10, 10, 0x8410);
          break;
        }
      }

      if (DebugMutex) {
        Serial.println("Mutex released");
      }
      xSemaphoreGive(xMutex);  // Rilascia il mutex
    }
    delay(400);
  }
}


void DrawLogo() {
  int h = 128, w = 160, row, col, buffidx = 0;
  for (row = 0; row < h; row++) {    // For each scanline...
    for (col = 0; col < w; col++) {  // For each pixel...
      //To read from Flash Memory, pgm_read_XXX is required.
      //Since image is stored as uint16_t, pgm_read_word is used as it uses 16bit address
      tft.drawPixel(col, row, pgm_read_word(image_data_LOGO + buffidx));
      buffidx++;
    }  // end pixel
  }
}
