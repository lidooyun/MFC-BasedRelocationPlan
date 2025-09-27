#pragma once
#include "afxdialogex.h"
#include "afxwin.h"

// 在此处前向声明C平移置换Dlg类
class C平移置换Dlg;

// 需要在此处再次定义Rectangles和Line结构以避免循环包含
struct Rectangles;
struct Line;

// 声明填充模式枚举，确保与平移置换Dlg.h中的定义一致
enum RectFillMode;

// CEmbeddedDialog 对话框
class CEmbeddedDialog : public CDialogEx
{
    DECLARE_DYNAMIC(CEmbeddedDialog)

public:
    CEmbeddedDialog(CWnd* pParent = nullptr);   // 标准构造函数
    virtual ~CEmbeddedDialog();

    virtual BOOL OnInitDialog();

    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

    // 添加指向主对话框的指针
    void SetMainDialog(C平移置换Dlg* pMainDlg) { m_pMainDlg = pMainDlg; }

    // 添加绘制图像的方法
    void DrawImage(CImage& image);


private:
    // 保存正在显示的图像
    CImage m_Image;
    bool m_bHasImage;

    C平移置换Dlg* m_pMainDlg; // 指向主对话框的指针

    CPoint m_ptStart;
    CPoint m_ptEnd;
    BOOL m_bDrawRectangle;
    BOOL m_bDrawLine;

    // 获取主窗口的单选框状态
    void GetMainDialogRadioState();


    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_EMBEDDED_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    DECLARE_MESSAGE_MAP()
};
