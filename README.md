# Bikaya - Consegna 0
Per compilare il sorgente occorre eseguire il comando make nella cartella che contiene questo file README, specificando l'architettura di destinazione come di seguito.
```python
make umps       #per architettura umps 
make uarm       #per architettura uarm con il passaggio elf2uarm
make uarm-new   #per architettura uarm senza il passaggio elf2uarm
make clean      #per rimuovere tutti i file di output
```
I file di output si trovano nelle cartelle ```build/umps```  e ```build/uarm``` che verranno generate opportunamente dall'esecuzione del comando make.

Per eseguire il codice è sufficiente avviare il rispettivo emulatore e impostare nella configurazione un file di output per il device Printer 0 e abilitarlo.
Il programma legge la prima riga di input dal terminale 0 e ne stampa il contenuto nel file assegnato al device.
