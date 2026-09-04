/*----------------------- C H A N G E L O G -----------------------------------
v1.0 (03/09/2026): Schermata singola con ora (RTC Module) e temperatura (sensore DTH11)
v2.0 (03/09/2026): Navigazione con un pulsante, 5 schermate: Data e ora; temperatura e umidità; 3 schermate D6, D20, D100 aggiunto pulsante per lanciare dado
v3.0 (03/09/2026): Aggiunta schermata di avvio e schermata Idle con robot
v3.1 (04/09/2026): Aggiunto buzzer passivo e sveglia: jingle per lancio dei dadi e suono per la sveglia
v3.2 (04/09/2026): Giorno della settimana su RTC, toggle sveglia con pressione 2s (tasto 2) e icona campanella
v3.3 (04/09/2026): Riformattazione della logica pulsanti e schermate in funzioni dedicate
------------------------------------------------------------------------------
*/
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>
#include "RTClib.h"

// Note musicali (frequenze in Hz) per il cicalino passivo
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047

LiquidCrystal lcd(7, 8, 9, 10, 11, 12); 

// Slot 1: Occhi robot
byte quadrato[8] = {
  B11111, B11111, B11111, B11111,
  B11111, B11111, B11111, B11111
};

// Slot 2: Icona campanella sveglia (5x8 pixel)
byte campanella[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B11111,
  B11111,
  B00100,
  B00000
};

const char daysOfTheWeek[7][4] = {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"};

int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);
RTC_DS1307 rtc;

const int buzzerPin = 5; 

// ============================================================================
// STRUTTURA E GESTIONE PULSANTI
// ============================================================================
struct Button {
  const int pin;
  int state;
  int lastState;
  unsigned long lastDebounceTime;
  unsigned long pressStartTime;
  bool longPressHandled;
};

// Inizializzazione delle due istanze pulsante
Button btnScreen = {3, HIGH, HIGH, 0, 0, false};
Button btnRoll   = {4, HIGH, HIGH, 0, 0, false};

const unsigned long debounceDelay = 50;
const unsigned long longPressThreshold = 1000; // 2 secondi per pressione lunga

// 0: Nessun evento | 1: Click singolo | 2: Pressione lunga
int checkButton(Button &btn, unsigned long currentMillis) {
  int event = 0;
  int reading = digitalRead(btn.pin);

  if (reading != btn.lastState) {
    btn.lastDebounceTime = currentMillis;
  }

  if ((currentMillis - btn.lastDebounceTime) > debounceDelay) {
    if (reading != btn.state) {
      btn.state = reading;

      if (btn.state == LOW) { // Pulsante appena premuto verso GND
        btn.pressStartTime = currentMillis;
        btn.longPressHandled = false;
      } else {                // Pulsante rilasciato
        if (!btn.longPressHandled) {
          event = 1; // Click corto
        }
      }
    }
  }

  // Rilevamento pressione continua prolungata
  if (btn.state == LOW && !btn.longPressHandled) {
    if (currentMillis - btn.pressStartTime >= longPressThreshold) {
      btn.longPressHandled = true;
      event = 2; // Pressione lunga raggiunta
    }
  }

  btn.lastState = reading;
  return event;
}

// Impostazioni Sveglia
const int alarmHour = 12;
const int alarmMinute = 42;
bool alarmEnabled = true;          
bool alarmTriggered = false;
bool alarmDismissedToday = false;

// 0=Robot, 1=Clock, 2=Weather, 3=D6, 4=D20, 5=D100
int currentScreen = 0; 
int resultD6 = 0, resultD20 = 0, resultD100 = 0;

unsigned long previousMillis = 0;
const long interval = 1000; 
bool forceUpdate = false; 

int eyeFrame = 0;
unsigned long lastEyeUpdate = 0;
const unsigned long eyeInterval = 1000;

// ============================================================================
// FUNZIONI AUDIO E TRANSIZIONE SCHERMATA
// ============================================================================
void playRollJingle() {
  tone(buzzerPin, NOTE_C5, 80); delay(100);
  tone(buzzerPin, NOTE_E5, 80); delay(100);
  tone(buzzerPin, NOTE_G5, 80); delay(100);
  tone(buzzerPin, NOTE_C6, 200); delay(220);
  noTone(buzzerPin);
}

void nextScreen() {
  currentScreen++;
  if (currentScreen > 5) currentScreen = 0;
  lcd.clear();
  forceUpdate = true;
}

// ============================================================================
// RENDERING SCHERMI
// ============================================================================
void drawRobotScreen(unsigned long currentMillis) {
  if (currentMillis - lastEyeUpdate >= eyeInterval || forceUpdate) {
    lastEyeUpdate = currentMillis;
    forceUpdate = false;
    lcd.setCursor(0, 0);
    lcd.print("      Zzz...    "); 
    lcd.setCursor(0, 1);
    switch (eyeFrame) {
      case 0: lcd.print("     \x01    \x01   "); break;
      case 1: lcd.print("      \x01    \x01  "); break;
      case 2: lcd.print("     \x01    \x01   "); break;
      case 3: lcd.print("    \x01    \x01    "); break;
      case 4: lcd.print("     -    -   "); break;
    }
    eyeFrame++;
    if (eyeFrame > 4) eyeFrame = 0; 
  }
}

