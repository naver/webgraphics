#pragma once

class CSnapshotView : public CScrollWindowImpl<CSnapshotView>
    , public TestRunner::Client
{
public:
    DECLARE_WND_CLASS_EX(NULL, 0, -1)

    CBitmap m_bmp;
    SIZE m_size;

    CSnapshotView()
    {
        m_size.cx = m_size.cy = 1;
    }

    BOOL PreTranslateMessage(MSG* pMsg)
    {
        pMsg;
        return FALSE;
    }

    void SetBitmap(HBITMAP hBitmap)
    {
        if (!m_bmp.IsNull())
            m_bmp.DeleteObject();

        m_bmp = hBitmap;

        if (!m_bmp.IsNull())
            m_bmp.GetSize(m_size);
        else
            m_size.cx = m_size.cy = 1;

        SetScrollOffset(0, 0, FALSE);
        SetScrollSize(m_size);
    }

    BEGIN_MSG_MAP(CSnapshotView)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
        CHAIN_MSG_MAP(CScrollWindowImpl<CSnapshotView>);
    END_MSG_MAP()

    LRESULT OnEraseBackground(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& /*bHandled*/)
    {
        RECT rect;
        GetClientRect(&rect);
        int x = 0;
        int y = 0;
        if (!m_bmp.IsNull()) {
            x = m_size.cx + 1;
            y = m_size.cy + 1; 
        }

        CDCHandle dc = (HDC)wParam;
        if (rect.right > m_sizeAll.cx) {
            RECT rectRight = rect;
            rectRight.left = x;
            rectRight.bottom = y;
            dc.FillRect(&rectRight, COLOR_WINDOW);
        }

        if (rect.bottom > m_sizeAll.cy) {
            RECT rectBottom = rect;
            rectBottom.top = y;
            dc.FillRect(&rectBottom, COLOR_WINDOW);
        }

        if (!m_bmp.IsNull()) {
            dc.MoveTo(m_size.cx, 0);
            dc.LineTo(m_size.cx, m_size.cy);
            dc.LineTo(0, m_size.cy);
        }
        return 0;
    }

    void DoPaint(CDCHandle dc)
    {
        if (!m_bmp.IsNull()) {
            CDC dcMem;
            dcMem.CreateCompatibleDC(dc);
            HBITMAP hBmpOld = dcMem.SelectBitmap(m_bmp);
            dc.BitBlt(0, 0, m_size.cx, m_size.cy, dcMem, 0, 0, SRCCOPY);
            dcMem.SelectBitmap(hBmpOld);
        }
    }

    void DisplaySnapshot(DataSourceSurface* aSnapshot) override
    {
        BITMAPINFO bmi;
        ZeroMemory(&bmi, sizeof(BITMAPINFO));
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        bmi.bmiHeader.biHeight = -aSnapshot->GetSize().height;
        bmi.bmiHeader.biWidth = aSnapshot->GetSize().width;
        bmi.bmiHeader.biPlanes = 1;

        HDC dc = CreateCompatibleDC(NULL);
        BYTE* ppvBits = NULL;
        BYTE* srcData = aSnapshot->GetData();
        LONG srcLine = 0;
        HBITMAP hbitmap = CreateDIBSection(dc, &bmi, DIB_RGB_COLORS, (VOID**)&ppvBits, NULL, 0);
        const LONG srcStride = aSnapshot->Stride();
        const LONG dstStride = bmi.bmiHeader.biWidth * (bmi.bmiHeader.biBitCount / 8);
        for (LONG y = 0; y < -bmi.bmiHeader.biHeight; ++y) {
            BYTE* srcRead = srcData + srcStride * srcLine++;
            BYTE* dstData = ppvBits + dstStride * y;
            for (LONG x = 0; x < bmi.bmiHeader.biWidth; ++x) {
                dstData[x * 4 + 0] = srcRead[x * 4 + 0];
                dstData[x * 4 + 1] = srcRead[x * 4 + 1];
                dstData[x * 4 + 2] = srcRead[x * 4 + 2];
                dstData[x * 4 + 3] = srcRead[x * 4 + 3];
            }
        }
        SetBitmap(hbitmap);
    }
};
