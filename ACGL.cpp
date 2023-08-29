#include <windows.h>
#include <vector>
#include <string>

// グローバル変数
HWND g_hMainWindow;
HWND g_hStartButton;
HWND g_hGameSelectionWindow;
HWND g_hSwapButton;
HWND g_hPrevPageButton;
HWND g_hNextPageButton;
int g_timerDuration = 15 * 60; // タイマーの時間（秒）
bool g_timerActive = false; // タイマーがアクティブかどうか
bool g_notification10Shown = false; // 10分通知が表示されたかどうか
bool g_notification13Shown = false; // 13分通知が表示されたかどうか

// タイマーのコールバック関数
VOID CALLBACK TimerCallback(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) {
    static int elapsedTime = 0;
    elapsedTime++;

    if (!g_notification10Shown && elapsedTime >= 10 * 60) {
        g_notification10Shown = true;
        // 10分通知を表示（Windows 10以降で動作）
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 1;
        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        nid.uTimeout = 5000; // 通知が表示される時間（ミリ秒）
        wcscpy_s(nid.szInfoTitle, L"タイマー通知");
        wcscpy_s(nid.szInfo, L"10分が経過しました。");
        Shell_NotifyIcon(NIM_ADD, &nid);
    }

    if (!g_notification13Shown && elapsedTime >= 13 * 60) {
        g_notification13Shown = true;
        // 13分通知を表示（Windows 10以降で動作）
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 2;
        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        nid.uTimeout = 5000; // 通知が表示される時間（ミリ秒）
        wcscpy_s(nid.szInfoTitle, L"タイマー通知");
        wcscpy_s(nid.szInfo, L"13分が経過しました。");
        Shell_NotifyIcon(NIM_ADD, &nid);
    }

    if (elapsedTime >= g_timerDuration) {
        g_timerActive = false; // タイマーを停止

        // タイマーの通知を表示（Windows 10以降で動作）
        NOTIFYICONDATA nid = {};
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hWnd;
        nid.uID = 3;
        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_INFO;
        nid.uTimeout = 5000; // 通知が表示される時間（ミリ秒）
        wcscpy_s(nid.szInfoTitle, L"タイマー通知");
        wcscpy_s(nid.szInfo, L"タイマーが終了しました。");
        Shell_NotifyIcon(NIM_ADD, &nid);

        // ゲーム選択画面を無効化
        EnableWindow(g_hGameSelectionWindow, FALSE);
        EnableWindow(g_hSwapButton, TRUE); // 交代ボタンをアクティブにする
    }
}

