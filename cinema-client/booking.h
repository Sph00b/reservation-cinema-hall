#include <stdio.h>
#include <Windows.h>

//	Variabili globali:

typedef HANDLE HBOOKING;

//
//	FUNZIONE: InitSavefile(HINSTANCE, int)
//
//	SCOPO: Crea e inizializza lo storage
//
//	RETURN:	l'handle dell'oggetto in caso di successo NULL in caso di errore
//
//	COMMENTI:
//
//		In questa funzione viene aperta la sessione al file di salvataggio, se
//		il file non esiste questo viene creato insieme alla sua directory d'appartenenza.
//
extern HBOOKING CreateBooking(LPCTSTR);

//
//	FUNZIONE: savStore(LPCTSTR)
//
//	SCOPO: Sovrascrive la prenotazione
//
//	RETURN:	TRUE in caso di successo FALSE in caso di errore
//
extern BOOL SetBooking(HBOOKING, LPCTSTR);
//
//	FUNZIONE: InitInstance(HINSTANCE, int)
//
//	SCOPO: Restituisce la stringa contenente il codice di prenotazione
//
//	RETURN:	la stringa in caso di successo NULL in caso di errore
//
extern LPTSTR GetBooking(HBOOKING);
