#include <windows.h>
#include <commctrl.h>
#include <direct.h> // For getcwd

// Function to launch the game from the current directory
void LaunchGame() {
    // Get the current directory path
    char buffer[MAX_PATH];
    _getcwd(buffer, MAX_PATH);

    // ShellExecute starts Game.exe using the current folder as the "working directory"
    // This is crucial so the game can find its textures and DLLs
    HINSTANCE result = ShellExecuteA(NULL, "open", "Game.exe", NULL, buffer, SW_SHOW);
    
    if ((intptr_t)result <= 32) {
        MessageBoxA(NULL, "Could not find Game.exe! Make sure it is in the same folder as the Launcher.", "Error", MB_ICONERROR);
    } else {
        exit(0); // Close launcher if game starts successfully
    }
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
        { // Brackets prevent "jump to case label" errors
            CreateWindowW(L"Static", L"MY SUMMER BABETTA", WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 10, 400, 20, hWnd, NULL, NULL, NULL);
            
            // Settings Section
            CreateWindowW(L"Static", L"— SETTINGS —", WS_VISIBLE | WS_CHILD | SS_CENTER, 20, 40, 160, 20, hWnd, NULL, NULL, NULL);
            HWND hCombo = CreateWindowW(L"ComboBox", NULL, WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST, 20, 85, 150, 200, hWnd, NULL, NULL, NULL);
            SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"1920x1080");
            SendMessage(hCombo, CB_SETCURSEL, 0, 0);

            // User Account Section
            CreateWindowW(L"Static", L"— USER ACCOUNT (WIP) —", WS_VISIBLE | WS_CHILD | SS_CENTER, 210, 40, 160, 20, hWnd, NULL, NULL, NULL);
            CreateWindowW(L"Edit", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, 210, 85, 160, 20, hWnd, NULL, NULL, NULL);
            
            // The Start Button
            CreateWindowW(L"Button", L"START ENGINE", WS_VISIBLE | WS_CHILD, 100, 230, 200, 50, hWnd, (HMENU)1, NULL, NULL);
            CreateWindowW(L"Button", L"EXIT TO GARAGE", WS_VISIBLE | WS_CHILD, 125, 300, 150, 30, hWnd, (HMENU)2, NULL, NULL);
        }
        break;

        case WM_COMMAND:
            if (LOWORD(wp) == 1) LaunchGame(); 
            if (LOWORD(wp) == 2) PostQuitMessage(0); 
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProcW(hWnd, msg, wp, lp);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR args, int ncmdshow) {
    WNDCLASSW wc = {0};
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hInstance = hInst;
    wc.lpszClassName = L"BabettaLauncher";
    wc.lpfnWndProc = WindowProcedure;

    if (!RegisterClassW(&wc)) return -1;
    CreateWindowW(L"BabettaLauncher", L"My Summer Babetta (ALPHA)", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 400, 200, 420, 400, NULL, NULL, hInst, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}