// スタート画面のプロシージャ
LRESULT CALLBACK GameSelectionWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // 「交代」ボタンを作成
            g_hSwapButton = CreateWindow(
                L"BUTTON", L"交代", WS_VISIBLE | WS_CHILD,
                100, 150, 200, 40, hWnd, reinterpret_cast<HMENU>(2), NULL, NULL
            );
            SendMessage(g_hSwapButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
            EnableWindow(g_hSwapButton, FALSE); // 最初は非アクティブにする

            // 「前ページ」ボタンを作成
            g_hPrevPageButton = CreateWindow(
                L"BUTTON", L"前ページ", WS_VISIBLE | WS_CHILD,
                350, 150, 100, 40, hWnd, reinterpret_cast<HMENU>(3), NULL, NULL
            );
            SendMessage(g_hPrevPageButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

            // 「次ページ」ボタンを作成
            g_hNextPageButton = CreateWindow(
                L"BUTTON", L"次ページ", WS_VISIBLE | WS_CHILD,
                450, 150, 100, 40, hWnd, reinterpret_cast<HMENU>(4), NULL, NULL
            );
            SendMessage(g_hNextPageButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                int buttonId = LOWORD(wParam);
                if (buttonId == 2) { // 「交代」ボタンがクリックされたら
                    ShowWindow(g_hMainWindow, SW_SHOW); // スタート画面を表示
                    ShowWindow(hWnd, SW_HIDE); // ゲーム選択画面を非表示
                    KillTimer(hWnd, 1); // タイマーを停止
                    g_timerActive = false;
                    EnableWindow(g_hSwapButton, FALSE); // 「交代」ボタンを非アクティブにする
                } else if (buttonId == 3) { // 「前ページ」ボタンがクリックされたら
                    // 前ページに移動
                    g_currentPage = (g_currentPage - 1 + g_maxPages) % g_maxPages;
                    // ボタンの表示を更新
                    UpdateGameButtons(hWnd);
                } else if (buttonId == 4) { // 「次ページ」ボタンがクリックされたら
                    // 次ページに移動
                    g_currentPage = (g_currentPage + 1) % g_maxPages;
                    // ボタンの表示を更新
                    UpdateGameButtons(hWnd);
                }
            }
            break;

        case WM_DESTROY:
            KillTimer(hWnd, 1); // タイマーを停止
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ゲーム選択ウィンドウのプロシージャ
LRESULT CALLBACK GameSelectionWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            // 「交代」ボタンを作成
            g_hSwapButton = CreateWindow(
                L"BUTTON", L"交代", WS_VISIBLE | WS_CHILD,
                100, 150, 200, 40, hWnd, reinterpret_cast<HMENU>(2), NULL, NULL
            );
            SendMessage(g_hSwapButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
            EnableWindow(g_hSwapButton, FALSE); // 最初は非アクティブにする

            // 「前ページ」ボタンを作成
            g_hPrevPageButton = CreateWindow(
                L"BUTTON", L"前ページ", WS_VISIBLE | WS_CHILD,
                350, 150, 100, 40, hWnd, reinterpret_cast<HMENU>(3), NULL, NULL
            );
            SendMessage(g_hPrevPageButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);

            // 「次ページ」ボタンを作成
            g_hNextPageButton = CreateWindow(
                L"BUTTON", L"次ページ", WS_VISIBLE | WS_CHILD,
                450, 150, 100, 40, hWnd, reinterpret_cast<HMENU>(4), NULL, NULL
            );
            SendMessage(g_hNextPageButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
            break;

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                int buttonId = LOWORD(wParam);
                if (buttonId == 2) { // 「交代」ボタンがクリックされたら
                    ShowWindow(g_hMainWindow, SW_SHOW); // スタート画面を表示
                    ShowWindow(hWnd, SW_HIDE); // ゲーム選択画面を非表示
                    KillTimer(hWnd, 1); // タイマーを停止
                    g_timerActive = false;
                    EnableWindow(g_hSwapButton, FALSE); // 「交代」ボタンを非アクティブにする
                } else if (buttonId == 3) { // 「前ページ」ボタンがクリックされたら
                    // 前ページに移動
                    g_currentPage = (g_currentPage - 1 + g_maxPages) % g_maxPages;
                    // ボタンの表示を更新
                    UpdateGameButtons(hWnd);
                } else if (buttonId == 4) { // 「次ページ」ボタンがクリックされたら
                    // 次ページに移動
                    g_currentPage = (g_currentPage + 1) % g_maxPages;
                    // ボタンの表示を更新
                    UpdateGameButtons(hWnd);
                }
            }
            break;

        case WM_DESTROY:
            KillTimer(hWnd, 1); // タイマーを停止
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

// ゲーム選択ボタンを更新する関数
void UpdateGameButtons(HWND hWnd) {
    // ページ内の最大ボタン数
    const int maxButtonsPerPage = 6;
    // 現在のページで表示すべきボタンのインデックス範囲を計算
    int startIndex = g_currentPage * maxButtonsPerPage;
    int endIndex = std::min(startIndex + maxButtonsPerPage, static_cast<int>(g_gameNames.size()));

    // 以前のボタンを削除
    HWND child = GetWindow(hWnd, GW_CHILD);
    while (child != NULL) {
        DestroyWindow(child);
        child = GetWindow(child, GW_HWNDNEXT);
    }

    // ゲーム名に対応するボタンを作成
    int buttonIndex = 0;
    for (int i = startIndex; i < endIndex; ++i) {
        HWND hButton = CreateWindow(
            L"BUTTON", g_gameNames[i].c_str(), WS_VISIBLE | WS_CHILD,
            100, 200 + buttonIndex * 50, 300, 40, hWnd, reinterpret_cast<HMENU>(i + 1), NULL, NULL
        );
        SendMessage(hButton, WM_SETFONT, reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
        ++buttonIndex;
    }
}

// メイン関数
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ゲーム情報を設定（例として6つのゲームを追加）
    std::vector<std::wstring> g_gameNames;
    g_gameNames.push_back(L"game1");
    g_gameNames.push_back(L"game2");
    g_gameNames.push_back(L"game3");
    g_gameNames.push_back(L"game4");
    g_gameNames.push_back(L"game5");
    g_gameNames.push_back(L"game6");
    // 最大ページ数を計算
    g_maxPages = (g_gameNames.size() + 5) / 6;

    // ゲーム選択ウィンドウのクラスを登録
    WNDCLASS wc = {};
    wc.lpfnWndProc = GameSelectionWindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GameSelectionWindow";
    RegisterClass(&wc);

    // スタート画面のクラスを登録
    WNDCLASS startWc = {};
    startWc.lpfnWndProc = StartWindowProc;
    startWc.hInstance = hInstance;
    startWc.lpszClassName = L"StartWindow";
    RegisterClass(&startWc);

    // ゲーム選択ウィンドウを作成
    g_hGameSelectionWindow = CreateWindow(
        L"GameSelectionWindow", L"ゲーム選択",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        500, 300, NULL, NULL, hInstance, NULL
    );

    // スタート画面を作成
    g_hMainWindow = CreateWindow(
        L"StartWindow", L"スタート画面",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        500, 300, NULL, NULL, hInstance, NULL
    );

    // メッセージループ開始
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    if (g_timerActive) {
        KillTimer(g_hGameSelectionWindow, 1); // タイマーを停止
    }

    return static_cast<int>(msg.wParam);
}

