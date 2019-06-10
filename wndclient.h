#pragma once
#include <Windows.h>
#include <tchar.h>

//	Variabili globali:

HINSTANCE hInst;			// istanza corrente
LPTSTR szTitle;				// Testo della barra del titolo
LPTSTR szWindowClass;		// nome della classe della finestra principale

//
//	FUNZIONE: MyRegisterClass(HINSTANCE)
//
//	SCOPO: Registra la classe di finestre.
//
extern BOOL MyRegisterClass(HINSTANCE);
//
//	FUNZIONE: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//	SCOPO: Elabora i messaggi per la finestra principale.
//
extern LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
//
//	FUNZIONE: InitInstance(HINSTANCE, int)
//
//	SCOPO: Salva l'handle di istanza e crea la finestra principale
//
//	COMMENTI:
//
//		In questa funzione l'handle di istanza viene salvato in una variabile globale e
//		viene creata e visualizzata la finestra principale del programma.
//
extern BOOL InitInstance(HINSTANCE, int);
//
//	FUNZIONE: wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWTTR, _In_ int);
//
//	SCOPO: Punto di ingresso dell'applicazione.
//
extern int APIENTRY WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ int);
