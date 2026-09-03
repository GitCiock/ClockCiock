# Changelog - ClockCiock

Tutte le modifiche degne di nota a questo progetto sono documentate in questo file.

Il formato è basato su [Keep a Changelog](https://keepachangelog.com/it/1.0.0/)
e segue il Semantic Versioning.

---

## [v3.1] - 2026-09-04
### Aggiunto
- **Sveglia attiva**: Allarme sonoro e visivo silenziabile tramite la pressione di un tasto qualsiasi.
- **Feedback audio dadi**: Jingle a 4 note riprodotto a ogni tiro di dado (D6, D20, D100).
- Gestione del cicalino passivo (Passive Buzzer) sul pin D5.

---

## [v3.0] - 2026-09-03
### Aggiunto
- **Desktop Companion (Schermata 0)**: Animazione autonoma degli occhi del robot con scansione ciclica dello sguardo.
- Schermata di benvenuto iniziale all'avvio (`ClockCiock by ciock ver 3.0`).
- Sistema di navigazione esteso a 6 schermate totali.

---

## [v2.0] - 2026-09-03
### Aggiunto
- Sistema di navigazione multischermo tramite pulsante fisico con antirimbalzo software (debounce).
- Schermate dedicate al lancio dei dadi RPG: **D6**, **D20** e **D100**.
- Secondo pulsante dedicato esclusivamente al roll dei dadi.
- Sincronizzazione automatica dell'orario con l'host tramite costanti di compilazione (`__DATE__`, `__TIME__`).
- Generazione casuale agganciata al rumore analogico del pin floating (`randomSeed`).
### Corretto
- Risolto il bug grafico dei caratteri residui su LCD tramite padding con spazi bianchi.

---

## [v1.0] - 2026-09-03
### Aggiunto
- Primo prototipo funzionante.
- Lettura data e ora da modulo RTC DS1307.
- Monitoraggio temperatura e umidità da sensore DHT11.
- Visualizzazione combinata dei parametri su display LCD1602 (modalità 4-bit).
