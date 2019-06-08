#include "wndclient.h"

#define DEBUG

#define FILL_TSTR(tstr, txt)							\
			tstr = malloc(sizeof(TEXT(txt)));			\
			_tcscpy_s(tstr, sizeof(TEXT(txt)), TEXT(txt))

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
		(LPCTSTR) szWindowClass,	//	CLASS
		(LPCTSTR) szTitle,			//	TITLE
		WS_OVERLAPPEDWINDOW,		//	STYLE
		CW_USEDEFAULT,				//	X
		CW_USEDEFAULT,				//	Y
		CW_USEDEFAULT,				//	WIDTH
		CW_USEDEFAULT,				//	HEIGHT
		NULL,						//	NO PARENT WINDOW
		NULL,						//	NO MENU
		hInstance,					//	INSTANCE
		NULL						//	NO PARAMETER
	);

	if (!hWnd){
		return FALSE;
	}

#ifdef DEBUG
	printf("WINDOW SUCCESFULLY CREATED\n");
#endif

	//	Create Button
	HWND hWndButton = CreateWindow(
		"BUTTON",												//	PREDEFINED CLASS 
		"Button",												//	Button text 
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  //	Styles 
		10,														//	x position 
		10,														//	y position 
		100,													//	Button width
		60,														//	Button height
		hWnd,													//	PARENT WINDOW window
		NULL,													//	NO MENU
		hInstance,												//	INSTANCE
		NULL);													//	NO PARAMETER

	if (!hWndButton) {
		return FALSE;
	}

#ifdef DEBUG
	printf("BUTTON SUCCESFULLY CREATED\n");
#endif

	//	Create Textxbox

	HWND hEdit = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		"Static",
		"Text",
		WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
		120,
		20,
		500,
		100,
		hWnd,													//	PARENT WINDOW window
		NULL,
		hInstance,
		NULL);

	if (!hWndButton) {
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

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {
	// Inizializzare le stringhe globali
	FILL_TSTR(szTitle, "Preonatazione");
	FILL_TSTR(szWindowClass, "gclass");
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
