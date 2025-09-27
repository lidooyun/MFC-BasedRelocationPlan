// CEmbeddedDialog.cpp: 实现文件
//

#include "pch.h"
#include "平移置换.h"
#include "afxdialogex.h"
#include "CEmbeddedDialog.h"
#include "平移置换Dlg.h" // 在这里添加Dlg的头文件

// CEmbeddedDialog 对话框

IMPLEMENT_DYNAMIC(CEmbeddedDialog, CDialogEx)

CEmbeddedDialog::CEmbeddedDialog(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_EMBEDDED_DIALOG, pParent),
    m_bHasImage(false),
    m_pMainDlg(nullptr),
    m_bDrawRectangle(FALSE),
    m_bDrawLine(FALSE)
{
    //ModifyStyleEx(0, WS_EX_TRANSPARENT);
}

BOOL CEmbeddedDialog::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 在这里修改窗口样式，此时窗口已经创建
    ModifyStyleEx(0, WS_EX_TRANSPARENT);

    return TRUE;
}

CEmbeddedDialog::~CEmbeddedDialog()
{
    // 释放图像资源
    if (m_bHasImage)
    {
        m_Image.Destroy();
    }
}


void CEmbeddedDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CEmbeddedDialog, CDialogEx)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

// CEmbeddedDialog 消息处理程序
void CEmbeddedDialog::OnPaint()
{
    CPaintDC dc(this); // 用于绘制的设备上下文

    // 获取客户区矩形
    CRect clientRect;
    GetClientRect(clientRect);

    // 创建内存DC进行双缓冲绘制
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap memBitmap;
    memBitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
    CBitmap* pOldBitmap = memDC.SelectObject(&memBitmap);

    // 清除背景
    memDC.FillSolidRect(clientRect, GetSysColor(COLOR_WINDOW));

    // 设置背景透明
    memDC.SetBkMode(TRANSPARENT);

    // 如果有图像，先绘制图像
    if (m_bHasImage && m_Image.IsNull() == FALSE)
    {
        try
        {
            m_Image.Draw(memDC.GetSafeHdc(), 0, 0, clientRect.Width(), clientRect.Height());
        }
        catch (...)
        {
            // 绘制出错，可能是图像无效
            m_bHasImage = false;
        }
    }

    // 确保主窗口指针有效
    if (m_pMainDlg && ::IsWindow(m_pMainDlg->GetSafeHwnd()))
    {
        // 获取当前的矩形填充模式
        RectFillMode fillMode = m_pMainDlg->GetRectFillMode();

        // 绘制矩形
        for (int i = 0; i < m_pMainDlg->GetRectanglesCount(); i++)
        {
            Rectangles rect = m_pMainDlg->GetRectangleAt(i);
            CRect r(
                rect.x - rect.len / 2,
                rect.y - rect.wid / 2,
                rect.x + rect.len / 2,
                rect.y + rect.wid / 2
            );

            // 根据填充模式决定画刷
            CBrush brush;

            if (fillMode == RECT_FILL_NONE)
            {
                // 不填充，使用透明画刷
                brush.CreateStockObject(NULL_BRUSH);
            }
            else
            {
                // 获取填充比例
                double ratio = m_pMainDlg->GetVacancyFillRatio(i, fillMode == RECT_FILL_BEFORE);

                // 线性插值计算颜色：从白色(255,255,255)到红色(255,0,0)
                int red = 255;
                int green = static_cast<int>(255 * (1.0 - ratio));
                int blue = static_cast<int>(255 * (1.0 - ratio));

                brush.CreateSolidBrush(RGB(red, green, blue));

                // 如果需要，也可以在这里添加一个小文本显示填充比例
                CString strRatio;
                strRatio.Format(_T("%.2f"), ratio);
                memDC.SetTextColor(RGB(0, 0, 0));
                memDC.DrawText(strRatio, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            CBrush* pOldBrush = memDC.SelectObject(&brush);

            // 设置画笔
            CPen pen(PS_SOLID, 1, RGB(255, 0, 0)); // 红色画笔
            CPen* pOldPen = memDC.SelectObject(&pen);

            memDC.Rectangle(r);

            // 恢复原始画笔和画刷
            memDC.SelectObject(pOldPen);
            memDC.SelectObject(pOldBrush);

            // 如果需要显示院落编号，可以在这里添加
            CString strId;
            strId.Format(_T("%d"), i + 1);
            memDC.SetTextColor(RGB(0, 0, 0));
            memDC.DrawText(strId, r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        // 绘制直线
        for (int i = 0; i < m_pMainDlg->GetLinesCount(); i++)
        {
            Line line = m_pMainDlg->GetLineAt(i);

            // 设置画笔
            CPen pen(PS_SOLID, 2, RGB(0, 0, 255)); // 蓝色画笔
            CPen* pOldPen = memDC.SelectObject(&pen);

            memDC.MoveTo(line.x1, line.y1);
            memDC.LineTo(line.x2, line.y2);

            // 恢复原始画笔
            memDC.SelectObject(pOldPen);
        }
    }

    // 将内存DC的内容复制到屏幕DC
    dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);

    // 清理
    memDC.SelectObject(pOldBitmap);
}


void CEmbeddedDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
    m_ptStart = point;

    // 获取主窗口的单选框状态
    GetMainDialogRadioState();

    CDialogEx::OnLButtonDown(nFlags, point);
}

void CEmbeddedDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
    m_ptEnd = point;

    // 获取主窗口的单选框状态
    GetMainDialogRadioState();

    // 确保主窗口指针有效
    if (m_pMainDlg && ::IsWindow(m_pMainDlg->GetSafeHwnd()) && (m_bDrawRectangle || m_bDrawLine))
    {
        if (m_bDrawRectangle)
        {
            double len = abs(m_ptEnd.x - m_ptStart.x);
            double wid = abs(m_ptEnd.y - m_ptStart.y);
            double x = (m_ptStart.x + m_ptEnd.x) / 2.0;
            double y = (m_ptStart.y + m_ptEnd.y) / 2.0;

            m_pMainDlg->AddRectangle(x, y, len, wid);
        }
        else if (m_bDrawLine)
        {
            m_pMainDlg->AddLine(
                static_cast<double>(m_ptStart.x),
                static_cast<double>(m_ptStart.y),
                static_cast<double>(m_ptEnd.x),
                static_cast<double>(m_ptEnd.y)
            );
        }

        // 重绘
        Invalidate();
    }

    CDialogEx::OnLButtonUp(nFlags, point);
}

void CEmbeddedDialog::GetMainDialogRadioState()
{
    if (m_pMainDlg)
    {
        m_bDrawRectangle = m_pMainDlg->IsDlgButtonChecked(IDC_RADIO2) == BST_CHECKED;
        m_bDrawLine = m_pMainDlg->IsDlgButtonChecked(IDC_RADIO3) == BST_CHECKED;
    }
}

void CEmbeddedDialog::DrawImage(CImage& image)
{
    // 先释放之前的图像资源
    if (m_bHasImage)
    {
        m_Image.Destroy();
    }

    // 创建与源图像相同大小和格式的新图像
    m_Image.Create(image.GetWidth(), image.GetHeight(), image.GetBPP());

    // 复制图像数据
    HDC hdcSrc = image.GetDC();
    HDC hdcDst = m_Image.GetDC();

    ::BitBlt(hdcDst, 0, 0, image.GetWidth(), image.GetHeight(),
        hdcSrc, 0, 0, SRCCOPY);

    // 释放DC
    m_Image.ReleaseDC();
    image.ReleaseDC();

    m_bHasImage = true;

    // 重绘对话框
    Invalidate();
}

