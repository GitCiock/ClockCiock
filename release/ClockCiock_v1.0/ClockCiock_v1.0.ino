/*----------------------- C H A N G E L O G -----------------------------------
v1.0 (03/09/2026): Schermata singola con ora (RTC Module) e temperatura (sensore DTH11)
------------------------------------------------------------------------------
*/

#include <Wire.h>
#include <LiquidCrystal.h>
#include <SimpleDHT.h>
#include "RTClib.h"

// Inizializzazione LCD con i pin: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Inizializzazione Sensore DHT11 sul pin digitale 2
int pinDHT11 = 2;
SimpleDHT11 dht11(pinDHT11);

// Inizializzazione modulo RTC
RTC_DS1307 rtc;

void setup() {
  lcd.begin(16, 2);
  Wire.begin();
  
  if (!rtc.begin()) {
    lcd.print("Errore modulo RTC");
    while (1);
  }
  
  // Utilizzare solo la prima volta per sincrinizzarlo
    //rtc.adjust(DateTime(2026, 9, 3, 22, 36, 45));
  
}

void loop() {
  byte temperature = 0;
  byte humidity = 0;
  
  // Lettura dati dal sensore DHT11
  dht11.read(&temperature, &humidity, NULL);
  
  // Lettura ora dal modulo RTC
  DateTime now = rtc.now();
  
  // Stampa dell'ora sulla prima riga
  lcd.setCursor(0, 0);
  lcd.print("Ora: ");
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  
  // Stampa dei dati meteo sulla seconda riga
  lcd.setCursor(0, 1);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.print("C U:");
  lcd.print(humidity);
  lcd.print("%    ");
  
  // Aggiorna lo schermo ogni 1 second0
  delay(1000);
}