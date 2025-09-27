// 平移置换Dlg.h: 头文件
//

#pragma once

#include "afxwin.h"

// 定义最大大小
#define MAX_RECTANGLES 200
#define MAX_LINES  50
#define MAX_PLAN 1000
struct RelocationPlan {
    int fromId;  // 搬迁前地块编号
    int toId;    // 搬迁后地块编号
};
// 定义矩形结构体
struct Rectangles {
    double x;   // 位置x
    double y;   // 位置y
    double len; // 长度
    double wid; // 宽度
};

// 定义线结构体
struct Line {
    double x1; // 起始位置x
    double y1; // 起始位置y
    double x2; // 终点位置x
    double y2; // 终点位置y
};
// 定义院落空置信息结构体
struct CourtyardVacancyInfo {
    int nonVacantBlocks;  // 非空置块数
    int totalBlocks;      // 总块数
};
// 定义矩形填充模式枚举
enum RectFillMode {
    RECT_FILL_NONE,       // 不填充颜色（默认边框模式）
    RECT_FILL_BEFORE,     // 根据"之前"数据填充颜色
    RECT_FILL_AFTER       // 根据"之后"数据填充颜色
};
// 前向声明
class CEmbeddedDialog;

// C平移置换Dlg 对话框
class C平移置换Dlg : public CDialogEx
{
    // 构造
public:
    C平移置换Dlg(CWnd* pParent = nullptr);    // 标准构造函数
    ~C平移置换Dlg();
    

    // 添加图形的方法
    void AddRectangle(double x, double y, double len, double wid);
    void AddLine(double x1, double y1, double x2, double y2);

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_MY_DIALOG };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

private:
    BOOL m_bDrawRectangle; // 是否绘制矩形
    BOOL m_bDrawLine;      // 是否绘制直线
    CPoint m_ptStart;      // 起始点
    CPoint m_ptEnd;        // 终点

    // 图形集合
    Rectangles rectanglesCollection[MAX_RECTANGLES];
    int rectanglesCount = 0;

    Line linesCollection[MAX_LINES];
    int linesCount = 0;

    CRichEditCtrl m_richEdit; // 富文本编辑框控件
    void AppendTextToRichEdit(const CString& strText, COLORREF color = RGB(0, 0, 0));

    bool ExportRectanglesToCSV(); // 导出院落位置信息到CSV
    bool ExportLinesToCSV();      // 导出街道位置信息到CSV

    bool ImportRectanglesFromCSV();  // 从CSV导入院落位置信息
    bool ImportLinesFromCSV();       // 从CSV导入街道位置信息

    // 存储院落到最近街道的距离数组
    double m_distanceArray[MAX_RECTANGLES];
    bool m_bDistanceCalculated; // 标记距离是否已计算

    // 计算点到线段的最短距离
    double CalculatePointToLineDistance(double px, double py, double x1, double y1, double x2, double y2);

    // 计算所有院落到最近街道的距离
    bool CalculateAllDistances();

    // 导出距离数据到CSV文件
    bool ExportDistancesToCSV();

    // 院落空置信息集合（"之前"和"之后"）
    CourtyardVacancyInfo beforeVacancyInfoCollection[MAX_RECTANGLES];
    CourtyardVacancyInfo afterVacancyInfoCollection[MAX_RECTANGLES];
    // 处理"之前"的院落空置信息
    bool ProcessBeforeVacancyInfo2();
    // 处理"之后"的院落空置信息
    bool ProcessAfterVacancyInfo2();
    // 处理"之前"的院落空置信息
    bool ProcessBeforeVacancyInfo3();
    // 处理"之后"的院落空置信息
    bool ProcessAfterVacancyInfo3();
    bool ProcessOriginalVacancyInfoFromCSV();
    RectFillMode m_rectFillMode; // 矩形填充模式

    // 搬迁方案数组
    RelocationPlan m_relocationPlan[MAX_PLAN];
    int m_relocationPlanCount;  // 搬迁方案数量

    // 从CSV读取搬迁方案
    bool ImportRelocationPlan(const CString& filePath);

    // 基于搬迁方案更新afterVacancyInfoCollection
    void UpdateAfterVacancyInfo();
    // 实现
protected:
    HICON m_hIcon;
    CEmbeddedDialog* m_pEmbeddedDialog; // 使用指针而不是对象

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedButtonLoadImage();
    void DisplayImageInEmbeddedDialog(CImage& image);

    afx_msg void OnBnClickedRadio1();
    afx_msg void OnBnClickedRadio2();
    afx_msg void OnBnClickedRadio3();
    afx_msg void OnBnClickedButtonBack();
    afx_msg void OnStnClickedStaticEmbedded();

    // 添加公共方法供子对话框访问
    int GetRectanglesCount() const { return this->rectanglesCount; }
    int GetLinesCount() const { return this->linesCount; }
    Rectangles GetRectangleAt(int index) const { return this->rectanglesCollection[index]; }
    Line GetLineAt(int index) const { return this->linesCollection[index]; }

    // 获取院落空置信息的公共方法
    CourtyardVacancyInfo GetBeforeVacancyInfoAt(int index) const { return this->beforeVacancyInfoCollection[index]; }
    CourtyardVacancyInfo GetAfterVacancyInfoAt(int index) const { return this->afterVacancyInfoCollection[index]; }
    bool IsBeforeCourtYardVacant(int index) const;
    bool IsAfterCourtYardVacant(int index) const;
    // 获取当前填充模式
    RectFillMode GetRectFillMode() const { return m_rectFillMode; }
    bool ProcessVacancyInfo(const CString& fileName);
    // 获取颜色填充比例
    bool ProcessRelocationData(const CString& fileName);
    double GetVacancyFillRatio(int index, bool isBeforeData) const;
    afx_msg void OnBnClickedButton2();
    afx_msg void OnBnClickedButton3();
    afx_msg void OnBnClickedButton4();
    afx_msg void OnBnClickedButton5();
    afx_msg void OnBnClickedButton6();
    afx_msg void OnBnClickedButton7();
    afx_msg void OnBnClickedButton8();
    afx_msg void OnBnClickedButton9();
    afx_msg void OnBnClickedButton12();
    afx_msg void OnBnClickedButton10();
    afx_msg void OnBnClickedButton11();
};
