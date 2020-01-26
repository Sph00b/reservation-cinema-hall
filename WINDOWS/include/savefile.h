#include <stdio.h>
#include <Windows.h>

//	Variabili globali:

HANDLE hSaveFile;		// File di salvataggio

//
//	FUNZIONE: InitSavefile(HINSTANCE, int)
//
//	SCOPO: Crea e salva l'handle del file di salvataggio nella varaibile globale
//
//	RETURN:	0 in caso di successo 1 in caso di errore
//
//	COMMENTI:
//
//		In questa funzione viene aperta la sessione al file di salvataggio, se
//		il file non esiste questo viene creato insieme alla sua directory d'appartenenza.
//
extern int InitSavefile(LPCTSTR);
//
//	FUNZIONE: savStore(LPCTSTR)
//
//	SCOPO: Sovrascrive il file di salvataggio
//
extern int savStore(LPCTSTR);
//
//	FUNZIONE: InitInstance(HINSTANCE, int)
//
//	SCOPO: Restituisce i dati del file di salvataggio
//
//
//	RETURN:	0 in caso di successo 1 in caso di errore
//
extern LPTSTR savLoad();
