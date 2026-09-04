/*----------------------- C H A N G E L O G -----------------------------------
v1.0 (03/09/2026): Schermata singola con ora (RTC Module) e temperatura (sensore DTH11)
v2.0 (03/09/2026): Navigazione con un pulsante, 5 schermate: Data e ora; temperatura e umidità; 3 schermate D6, D20, D100 aggiunto pulsante per lanciare dado
v3.0 (03/09/2026): Aggiunta schermata di avvio e schermata Idle con robot
v3.1 (04/09/2026): Aggiunto buzzer passivo e sveglia: jingle per lancio dei dadi e suono per la sveglia
v3.2 (04/09/2026): Giorno della settimana su RTC, toggle sveglia con pressione 2s (tasto 2) e icona campanella
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

// Nomi dei giorni della settimana (0 = Domenica)
const char daysOfTheWeek[7][4] = {"Dom", "Lun", "Mar", "Mer", "Gio", "Ven", "Sab"};

int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

RTC_DS1307 rtc;

// Pin componenti
const int screenButtonPin = 3;  
const int rollButtonPin = 4;    
const int buzzerPin = 5; 

// Impostazioni Sveglia
const int alarmHour = 23;
const int alarmMinute = 44;
bool alarmEnabled = true;          // Abilita/disabilita sveglia
bool alarmTriggered = false;
bool alarmDismissedToday = false;

// Variabili pressione prolungata pulsante 2
unsigned long rollButtonPressTime = 0;
bool rollButtonHeld = false;
const unsigned long longPressDuration = 1000; // 2 secondi

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
  lcd.createChar(2, campanella);
  Wire.begin();
  
  pinMode(buzzerPin, OUTPUT);
  pinMode(screenButtonPin, INPUT_PULLUP);
  pinMode(rollButtonPin, INPUT_PULLUP);
  randomSeed(analogRead(0));

  lcd.setCursor(0, 0);
  lcd.print("   ClockCiock    ");
  lcd.setCursor(0, 1);
  lcd.print("by Ciock ver 3.2  "); 
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
  if (alarmEnabled && now.hour() == alarmHour && now.minute() == alarmMinute && !alarmDismissedToday) {
    alarmTriggered = true;
  }
  if (now.minute() != alarmMinute) {
    alarmDismissedToday = false;
  }

  // --- GESTIONE ALLARME ATTIVO ---
  if (alarmTriggered) {
    lcd.setCursor(0, 0);
    lcd.print("*** SVEGLIA! ***");
    lcd.setCursor(0, 1);
    lcd.print(" Premi un tasto ");
    
    tone(buzzerPin, 1000, 200);
    delay(300);
    
    if (digitalRead(screenButtonPin) == LOW || digitalRead(rollButtonPin) == LOW) {
      alarmTriggered = false;
      alarmDismissedToday = true;
      noTone(buzzerPin);
      lcd.clear();
      forceUpdate = true;
    }
    return;
  }

  // --- PULSANTE CAMBIO SCHERMATA (Pin D3) ---
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

  // --- PULSANTE 2: GESTIONE LANCIO DADI E ATTIVAZIONE SVEGLIA (Pin D4) ---
  int readingRoll = digitalRead(rollButtonPin);
  if (readingRoll != lastRollButtonState) {
    lastRollDebounceTime = currentMillis;
  }
  if ((currentMillis - lastRollDebounceTime) > debounceDelay) {
    if (readingRoll != rollButtonState) {
      rollButtonState = readingRoll;

      // Tracciamento inizio pressione
      if (rollButtonState == LOW) {
        rollButtonPressTime = currentMillis;
        rollButtonHeld = false;
      } else {
        // Rilascio: se non era stata attivata la pressione lunga ed è una schermata dadi, esegui il roll
        if (!rollButtonHeld) {
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
  }

  // Rilevamento pressione continua di 2 secondi (solo su Schermata Orologio)
  if (rollButtonState == LOW && !rollButtonHeld && currentScreen == 1) {
    if (currentMillis - rollButtonPressTime >= longPressDuration) {
      rollButtonHeld = true;
      alarmEnabled = !alarmEnabled; // Inverte stato sveglia
      
      // Beep di conferma
      tone(buzzerPin, alarmEnabled ? 1500 : 750, 150);
      delay(150);
      noTone(buzzerPin);
      
      forceUpdate = true;
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
        // Riga 0: Giorno settimanale + Data + Campanella
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

        // Mostra la campanella (Slot 2) o pulisce il pixel
        lcd.setCursor(15, 0);
        if (alarmEnabled) {
          lcd.write(byte(2));
        } else {
          lcd.print(" ");
        }

        // Riga 1: Ora
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