#include <windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>  // For std::shuffle
#include <ctime>      // For std::time

// ... (GameInfo struct and global variables remain the same)

// Additional structure to hold folder information
struct FolderInfo {
    std::string name;
    std::string folderPath;
};

// Global variables
std::vector<FolderInfo> folders;
HWND hwndMain;          // Handle to the main window
HWND hwndGameList;      // Handle to the game list window
HWND hwndTimerDisplay;  // Handle to the timer display window
HWND hwndAdmin;         // Handle to the admin window

const int TIMER_ID = 1;  // Timer ID for the 15-minute timer
const int TIMER_INTERVAL = 1000;  // Timer interval in milliseconds (1 second)

time_t startTime = 0;   // Timestamp when the timer started
bool isTimerRunning = false;  // Flag to track whether the timer is running

bool isAdminMode = false; // Flag to track if the admin mode is active
const std::string adminPassword = "your_admin_password"; // Replace with your admin password

// Callback function for button click on the main screen
void OnStartButtonClick() {
    ShowWindow(hwndMain, SW_HIDE);  // Hide the main window

    // Create the game list window
    WNDCLASS wc = {};
    wc.lpfnWndProc = GameListWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "GameListWindowClass";
    RegisterClass(&wc);

    hwndGameList = CreateWindow(wc.lpszClassName, "Game List", WS_OVERLAPPEDWINDOW,
                                100, 100, 300, 300, NULL, NULL, wc.hInstance, NULL);

    // Create the timer display window
    hwndTimerDisplay = CreateWindow("STATIC", "Timer: 15:00", WS_VISIBLE | WS_CHILD,
                                    20, 220, 100, 25, hwndGameList, NULL, NULL, NULL);

    // Start the 15-minute timer
    SetTimer(hwndGameList, TIMER_ID, TIMER_INTERVAL, NULL);
    startTime = time(NULL);
    isTimerRunning = true;

    // Show the game list window
    ShowWindow(hwndGameList, SW_SHOWDEFAULT);
}

// Callback function for button click on the admin screen
void OnAdminButtonClick(HWND hwnd) {
    char enteredPassword[100];
    GetWindowText(hwndAdmin, enteredPassword, sizeof(enteredPassword));

    if (enteredPassword == adminPassword) {
        isAdminMode = true;
        // Show the admin controls or perform admin tasks here
        // ...
    } else {
        MessageBox(hwnd, "Incorrect password. Please try again.", "Authentication Failed", MB_ICONERROR);
    }
}

// Callback function for button click on the game list screen
void OnFolderButtonClick(HWND hwnd, int folderId) {
    if (isAdminMode) {
        // Perform admin-specific actions here
        // ...
    } else {
        if (folderId >= 0 && folderId < folders.size()) {
            std::string folderPath = folders[folderId].folderPath;
            ShellExecute(NULL, "open", folderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
        }
    }
}

// Callback function for "交代" button click on the game list screen
void OnReplaceButtonClick(HWND hwnd) {
    currentPage = 0;
    isFirstPage = true;

    // Reshuffle remaining games on the first page
    std::vector<GameInfo> remainingGames;
    for (int i = 3; i < games.size() && i < gamesPerPage; ++i) {
        remainingGames.push_back(games[i]);
    }
    std::shuffle(remainingGames.begin(), remainingGames.end(), std::mt19937(std::time(0)));

    // Replace the reshuffled games on the first page
    for (int i = 0; i < remainingGames.size(); ++i) {
        games[i + 3] = remainingGames[i];
    }

    // Clear folders
    folders.clear();

    // Stop the timer
    KillTimer(hwnd, TIMER_ID);
    isTimerRunning = false;

    // Hide the game list window
    ShowWindow(hwnd, SW_HIDE);

    // Show the main window again
    ShowWindow(hwndMain, SW_SHOWDEFAULT);
}

// ... (OnTimer and other functions remain the same)

// Callback function for window procedure of the admin screen
LRESULT CALLBACK AdminWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | ES_PASSWORD, 20, 20, 200, 25, hwnd, NULL, NULL, NULL);
            CreateWindow("BUTTON", "Authenticate", WS_VISIBLE | WS_CHILD, 20, 60, 100, 40, hwnd, (HMENU)1, NULL, NULL);
            hwndAdmin = hwnd;
            break;
        case WM_COMMAND:
            if (LOWORD(wParam) == 1) {
                OnAdminButtonClick(hwnd);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ... (GameListWindowProc and MainWindowProc remain the same)

int main() {
    // ... (GameInfo struct and games/folders vector initialization remain the same)

    // Create the main window
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASS wc = {};
    wc.lpfnWndProc = MainWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "MainWindowClass";
    RegisterClass(&wc);

    HWND hwndMain = CreateWindow(wc.lpszClassName, "Game Launcher", WS_OVERLAPPEDWINDOW,
                                 100, 100, 300, 250, NULL, NULL, hInstance, NULL);

    // Show the main window
    ShowWindow(hwndMain, SW_SHOWDEFAULT);

    // Message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
