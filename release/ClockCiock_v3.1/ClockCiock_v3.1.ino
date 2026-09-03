/*----------------------- C H A N G E L O G -----------------------------------
v1.0 (03/09/2026): Schermata singola con ora (RTC Module) e temperatura (sensore DTH11)
v2.0 (03/09/2026): Navigazione con un pulsante, 5 schermate: Data e ora; temperatura e umidità; 3 schermate D6, D20, D100 aggiunto pulsante per lanciare dado
v3.0 (03/09/2026): Aggiunta schermata di avvio e schermata Idle con robot
v3.1 (04/09/2026): Aggiunto buzzer passivo e sveglia: jingle per lancio dei dadi e suono per la sveglia
------------------------------------------------------------------------------
*/
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>
#include "RTClib.h"

// Note musicali (frequenze in Hz) per il cicalino passivo[cite: 1]
#define NOTE_C5  523
#define NOTE_E5  659
#define NOTE_G5  784
#define NOTE_C6  1047

LiquidCrystal lcd(7, 8, 9, 10, 11, 12); 

byte quadrato[8] = {
  B11111, B11111, B11111, B11111,
  B11111, B11111, B11111, B11111
};

int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

RTC_DS1307 rtc;

// Pin componenti
const int screenButtonPin = 3;  
const int rollButtonPin = 4;    
const int buzzerPin = 5; // Pin per il Passive Buzzer

// Impostazioni Sveglia (modifica questi valori per provarla)
const int alarmHour = 23;
const int alarmMinute = 44;
bool alarmTriggered = false;
bool alarmDismissedToday = false;

int screenButtonState = HIGH;             
int lastScreenButtonState = HIGH;         
unsigned long lastScreenDebounceTime = 0;  

int rollButtonState = HIGH;             
int lastRollButtonState = HIGH;         
unsigned long lastRollDebounceTime = 0;  

unsigned long debounceDelay = 50;    

int currentScreen = 0; 
int resultD6 = 0, resultD20 = 0, resultD100 = 0;

unsigned long previousMillis = 0;
const long interval = 1000; 
bool forceUpdate = false; 

int eyeFrame = 0;
unsigned long lastEyeUpdate = 0;
const unsigned long eyeInterval = 1000;

// Funzione per suonare il Jingle quando tiri un dado
void playRollJingle() {
  tone(buzzerPin, NOTE_C5, 80); delay(100);
  tone(buzzerPin, NOTE_E5, 80); delay(100);
  tone(buzzerPin, NOTE_G5, 80); delay(100);
  tone(buzzerPin, NOTE_C6, 200); delay(220);
  noTone(buzzerPin);
}

void setup() {
  lcd.begin(16, 2);
  lcd.createChar(1, quadrato);
  Wire.begin();
  
  pinMode(buzzerPin, OUTPUT);
  pinMode(screenButtonPin, INPUT_PULLUP);
  pinMode(rollButtonPin, INPUT_PULLUP);
  randomSeed(analogRead(0));

  lcd.setCursor(0, 0);
  lcd.print("   ClockCiock    ");
  lcd.setCursor(0, 1);
  lcd.print("by Ciock ver 3.1  "); 
  delay(3000); 
  lcd.clear(); 

  if (!rtc.begin()) {
    lcd.print("Errore RTC");
    while (1);
  }
  
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
  forceUpdate = true;
}

void loop() {
  unsigned long currentMillis = millis();
  DateTime now = rtc.now();

  // --- CONTROLLO SVEGLIA ---
  if (now.hour() == alarmHour && now.minute() == alarmMinute && !alarmDismissedToday) {
    alarmTriggered = true;
  }
  // Resetta il flag al cambio di minuto per il giorno successivo
  if (now.minute() != alarmMinute) {
    alarmDismissedToday = false;
  }

  // --- GESTIONE ALLARME ATTIVO ---
  if (alarmTriggered) {
    lcd.setCursor(0, 0);
    lcd.print("*** SVEGLIA! ***");
    lcd.setCursor(0, 1);
    lcd.print("Premi un tasto  ");
    
    // Suono intermittente di sveglia
    tone(buzzerPin, 1000, 200);
    delay(300);
    
    // Se premi un tasto qualsiasi, spegni la sveglia
    if (digitalRead(screenButtonPin) == LOW || digitalRead(rollButtonPin) == LOW) {
      alarmTriggered = false;
      alarmDismissedToday = true;
      noTone(buzzerPin);
      lcd.clear();
      forceUpdate = true;
    }
    return; // Blocca il resto del loop finché non viene spenta
  }

  // --- PULSANTE CAMBIO SCHERMATA ---
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

  // --- PULSANTE LANCIO DADI (Con Jingle) ---
  int readingRoll = digitalRead(rollButtonPin);
  if (readingRoll != lastRollButtonState) {
    lastRollDebounceTime = currentMillis;
  }
  if ((currentMillis - lastRollDebounceTime) > debounceDelay) {
    if (readingRoll != rollButtonState) {
      rollButtonState = readingRoll;
      if (rollButtonState == LOW) {
        if (currentScreen == 3) {
          resultD6 = random(1, 7); 
          playRollJingle();
        } 
        else if (currentScreen == 4) {
          resultD20 = random(1, 21); 
          playRollJingle();
        } 
        else if (currentScreen == 5) {
          resultD100 = random(1, 101); 
          playRollJingle();
        } 
        forceUpdate = true; 
      }
    }
  }
  lastRollButtonState = readingRoll;

  // --- AGGIORNAMENTO DEL DISPLAY ---
  if (currentScreen == 0) {
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
  } else {
    if (currentMillis - previousMillis >= interval || forceUpdate || currentMillis < 100) {
      previousMillis = currentMillis;
      forceUpdate = false;
      
      if (currentScreen == 1) { 
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
        if (resultD6 == 0) lcd.print("Premi per tirare"); 
        else { lcd.print("Risultato: "); lcd.print(resultD6); lcd.print("    "); }
      }
      else if (currentScreen == 4) { 
        lcd.setCursor(0, 0); lcd.print("Lancio Dado: D20");
        lcd.setCursor(0, 1);
        if (resultD20 == 0) lcd.print("Premi per tirare"); 
        else { lcd.print("Risultato: "); lcd.print(resultD20); lcd.print("    "); }
      }
      else if (currentScreen == 5) { 
        lcd.setCursor(0, 0); lcd.print("Lancio Dado:D100");
        lcd.setCursor(0, 1);
        if (resultD100 == 0) lcd.print("Premi per tirare"); 
        else { lcd.print("Risultato: "); lcd.print(resultD100); lcd.print("    "); }
      }
    }
  }
}