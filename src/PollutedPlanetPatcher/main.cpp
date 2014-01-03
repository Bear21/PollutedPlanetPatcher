//License:
//Author: Stephen Wheeler
//E-mail: bear@8bitbear.com
//You may edit this code in anyway but you must provide credit to original author and must provide the source publicly to any changes.

#include "windows.h"
#include "Windowsx.h"
#include "wchar.h"
#include "time.h"
#include "stdio.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

#define IDC_EDIT		2000
#define IDC_EDIT1		2001
#define IDC_BUTTON		2002
#define IDC_CHECK		2003
HWND					g_checkHwnd;

wchar_t g_width[16], g_height[16];

int g_style;

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = L"PollutedPatcher";
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, L"Error registering class!", L"Error", NULL);
		return 1;
    }

	FILE *FP;
	if(_wfopen_s(&FP, L"Polluted_Planet.exe", L"rb") !=0)
	{
		MessageBox(NULL, L"Cannot find Polluted_Planet.exe", L"Error", NULL);
		return -1;
	}
	fseek(FP, 0x25e0c, 0);
	char set=0;
	fread(&set, 1, 1, FP);
	if(set==0x6A)
	{
		int val;
		fseek(FP, 0x25E0F, 0);
		fread(&val, 4, 1, FP);
		swprintf_s(g_height, L"%d", val, 4);
		fseek(FP, 1, SEEK_CUR);
		
		fread(&val, 4, 1, FP);
		swprintf_s(g_width, L"%d", val, 4);
	}
	else if(set==0xF8)
	{
		memcpy(g_width, L"1024", sizeof(L"1024"));
		memcpy(g_height, L"768", sizeof(L"768"));
	}
	else
	{
		MessageBox(NULL, L"Invalid or corrupt version", L"Error", NULL);
		return -1;
	}
	fseek(FP, 0x25D25, 0);
	fread(&g_style, 4, 1, FP);
	if((g_style != 0x00CF0000) && (g_style != 0x86000000))
	{
		MessageBox(NULL, L"Invalid or corrupt version", L"Error", NULL);
		return -1;
	}

	int posX = (GetSystemMetrics(SM_CXSCREEN) - 220) / 2;
	int posY = (GetSystemMetrics(SM_CYSCREEN) - 110) / 2;
	HWND ourWindow; 
	ourWindow = CreateWindowEx(
		WS_EX_APPWINDOW,
		L"PollutedPatcher",
		L"PollutedPatcher",
		WS_VISIBLE | WS_CAPTION | WS_SYSMENU,
		posX,
		posY,
		220,
		110,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	ShowWindow(ourWindow, 5);
	UpdateWindow(ourWindow);
	


	fclose(FP);

	MSG msg;
	BOOL bRet;
	while((bRet = GetMessage(&msg, ourWindow, 0, 0)) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

    return (int) msg.wParam;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;

	WCHAR heightText[] = L"height:";
	WCHAR widthText[] = L"width:";

	LRESULT result;

    switch (message)
    {
    case WM_CREATE:
        {
			CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"edit",g_width, WS_CHILD | WS_VISIBLE | ES_NUMBER,
                50, 10, 60, 20,
                hWnd, (HMENU)IDC_EDIT,
                NULL, 0
                );
			CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"edit",g_height, WS_CHILD | WS_VISIBLE | ES_NUMBER,
                50, 40, 60, 20,
                hWnd, (HMENU)IDC_EDIT1,
                NULL, 0
                );

			
			CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"button", L"Patch", WS_CHILD | WS_VISIBLE,
                120, 40, 80, 20,							//patch button size
                hWnd, (HMENU)IDC_BUTTON,
                NULL, 0
                );

			g_checkHwnd = CreateWindowEx(
                WS_EX_CLIENTEDGE,
                L"button", L"Border", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                120, 10, 80, 20,
                hWnd, (HMENU)IDC_CHECK,
                NULL, 0
                );

			if(g_style == 0x00CF0000)
			{
				Button_SetCheck(g_checkHwnd, true);
			}

		}
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);

        TextOut(hdc,
            5, 10,
            widthText, sizeof(widthText)/2);
		TextOut(hdc,
			5, 40,
			heightText, sizeof(heightText)/2);

        EndPaint(hWnd, &ps);
        break;
    case WM_DESTROY:
		{
			result = 1;
			PostQuitMessage(0);
		}
        break;
	case WM_COMMAND:
		{
			if(LOWORD(wParam)==IDC_BUTTON)
			{
				FILE *FP;
				wchar_t lpString[11];
				int width, height;

				GetWindowText(GetDlgItem(hWnd, IDC_EDIT), lpString, 11);
				swscanf_s(lpString, L"%d", &width);
				GetWindowText(GetDlgItem(hWnd, IDC_EDIT1), lpString, 11);
				swscanf_s(lpString, L"%d", &height);

				if(_wfopen_s(&FP, L"Polluted_Planet.exe", L"rb+") !=0)
				{
					MessageBox(NULL, L"Error opening file, check to see if it's in the correct directory", NULL, NULL);
				}
				else
				{
					fseek(FP, 0x25e0c, 0);

					//Over write the asm
					char push64[] = {0x6A, 0x40};

					fwrite(push64, sizeof(push64), 1, FP); //write push
					char push =0x68;
					char end =0x90;

					
					fwrite(&push, 1, 1, FP); //push
					fwrite(&height, 4, 1, FP); //height

					fwrite(&push, 1, 1, FP); //push
					fwrite(&width, 4, 1, FP); //width
					fwrite(&end, 1, 1, FP); //NOP
					fwrite(&end, 1, 1, FP); //NOP

				
					fseek(FP, 0x25D25, 0);
					int style;
					if(!Button_GetCheck(g_checkHwnd))
					{
						style = 0x86000000;
					}
					else
					{
						style = 0x00CF0000;
					}
					fwrite(&style, 4, 1, FP); //change window style

					fclose(FP);
					MessageBox(NULL, L"Patched", L"Win", NULL);
				}

			}
		}
		break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return result;
}