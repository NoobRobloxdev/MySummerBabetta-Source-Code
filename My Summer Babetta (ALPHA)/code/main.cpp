#include <windows.h>
#include <commctrl.h>
#include <string>

void StartEngine() {
    char path[MAX_PATH];
    GetModuleFileNameA(NULL, path, MAX_PATH);
    
    // Convert path to a string and find the folder
    std::string currentPath = path;
    size_t pos = currentPath.find_last_of("\\/");
    std::string folder = currentPath.substr(0, pos);
    std::string gameExe = folder + "\\Game.exe";

    // Run the game using the absolute folder path as the working directory
    HINSTANCE result = ShellExecuteA(NULL, "open", gameExe.c_str(), NULL, folder.c_str(), SW_SHOW);
    
    if ((intptr_t)result <= 32) {
        MessageBoxA(NULL, "Launcher error: Game.exe not found in this folder!", "Critical Error", MB_ICONERROR);
    } else {
        exit(0); 
    }
}

LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
        {
            // Restore your exact UI design
            CreateWindowW(L"Static", L"MY SUMMER BABETTA", WS_VISIBLE | WS_CHILD | SS_CENTER, 0, 10, 400, 20, hWnd, NULL, NULL, NULL);
            CreateWindowW(L"Button", L"START ENGINE", WS_VISIBLE | WS_CHILD, 100, 100, 200, 60, hWnd, (HMENU)1, NULL, NULL);
            CreateWindowW(L"Button", L"EXIT TO GARAGE", WS_VISIBLE | WS_CHILD, 125, 180, 150, 30, hWnd, (HMENU)2, NULL, NULL);
        }
        break;
        case WM_COMMAND:
            if (LOWORD(wp) == 1) StartEngine();
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
    RegisterClassW(&wc);
    CreateWindowW(L"BabettaLauncher", L"My Summer Babetta (ALPHA)", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 400, 200, 420, 300, NULL, NULL, hInst, NULL);
    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}