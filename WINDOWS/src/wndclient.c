#include "wndclient.h"
#include "framework.h"

#ifdef _DEBUG
#include <stdio.h>
#include <tchar.h>
#endif

//	Variabili globali:
HINSTANCE hInst;		// Istanza corrente
LPTSTR szTitle;			// Testo della barra del titolo
LPTSTR szWindowClass;	// Nome della classe della finestra principale

LPTSTR pID;		//ID codice di prenotazione

// Dichiarazioni con prototipo di funzioni incluse in questo modulo di codice:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);


int APIENTRY WinMain(_In_ HINSTANCE hInstance, 
					_In_opt_ HINSTANCE hPrevInstance, 
					_In_ LPTSTR lpCmdLine, 
					_In_ int nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	pID = savLoad();

	// Inizializzare le stringhe globali
	szTitle = TEXT("Prenotazione");
	szWindowClass = TEXT("generic_class");
	if (!MyRegisterClass(hInstance)) {
		return 0;
	}

#ifdef _DEBUG
	_tprintf(TEXT("WINDOW CLASS SUCCESFULLY REGITRATED\n"));
#endif

	// Eseguire l'inizializzazione dall'applicazione:
	if (!InitInstance(hInstance, nCmdShow)) {
		return FALSE;
	}

#ifdef _DEBUG
	_tprintf(TEXT("WINDOW SUCCESFULLY INITIALIZATED\n"));
#endif
	MSG msg;

	// Ciclo di messaggi principale:
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

#ifdef _DEBUG
	_tprintf(TEXT("RECEIVED DESTROY OR INVALID MESSAGE\n"));
#endif

	return (int)msg.wParam;
}

//
//  FUNZIONE: MyRegisterClass()
//
//  SCOPO: Registra la classe di finestre.
//
ATOM MyRegisterClass(HINSTANCE hInstance) {

	WNDCLASS wndc;
	
	wndc.style = CS_HREDRAW | CS_VREDRAW;
	wndc.lpfnWndProc = WndProc;
	wndc.cbClsExtra = 0;
	wndc.cbWndExtra = 0;
	wndc.hInstance = hInstance;
	wndc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndc.lpszMenuName = NULL;
	wndc.lpszClassName = (LPCTSTR)szWindowClass;
	
	return RegisterClass(&wndc);
}

//
//   FUNZIONE: InitInstance(HINSTANCE, int)
//
//   SCOPO: Salva l'handle di istanza e crea la finestra principale
//
//   COMMENTI:
//
//        In questa funzione l'handle di istanza viene salvato in una variabile globale e
//        viene creata e visualizzata la finestra principale del programma.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
	hInst = hInstance;	//	Archivia l'handle di istanza nella variabile globale

	HWND hWnd = CreateWindow(
		(LPCTSTR)szWindowClass,										//	CLASS
		(LPCTSTR)szTitle,											//	TITLE
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,		//	STYLE
		CW_USEDEFAULT,												//	X
		CW_USEDEFAULT,												//	Y
		650,														//	WIDTH
		420,														//	HEIGHT
		NULL,														//	NO PARENT WINDOW
		NULL,														//	NO MENU
		hInstance,													//	INSTANCE
		NULL														//	NO PARAMETER
	);

	if (!hWnd) {
		return FALSE;
	}


#ifdef _DEBUG
	_tprintf(TEXT("WINDOW SUCCESFULLY CREATED\n"));
#endif

	//	Create Button
	HWND hButton1 = CreateWindow(
		"BUTTON",										//	PREDEFINED CLASS 
		"Prenota",										//	Button text 
		WS_TABSTOP | WS_CHILD,							//	Styles 
		210,											//	x position 
		300,											//	y position 
		210,											//	Button width
		60,												//	Button height
		hWnd,											//	PARENT WINDOW window
		NULL,											//	NO MENU
		hInstance,										//	INSTANCE
		NULL);											//	NO PARAMETER

	if (!hButton1) {
		return FALSE;
	}

	//	Create Button
	HWND hButton2 = CreateWindow(
		"BUTTON",												//	PREDEFINED CLASS 
		"Modifica prenotazione",								//	Button text 
		WS_CHILD | WS_VISIBLE,									//	Styles 
		70,														//	x position 
		300,													//	y position 
		210,													//	Button width
		60,														//	Button height
		hWnd,													//	PARENT WINDOW window
		NULL,													//	NO MENU
		hInstance,												//	INSTANCE
		NULL);													//	NO PARAMETER

	if (!hButton2) {
		return FALSE;
	}

	//	Create Button
	HWND hButton3 = CreateWindow(
		"BUTTON",												//	PREDEFINED CLASS 
		"Elimina prenotazione",									//	Button text 
		WS_VISIBLE | WS_CHILD,									//	Styles 
		350,													//	x position 
		300,													//	y position 
		210,													//	Button width
		60,														//	Button height
		hWnd,													//	PARENT WINDOW window
		NULL,													//	NO MENU
		hInstance,												//	INSTANCE
		NULL);													//	NO PARAMETER

	if (!hButton3) {
		return FALSE;
	}

#ifdef _DEBUG
	_tprintf(TEXT("BUTTONS SUCCESFULLY CREATED\n"));
#endif

	//	Create Textxbox

	HWND hEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"Static",
		pID,
		WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		210,
		20,
		210,
		20,
		hWnd,													//	PARENT WINDOW window
		NULL,
		hInstance,
		NULL);

	if (!hEdit) {
		return FALSE;
	}

#ifdef _DEBUG
	_tprintf(TEXT("TEXTBOX SUCCESFULLY CREATED\n"));
#endif

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
#ifdef _DEBUG
	_tprintf(TEXT("WINDOW SHOWED\n"));
#endif
	return TRUE;
}

//
//  FUNZIONE: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  SCOPO: Elabora i messaggi per la finestra principale.
//
//  WM_COMMAND  - elabora il menu dell'applicazione
//  WM_PAINT    - Disegna la finestra principale
//  WM_DESTROY  - inserisce un messaggio di uscita e restituisce un risultato
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE:
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED CREATE MESSAGE\n"));
#endif
		break;

	case WM_SIZE:
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED SIZE MESSAGE\n"));
#endif
		break;

	case WM_PAINT:
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED PAINT MESSAGE\n"));
#endif
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			//	TODO: Aggiungere qui il codice di disegno che usa HDC...
			EndPaint(hWnd, &ps);
		}
		break;

	case WM_DESTROY:
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED DESTROY MESSAGE\n"));
#endif
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Gestore di messaggi per la finestra Informazioni su.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
