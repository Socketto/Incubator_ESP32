unsigned long MillisTimeLiters = 0;
bool flashing = false;
bool red = false;
bool green = false;
bool blue = false;

int PIN_RW485 = 0;
int PIN_RELE_2 = 0;
int PIN_LED_RGB_GREEN = 0;


void setupIO() {
  Serial.println("START SET PINS");


  pinMode(27, INPUT);  // LED RGB
  delay(200);

  pinMode(0, INPUT);  //BOOT PIN
  //1 Debug
  pinMode(2, OUTPUT);  // RELAY 1
  //3 Debug
  //pinMode(13, INPUT);  // FLOW METER
  //16 RS-485
  //17 RS-485

  PIN_RW485 = 4;
  PIN_RELE_2 = 5;
  PIN_LED_RGB_GREEN = 32;
  pinMode(23, OUTPUT);  // SDA
  pinMode(18, OUTPUT);  // SCK
  pinMode(12, OUTPUT);  // DC
  pinMode(14, OUTPUT);  // RESET
  pinMode(15, OUTPUT);  // CS
  pinMode(33, OUTPUT);  // LED
  digitalWrite(33, HIGH);

  pinMode(PIN_RW485, OUTPUT);  //R/W RS-485
  // 21 i2C connector
  // 22 i2c connector
  pinMode(PIN_RELE_2, OUTPUT);         // RELAY 2
  pinMode(25, OUTPUT);                 // LED RGB
  pinMode(26, OUTPUT);                 // LED RGB
  pinMode(PIN_LED_RGB_GREEN, OUTPUT);  // LED RGB
  //pinMode(34, INPUT);                  // TEST 2
  pinMode(13, INPUT);  // TEST 1



  Serial.println("SET TASK LED");
  xTaskCreate(
    RGBTask,   /* Task function. */
    "RGBTask", /* String with name of task. */
    10000,     /* Stack size in bytes. */
    NULL,      /* Parameter passed as input of the task */
    1,         /* Priority of the task. */
    NULL);     /* Task handle. */
  Serial.println("SETTED TASKLED");
}

void RGBTask(void* pvParameters) {
  while (true) {
    delay(300);
    if (xSemaphoreTake(xMutex, portMAX_DELAY)) {  // Prendi il mutex
      if (DebugMutex) {
        Serial.println("Mutex RGBTask");
      }
      if (red) {
        digitalWrite(26, HIGH);
      } else {
        digitalWrite(26, LOW);
      }
      if (green) {
        digitalWrite(PIN_LED_RGB_GREEN, HIGH);
      } else {
        digitalWrite(PIN_LED_RGB_GREEN, LOW);
      }
      if (blue) {
        digitalWrite(25, HIGH);
      } else {
        digitalWrite(25, LOW);
      }
      if (flashing) {
        delay(200);
        digitalWrite(25, LOW);
        digitalWrite(26, LOW);
        digitalWrite(PIN_LED_RGB_GREEN, LOW);
      }
      if (DebugMutex) {
        Serial.println("Mutex released");
      }
      xSemaphoreGive(xMutex);  // Rilascia il mutex
    }
  }
}

void Red(bool fl) {
  red = false;
  green = false;
  blue = false;
  red = true;
  flashing = fl;
}

void Green(bool fl) {
  red = false;
  green = false;
  blue = false;
  green = true;
  flashing = fl;
}

void Blue(bool fl) {
  red = false;
  green = false;
  blue = false;
  blue = true;
  flashing = fl;
}

void Yellow(bool fl) {
  blue = false;
  red = true;
  green = true;
  flashing = fl;
}

void Cian(bool fl) {
  red = false;
  blue = true;
  green = true;
  flashing = fl;
}

void Magenta(bool fl) {
  green = false;
  blue = true;
  red = true;
  flashing = fl;
}

void White(bool fl) {
  blue = true;
  red = true;
  green = true;
  flashing = fl;
}


void Relay1(bool value) {
  if (value) {
    digitalWrite(2, HIGH);
  } else {
    digitalWrite(2, LOW);
  }
}

void Relay2(bool value) {
  if (value) {
    digitalWrite(PIN_RELE_2, HIGH);
  } else {
    digitalWrite(PIN_RELE_2, LOW);
  }
}