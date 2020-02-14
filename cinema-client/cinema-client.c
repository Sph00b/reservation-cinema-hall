#include "connection.h"
#include "cinema-client.h"
#include "framework.h"
#include "savefile.h"
#include "asprintf.h"
#ifdef _DEBUG
#include <io.h>
#include <stdio.h>
#include <fcntl.h>
#include <tchar.h>
#endif

#define WS_CSTYLE WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX

//	Variabili globali:
HINSTANCE hInst;		// Istanza corrente
LPTSTR szTitle;			// Testo della barra del titolo
LPTSTR szWindowClass;	// Nome della classe della finestra principale
int rows;
int columns;
LPTSTR film;
LPTSTR showtime;
LPTSTR pID;		//ID codice di prenotazione
HWND hButton1;
HWND hButton2;
HWND hButton3;
HWND hStaticTextbox;
HWND* hStaticS;	//Pointer to seat control vector
HWND hStaticLabelScreen;

HBITMAP hBitmapDefault;
HBITMAP hBitmapBooked;
HBITMAP hBitmapSelected;
HBITMAP hBitmapRemove;
HBITMAP hBitmapDisabled;

// Dichiarazioni con prototipo di funzioni incluse in questo modulo di codice:
ATOM                MyRegisterClass(HINSTANCE);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void				buttonViewMngr();
int					buttonOnClick(HWND hWnd, LPCTSTR query);
int					query_server(LPCTSTR, LPTSTR*);
void				errorhandler(int e);

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow) {

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#ifdef _DEBUG
	int hCrt;
	
	AllocConsole();
	
	HANDLE handle_in = GetStdHandle(STD_INPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_in, _O_TEXT);
	FILE* hf_in = _fdopen(hCrt, "r");
	_tfreopen_s(&hf_in, TEXT("CONIN$"), TEXT("r"), stdin);

	HANDLE handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
	hCrt = _open_osfhandle((long)handle_out, _O_TEXT);
	FILE* hf_out = _fdopen(hCrt, "w");
	_tfreopen_s(&hf_out, TEXT("CONOUT$"), TEXT("w"), stdout);
	
	HANDLE handle_err = GetStdHandle(STD_ERROR_HANDLE);
	hCrt = _open_osfhandle((long)handle_err, _O_TEXT);
	FILE* hf_err = _fdopen(hCrt, "w");
	_tfreopen_s(&hf_err, TEXT("CONOUT$"), TEXT("w"), stderr);
	
#endif
	
	LPTSTR buffer;

	if (InitSavefile(TEXT("PrenotazioneCinema"))) {
		errorhandler(GetLastError());
	}
	//	Inizializzare le stringhe globali
	szTitle = TEXT("Prenotazione");
	szWindowClass = TEXT("generic_class");
	if ((pID = savLoad()) == NULL) {
		errorhandler(GetLastError());
	}
	//	Retrive number of seats and rows
	if (query_server(TEXT("GET ROWS FROM CONFIG"), &buffer)) {
		errorhandler(WSAGetLastError());
	}
	rows = _tstoi(buffer);
	free(buffer);
	if (query_server(TEXT("GET COLUMNS FROM CONFIG"), &buffer)) {
		errorhandler(WSAGetLastError());
	}
	columns = _tstoi(buffer);
	free(buffer);
	if (query_server(TEXT("GET FILM FROM CONFIG"), &buffer)) {
		errorhandler(WSAGetLastError());
	}
	if (asprintf(&film, TEXT("Film: %s"), buffer) == -1) {
		errorhandler(GetLastError());
	}
	free(buffer);
	if (query_server(TEXT("GET SHOWTIME FROM CONFIG"), &buffer)) {
		errorhandler(WSAGetLastError());
	}
	if (asprintf(&showtime, TEXT("Orario: %s"), buffer) == -1) {
		errorhandler(GetLastError());
	}
	free(buffer);
	//	Initialize seats buttons
	hStaticS = malloc(rows * columns * sizeof(HWND));
	if (!MyRegisterClass(hInstance)) {
		errorhandler(GetLastError());
	}

#ifdef _DEBUG
	_tprintf(TEXT("WINDOW CLASS REGITRATED\n"));
#endif

	// Eseguire l'inizializzazione dall'applicazione:
	if (!InitInstance(hInstance, nCmdShow)) {
		errorhandler(GetLastError());
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

	int XRes = 1280;
	int YRes = 720;

	HWND hWnd = CreateWindow(
		(LPCTSTR)szWindowClass,										//	CLASS
		(LPCTSTR)szTitle,											//	TITLE
		WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,		//	STYLE
		CW_USEDEFAULT,												//	X
		CW_USEDEFAULT,												//	Y
		XRes,														//	WIDTH
		YRes,														//	HEIGHT
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
		(XRes / 2) - (210 / 2),			//	x position
		YRes - (2 * 60),				//	y position
		210, 60,						//	w,h size
		hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
		NULL							//	PARAMETER
	);
	if (!hButton1) {
		return FALSE;
	}

	//	Create Button
	hButton2 = CreateWindow(
		TEXT("BUTTON"),					//	PREDEFINED CLASS 
		TEXT("Modifica prenotazione"),	//	Button text 
		WS_CHILD,						//	Styles 
		(XRes / 2) - (210 * 4 / 3),		//	x position
		YRes - (2 * 60),				//	y position
		210, 60,						//	w,h SIZE
		hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
		NULL							//	PARAMETER
	);
	if (!hButton2) {
		return FALSE;
	}

	//	Create Button
	hButton3 = CreateWindow(
		TEXT("BUTTON"),					//	PREDEFINED CLASS
		TEXT("Elimina prenotazione"),	//	Button text 
		WS_CHILD,						//	Styles 
		(XRes / 2) + (210 * 2 / 7),		//	x position
		YRes - (2 * 60),				//	y position
		210, 60,						//	w,h size
		hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
		NULL							//	PARAMETER
	);
	if (!hButton3) {
		return FALSE;
	}

#ifdef _DEBUG
	_tprintf(TEXT("BUTTONS CREATED\n"));
#endif

	//	Create Label

	hStaticTextbox = CreateWindowEx(
		WS_EX_CLIENTEDGE,					//	EX style
		TEXT("STATIC"),						//	PREDEFINED CLASS
		pID,								//	text 
		WS_CHILD | WS_VISIBLE | SS_CENTER,	//	Styles 
		(XRes / 2) - (210 / 2),				//	x position
		15,									//	y position
		210, 20,							//	w,h size
		hWnd, NULL, hInstance,				//	PARENT WINDOW, MENU, INSTANCE
		NULL								//	PARAMETER
	);
	if (!hStaticTextbox) {
		return FALSE;
	}

#ifdef _DEBUG
	_tprintf(TEXT("TEXTBOX CREATED\n"));
#endif

	CreateWindow(
		TEXT("STATIC"),							//	PREDEFINED CLASS
		film,									//	text 
		WS_CHILD | WS_VISIBLE,					//	Styles
		10, 5,									//	x,y position
		255, 20,								//	w,h size
		hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
		NULL									//	PARAMETER
	);

	CreateWindow(
		TEXT("STATIC"),							//	PREDEFINED CLASS
		showtime,								//	text 
		WS_CHILD | WS_VISIBLE,					//	Styles
		10, 25,									//	x,y position
		255, 20,								//	w,h size
		hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
		NULL									//	PARAMETER
	);

	//	Create seat controls

	for (int i = 0; i < rows; i++) {
		LPTSTR str;
		asprintf(&str, TEXT("%c"), i + 65);
		CreateWindow(
			TEXT("STATIC"),											//	PREDEFINED CLASS
			str,													//	text 
			WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,		//	Styles
			(XRes / 2) - (16 * columns) - 32,						//	x position
			32 + ((YRes - 150) / 2) - (16 * rows) + (32 * i),		//	y position
			32, 32,													//	w,h size
			hWnd, NULL, hInstance,									//	PARENT WINDOW, MENU, INSTANCE
			NULL													//	PARAMETER
		);
		free(str);
	}
	
	for (int j = 0; j < columns; j++) {
		LPTSTR str;
		asprintf(&str, TEXT("%d"), j + 1);
		CreateWindow(
			TEXT("STATIC"),											//	PREDEFINED CLASS
			str,													//	text 
			WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,		//	Styles
			(XRes / 2) - (16 * columns) + (32 * j),					//	x position
			((YRes - 150) / 2) - (16 * rows),						//	y position
			32, 32,													//	w,h size
			hWnd, NULL, hInstance,									//	PARENT WINDOW, MENU, INSTANCE
			NULL													//	PARAMETER
		);
		free(str);
	}

	for (int i = 0; i < rows; i++) {
		for (int j = 0; j < columns; j++) {
			hStaticS[(i * columns) + j] = CreateWindow(
				TEXT("STATIC"),									//	PREDEFINED CLASS
				TEXT(""),										//	text 
				WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,	//	Styles
				(XRes / 2) - (16 * columns) + (32 * j),			//	x position
				32 + ((YRes - 150) / 2) - (16 * rows) + (32 * i),//	y position
				32, 32,											//	w,h size
				hWnd, NULL, hInstance,							//	PARENT WINDOW, MENU, INSTANCE
				NULL											//	PARAMETER
			);
			if (!hStaticS[(i * columns) + j]) {
				return FALSE;
			}
			SendMessage(hStaticS[(i * columns) + j], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
		}
	}

	hStaticLabelScreen = CreateWindow(
		TEXT("STATIC"),							//	PREDEFINED CLASS
		TEXT("SCHERMO"),						//	text 
		WS_CHILD | WS_VISIBLE |	SS_CENTER,		//	Styles
		(XRes / 2) - (16 * columns) - 24,		//	x position
		48 + ((YRes - 150) / 2) + (16 * rows),	//	y position
		(32 * columns) + 48, 20,				//	w,h size
		hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
		NULL									//	PARAMETER
	);
	if (!hStaticLabelScreen) {
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
		{
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED CREATE MESSAGE\n"));
#endif
		hBitmapDisabled = (HBITMAP)LoadBitmap((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDB_BITMAP1));
		if (hBitmapDisabled == NULL) {
			errorhandler(GetLastError());
		}
		hBitmapDefault = (HBITMAP)LoadBitmap((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDB_BITMAP2));
		if (hBitmapDefault == NULL) {
			errorhandler(GetLastError());
		}
		hBitmapBooked = (HBITMAP)LoadBitmap((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDB_BITMAP3));
		if (hBitmapBooked == NULL) {
			errorhandler(GetLastError());
		}
		hBitmapSelected = (HBITMAP)LoadBitmap((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDB_BITMAP4));
		if (hBitmapSelected == NULL) {
			errorhandler(GetLastError());
		}
		hBitmapRemove = (HBITMAP)LoadBitmap((HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDB_BITMAP5));
		if (hBitmapRemove == NULL) {
			errorhandler(GetLastError());
		}

		if (!SetTimer(hWnd, 0, 1000, (TIMERPROC)NULL)) {
			errorhandler(GetLastError());
		}
		SendMessage(hWnd, WM_TIMER, 0, 0);
		}
	break;
	case WM_SIZE:
	{
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED SIZE MESSAGE\n"));
#endif
		break;
	}
	case WM_PAINT:
	{
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED PAINT MESSAGE\n"));
#endif
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		//	TODO: Aggiungere qui il codice di disegno che usa HDC...
		EndPaint(hWnd, &ps);
		buttonViewMngr();
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_CTLCOLORSTATIC:
	{
		if ((HWND)lParam == hStaticLabelScreen) {
			SetBkColor((HDC)wParam, GetSysColor(COLOR_SCROLLBAR));
			return (LRESULT)GetSysColorBrush(COLOR_SCROLLBAR);
		}
		else if ((HWND)lParam != hStaticTextbox) {
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		}
		break;
	}
	case WM_TIMER:
	{
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED TIMER MESSAGE\n"));
#endif
		LPTSTR query;
		LPTSTR result;
		int code_unused = 0;
		asprintf(&query, TEXT("~%s"), pID);
		if (query_server(query, &result)) {
			errorhandler(WSAGetLastError());
		}
		free(query);
		for (int i = 0; i < rows * columns; i++) {
			HBITMAP tmp;
			tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
			if (result[i * 2] == TEXT('1')) {
				code_unused = 1;
			}
			if (tmp == hBitmapDefault) {
				if (result[i * 2] == TEXT('1')) {
					SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapBooked);
				}
				else if (result[i * 2] == TEXT('2')) {
					SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDisabled);
				}
			}
			else if (tmp == hBitmapDisabled && result[i * 2] == TEXT('0')) {
				SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
			}
		}
		free(result);
		if (!code_unused) {
			savStore(TEXT(""));
			if ((pID = savLoad()) == NULL) {
				errorhandler(GetLastError());
			}
			SendMessage(hStaticTextbox, WM_SETTEXT, _tcslen(pID), (LPARAM)pID);
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
		}
		break;
	}
	case WM_COMMAND:
	{
		if (wParam == BN_CLICKED) {
#ifdef _DEBUG
			_tprintf(TEXT("RECEIVED WM_COMMAND CLICKED MESSAGE FROM CONTROL %d\n"), lParam);
#endif
			{
				for (int i = 0; i < rows * columns; i++) {
					if ((HWND)lParam == hStaticS[i]) {
						HBITMAP tmp;
						tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
						if (tmp == hBitmapDefault) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapSelected);
						}
						else if (tmp == hBitmapSelected) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
						}
						else if (tmp == hBitmapBooked) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapRemove);
						}
						else if (tmp == hBitmapRemove) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapBooked);
						}
						return 0;
					}
				}
				if ((HWND)lParam == hButton1) {
					LPTSTR query;
					int slctd_flag = 0;
					asprintf(&query, TEXT("#0"));
					for (int i = 0; i < rows * columns; i++) {
						HBITMAP tmp;
						tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
						if (tmp == hBitmapSelected) {
							slctd_flag = 1;
							asprintf(&query, TEXT("%s %d"), query, i);
						}
					}
					if (!slctd_flag) {
						return 0;
					}
					buttonOnClick(hWnd, query);
					free(query);
				}
				else if ((HWND)lParam == hButton2) {
					LPTSTR query;
					LPTSTR result;
					int slctd_flag = 0;
					asprintf(&query, TEXT("#%s"), pID);
					for (int i = 0; i < rows * columns; i++) {
						HBITMAP tmp;
						tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
						if (tmp == hBitmapSelected) {
							slctd_flag = 1;
							asprintf(&query, TEXT("%s %d"), query, i);
						}
					}
					if (slctd_flag) {
						if (query_server(query, &result)) {
							errorhandler(WSAGetLastError());
						}
						free(result);
					}
					free(query);
					asprintf(&query, TEXT("@%s"), pID);
					for (int i = 0; i < rows * columns; i++) {
						HBITMAP tmp;
						tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
						if (tmp == hBitmapRemove) {
							slctd_flag = 1;
							asprintf(&query, TEXT("%s %d"), query, i);
						}
					}
					if (slctd_flag) {
						if (query_server(query, &result)) {
							errorhandler(WSAGetLastError());
						}
						free(result);
					}
					free(query);
					for (int i = 0; i < rows * columns; i++) {
						HBITMAP tmp;
						tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
						if (tmp == hBitmapSelected) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
						}
						else if (tmp == hBitmapRemove) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
						}
					}
					RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
					SendMessage(hWnd, WM_TIMER, 0, 0);
					break;
				}
				else if ((HWND)lParam == hButton3) {
					int MessageBoxResult;
					MessageBoxResult = MessageBox(
						hWnd,
						TEXT("Sei sicuro di voler eliminare la tua prenotazione?"),
						TEXT("Elimina prenotazione"),
						MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL
					);
					if (MessageBoxResult == IDNO) {
						return 0;
					}
					/*	Create MessageBox to warn the user	*/
					LPTSTR query;
					LPTSTR result;
					asprintf(&query, TEXT("~%s"), pID);
					if (query_server(query, &result)) {
						errorhandler(WSAGetLastError());
					}
					free(query);
					/*	Use a pointer to free memory	*/
					asprintf(&query, TEXT("@%s"), pID);
					for (int i = 0; i < rows * columns; i++) {
						if (result[2 * i] == TEXT('1')) {
							asprintf(&query, TEXT("%s %d"), query, i);
						}
					}
					free(result);
					buttonOnClick(hWnd, query);
					free(query);
					for (int i = 0; i < rows * columns; i++) {
						HBITMAP tmp;
						tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
						if (tmp == hBitmapBooked) {
							SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
						}
					}
				}
				break;
			}
		}
		else {
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	case WM_DRAWITEM:
	{
		(LPDRAWITEMSTRUCT)lParam;
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_DESTROY:
	{
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED DESTROY MESSAGE\n"));
#endif
		PostQuitMessage(0);
		break;
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Gestore di messaggi per la finestra Informazioni su.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam);
	switch (message) {
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
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

int buttonOnClick(HWND hWnd, LPCTSTR query) {
	LPTSTR buffer;
	if (query_server(query, &buffer)) {
		errorhandler(WSAGetLastError());
	}
	if (!(_tcscmp(buffer, TEXT("OPERATION SUCCEDED")))) {
		savStore(TEXT(""));
	}
	else {
		savStore(buffer);
	}
	free(buffer);
	if ((pID = savLoad()) == NULL) {
		errorhandler(GetLastError());
	}
	SendMessage(hStaticTextbox, WM_SETTEXT, _tcslen(pID), (LPARAM)pID);
	for (int i = 0; i < rows * columns; i++) {
		HBITMAP tmp;
		tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
		if (tmp == hBitmapSelected) {
			SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
		}
		else if (tmp == hBitmapRemove) {
			SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
		}
	}
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	SendMessage(hWnd, WM_TIMER, 0, 0);
	return 0;
}

int query_server(LPCTSTR query, LPTSTR* result) {
	connection_t cntn;
	if (connection_init(&cntn, TEXT("127.0.0.1"), 55555)) {
		return 1;
	}
	if (connetcion_connect(&cntn)) {
		return 1;
	}
	if (connection_send(&cntn, query) == -1) {
		return 1;
	}
	while (connection_recv(&cntn, result) == -1) {
		return 1;
	}
#ifdef _DEBUG
	_tprintf(TEXT("QUERY: %s\nRESULT: %s\n"), query, *result);
#endif
	if (connection_close(&cntn) == -1) {
		return 1;
	}
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
#ifdef _DEBUG
	_ftprintf(stderr, TEXT("%s\n"), p_errmsg);
#endif
	MessageBox(
		NULL,
		p_errmsg,
		NULL,
		MB_OK | MB_ICONERROR | MB_TASKMODAL
	);
	exit(e);
}
