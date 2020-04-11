# Bikaya - Consegna 1.5
Per compilare il sorgente occorre eseguire il comando make nella cartella che contiene questo file README, specificando l'architettura di destinazione come di seguito.
```python
make umps       #per architettura umps 
make uarm       #per architettura uarm con il passaggio elf2uarm
make uarm-new   #per architettura uarm senza il passaggio elf2uarm
make clean      #per rimuovere tutti i file di output
```
I file di output si trovano nelle cartelle ```build/umps```  e ```build/uarm``` che verranno generate opportunamente dall'esecuzione del comando make.

Per eseguire il codice è sufficiente avviare il rispettivo emulatore e aprire il terminale 0.
Il programma inizializza il sistema, crea 3 processi che eseguono le funzioni test1, test2 e test contenute in ```p1.5test_bikaya_v0.c``` in modo concorrente le quali stampano l'output al terminale 0.

Inoltre è possibile abilitare il terminale 7 nella configurazione dell'emulatore per ricevere un output di debug relativo all'esecuzione del kernel. Al momento il kernel comunica solamente casi di errore e avvio del processo idle alla conclusione degli altri processi.