void drawClockScreen(const DateTime& now) {
  lcd.setCursor(0, 0);
  lcd.print(daysOfTheWeek[now.dayOfTheWeek()]);
  lcd.print(" ");
  if (now.day() < 10) lcd.print('0');
  lcd.print(now.day());
  lcd.print('/');
  if (now.month() < 10) lcd.print('0');
  lcd.print(now.month());
  lcd.print('/');
  lcd.print(now.year());

  lcd.setCursor(15, 0);
  if (alarmEnabled) lcd.write(byte(2));
  else lcd.print(" ");

  lcd.setCursor(0, 1);
  lcd.print("    ");
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second());
}

void drawWeatherScreen() {
  byte temperature = 0;
  byte humidity = 0;
  if (dht11.read(&temperature, &humidity, NULL) == 0) {
    lcd.setCursor(0, 0);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print(" C     "); 
    lcd.setCursor(0, 1);
    lcd.print("Umid: ");
    lcd.print(humidity);
    lcd.print(" %     ");
  }
}

void drawDiceScreen(const char* diceName, int result) {
  lcd.setCursor(0, 0);
  lcd.print("Lancio Dado:");
  lcd.print(diceName);
  
  lcd.setCursor(0, 1);
  if (result == 0) {
    lcd.print("Premi per tirare");
  } else {
    lcd.print("Risultato: ");
    lcd.print(result);
    lcd.print("    ");
  }
}

void updateDisplay(unsigned long currentMillis, const DateTime& now) {
  if (currentScreen == 0) {
    drawRobotScreen(currentMillis);
    return;
  }

  if (currentMillis - previousMillis >= interval || forceUpdate || currentMillis < 100) {
    previousMillis = currentMillis;
    forceUpdate = false;

    switch (currentScreen) {
      case 1: drawClockScreen(now); break;
      case 2: drawWeatherScreen(); break;
      case 3: drawDiceScreen("D6 ", resultD6); break;
      case 4: drawDiceScreen("D20", resultD20); break;
      case 5: drawDiceScreen("D100", resultD100); break;
    }
  }
}

// ============================================================================
// SETUP & LOOP
// ============================================================================
void setup() {
  lcd.begin(16, 2);
  lcd.createChar(1, quadrato);
  lcd.createChar(2, campanella);
  Wire.begin();
  
  pinMode(buzzerPin, OUTPUT);
  pinMode(btnScreen.pin, INPUT_PULLUP);
  pinMode(btnRoll.pin, INPUT_PULLUP);
  randomSeed(analogRead(0));

  lcd.setCursor(0, 0);
  lcd.print("   ClockCiock    ");
  lcd.setCursor(0, 1);
  lcd.print("by Ciock ver 3.3  "); 
  delay(3000); 
  lcd.clear(); 

  if (!rtc.begin()) {
    lcd.print("Errore RTC");
    while (1);
  }
  
  forceUpdate = true;
}

void loop() {
  unsigned long currentMillis = millis();
  DateTime now = rtc.now();

  // Controllo scatto sveglia
  if (alarmEnabled && now.hour() == alarmHour && now.minute() == alarmMinute && !alarmDismissedToday) {
    alarmTriggered = true;
  }
  if (now.minute() != alarmMinute) {
    alarmDismissedToday = false;
  }

  // Allarme attivo
  if (alarmTriggered) {
    lcd.setCursor(0, 0);
    lcd.print("*** SVEGLIA! ***");
    lcd.setCursor(0, 1);
    lcd.print(" Premi un tasto ");
    
    tone(buzzerPin, 1000, 200);
    delay(300);
    tone(buzzerPin, 2000, 200);
    delay(300);
    
    if (digitalRead(btnScreen.pin) == LOW || digitalRead(btnRoll.pin) == LOW) {
      alarmTriggered = false;
      alarmDismissedToday = true;
      noTone(buzzerPin);
      lcd.clear();
      forceUpdate = true;
    }
    return;
  }

  // Lettura eventi pulsanti tramite la funzione dedicata
  int screenEvent = checkButton(btnScreen, currentMillis);
  int rollEvent   = checkButton(btnRoll, currentMillis);

  // Azioni Pulsante 1 (Cambio Schermata)
  if (screenEvent == 1) {
    nextScreen();
  }

  // Azioni Pulsante 2: Click Singolo (Tiro Dado)
  if (rollEvent == 1) {
    if (currentScreen == 3) { resultD6 = random(1, 7); playRollJingle(); }
    else if (currentScreen == 4) { resultD20 = random(1, 21); playRollJingle(); }
    else if (currentScreen == 5) { resultD100 = random(1, 101); playRollJingle(); }
    forceUpdate = true;
  }

  // Azioni Pulsante 2: Pressione Lunga (Toggle Sveglia nella schermata Orologio)
  if (rollEvent == 2 && currentScreen == 1) {
    alarmEnabled = !alarmEnabled;
    tone(buzzerPin, alarmEnabled ? 1500 : 750, 150);
    delay(150);
    noTone(buzzerPin);
    forceUpdate = true;
  }

  // Aggiornamento grafico LCD
  updateDisplay(currentMillis, now);
}