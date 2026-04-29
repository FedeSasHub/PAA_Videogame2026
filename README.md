# PAA - Videogame Strategico a Turni 3D (2025/2026)

## Descrizione del Progetto
Questo progetto è un gioco strategico a turni 1vs1 (Umano contro AI) sviluppato in **Unreal Engine 5.6** utilizzando esclusivamente **C++** per la logica di gioco e **UMG (Widget)** per l'interfaccia grafica. Il gioco prevede il controllo di unità su una griglia 25x25 generata proceduralmente, con l'obiettivo di conquistare torri o eliminare le unità nemiche.

## Requisiti Implementati (Checklist)
Come richiesto dalle specifiche d'esame, di seguito la lista dei requisiti e il relativo stato di implementazione:

1.  **Compilazione e Struttura**: OK. Il progetto compila correttamente, il codice è strutturato con l'uso di ereditarietà e polimorfismo.
2.  **Griglia di Gioco**: OK. Griglia 25x25 interamente visibile con visuale dall'alto.
3.  **Posizionamento Unità e Torri**: OK. Meccanismo di schieramento alternato e piazzamento simmetrico delle torri.
4.  **AI con A-STAR**: OK. L'intelligenza artificiale utilizza l'algoritmo A* per il calcolo dei percorsi ottimali.
5.  **Turni e Vittoria**: OK. Gestione dei turni alternati e condizioni di vittoria (controllo torri).
6.  **Interfaccia Grafica (HUD)**: OK. Visualizzazione turni, HP delle unità e stato delle torri.
7.  **Range di Movimento**: OK. Evidenziazione dinamica delle celle raggiungibili e dei bersagli a tiro.
8.  **Danno da Contrattacco**: OK. Implementazione delle regole specifiche per il contrattacco tra Sniper e Brawler.
9.  **Storico Mosse**: OK. Lista testuale delle azioni eseguite durante la partita (formattata per fazione e unità).
10. **Euristica Ottimizzata (diversa da A*)**: NON IMPLEMENTATO.

## Guida al Gioco

### Controlli
- **Click Sinistro**: Seleziona un'unità alleata per vederne il raggio d'azione o una cella vuota per deselezionare.
- **Click Destro**: Muovi l'unità selezionata o attacca un nemico nel raggio d'azione.
- **Tasto "SALTA TURNO"**: Permette di terminare il turno dell'unità selezionata senza compiere azioni.

### Unità e Simboli
- ■ **Brawler (Cubo)**: Alta vita (40 HP), attacco a corto raggio (1 cella), elevata mobilità.
- ● **Sniper (Cilindro)**: Vita ridotta (20 HP), attacco a lunga distanza (10 celle), mobilità limitata.

### Legenda Colori Griglia
- **Giallo**: Area di movimento possibile.
- **Rosso**: Area di attacco (bersagli nemici a tiro).
- **Arancione**: Cella valida sia per il movimento che per l'attacco.

## Dettagli Tecnici
- **Mappa Procedurale**: Generata tramite algoritmo **Perlin Noise** con seed randomico per garantire mappe diverse a ogni avvio. L'acqua (livello 0) funge da ostacolo al movimento.
- **Sistema di Conquista**: Le torri vengono catturate per prossimità (area 5x5). Una torre è conquistata se presidiata da un solo giocatore, altrimenti entra in stato di "Contesa".
- **Meccanismo di Respawn**: Le unità sconfitte vengono rimosse e rigenerate istantaneamente nelle rispettive zone di schieramento iniziali con vita piena.
- **Pathfinding**: L'AI calcola il costo di movimento in base all'elevazione (Costo 2 per la salita, 1 per piano/discesa).

## Aggiunte o Modifiche rispetto alle Specifiche
- **Nomenclatura Storico Mosse**: Per indicare il giocatore umano è stata scelta la sigla "TU" invece di "HP" (come suggerito nelle specifiche), poiché "HP" avrebbe generato confusione visiva e concettuale con l'acronimo dei Punti Vita (Health Points).
- **Tasto "SALTA TURNO"**: Aggiunta la possibilità di terminare volontariamente il turno di un'unità senza dover per forza compiere un'azione offensiva o di movimento.
- **Storico Mosse Migliorato**: Il log testuale è stato arricchito con formattazione a colori per distinguere intuitivamente a chi appartiene l'unità che ha agito. Inoltre, per facilitare la lettura, le mosse più recenti vengono inserite in cima alla lista (scalando le altre verso il basso).
- **Reset Automatico**: Al termine della partita, il messaggio di vittoria o sconfitta è accompagnato da un timer di 10 secondi, concluso il quale il livello si riavvia automaticamente per una nuova sfida.
- **Tutorial a Schermo**: Integrato un piccolo pannello visivo riassuntivo nell'HUD con i controlli base e la legenda dei colori per migliorare l'esperienza utente.

## Eventuali Problemi (Troubleshooting)
In rari casi (circa 1 volta su 10), il sistema di Live Coding di Unreal Engine potrebbe non aggiornare correttamente la cache di tutti i file. In questi frangenti, nonostante appaia a schermo il messaggio di "Compilazione Completata" con successo, il gioco si avvia come se fosse stato compilato "a metà", ignorando alcune funzioni o saltando completamente alcune modifiche recenti ai file.
Se si riscontra questo comportamento anomalo, è possibile risolvere forzando una ricompilazione pulita:
1. Chiudere l'editor di Unreal Engine.
2. Avviare una ricompilazione completa della soluzione direttamente da Visual Studio (`Ctrl + Shift + B`).
3. Riaprire Unreal Engine ed effettuare una nuova compilazione tramite Live Coding (`Ctrl + Alt + F11`).
A questo punto il progetto riprenderà a funzionare correttamente con tutto il codice aggiornato.

---
**Studente**: Savio Federico / 5609837  
**Corso**: Progettazione e analisi di algoritmi
