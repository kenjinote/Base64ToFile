#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#pragma comment(lib, "crypt32")

#include <windows.h>

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit, hButton;
	switch (msg)
	{
	case WM_CREATE:
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_VSCROLL | ES_MULTILINE | ES_AUTOHSCROLL | ES_AUTOVSCROLL, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		SendMessage(hEdit, EM_LIMITTEXT, 0, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("ファイル出力..."), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 10, 10, LOWORD(lParam) - 20, HIWORD(lParam) - 62, TRUE);
		MoveWindow(hButton, 10, HIWORD(lParam) - 42, 256, 32, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			TCHAR szFilePath[MAX_PATH] = { 0 };
			OPENFILENAME of = { 0 };
			of.lStructSize = sizeof(OPENFILENAME);
			of.hwndOwner = hWnd;
			of.lpstrFilter = TEXT("任意のファイル(*.*)\0*.*\0\0");
			of.lpstrFile = szFilePath;
			of.nMaxFile = MAX_PATH;
			of.Flags = OFN_OVERWRITEPROMPT;
			of.lpstrDefExt = TEXT("bin");
			of.lpstrTitle = TEXT("ファイルに書き出す");
			if (GetSaveFileName(&of))
			{
				const int nInputTextSize = GetWindowTextLength(hEdit);
				if (nInputTextSize)
				{
					LPTSTR lpByte = (LPTSTR)GlobalAlloc(0, (nInputTextSize + 1) * sizeof(TCHAR));
					if (lpByte)
					{
						GetWindowText(hEdit, lpByte, nInputTextSize + 1);
						DWORD nDestinationSize;
						if (CryptStringToBinary(lpByte, nInputTextSize, CRYPT_STRING_BASE64, NULL, &nDestinationSize, NULL, NULL))
						{
							LPBYTE pszDestination = (LPBYTE)GlobalAlloc(0, nDestinationSize);
							if (pszDestination)
							{
								if (CryptStringToBinary(lpByte, nInputTextSize, CRYPT_STRING_BASE64, pszDestination, &nDestinationSize, NULL, NULL))
								{
									HANDLE hFile = CreateFile(szFilePath, GENERIC_WRITE, FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);
									if (hFile != INVALID_HANDLE_VALUE)
									{
										DWORD tmp;
										WriteFile(hFile, pszDestination, nDestinationSize, &tmp, 0);
										CloseHandle(hFile);
									}
								}
								GlobalFree(pszDestination);
							}
						}
						GlobalFree(lpByte);
					}
				}
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	TCHAR szClassName[] = TEXT("Window");
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("入力されたBase64のコードからファイルを生成する"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}
