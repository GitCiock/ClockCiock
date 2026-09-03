/*----------------------- C H A N G E L O G -----------------------------------
v1.0 (03/09/2026): Schermata singola con ora (RTC Module) e temperatura (sensore DTH11)
v2.0 (03/09/2026): Navigazione con un pulsante, 5 schermate: Data e ora; temperatura e umidità; 3 schermate D6, D20, D100 aggiunto pulsante per lanciare dado
v3.0 (03/09/2026): Aggiunta schermata di avvio e schermata Idle con robot
------------------------------------------------------------------------------
*/
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>
#include "RTClib.h"

// Inizializzazione LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); 

// Disegno del quadrato completamente colorato
byte quadrato[8] = {
  B11111, B11111, B11111, B11111,
  B11111, B11111, B11111, B11111
};

// Inizializzazione DHT11
int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

// Inizializzazione modulo RTC
RTC_DS1307 rtc;

// Impostazioni pulsanti
const int screenButtonPin = 3;  
const int rollButtonPin = 4;    

int screenButtonState = HIGH;             
int lastScreenButtonState = HIGH;         
unsigned long lastScreenDebounceTime = 0;  

int rollButtonState = HIGH;             
int lastRollButtonState = HIGH;         
unsigned long lastRollDebounceTime = 0;  

unsigned long debounceDelay = 50;    

// 0=Robot, 1=Data/Ora, 2=Temp/Umid, 3=D6, 4=D20, 5=D100
int currentScreen = 0; 
int resultD6 = 0, resultD20 = 0, resultD100 = 0;

// Variabili per il timing dello schermo (1000 ms = 1 secondo)
unsigned long previousMillis = 0;
const long interval = 1000; 
bool forceUpdate = false; 

// Variabili per l'animazione degli occhi
int eyeFrame = 0;
unsigned long lastEyeUpdate = 0;
const unsigned long eyeInterval = 1000; // Velocità movimento occhi

void setup() {
  lcd.begin(16, 2); // Definisce quante colonne e righe ha il display[cite: 1]
  
  // Salva il carattere speciale nello slot 1
  lcd.createChar(1, quadrato);
  
  Wire.begin();
  
  // --- SCHERMATA DI AVVIO ---
  lcd.setCursor(0, 0);
  lcd.print("   ClockCiock    "); // Visualizza il messaggio che vedremo sullo schermo[cite: 1]
  lcd.setCursor(0, 1);
  lcd.print("by Ciock ver 3.0  "); 
  delay(3000); 
  lcd.clear(); 
  
  pinMode(screenButtonPin, INPUT_PULLUP);
  pinMode(rollButtonPin, INPUT_PULLUP);
  randomSeed(analogRead(0));

  if (!rtc.begin()) {
    lcd.print("Errore RTC");
    while (1);
  }
  
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  forceUpdate = true;
}

void loop() {
  unsigned long currentMillis = millis();

  // --- GESTIONE PULSANTE CAMBIO SCHERMATA ---
  int readingScreen = digitalRead(screenButtonPin);
  if (readingScreen != lastScreenButtonState) {
    lastScreenDebounceTime = currentMillis;
  }
  if ((currentMillis - lastScreenDebounceTime) > debounceDelay) {
    if (readingScreen != screenButtonState) {
      screenButtonState = readingScreen;
      
      if (screenButtonState == LOW) {
        currentScreen++;
        if (currentScreen > 5) currentScreen = 0; 
        lcd.clear(); 
        forceUpdate = true; 
      }
    }
  }
  lastScreenButtonState = readingScreen;

  // --- GESTIONE PULSANTE LANCIO DADI ---
  int readingRoll = digitalRead(rollButtonPin);
  if (readingRoll != lastRollButtonState) {
    lastRollDebounceTime = currentMillis;
  }
  if ((currentMillis - lastRollDebounceTime) > debounceDelay) {
    if (readingRoll != rollButtonState) {
      rollButtonState = readingRoll;
      
      if (rollButtonState == LOW) {
        if (currentScreen == 3) resultD6 = random(1, 7); 
        else if (currentScreen == 4) resultD20 = random(1, 21); 
        else if (currentScreen == 5) resultD100 = random(1, 101); 
        forceUpdate = true; 
      }
    }
  }
  lastRollButtonState = readingRoll;

  // --- AGGIORNAMENTO DEL DISPLAY ---
  
  // SCHERMATA 0: ROBOT MANUALE
  if (currentScreen == 0) {
    if (currentMillis - lastEyeUpdate >= eyeInterval || forceUpdate) {
      lastEyeUpdate = currentMillis;
      forceUpdate = false;
      
      lcd.setCursor(0, 0);
      lcd.print("      Zzz...    "); 
      lcd.setCursor(0, 1);
      
      // \x01 richiama il quadrato colorato salvato nello slot 1
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
  // SCHERMATE NORMALI
  else {
    if (currentMillis - previousMillis >= interval || forceUpdate || currentMillis < 100) {
      previousMillis = currentMillis;
      forceUpdate = false;
      
      if (currentScreen == 1) { 
        DateTime now = rtc.now();
        lcd.setCursor(0, 0);
        lcd.print("Data: ");
        if (now.day() < 10) lcd.print('0');
        lcd.print(now.day());
        lcd.print('/');
        if (now.month() < 10) lcd.print('0');
        lcd.print(now.month());
        lcd.print('/');
        lcd.print(now.year());

        lcd.setCursor(0, 1);
        lcd.print("Ora:  ");
        if (now.hour() < 10) lcd.print('0');
        lcd.print(now.hour());
        lcd.print(':');
        if (now.minute() < 10) lcd.print('0');
        lcd.print(now.minute());
        lcd.print(':');
        if (now.second() < 10) lcd.print('0');
        lcd.print(now.second());
      } 
      else if (currentScreen == 2) { 
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
      else if (currentScreen == 3) { 
        lcd.setCursor(0, 0); lcd.print("Lancio Dado: D6 ");
        lcd.setCursor(0, 1);
        if (resultD6 == 0) lcd.print("Premi per tirare"); else { lcd.print("Risultato: "); lcd.print(resultD6); lcd.print("    "); }
      }
      else if (currentScreen == 4) { 
        lcd.setCursor(0, 0); lcd.print("Lancio Dado: D20");
        lcd.setCursor(0, 1);
        if (resultD20 == 0) lcd.print("Premi per tirare"); else { lcd.print("Risultato: "); lcd.print(resultD20); lcd.print("    "); }
      }
      else if (currentScreen == 5) { 
        lcd.setCursor(0, 0); lcd.print("Lancio Dado:D100");
        lcd.setCursor(0, 1);
        if (resultD100 == 0) lcd.print("Premi per tirare"); else { lcd.print("Risultato: "); lcd.print(resultD100); lcd.print("    "); }
      }
    }
  }
}