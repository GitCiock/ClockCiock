# ClockCiock
[![Changelog](https://img.shields.io/badge/Changelog-Aggiornamenti-blue?style=flat&logo=git&logoColor=white)](CHANGELOG.md)

ClockCiock è un orologio multifunzione da scrivania e desktop companion interattivo basato su Arduino Mega 2560 (compatibile con Nano V3), progettato per unire utilità quotidiana, monitoraggio ambientale e intrattenimento.

Il firmware gestisce un'interfaccia a più schermate navigabile tramite pulsanti fisici su un display LCD1602, accompagnata da feedback sonoro su cicalino passivo.

Funzionalità Principali:

Desktop Companion (Idle Mode): Animazione interattiva con occhi personalizzati a matrice di pixel sul display LCD.

Data e Ora con Sveglia: Lettura precisa e continua tramite modulo Real Time Clock (DS1307) con allarme sonoro dedicato.

Stazione Meteo: Monitoraggio in tempo reale di temperatura ambiente e umidità dell'aria tramite sensore DHT11.

Lancio Dadi GDR: Generatore di numeri casuali non pseudocasuali (agganciati al rumore analogico) per dadi D6, D20 e D100, completi di jingle sonoro all'estrazione.


Hardware Utilizzato

Microcontrollore: Elegoo Mega 2560 R3 

Display: Modulo LCD 1602 (modalità 4-bit) con potenziometro per contrasto

RTC: Modulo DS1307  con batteria tampone (I2C)

Sensore Ambientale: DHT11 (Temperatura e Umidità)

Audio: Passive Buzzer

Input: 2x Pulsanti tattili (configurazione INPUT_PULLUP con antirimbalzo software)

Librerie Richieste

Wire.h (inclusa nel core Arduino)

LiquidCrystal.h (inclusa nel core Arduino)

RTClib (Adafruit)

SimpleDHT
