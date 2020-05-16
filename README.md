# Bikaya - Consegna 2.0
Per compilare il sorgente occorre eseguire il comando make nella cartella che contiene questo file README, specificando l'architettura di destinazione come di seguito.
```python
make umps       #per architettura umps 
make uarm       #per architettura uarm con il passaggio elf2uarm
make uarm-new   #per architettura uarm senza il passaggio elf2uarm
make clean      #per rimuovere tutti i file di output
```
I file di output si trovano nelle cartelle ```build/umps```  e ```build/uarm``` che verranno generate opportunamente dall'esecuzione del comando make.

Per eseguire il codice è sufficiente avviare il rispettivo emulatore e aprire il terminale 0.
Il programma esegue la funzione di test nel file ```p2test_bikaya_v0.2.c``` il cui output compare sul terminale 0. 

Inoltre è possibile abilitare il terminale 7 nella configurazione dell'emulatore per ricevere un output di debug relativo ad errori che possono presentarsi durante l'esecuzione del kernel.

