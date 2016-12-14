#pragma once

#include "CommandList.h"
#include "PanedWindowContainer.h"
#include "SplitWindow.h"

#define FILE_MENU_POSITION    0
#define RECENT_MENU_POSITION    6
#define POPUP_MENU_POSITION    0

LPCTSTR g_lpcstrMRURegKey = _T("Software\\Naver\\Sling\\GfxTestView");
LPCTSTR g_lpcstrApp = _T("GfxTestView");

class CMainFrame : public CFrameWindowImpl<CMainFrame>, public CUpdateUI<CMainFrame>,
        public CMessageFilter, public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
    CGfxVerticalSplitterWindow m_wndVertSplit;
    CGfxHorizontalSplitterWindow m_wndHorzSplit;
    CPanedWindowContainer m_wndVerticalLeftContainer;
    CPanedWindowContainer m_wndHorzBottomContainer;

    CCommandList m_commandList;
    CCommandList m_outputList;

    CCommandBarCtrl m_CmdBar;
    CRecentDocumentList m_mru;
    CMruList m_list;
    CSnapshotView m_gfxView;

    TCHAR m_szFilePath[MAX_PATH];

    TestRunner* m_testRunner;

    CMainFrame() 
    {
        m_szFilePath[0] = 0;
    }

    virtual BOOL PreTranslateMessage(MSG* pMsg)
    {
        if (pMsg->message == WM_LBUTTONDBLCLK) {
            LVITEM item = { LVIF_PARAM };
            int selectedItem = m_commandList.GetSelectedItem(&item);
            if (selectedItem) {
                CString step;
                m_commandList.GetItemText(item.iItem, 0, step);
                
                CString objPtr;
                m_commandList.GetItemText(item.iItem, 1, objPtr);

                int stepNum = _ttoi(step);
                m_testRunner->GetPlaybackManager()->PlaybackToEvent(stepNum);

                int objPtrNum = _tcstol(objPtr, NULL, 16);
                m_testRunner->UpdateSnapshot((void*)(intptr_t)objPtrNum);
            }
        }

        if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
            return TRUE;

        return m_gfxView.PreTranslateMessage(pMsg);
    }

    virtual BOOL OnIdle()
    {
        BOOL bEnable = !m_gfxView.m_bmp.IsNull();
        UIEnable(ID_EDIT_COPY, bEnable);
        UIEnable(ID_EDIT_PASTE, ::IsClipboardFormatAvailable(CF_BITMAP));
        UIEnable(ID_EDIT_CLEAR, bEnable);
        UIEnable(ID_VIEW_PROPERTIES, bEnable);
        UISetCheck(ID_RECENT_BTN, m_list.IsWindowVisible());
        UIUpdateToolBar();

        return FALSE;
    }

    void TogglePrintPreview()
    {
        UpdateLayout();
    }

    void UpdateTitleBar(LPCTSTR lpstrTitle)
    {
        CString strDefault;
        strDefault.LoadString(IDR_MAINFRAME);
        CString strTitle = strDefault;
        if (lpstrTitle != NULL) {
            strTitle = lpstrTitle;
            strTitle += _T(" - ");
            strTitle += strDefault;
        }
        SetWindowText(strTitle);
    }

    void InitLayout()
    {
        // Create the splitter windows.
        m_wndVertSplit.Create(*this, rcDefault);
        m_wndHorzSplit.Create(m_wndVertSplit, rcDefault);

        // Create a left pane client of the gfx command list.
        m_commandList.Create(m_wndVertSplit, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);

        // Create a right pane container for the gfx view.
        m_wndVerticalLeftContainer.Create(m_wndVertSplit, _T("GFX Commands"));

        // Create a bottom pane container of the webview
        m_wndHorzBottomContainer.Create(m_wndHorzSplit, _T("output"));

        // Create the gfx view window
        m_gfxView.Create(m_wndHorzSplit, rcDefault, _T("GFX View"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);
        m_gfxView.SetBitmap(NULL);

        // Create a bottom pane client of the output list.
        m_outputList.Create(m_wndHorzBottomContainer, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_HSCROLL | WS_VSCROLL, WS_EX_CLIENTEDGE);

        m_hWndClient = m_wndVertSplit;

        // Set up the pane containers
        m_wndVerticalLeftContainer.SetClient(m_commandList);
        m_wndHorzBottomContainer.SetClient(m_outputList);

        m_wndVerticalLeftContainer.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);
        m_wndHorzBottomContainer.SetPaneContainerExtendedStyle(PANECNT_NOCLOSEBUTTON);

        // Set up the splitter panes
        m_wndHorzSplit.SetSplitterPanes(m_gfxView, m_wndHorzBottomContainer);
        m_wndVertSplit.SetSplitterPanes(m_wndVerticalLeftContainer, m_wndHorzSplit);

        // Set the splitter as the client area window, and resize the splitter to match the frame size.
        UpdateLayout();

        m_wndVertSplit.m_cxyMin = 150;
        m_wndHorzSplit.m_cxyMin = 100;

        m_wndVertSplit.SetSplitterPos(530);
        m_wndHorzSplit.SetSplitterExtendedStyle(SPLIT_BOTTOMALIGNED);
        m_wndHorzSplit.SetSplitterPos(500);

        m_commandList.Init();

        m_outputList.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT);
        m_outputList.InsertColumn(0, CString(LPCTSTR(_T("Command                                "))), LVCFMT_LEFT, 0, 0);
        m_outputList.InsertColumn(1, CString(LPCTSTR(_T("Sum of times     "))), LVCFMT_LEFT, 0, 1);
        m_outputList.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);
        m_outputList.SetColumnWidth(1, LVSCW_AUTOSIZE_USEHEADER);
    }

    BEGIN_MSG_MAP_EX(CMainFrame)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CONTEXTMENU(OnContextMenu)

        COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, OnFileOpen)
        COMMAND_RANGE_HANDLER_EX(ID_FILE_MRU_FIRST, ID_FILE_MRU_LAST, OnFileRecent)
        COMMAND_ID_HANDLER_EX(ID_RECENT_BTN, OnRecentButton)
        COMMAND_ID_HANDLER_EX(ID_TEST, OnTest);
        COMMAND_ID_HANDLER_EX(ID_FILE_PAGE_SETUP, OnFilePageSetup)
        COMMAND_ID_HANDLER_EX(ID_FILE_PRINT_PREVIEW, OnFilePrintPreview);
        COMMAND_ID_HANDLER_EX(ID_APP_EXIT, OnFileExit)
        COMMAND_ID_HANDLER_EX(ID_EDIT_COPY, OnEditCopy)
        COMMAND_ID_HANDLER_EX(ID_EDIT_PASTE, OnEditPaste)
        COMMAND_ID_HANDLER_EX(ID_EDIT_CLEAR, OnEditClear)
        COMMAND_ID_HANDLER_EX(ID_VIEW_TOOLBAR, OnViewToolBar)
        COMMAND_ID_HANDLER_EX(ID_VIEW_STATUS_BAR, OnViewStatusBar)
        COMMAND_ID_HANDLER_EX(ID_VIEW_PROPERTIES, OnViewProperties)
        COMMAND_ID_HANDLER_EX(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER_EX(ID_RECORD_REWIND, OnRecordRewind)
        COMMAND_ID_HANDLER_EX(ID_RECORD_BACK, OnRecordBack)
        COMMAND_ID_HANDLER_EX(ID_RECORD_PAUSE, OnRecordPause)
        COMMAND_ID_HANDLER_EX(ID_RECORD_FORWARD, OnRecordForward)
        COMMAND_ID_HANDLER_EX(ID_RECORD_INSPECTOR, OnRecordInspector)
        COMMAND_ID_HANDLER_EX(ID_RECORD_START, OnRecordStart)
        COMMAND_ID_HANDLER_EX(IDC_SHOW_BOTTOM_PANE, OnShowBottomPane)
        COMMAND_ID_HANDLER_EX(IDC_HIDE_BOTTOM_PANE, OnHideBottomPane)
        COMMAND_ID_HANDLER_EX(ID_PANE_CLOSE, OnHideBottomPane)
        COMMAND_ID_HANDLER_EX(IDC_LOCK_SPLITTERS, OnLockSplitters)
        COMMAND_ID_HANDLER_EX(IDC_UNLOCK_PANES, OnUnlockSplitters)
        COMMAND_ID_HANDLER_EX(IDC_PATTERN_BARS, OnPatternBars)
        COMMAND_ID_HANDLER_EX(IDC_NORMAL_BARS, OnNormalBars)
        COMMAND_ID_HANDLER_EX(IDC_COLOR_CONTAINERS, OnColoredContainers)
        COMMAND_ID_HANDLER_EX(IDC_NORMAL_CONTAINERS, OnNormalContainers)
        COMMAND_ID_HANDLER_EX(IDC_SETTINGS_EVENT_TIME, OnSettingEventTime)
        CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
        CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()

    BEGIN_UPDATE_UI_MAP(CMainFrame)
        UPDATE_ELEMENT(ID_TEST, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_FILE_PRINT_PREVIEW, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_EDIT_COPY, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_EDIT_PASTE, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_EDIT_CLEAR, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_PROPERTIES, UPDUI_MENUPOPUP | UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_RECENT_BTN, UPDUI_TOOLBAR)
        UPDATE_ELEMENT(IDC_SETTINGS_EVENT_TIME, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    int OnCreate(LPCREATESTRUCT /*lpCreateStruct*/)
    {
        // create command bar window
        HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
        // atach menu
        m_CmdBar.AttachMenu(GetMenu());
        // load command bar images
        m_CmdBar.LoadImages(IDR_MAINFRAME);
        // remove old menu
        SetMenu(NULL);

        // create toolbar and rebar
        HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

        CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);
        AddSimpleReBarBand(hWndCmdBar);
        AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

        // create status bar
        CreateSimpleStatusBar();

        // set up MRU stuff
        CMenuHandle menu = m_CmdBar.GetMenu();
        CMenuHandle menuFile = menu.GetSubMenu(FILE_MENU_POSITION);
        CMenuHandle menuMru = menuFile.GetSubMenu(RECENT_MENU_POSITION);
        m_mru.SetMenuHandle(menuMru);
        m_mru.SetMaxEntries(12);

        m_mru.ReadFromRegistry(g_lpcstrMRURegKey);

        // create MRU list
        m_list.Create(m_hWnd);

        // set up update UI
        UIAddToolBar(hWndToolBar);
        UISetCheck(ID_VIEW_TOOLBAR, 1);
        UISetCheck(ID_VIEW_STATUS_BAR, 1);
        UISetCheck(IDC_SETTINGS_EVENT_TIME, 0);

        CMessageLoop* pLoop = _Module.GetMessageLoop();
        ATLASSERT(pLoop != NULL);
        pLoop->AddMessageFilter(this);
        pLoop->AddIdleHandler(this);

        m_testRunner = new TestRunner(m_gfxView);
        m_testRunner->InitDefaultDrawTarget();

        InitLayout();

        return 0;
    }

    void OnSettingChange(UINT uWhatChanged, LPCTSTR szWhatChanged)
    {
        m_wndVertSplit.SendMessage(WM_SETTINGCHANGE);
    }

    void OnContextMenu(CWindow wnd, CPoint point)
    {
        if (wnd.m_hWnd == m_gfxView.m_hWnd) {
            CMenu menu;
            menu.LoadMenu(IDR_CONTEXTMENU);
            CMenuHandle menuPopup = menu.GetSubMenu(POPUP_MENU_POSITION);
            m_CmdBar.TrackPopupMenu(menuPopup, TPM_RIGHTBUTTON | TPM_VERTICAL, point.x, point.y);
        } else
            SetMsgHandled(FALSE);
    }

    void OnFileExit(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        PostMessage(WM_CLOSE);
    }

    void SetBitmap(HBITMAP hBmp)
    {
        m_gfxView.SetBitmap(hBmp);
    }

    void OnFileOpen(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        CFileDialog dlg(TRUE, _T("aer"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("aer Files (*.aer)\0*.aer\0All Files (*.*)\0*.*\0"), m_hWnd);
        if (dlg.DoModal() == IDOK) {
            std::ifstream fstream(dlg.m_szFileName, std::ifstream::binary | std::ifstream::in);
            if (fstream.is_open() && m_testRunner->LoadTest(fstream, BackendType::DIRECT2D1_1, m_commandList, m_outputList)) {
                m_mru.AddToList(dlg.m_ofn.lpstrFile);
                m_mru.WriteToRegistry(g_lpcstrMRURegKey);
                UpdateTitleBar(dlg.m_szFileTitle);
                lstrcpy(m_szFilePath, dlg.m_szFileName);
            } else {
                CString strMsg = _T("Can't load file from:\n");
                strMsg += dlg.m_szFileName;
                MessageBox(strMsg, g_lpcstrApp, MB_OK | MB_ICONERROR);
            }
        }
    }

    void OnFileRecent(UINT /*uNotifyCode*/, int nID, CWindow /*wnd*/)
    {
        // get file name from the MRU list
        TCHAR szFile[MAX_PATH];
        if (m_mru.GetFromList(nID, szFile, MAX_PATH)) {
            // open file
            std::ifstream fstream(szFile, std::ifstream::binary | std::ifstream::in);
            if (fstream.is_open() && m_testRunner->LoadTest(fstream, BackendType::DIRECT2D1_1, m_commandList, m_outputList)) {
                lstrcpy(m_szFilePath, szFile);
            } else
                m_mru.RemoveFromList(nID);

            m_mru.WriteToRegistry(g_lpcstrMRURegKey);
        } else
            ::MessageBeep(MB_ICONERROR);
    }

    void OnRecentButton(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        UINT uBandID = ATL_IDW_BAND_FIRST + 1;    // toolbar is second added band
        CReBarCtrl rebar = m_hWndToolBar;
        int nBandIndex = rebar.IdToIndex(uBandID);
        REBARBANDINFO rbbi = { 0 };
        rbbi.cbSize = RunTimeHelper::SizeOf_REBARBANDINFO();
        rbbi.fMask = RBBIM_CHILD;
        rebar.GetBandInfo(nBandIndex, &rbbi);
        CToolBarCtrl wndToolBar = rbbi.hwndChild;

        int nIndex = wndToolBar.CommandToIndex(ID_RECENT_BTN);
        CRect rect;
        wndToolBar.GetItemRect(nIndex, rect);
        wndToolBar.ClientToScreen(rect);

        // build and display MRU list in a popup
        m_list.BuildList(m_mru);
        m_list.ShowList(rect.left, rect.bottom);
    }

    void OnTest(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        int failures = 0;
        int result = m_testRunner->RunTests(&failures);
        return;
    }

    void OnFilePrintPreview(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
    }

    void OnFilePageSetup(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
    }

    void OnEditCopy(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        if (::OpenClipboard(NULL)) {
            HBITMAP hBitmapCopy = (HBITMAP)::CopyImage(m_gfxView.m_bmp.m_hBitmap, IMAGE_BITMAP, 0, 0, 0);
            if (hBitmapCopy != NULL)
                ::SetClipboardData(CF_BITMAP, hBitmapCopy);
            else
                MessageBox(_T("Can't copy bitmap"), g_lpcstrApp, MB_OK | MB_ICONERROR);

            ::CloseClipboard();
        } else
            MessageBox(_T("Can't open clipboard to copy"), g_lpcstrApp, MB_OK | MB_ICONERROR);
    }

    void OnEditPaste(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        if (::OpenClipboard(NULL)) {
            HBITMAP hBitmap = (HBITMAP)::GetClipboardData(CF_BITMAP);
            ::CloseClipboard();
            if (hBitmap != NULL) {
                HBITMAP hBitmapCopy = (HBITMAP)::CopyImage(hBitmap, IMAGE_BITMAP, 0, 0, 0);
                if (hBitmapCopy != NULL) {
                    m_gfxView.SetBitmap(hBitmapCopy);
                    UpdateTitleBar(_T("(Clipboard)"));
                    m_szFilePath[0] = 0;
                } else
                    MessageBox(_T("Can't paste bitmap"), g_lpcstrApp, MB_OK | MB_ICONERROR);
            } else
                MessageBox(_T("Can't open bitmap from the clipboard"), g_lpcstrApp, MB_OK | MB_ICONERROR);
        } else
            MessageBox(_T("Can't open clipboard to paste"), g_lpcstrApp, MB_OK | MB_ICONERROR);
    }

    void OnEditClear(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        m_gfxView.SetBitmap(NULL);
        UpdateTitleBar(NULL);
        m_szFilePath[0] = 0;
    }

    void OnViewToolBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        static BOOL bNew = TRUE;    // initially visible
        bNew = !bNew;
        UINT uBandID = ATL_IDW_BAND_FIRST + 1;    // toolbar is second added band
        CReBarCtrl rebar = m_hWndToolBar;
        int nBandIndex = rebar.IdToIndex(uBandID);
        rebar.ShowBand(nBandIndex, bNew);
        UISetCheck(ID_VIEW_TOOLBAR, bNew);
        UpdateLayout();
    }

    void OnViewStatusBar(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        BOOL bNew = !::IsWindowVisible(m_hWndStatusBar);
        ::ShowWindow(m_hWndStatusBar, bNew ? SW_SHOWNOACTIVATE : SW_HIDE);
        UISetCheck(ID_VIEW_STATUS_BAR, bNew);
        UpdateLayout();
    }

    void OnViewProperties(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        CBmpProperties prop;
        if (lstrlen(m_szFilePath) > 0)    // we have a file name
            prop.SetFileInfo(m_szFilePath, NULL);
        else                // must be clipboard then
            prop.SetFileInfo(NULL, m_gfxView.m_bmp.m_hBitmap);
        prop.DoModal();
    }

    void OnShowBottomPane(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndHorzSplit.SetSinglePaneMode(SPLIT_PANE_NONE);
    }

    void OnHideBottomPane(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndHorzSplit.SetSinglePaneMode(SPLIT_PANE_TOP);
    }

    void OnLockSplitters(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndHorzSplit.SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE, SPLIT_NONINTERACTIVE);
        m_wndVertSplit.SetSplitterExtendedStyle(SPLIT_NONINTERACTIVE, SPLIT_NONINTERACTIVE);
    }

    void OnUnlockSplitters(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndHorzSplit.SetSplitterExtendedStyle(0, SPLIT_NONINTERACTIVE);
        m_wndVertSplit.SetSplitterExtendedStyle(0, SPLIT_NONINTERACTIVE);
    }

    void OnPatternBars(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndVertSplit.m_cxySplitBar = 10;
        m_wndHorzSplit.m_cxySplitBar = 12;

        m_wndVertSplit.EnablePatternBar();
        m_wndHorzSplit.EnablePatternBar();
    }

    void OnNormalBars(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndVertSplit.m_cxySplitBar = 4;
        m_wndHorzSplit.m_cxySplitBar = 4;

        m_wndVertSplit.EnablePatternBar(false);
        m_wndHorzSplit.EnablePatternBar(false);
    }

    void OnColoredContainers(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndVerticalLeftContainer.EnableColoredHeader();
        m_wndHorzBottomContainer.EnableColoredHeader();
    }

    void OnNormalContainers(UINT uCode, int nID, HWND hwndCtrl)
    {
        m_wndVerticalLeftContainer.EnableColoredHeader(false);
        m_wndHorzBottomContainer.EnableColoredHeader(false);
    }

    void OnAppAbout(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        CSimpleDialog<IDD_ABOUTBOX> dlg;
        dlg.DoModal();
    }

    void OnRecordRewind(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {

    }

    void OnRecordBack(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {

    }

    void OnRecordPause(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {

    }

    void OnRecordForward(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {

    }

    void OnRecordInspector(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {

    }

    void OnRecordStart(UINT /*uNotifyCode*/, int /*nID*/, CWindow /*wnd*/)
    {
        m_testRunner->StartRecording();
    }

    void OnSettingEventTime(UINT uCode, int nID, HWND hwndCtrl)
    {
        BOOL oldStatus = m_testRunner->EnableCaculateEventTime();
        BOOL currentStatus = m_testRunner->CaculateEventTime();
        UISetCheck(IDC_SETTINGS_EVENT_TIME, m_testRunner->EnableCaculateEventTime());
        UpdateLayout();

        if (currentStatus && !oldStatus) {
            std::ifstream fstream(m_szFilePath, std::ifstream::binary | std::ifstream::in);
            fstream.is_open();
            m_testRunner->LoadTest(fstream, BackendType::DIRECT2D1_1, m_commandList, m_outputList);
        }
    }
};
