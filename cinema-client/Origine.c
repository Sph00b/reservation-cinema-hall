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

#include "cinema-client.h"
#include <stdio.h>
#include <Windows.h>
#include <WinBase.h>

void errorhandler(int);

int _tmain(int argc, LPTSTR* argv) {
	int ret;
	if (ret = WinMain(GetModuleHandle(NULL), NULL, *argv, SW_SHOWNORMAL) == 0) {
		errorhandler(GetLastError());
	}
	else if (ret == -1) {
		errorhandler(WSAGetLastError());
	}
#ifdef _DEBUG
	_tprintf(TEXT("\nPROCESS TERMINATED PRESS ENTER TO QUIT\n"));
#endif
	if (getc(stdin)) return 0;	/**/
	return 0;
}

void errorhandler(int e) {
	LPTSTR p_errmsg = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		e,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&p_errmsg,
		0,
		NULL
	);
	_ftprintf(stderr, TEXT("%s\n"), p_errmsg);
	exit(e);
}
