/*<
Autore:					Fabio Buracchi
Data creazione:			06/06/2019
Ultima modifica:		11/06/2019
Specifiche:				Servizio di prenotazione posti per una sala cinematografica client/server.
						Ciascun posto è caratterizzato da un numero di fila, un numero di poltrona,
						e può essere libero o occupato. Il server accetta e processa sequenzialmente
						o in concorrenza le richieste di prenotazione di posti del client.
						Il server e il client risiedono su macchine diverse.

Pending request:		
	Lato Client:		Visualizzare mappa dei posti da prenotare (Indicati per fila e poltrona)
						Inviare al server l'elenco dei posti che si intende prenotare
						Attendere dal server la conferma di effettuata prenotazione ed un codice univoco di prenotazione
						Disdire una prenotazione per cui si possiede un codice.
	Lato Server:
Implementazioni:		Server lato Unix Client lato Windows
*/

#include <stdio.h>
#include <Windows.h>
#include <WinBase.h>
#include "wndclient.h"
#include "savefile.h"
#include "connection.h"

int _tmain(int argc, LPTSTR* argv) {
	if (!WinMain(GetModuleHandle(NULL), NULL, argv, SW_SHOWNORMAL)) {
		errorhandler();
	}
#ifdef _DEBUG
	_tprintf(TEXT("\nPROCESS TERMINATED PRESS ENTER TO QUIT\n"));
#endif
	if (getc(stdin)) return 0;	/**/
	return 0;
}
