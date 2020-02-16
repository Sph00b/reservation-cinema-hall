#include "connection.h"
#include "cinema-client.h"
#include "framework.h"
#include "booking.h"
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
HBOOKING hBooking;
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
BOOL				ButtonClickHandler(HWND, LPCTSTR*);
BOOL				UpdateSeats(HWND, BOOL);
BOOL				QueryServer(LPCTSTR, LPTSTR*);
BOOL				GetSeatsQuery(LPTSTR*, HBITMAP, HBITMAP);
void				ErrorHandler(int e);

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
	
#ifdef _DEBUG
	_tprintf(TEXT("BOOKING CREATED\n"));
#endif
	//	Inizializzare le variabili globali
	LPTSTR buffer;

	szTitle = TEXT("Prenotazione");
	szWindowClass = TEXT("generic_class");


	//	Create the booking
	hBooking = InitializeBooking(TEXT("PrenotazioneCinema"));
	if (hBooking == NULL) {
		ErrorHandler(GetLastError());
	}
	//	Retrive number of seats and rows
	if (!QueryServer(TEXT("GET ROWS FROM CONFIG"), &buffer)) {
		ErrorHandler(WSAGetLastError());
	}
	rows = _tstoi(buffer);
	free(buffer);
	if (!QueryServer(TEXT("GET COLUMNS FROM CONFIG"), &buffer)) {
		ErrorHandler(WSAGetLastError());
	}
	columns = _tstoi(buffer);
	free(buffer);
	//	Initialize seats buttons
	hStaticS = malloc((size_t)(rows * columns) * sizeof(HWND));

	if (!MyRegisterClass(hInstance)) {
		ErrorHandler(GetLastError());
	}

#ifdef _DEBUG
	_tprintf(TEXT("WINDOW CLASS REGITRATED\n"));
