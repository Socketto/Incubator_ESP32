
// Variabili per il controllo PWM con millis()
static unsigned long lastCycleStart = 0;
static unsigned long heaterOnStart = 0;
static int heaterOnTime = 0;
static bool heaterState = false; // Stato attuale del riscaldatore


// Funzione per regolare il riscaldamento
void regulateHeater(float setpoint) {
    setpoint =  setpoint + deltasetpoint;
    unsigned long currentTime = millis();

    // Legge la temperatura dal sensore ADC
    float currentTemp = tempext;

    Serial.print("Temperatura: ");
    Serial.print(currentTemp);
    Serial.println(" °C");
    
    // Calcola l'errore rispetto al setpoint
    float error = setpoint - currentTemp;

    // Controllo proporzionale: mappa l'errore nel range di accensione
    heaterOnTime = map(error * 100, 0, 100, minOnTime, maxOnTime); // Scala su 0-500 decimi di grado
    heaterOnTime = constrain(heaterOnTime, minOnTime, maxOnTime);  // Limita tra min e max

    Serial.print("Accensione riscaldatore per: ");
    Serial.print(heaterOnTime / 1000.0);
    Serial.println(" sec");


    // Gestione PWM con millis()
    if (!heaterState) {  
        // Controlla se è il momento di avviare un nuovo ciclo
        if (currentTime - lastCycleStart >= cycleTime) {
            lastCycleStart = currentTime; // Inizio nuovo ciclo
            heaterOnStart = currentTime;  // Salva il momento di accensione
            heaterState = true;
            heater = true;
            Relay1(heater);
        }
    } else {
        // Controlla se il tempo di accensione è terminato
        if (currentTime - heaterOnStart >= heaterOnTime) {
            heaterState = false;
            heater = false;
            Relay1(heater);
        }
    }
}

// Task FreeRTOS per gestire il riscaldamento
void heaterTask(void *parameter) {
    while (true) {
        if (Temp1Ready)
        {
          regulateHeater(desiredT);
        }
        vTaskDelay(pdMS_TO_TICKS(500)); // Aspetta 500ms prima di rieseguire il controllo
    }
}

// Funzione per inizializzare il task su ESP32
void initHeaterTask() {
    xTaskCreatePinnedToCore(
        heaterTask,     // Funzione del task
        "Heater Task",  // Nome del task
        4096,           // Stack size (4KB)
        NULL,           // Parametro da passare
        1,              // Priorità (bassa)
        NULL,           // Handle del task
        1               // Core su cui eseguire (1 = core secondario)
    );
}