#include "wndclient.h"
#include "framework.h"

#ifdef _DEBUG
#include <stdio.h>
#include <tchar.h>
#endif

#define WS_CSTYLE WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX

//	Variabili globali:
HINSTANCE hInst;		// Istanza corrente
LPTSTR szTitle;			// Testo della barra del titolo
LPTSTR szWindowClass;	// Nome della classe della finestra principale

LPTSTR pID;		//ID codice di prenotazione
HWND hButton1;
HWND hButton2;
HWND hButton3;
HWND hEdit;
HWND* hButtonS;	//Pointer to seat buttons rep

// Dichiarazioni con prototipo di funzioni incluse in questo modulo di codice:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				buttonViewMngr();
void errorhandler();

int APIENTRY WinMain(_In_ HINSTANCE hInstance, 
					_In_opt_ HINSTANCE hPrevInstance, 
					_In_ LPTSTR lpCmdLine, 
					_In_ int nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	
	if (InitSavefile(TEXT("PrenotazioneCinema"))) {
		errorhandler();
	}

	// Inizializzare le stringhe globali
	szTitle = TEXT("Prenotazione");
	szWindowClass = TEXT("generic_class");
	pID = savLoad();
	//retrive number of seats and rows
	hButtonS = (HWND*)malloc(10 * sizeof(HWND));
	if (!MyRegisterClass(hInstance)) {
		return 0;
	}

#ifdef _DEBUG
	_tprintf(TEXT("WINDOW CLASS REGITRATED\n"));
#endif

	// Eseguire l'inizializzazione dall'applicazione:
	if (!InitInstance(hInstance, nCmdShow)) {
		return 0;
	}

#ifdef _DEBUG
	_tprintf(TEXT("WINDOW INITIALIZATED\n"));
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
	hButton1 = CreateWindow(
		TEXT("BUTTON"),					//	PREDEFINED CLASS 
		TEXT("Prenota"),				//	Button text 
		WS_CHILD,						//	Styles 
		210, 300,						//	x,y position
		210, 60,						//	w,h size
		hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
		NULL							//	PARAMETER
	);
	if (!hButton1)
		return FALSE;

	//	Create Button
	hButton2 = CreateWindow(
		TEXT("BUTTON"),					//	PREDEFINED CLASS 
		TEXT("Modifica prenotazione"),	//	Button text 
		WS_CHILD,						//	Styles 
		70, 300,						//	x,y POSITION
		210, 60,						//	w,h SIZE
		hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
		NULL							//	PARAMETER
	);
	if (!hButton2)
		return FALSE;

	//	Create Button
	hButton3 = CreateWindow(
		TEXT("BUTTON"),					//	PREDEFINED CLASS
		TEXT("Elimina prenotazione"),	//	Button text 
		WS_CHILD,						//	Styles 
		350, 300,						//	x,y position
		210, 60,						//	w,h size
		hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
		NULL							//	PARAMETER
	);
	if (!hButton3)
		return FALSE;

#ifdef _DEBUG
	_tprintf(TEXT("BUTTONS CREATED\n"));
#endif

	//	Create Label

	hEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,					//	EX style
		TEXT("STATIC"),						//	PREDEFINED CLASS
		pID,								//	text 
		WS_CHILD | WS_VISIBLE | SS_CENTER,	//	Styles 
		210, 20,							//	x,y position
		210, 20,							//	w,h size
		hWnd, NULL, hInstance,				//	PARENT WINDOW, MENU, INSTANCE
		NULL								//	PARAMETER
	);
	if (!hEdit)
		return FALSE;

#ifdef _DEBUG
	_tprintf(TEXT("TEXTBOX CREATED\n"));
#endif

	//	Create seat Buttons
	for (int i = 0; i < 10; i++) {
		hButtonS[i] = CreateWindow(
			TEXT("BUTTON"),							//	PREDEFINED CLASS
			TEXT(""),								//	text 
			WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,	//	Styles 
			(10 + 1.25 * i * 32), 60,				//	x,y position
			32, 32,									//	w,h size
			hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
			NULL									//	PARAMETER
		);
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		//	TODO: Aggiungere qui il codice di disegno che usa HDC...
		EndPaint(hWnd, &ps);
		if (!_tcscmp(pID, TEXT(""))) {
			ShowWindow(hButton1, SW_SHOWNORMAL);
			ShowWindow(hButton2, SW_HIDE);
			ShowWindow(hButton3, SW_HIDE);
		}
		else {
			ShowWindow(hButton1, SW_HIDE);
			ShowWindow(hButton2, SW_SHOWNORMAL);
			ShowWindow(hButton3, SW_SHOWNORMAL);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_COMMAND:
		if (wParam == BN_CLICKED) {
#ifdef _DEBUG
			printf(TEXT("RECEIVED WM_COMMAND MESSAGE ON BUTTON %d\n"), lParam);
#endif
			if ((HWND)lParam == hButton1) {
				savStore(TEXT("TEST"));
			}
			if ((HWND)lParam == hButton2) {
				if (_tcscmp(pID, TEXT("TEST")))
					savStore(TEXT("TEST"));
				else
					savStore(TEXT("MTEST"));
			}
			if ((HWND)lParam == hButton3) {
				savStore(TEXT(""));
			}
			pID = savLoad();
			SendMessage(hEdit, WM_SETTEXT, _tcslen(pID), (LPARAM)pID);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
	}
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_DRAWITEM:
#ifdef _DEBUG
		printf(TEXT("RECEIVED WM_DRAWITEM MESSAGE\n"));
#endif
		(LPDRAWITEMSTRUCT)lParam;
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_DESTROY:
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED DESTROY MESSAGE\n"));
#endif
		PostQuitMessage(0);
		break;

	default:
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED GENERIC MESSAGE %d\n"), message);
#endif
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

void buttonViewMngr() {
	if (!_tcscmp(pID, TEXT(""))) {
		ShowWindow(hButton1, SW_SHOWNORMAL);
		ShowWindow(hButton2, SW_HIDE);
		ShowWindow(hButton3, SW_HIDE);
	}
	else {
		ShowWindow(hButton1, SW_HIDE);
		ShowWindow(hButton2, SW_SHOWNORMAL);
		ShowWindow(hButton3, SW_SHOWNORMAL);
	}
}

void errorhandler() {
	LPTSTR p_errmsg = NULL;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&p_errmsg,
		0,
		NULL
	);
	_ftprintf(stderr, TEXT("%s\n"), p_errmsg);
	exit(EXIT_FAILURE);
}