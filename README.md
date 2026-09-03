# ⏰ ClockCiock 🤖
> *Un orologio multifunzione da scrivania e desktop companion interattivo basato su Arduino Mega 2560.*
[![Changelog](https://img.shields.io/badge/Changelog-Aggiornamenti-blue?style=flat&logo=git&logoColor=white)](CHANGELOG.md)

---

### 📖 Panoramica
**ClockCiock** è un orologio multifunzione da scrivania e desktop companion interattivo basato su Arduino Mega 2560, progettato per unire utilità quotidiana, monitoraggio ambientale e intrattenimento.

Il firmware gestisce un'interfaccia a più schermate navigabile tramite pulsanti fisici su un display LCD1602, accompagnata da feedback sonoro su cicalino passivo.

---
### ✨ Funzionalità Principali

* 🤖 **Desktop Companion (Idle Mode)**  
  Animazione interattiva con occhi personalizzati a matrice di pixel sul display LCD che guardano in varie direzioni e sbattono le palpebre.
* 🕒 **Orologio & Sveglia Programmabile**  
  Lettura precisa e continua del tempo tramite modulo RTC (DS1307) dedicato con allarme sonoro dedicato.
* 🌡️ **Stazione Meteo Integrata**  
  Monitoraggio in tempo reale di temperatura ambiente (°C) e umidità relativa (%) tramite sensore DHT11.
* 🎲 **Lancio Dadi (D6, D20, D100)**  
  Generazione di numeri casuali non pseudocasuali (agganciata al rumore analogico) completa di fanfara audio di vittoria a ogni roll.

---
### 🛠️ Hardware Utilizzato

* 🧠 **Microcontrollore** | Elegoo Mega 2560 R3 
* 🖥️ **Display** | LCD1602 con potenziometro contrasto 
* ⏱️ **RTC Module** | DS1307 con batteria a bottone 
* 🌦️ **Sensore Meteo** | DHT11 (Umidità e Temperatura) 
* 🔊 **Audio** | Cicalino Passivo (*Passive Buzzer*) 
* 🔘 **Input** | 2x Pulsanti tattili 

---

### 📦 Librerie Esterne

Assicurati di aver installato le seguenti librerie prima di compilare:

* 📚 `Wire.h` *(Inclusa nel core Arduino)*
* 📚 `LiquidCrystal.h` *(Inclusa nel core Arduino)*
* 📚 `RTClib` *(di Adafruit)*
* 📚 `SimpleDHT`
