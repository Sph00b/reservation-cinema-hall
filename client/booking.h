#include <stdio.h>
#include <Windows.h>

//	Variabili globali:

typedef HANDLE HBOOKING;

//
//	FUNZIONE: InitializeBooking(LPCTSTR)
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
HBOOKING InitializeBooking(LPCTSTR);

//
//	FUNZIONE: SetBooking(HBOOKING, LPCTSTR)
//
//	SCOPO: Aggiorna la prenotazione
//
//	RETURN:	TRUE in caso di successo FALSE in caso di errore
//
BOOL SetBooking(HBOOKING, LPCTSTR);
//
//	FUNZIONE: GetBooking(HBOOKING)
//
//	SCOPO: Restituisce la stringa contenente il codice di prenotazione
//
//	RETURN:	la stringa in caso di successo NULL in caso di errore
//
LPTSTR GetBooking(HBOOKING);
