#include "wndclient.h"

#define DEBUG

LPTSTR pID;		//ID codice di prenotazione

BOOL MyRegisterClass(HINSTANCE hInstance) {
	WNDCLASS wndc;
	wndc.style = CS_HREDRAW | CS_VREDRAW;
	wndc.lpfnWndProc = WndProc;
	wndc.cbClsExtra = 0;
	wndc.cbWndExtra = 0;
	wndc.hInstance = hInstance;
	wndc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
	wndc.lpszMenuName = NULL;
	wndc.lpszClassName = (LPCTSTR) szWindowClass;
	return RegisterClass(&wndc) ? TRUE : FALSE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam){
	switch (message) {
	case WM_CREATE:
#ifdef DEBUG
		printf("RECEIVED CREATE MESSAGE\n");
#endif
		break;

	case WM_SIZE:
#ifdef DEBUG
		printf("RECEIVED SIZE MESSAGE\n");
#endif
		break;

	case WM_PAINT:
#ifdef DEBUG
		printf("RECEIVED PAINT MESSAGE\n");
#endif
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		//	TODO: Aggiungere qui il codice di disegno che usa HDC...
		EndPaint(hWnd, &ps);
	}
		break;

	case WM_DESTROY:
#ifdef DEBUG
		printf("RECEIVED DESTROY MESSAGE\n");
#endif
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow){
	hInst = hInstance;	//	Archivia l'handle di istanza nella variabile globale

	HWND hWnd = CreateWindow(
		(LPCTSTR) szWindowClass,									//	CLASS
		(LPCTSTR) szTitle,											//	TITLE
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

	if (!hWnd){
		return FALSE;
	}


#ifdef DEBUG
	printf("WINDOW SUCCESFULLY CREATED\n");
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

#ifdef DEBUG
	printf("BUTTONS SUCCESFULLY CREATED\n");
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

#ifdef DEBUG
	printf("TEXTBOX SUCCESFULLY CREATED\n");
#endif

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
#ifdef DEBUG
	printf("WINDOW SHOWED\n");
#endif
	return TRUE;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	// Inizializzare le stringhe globali
	szTitle = TEXT("Prenotazione");
	szWindowClass = TEXT("generic_class");
	pID = savLoad();
	// Registrazione della classe principale:
	if (!MyRegisterClass(hInstance))
		return GetLastError();
#ifdef DEBUG
	printf("WINDOW CLASS SUCCESFULLY REGITRATED\n");
#endif
	// Inizializzazione della finestra:
	if (!InitInstance(hInstance, nCmdShow))
		return GetLastError();
#ifdef DEBUG
	printf("WINDOW SUCCESFULLY INITIALIZATED\n");
#endif
	// Ciclo di messaggi principale:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
#ifdef DEBUG
	printf("RECEIVED DESTROY OR INVALID MESSAGE\n");
#endif
	return (int)msg.wParam;
}
