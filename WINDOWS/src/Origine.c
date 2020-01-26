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

#include "connection.h"
#include "savefile.h"
#include "wndclient.h"
#include <stdio.h>
#include <Windows.h>
#include <WinBase.h>

void connection_example();
void errorhandler(int);

int _tmain(int argc, LPTSTR* argv) {
	connection_example();
	if (!WinMain(GetModuleHandle(NULL), NULL, argv, SW_SHOWNORMAL)) {
		errorhandler(GetLastError());
	}
#ifdef _DEBUG
	_tprintf(TEXT("\nPROCESS TERMINATED PRESS ENTER TO QUIT\n"));
#endif
	if (getc(stdin)) return 0;	/**/
	return 0;
}

void connection_example() {
	connection_t cntn;
	LPTSTR buffer;
	if (connection_init(&cntn, TEXT("127.0.0.1"), 55555)) {
		errorhandler(WSAGetLastError());
	}
	if (connetcion_connect(&cntn)) {
		errorhandler(WSAGetLastError());
	}
	if ((buffer = (TCHAR*)malloc(sizeof(TCHAR) * 64)) == NULL) {
		errorhandler(WSAGetLastError());
	}
	memset(buffer, 0, 64);
	printf("inserire la stringa da spedire: ");
	fgets(buffer, 64, stdin);
	buffer[strlen(buffer) - 1] = 0;

	/*	send string to echo server, and retrive response	*/

	if (connection_send(&cntn, buffer) == -1) {
		errorhandler(WSAGetLastError());
	}
	free(buffer);
	while (connection_recv(&cntn, &buffer) == -1) {
		errorhandler(WSAGetLastError());
	}
	/*	output exhoed string	*/

	printf("risposta del server: %s\n", buffer);
	connection_close(&cntn);
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
