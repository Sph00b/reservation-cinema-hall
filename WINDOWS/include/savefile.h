#include <stdio.h>
#include <Windows.h>
#include <tchar.h>

//	Variabili globali:

HANDLE hSaveFile;		// File di salvataggio

//
//	FUNZIONE: getPath(LPTSTR*, LPCTSTR, LPCTSTR)
//
//	SCOPO: Restituisce il percorso della directory di salvataggio
//
//	COMMENTI:
//
//		Il secondo parametro specifica il path da estrarre dall'environment
//		il terzo il nome della directory
//
extern void getPath(LPTSTR*, LPCTSTR, LPCTSTR);
//
//	FUNZIONE: getSaveFileName(LPTSTR*, LPCTSTR)
//
//	SCOPO: Restituisce il pathname del file di salvataggio
//
extern void getSaveFileName(LPTSTR*, LPCTSTR);
//
//	FUNZIONE: getSavefile(LPCTSTR)
//
//	SCOPO: Restituisce l'handle del file di salvataggio
//
//	COMMENTI:
//
//		In questa funzione viene aperta la sessione al file di salvataggio, se
//		il file non esiste questo viene creato insieme alla sua directory d'appartenenza.
//
extern HANDLE getSavefile(LPCTSTR);
//
//	FUNZIONE: InitSavefile(HINSTANCE, int)
//
//	SCOPO: Crea e salva l'handle del file di salvataggio nella varaibile globale
//
extern BOOL InitSavefile(LPCTSTR);
//
//	FUNZIONE: savStore(LPCTSTR)
//
//	SCOPO: Sovrascrive il file di salvataggio
//
extern BOOL savStore(LPCTSTR);
//
//	FUNZIONE: InitInstance(HINSTANCE, int)
//
//	SCOPO: Restituisce i dati del file di salvataggio
//
extern LPTSTR savLoad();