#endif

	// Eseguire l'inizializzazione dall'applicazione:
	if (!InitInstance(hInstance, nCmdShow)) {
		ErrorHandler(GetLastError());
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
	HWND hWnd;
	int XRes = 1280;
	int YRes = 720;

	hInst = hInstance;	//	Archivia l'handle di istanza nella variabile globale

	if (!(hWnd = CreateWindow(
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
	))) return FALSE;
#ifdef _DEBUG
	_tprintf(TEXT("WINDOW SUCCESFULLY CREATED\n"));
#endif

	//	Inizializza le risorse
	{
		if ((hBitmapDisabled = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP1))) == NULL) {
			ErrorHandler(GetLastError());
		}
		if ((hBitmapDefault = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP2))) == NULL) {
			ErrorHandler(GetLastError());
		}
		if ((hBitmapBooked = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP3))) == NULL) {
			ErrorHandler(GetLastError());
		}
		if ((hBitmapSelected = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP4))) == NULL) {
			ErrorHandler(GetLastError());
		}
		if ((hBitmapRemove = LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BITMAP5))) == NULL) {
			ErrorHandler(GetLastError());
		}
	}

	//	Creazione Label
	{
		LPTSTR film;
		LPTSTR showtime;
		LPTSTR buffer;
		if (!(CreateWindow(
			TEXT("STATIC"),							//	PREDEFINED CLASS
			TEXT("Codice prenotazione:"),			//	text 
			WS_CHILD | WS_VISIBLE,					//	Styles
			(XRes / 2) - (210 / 2) - 170,			//	x position
			15,										//	y position
			140, 20,								//	w,h size
			hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
			NULL									//	PARAMETER
		))) return FALSE;

		if (!QueryServer(TEXT("GET FILM FROM CONFIG"), &buffer)) {
			ErrorHandler(WSAGetLastError());
		}
		if (asprintf(&film, TEXT("Film: %s"), buffer) == -1) {
			return FALSE;
		}
		free(buffer);
		
		if (!(CreateWindow(
			TEXT("STATIC"),							//	PREDEFINED CLASS
			film,									//	text 
			WS_CHILD | WS_VISIBLE,					//	Styles
			10, 5,									//	x,y position
			255, 20,								//	w,h size
			hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
			NULL									//	PARAMETER
		))) return FALSE;

		if (!QueryServer(TEXT("GET SHOWTIME FROM CONFIG"), &buffer)) {
			ErrorHandler(WSAGetLastError());
		}
		if (asprintf(&showtime, TEXT("Orario: %s"), buffer) == -1) {
			return FALSE;
		}
		free(buffer);
		
		if (!(CreateWindow(
			TEXT("STATIC"),							//	PREDEFINED CLASS
			showtime,								//	text 
			WS_CHILD | WS_VISIBLE,					//	Styles
			10, 25,									//	x,y position
			255, 20,								//	w,h size
			hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
			NULL									//	PARAMETER
		))) return FALSE;

		if (!(hStaticLabelScreen = CreateWindow(
			TEXT("STATIC"),							//	PREDEFINED CLASS
			TEXT("SCHERMO"),						//	text 
			WS_CHILD | WS_VISIBLE | SS_CENTER,		//	Styles
			(XRes / 2) - (16 * columns) - 24,		//	x position
			48 + ((YRes - 150) / 2) + (16 * rows),	//	y position
			(32 * columns) + 48, 20,				//	w,h size
			hWnd, NULL, hInstance,					//	PARENT WINDOW, MENU, INSTANCE
			NULL									//	PARAMETER
		))) return FALSE;

	}

	//	Creazione Textbox
	{
		LPTSTR bookingCode;

		if ((bookingCode = GetBooking(hBooking)) == NULL) {
			ErrorHandler(GetLastError());
		}
		if (!(hStaticTextbox = CreateWindowEx(
			WS_EX_CLIENTEDGE,					//	EX style
			TEXT("STATIC"),						//	PREDEFINED CLASS
			bookingCode,						//	text 
			WS_CHILD | WS_VISIBLE | SS_CENTER,	//	Styles 
			(XRes / 2) - (210 / 2),				//	x position
			15,									//	y position
			210, 20,							//	w,h size
			hWnd, NULL, hInstance,				//	PARENT WINDOW, MENU, INSTANCE
			NULL								//	PARAMETER
		))) return FALSE;
		free(bookingCode);
#ifdef _DEBUG
		_tprintf(TEXT("TEXTBOX CREATED\n"));
#endif
	}

	//	Create Button
	{
		if (!(hButton1 = CreateWindow(
			TEXT("BUTTON"),					//	PREDEFINED CLASS 
			TEXT("Prenota"),				//	Button text 
			WS_CHILD,						//	Styles 
			(XRes / 2) - (210 / 2),			//	x position
			YRes - (2 * 60),				//	y position
			210, 60,						//	w,h size
			hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
			NULL							//	PARAMETER
		))) return FALSE;

		//	Create Button
		if (!(hButton2 = CreateWindow(
			TEXT("BUTTON"),					//	PREDEFINED CLASS 
			TEXT("Modifica prenotazione"),	//	Button text 
			WS_CHILD,						//	Styles 
			(XRes / 2) - (210 * 4 / 3),		//	x position
			YRes - (2 * 60),				//	y position
			210, 60,						//	w,h SIZE
			hWnd, NULL, hInstance,			//	PARENT WINDOW, MENU, INSTANCE
			NULL							//	PARAMETER
		))) return FALSE;

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
	}
	
	//	Creazione Contolli Poltrone
	{

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
					TEXT("STATIC"),										//	PREDEFINED CLASS
					TEXT(""),											//	text 
					WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY,		//	Styles
					(XRes / 2) - (16 * columns) + (32 * j),				//	x position
					32 + ((YRes - 150) / 2) - (16 * rows) + (32 * i),	//	y position
					32, 32,												//	w,h size
					hWnd, NULL, hInstance,								//	PARENT WINDOW, MENU, INSTANCE
					NULL												//	PARAMETER
				);
				if (!hStaticS[(i * columns) + j]) {
					return FALSE;
				}
				SendMessage(hStaticS[(i * columns) + j], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
			}
		}

		if (!UpdateSeats(hWnd, FALSE)) {
			ErrorHandler(GetLastError());
		}
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
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
	case WM_CREATE: {
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED CREATE MESSAGE\n"));
#endif
		if (!SetTimer(hWnd, 0, 1000, (TIMERPROC)NULL)) {
			ErrorHandler(GetLastError());
		}
	}
		break;
	case WM_PAINT: {
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED PAINT MESSAGE\n"));
#endif
		LPTSTR bookingCode;
		PAINTSTRUCT ps;
		HDC hdc;

		hdc = BeginPaint(hWnd, &ps);
		//	TODO: Aggiungere qui il codice di disegno che usa HDC...
		EndPaint(hWnd, &ps);

		if ((bookingCode = GetBooking(hBooking)) == NULL) {
			ErrorHandler(GetLastError());
		}
		if (!_tcscmp(bookingCode, TEXT(""))) {
			ShowWindow(hButton1, SW_SHOWNORMAL);
			ShowWindow(hButton2, SW_HIDE);
			ShowWindow(hButton3, SW_HIDE);
		}
		else {
			ShowWindow(hButton1, SW_HIDE);
			ShowWindow(hButton2, SW_SHOWNORMAL);
			ShowWindow(hButton3, SW_SHOWNORMAL);
		}
		free(bookingCode);
	}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_CTLCOLORSTATIC: {
		if ((HWND)lParam == hStaticLabelScreen) {
			SetBkColor((HDC)wParam, GetSysColor(COLOR_SCROLLBAR));
			return (LRESULT)GetSysColorBrush(COLOR_SCROLLBAR);
		}
		else if ((HWND)lParam != hStaticTextbox) {
			return (LRESULT)GetSysColorBrush(COLOR_WINDOW);
		}
	}
		break;
	case WM_TIMER: {
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED TIMER MESSAGE\n"));
#endif
		if (!UpdateSeats(hWnd, FALSE)) {
			ErrorHandler(GetLastError());
		}
	}
		break;
	case WM_COMMAND: {
		if (wParam == BN_CLICKED) {
#ifdef _DEBUG
			_tprintf(TEXT("RECEIVED WM_COMMAND CLICKED MESSAGE FROM CONTROL %d\n"), lParam);
#endif
			/*	Static Control	*/
			{
				HBITMAP tmp;

				tmp = (HBITMAP)SendMessage((HWND)lParam, STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
				if (tmp != NULL) {
					if (tmp == hBitmapDefault) {
						SendMessage((HWND)lParam, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapSelected);
					}
					else if (tmp == hBitmapSelected) {
						SendMessage((HWND)lParam, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
					}
					else if (tmp == hBitmapBooked) {
						SendMessage((HWND)lParam, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapRemove);
					}
					else if (tmp == hBitmapRemove) {
						SendMessage((HWND)lParam, STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapBooked);
					}
					else {
						return DefWindowProc(hWnd, message, wParam, lParam);
					}
					return 0;
				}
			}
			/*	Button Control	*/
			LPTSTR bookingCode = NULL;
			LPTSTR* queries = NULL;

			if ((bookingCode = GetBooking(hBooking)) == NULL) {
				ErrorHandler(GetLastError());
			}

			if ((HWND)lParam == hButton1) {
				if ((queries = malloc(sizeof(LPTSTR) * 2)) == NULL) {
					ErrorHandler(GetLastError());
				}
				if (asprintf(&queries[0], TEXT("#0")) == -1) {
					ErrorHandler(GetLastError());
				}
				if (!GetSeatsQuery(&queries[0], hBitmapSelected, (HBITMAP)NULL)) {
					free(queries[0]);
					free(queries);
					return 0;
				}
				queries[1] = NULL;
				if (!ButtonClickHandler(hWnd, queries)) {
					ErrorHandler(GetLastError());
				}
				free(queries[0]);
				free(queries);
			}
			else if ((HWND)lParam == hButton2) {
				int i = 1;

				if ((queries = malloc(sizeof(LPTSTR) * 3)) == NULL) {
					ErrorHandler(GetLastError());
				}
				if (asprintf(&queries[0], TEXT("#%s"), bookingCode) == -1) {
					ErrorHandler(GetLastError());
				}
				if (!GetSeatsQuery(&queries[0], hBitmapSelected, (HBITMAP)NULL)) {
					free(queries[0]);
					queries[0] = NULL;
					queries[1] = NULL;
					i = 0;
				}
				if (asprintf(&queries[i], TEXT("@%s"), bookingCode) == -1) {
					ErrorHandler(GetLastError());
				}
				free(bookingCode);
				if (!GetSeatsQuery(&queries[i], hBitmapRemove, (HBITMAP)NULL)) {
					free(queries[i]);
					queries[i] = NULL;
				}
				queries[2] = NULL;
				if (!ButtonClickHandler(hWnd, queries)) {
					ErrorHandler(GetLastError());
				}
				free(queries);
			}
			else if ((HWND)lParam == hButton3) {
				int MessageBoxResult;

				MessageBoxResult = MessageBox(
					hWnd,
					TEXT("Sei sicuro di voler eliminare la tua prenotazione?"),
					TEXT("Elimina prenotazione"),
					MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2 | MB_APPLMODAL
				);
				if (MessageBoxResult == IDYES) {
					BOOL booking;
					if ((queries = malloc(sizeof(LPTSTR) * 2)) == NULL) {
						ErrorHandler(GetLastError());
					}
					if (asprintf(&queries[0], TEXT("@%s"), bookingCode) == -1) {
						ErrorHandler(GetLastError());
					}
					free(bookingCode);
					booking = GetSeatsQuery(&queries[0], hBitmapBooked, hBitmapRemove);
					if (!booking) {
						free(queries[0]);
						free(queries);
						return 0;
					}
					queries[1] = NULL;
					if (!ButtonClickHandler(hWnd, queries)) {
						ErrorHandler(GetLastError());
					}
					free(queries[0]);
					free(queries);
				}
			}
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	case WM_DRAWITEM: {
		(LPDRAWITEMSTRUCT)lParam;
	}
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_DESTROY: {
#ifdef _DEBUG
		_tprintf(TEXT("RECEIVED DESTROY MESSAGE\n"));
#endif
		PostQuitMessage(0);
	}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

BOOL UpdateSeats(HWND hWnd, BOOL reset) {
	LPTSTR query;
	LPTSTR result;
	LPTSTR bookingCode;
	BOOL booking = FALSE;

	if ((bookingCode = GetBooking(hBooking)) == NULL) {
		return FALSE;
	}
	if (asprintf(&query, TEXT("~%s"), bookingCode) == -1) {
		return FALSE;
	}
	free(bookingCode);
	if (!QueryServer(query, &result)) {
		ErrorHandler(WSAGetLastError());
	}
	free(query);
	for (int i = 0; i < rows * columns; i++) {
		if (result[i * 2] == TEXT('1')) {
			booking = TRUE;
		}
		if (reset) {
			if (result[i * 2] == TEXT('0')) {
				SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDefault);
			}
			else if (result[i * 2] == TEXT('1')) {
				SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapBooked);
			}
			else {
				SendMessage(hStaticS[i], STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)hBitmapDisabled);
			}
		}
		else {
			HBITMAP tmp;
			if ((tmp = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0)) == NULL) {
				return FALSE;
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
	}
	free(result);
	if (!booking) {
		if (!SetBooking(hBooking, TEXT(""))) {
			ErrorHandler(GetLastError());
		}
		SendMessage(hStaticTextbox, WM_SETTEXT, _tcslen(TEXT("")), (LPARAM)TEXT(""));
	}
	RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	return TRUE;
}

BOOL GetSeatsQuery(LPTSTR* lppQuery, HBITMAP hBitmapType1, HBITMAP hBitmapType2) {
	BOOL result = FALSE;
	HBITMAP hBitmap = NULL;
	LPTSTR lpTmp = NULL;

	for (int i = 0; i < rows * columns; i++) {
		hBitmap = (HBITMAP)SendMessage(hStaticS[i], STM_GETIMAGE, (WPARAM)IMAGE_BITMAP, 0);
		if (hBitmap == hBitmapType1 || hBitmap == hBitmapType2) {
			result = TRUE;
			lpTmp = *lppQuery;
			if (asprintf(lppQuery, TEXT("%s %d"), *lppQuery, i) == -1) {
				ErrorHandler(GetLastError());
			}
			free(lpTmp);
		}
	}
	return result;

}

BOOL ButtonClickHandler(HWND hWnd, LPCTSTR* queries) {
	LPTSTR buffer;
	
	for (int i = 0; queries[i] != NULL; i++) {
		if (!QueryServer(queries[i], &buffer)) {
			ErrorHandler(WSAGetLastError());
		}
		if (!(_tcscmp(buffer, TEXT("OPERATION FAILED")))) {
			MessageBox(
				hWnd,
				TEXT("Prenotazione falita, in caso di prenotazioni simultanee una prenotazione potrebbe fallire.\nRiprovare"),
				NULL,
				MB_OK | MB_ICONERROR | MB_APPLMODAL
			);
		}
		if ((_tcscmp(buffer, TEXT("OPERATION SUCCEDED")))) {
			if (!SetBooking(hBooking, buffer)) {
				ErrorHandler(GetLastError());
			}
			SendMessage(hStaticTextbox, WM_SETTEXT, _tcslen(buffer), (LPARAM)buffer);
		}
		free(buffer);
	}
	if (!UpdateSeats(hWnd, TRUE)) {
		ErrorHandler(GetLastError());
	}
	return TRUE;
}

BOOL QueryServer(LPCTSTR query, LPTSTR* result) {
	connection_t cntn;
	if (connection_init(&cntn, TEXT("127.0.0.1"), 55555)) {
		return FALSE;
	}
	if (connetcion_connect(&cntn)) {
		return FALSE;
	}
	if (connection_send(&cntn, query) == -1) {
		return FALSE;
	}
	while (connection_recv(&cntn, result) == -1) {
		return FALSE;
	}
#ifdef _DEBUG
	_tprintf(TEXT("QUERY: %s\nRESULT: %s\n"), query, *result);
#endif
	if (connection_close(&cntn) == -1) {
		return FALSE;
	}
	return TRUE;
}

void ErrorHandler(int e) {
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
