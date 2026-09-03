/*----------------------- C H A N G E L O G -----------------------------------
v1.0 (03/09/2026): Schermata singola con ora (RTC Module) e temperatura (sensore DTH11)
v2.0 (03/09/2026): Navigazione con un pulsante, 5 schermate: Data e ora; temperatura e umidità; 3 schermate D6, D20, D100 aggiunto pulsante per lanciare dado
------------------------------------------------------------------------------
*/
#include <Wire.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>
#include "RTClib.h"

// Inizializzazione LCD
LiquidCrystal lcd(7, 8, 9, 10, 11, 12); 

// Inizializzazione DHT11
int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

// Inizializzazione modulo RTC
RTC_DS1307 rtc;

// Impostazioni pulsanti
const int screenButtonPin = 3;  // Pulsante per cambiare schermata
const int rollButtonPin = 4;    // Pulsante per lanciare i dadi

int screenButtonState = HIGH;             
int lastScreenButtonState = HIGH;         
unsigned long lastScreenDebounceTime = 0;  

int rollButtonState = HIGH;             
int lastRollButtonState = HIGH;         
unsigned long lastRollDebounceTime = 0;  

unsigned long debounceDelay = 50;    

// 0=Data/Ora, 1=Temp/Umid, 2=D6, 3=D20, 4=D100
int currentScreen = 0; 

// Variabili per i risultati dei dadi
int resultD6 = 0;
int resultD20 = 0;
int resultD100 = 0;

// Variabili per il timing dello schermo
unsigned long previousMillis = 0;
const long interval = 1000; 
bool forceUpdate = false; // Forza l'aggiornamento dello schermo quando premiamo un tasto

void setup() {
  lcd.begin(16, 2);
  Wire.begin();
  
  // Imposta i pin dei pulsanti con resistenza di pull-up interna
  pinMode(screenButtonPin, INPUT_PULLUP);
  pinMode(rollButtonPin, INPUT_PULLUP);
  
  // Inizializza la generazione casuale leggendo il rumore dal pin analogico A0
  randomSeed(analogRead(0));

  if (!rtc.begin()) {
    lcd.print("Errore RTC");
    while (1);
  }
  
  // Ricorda di togliere i commenti (//) se devi sincronizzare l'ora!
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); 
}

void loop() {
  unsigned long currentMillis = millis();

  // --- 1. GESTIONE PULSANTE CAMBIO SCHERMATA ---
  int readingScreen = digitalRead(screenButtonPin);
  if (readingScreen != lastScreenButtonState) {
    lastScreenDebounceTime = currentMillis;
  }
  if ((currentMillis - lastScreenDebounceTime) > debounceDelay) {
    if (readingScreen != screenButtonState) {
      screenButtonState = readingScreen;
      
      if (screenButtonState == LOW) {
        currentScreen++;
        if (currentScreen > 4) {
          currentScreen = 0; // Torna alla prima schermata
        }
        lcd.clear(); 
        forceUpdate = true; // Aggiorna la grafica all'istante
      }
    }
  }
  lastScreenButtonState = readingScreen;

  // --- 2. GESTIONE PULSANTE LANCIO DADI ---
  int readingRoll = digitalRead(rollButtonPin);
  if (readingRoll != lastRollButtonState) {
    lastRollDebounceTime = currentMillis;
  }
  if ((currentMillis - lastRollDebounceTime) > debounceDelay) {
    if (readingRoll != rollButtonState) {
      rollButtonState = readingRoll;
      
      // Se viene premuto il pulsante "Roll"
      if (rollButtonState == LOW) {
        // Lancia il dado in base alla schermata corrente
        if (currentScreen == 2) {
          resultD6 = random(1, 7); // Genera da 1 a 6
        } 
        else if (currentScreen == 3) {
          resultD20 = random(1, 21); // Genera da 1 a 20
        } 
        else if (currentScreen == 4) {
          resultD100 = random(1, 101); // Genera da 1 a 100
        }
        forceUpdate = true; // Mostra subito il risultato
      }
    }
  }
  lastRollButtonState = readingRoll;

  // --- 3. AGGIORNAMENTO DEL DISPLAY ---
  if (currentMillis - previousMillis >= interval || forceUpdate || currentMillis < 100) {
    previousMillis = currentMillis;
    forceUpdate = false;
    
    // SCHERMATA 0: DATA E ORA
    if (currentScreen == 0) {
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
    // SCHERMATA 1: TEMPERATURA E UMIDITÀ
    else if (currentScreen == 1) {
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
    // SCHERMATA 2: DADO D6
    else if (currentScreen == 2) {
      lcd.setCursor(0, 0);
      lcd.print("Lancio Dado: D6 ");
      lcd.setCursor(0, 1);
      if (resultD6 == 0) lcd.print("Premi per tirare");
      else {
        lcd.print("Risultato: ");
        lcd.print(resultD6);
        lcd.print("     ");
      }
    }
    // SCHERMATA 3: DADO D20
    else if (currentScreen == 3) {
      lcd.setCursor(0, 0);
      lcd.print("Lancio Dado: D20");
      lcd.setCursor(0, 1);
      if (resultD20 == 0) lcd.print("Premi per tirare");
      else {
        lcd.print("Risultato: ");
        lcd.print(resultD20);
        lcd.print("     ");
      }
    }
    // SCHERMATA 4: DADO D100
    else if (currentScreen == 4) {
      lcd.setCursor(0, 0);
      lcd.print("Lancio Dado:D100");
      lcd.setCursor(0, 1);
      if (resultD100 == 0) lcd.print("Premi per tirare");
      else {
        lcd.print("Risultato: ");
        lcd.print(resultD100);
        lcd.print("     ");
      }
    }
  }
}