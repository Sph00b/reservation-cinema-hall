/*
Autore:									Fabio Buracchi
Data creazione:							06/06/2019
Ultima modifica:						08/06/2019
Specifiche:								Servizio di prenotazione posti per una sala cinematografica client/server.
										Ciascun posto è caratterizzato da un numero di fila, un numero di poltrona,
										e può essere libero o occupato. Il server accetta e processa sequenzialmente
										o in concorrenza le richieste di prenotazione di posti del client.
										Il server e il client risiedono su macchine diverse.

Richieste d'implementazioni future:		
	Lato Client:						Visualizzare mappa dei posti da prenotare (Indicati per fila e poltrona)
										Inviare al server l'elenco dei posti che si intende prenotare
										Attendere dal server la conferma di effettuata prenotazione ed un codice univoco di prenotazione
										Disdire una prenotazione per cui si possiede un codice.
	Lato Server:
Implementazioni:						Server lato Unix Client lato Windows
										Scelta se utilizzaere lista di liste o matrice (bloccaggio della memoria) [liste di liste di puntaori?]
*/

#include <stdio.h>
#include "WndClient.h"

int main(int argc, char** argv) {
	printf("\nPROCESS TERMINATED WITH STATUS %d\n", wWinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOWNORMAL));
	getch();
	return 0;
}
