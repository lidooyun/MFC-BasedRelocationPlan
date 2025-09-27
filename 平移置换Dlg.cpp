
// 平移置换Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "平移置换.h"
#include "平移置换Dlg.h"
#include "afxdialogex.h"
#include "CEmbeddedDialog.h"
#include <Windows.h>
#include <Shlwapi.h>  // 用于PathFileExists函数


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	
END_MESSAGE_MAP()


// C平移置换Dlg 对话框

C平移置换Dlg::~C平移置换Dlg()
{
	// 释放嵌入式对话框资源
	if (m_pEmbeddedDialog)
	{
		m_pEmbeddedDialog->DestroyWindow();
		delete m_pEmbeddedDialog;
		m_pEmbeddedDialog = NULL;
	}
}




C平移置换Dlg::C平移置换Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MY_DIALOG, pParent)
	, m_bDrawRectangle(FALSE)
	, m_bDrawLine(FALSE)
	, m_pEmbeddedDialog(NULL)
	, rectanglesCount(0)  
	, linesCount(0)       
	, m_bDistanceCalculated(false) // 初始化距离计算标记
	, m_rectFillMode(RECT_FILL_NONE) // 初始化填充模式为不填充
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	// 初始化距离数组
	memset(m_distanceArray, 0, sizeof(m_distanceArray));
	// 初始化院落空置信息数组
	memset(beforeVacancyInfoCollection, 0, sizeof(beforeVacancyInfoCollection));
	memset(afterVacancyInfoCollection, 0, sizeof(afterVacancyInfoCollection));
}



void C平移置换Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richEdit); // 关联富文本编辑框控件
}

BEGIN_MESSAGE_MAP(C平移置换Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_LOAD_IMAGE, &C平移置换Dlg::OnBnClickedButtonLoadImage)
	ON_BN_CLICKED(IDC_RADIO1, &C平移置换Dlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &C平移置换Dlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &C平移置换Dlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_BUTTON1, &C平移置换Dlg::OnBnClickedButtonBack)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_STN_CLICKED(IDC_STATIC_EMBEDDED, &C平移置换Dlg::OnStnClickedStaticEmbedded)
	ON_BN_CLICKED(IDC_BUTTON2, &C平移置换Dlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &C平移置换Dlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &C平移置换Dlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &C平移置换Dlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &C平移置换Dlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &C平移置换Dlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &C平移置换Dlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &C平移置换Dlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON12, &C平移置换Dlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON10, &C平移置换Dlg::OnBnClickedButton10)
	ON_BN_CLICKED(IDC_BUTTON11, &C平移置换Dlg::OnBnClickedButton11)
END_MESSAGE_MAP()


// C平移置换Dlg 消息处理程序

BOOL C平移置换Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	// 创建嵌入对话框
	// 创建嵌入对话框
	m_pEmbeddedDialog = new CEmbeddedDialog(this);
	if (!m_pEmbeddedDialog)
	{
		AfxMessageBox(L"无法分配内存创建嵌入式对话框!");
		return FALSE;
	}

	// 尝试创建对话框，并检查是否成功
	if (!m_pEmbeddedDialog->Create(IDD_EMBEDDED_DIALOG, this))
	{
		AfxMessageBox(L"无法创建嵌入式对话框!");
		delete m_pEmbeddedDialog;
		m_pEmbeddedDialog = NULL;
		return FALSE;
	}
	

	// 设置主对话框指针
	m_pEmbeddedDialog->SetMainDialog(this);

	// 获取Static Text控件的矩形区域
	CWnd* pStatic = GetDlgItem(IDC_STATIC_EMBEDDED);
	if (!pStatic || !::IsWindow(pStatic->GetSafeHwnd()))
	{
		AfxMessageBox(L"无法找到嵌入式控件!");
		delete m_pEmbeddedDialog;
		m_pEmbeddedDialog = NULL;
		return FALSE;
	}

	CRect rectStatic;
	pStatic->GetWindowRect(&rectStatic);
	ScreenToClient(&rectStatic); // 将屏幕坐标转换为客户区坐标

	// 调整嵌入对话框的位置和大小
	m_pEmbeddedDialog->MoveWindow(rectStatic.left, rectStatic.top, rectStatic.Width(), rectStatic.Height());

	// 显示嵌入对话框
	m_pEmbeddedDialog->ShowWindow(SW_SHOW);

	// 初始化富文本编辑框
	m_richEdit.SetBackgroundColor(FALSE, GetSysColor(COLOR_WINDOW));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void C平移置换Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void C平移置换Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}
 //在OnLButtonDown函数中处理鼠标按下事件

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR C平移置换Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void C平移置换Dlg::DisplayImageInEmbeddedDialog(CImage& image)
{
	// 直接调用嵌入对话框的DrawImage方法
	if (m_pEmbeddedDialog && ::IsWindow(m_pEmbeddedDialog->GetSafeHwnd()))
	{
		m_pEmbeddedDialog->DrawImage(image);
	}
}

void C平移置换Dlg::OnBnClickedButtonLoadImage()
{
	// 打开文件对话框，让用户选择图片文件
	CFileDialog fileDialog(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		L"Image Files (*.bmp;*.jpg;*.png)|*.bmp;*.jpg;*.png||");

	if (fileDialog.DoModal() == IDOK)
	{
		CString strFilePath = fileDialog.GetPathName(); // 获取用户选择的文件路径

		// 加载图片
		CImage image;
		HRESULT hr = image.Load(strFilePath); // 使用CImage加载图片

		if (SUCCEEDED(hr))
		{
			try
			{
				// 将图片显示在子窗口中
				DisplayImageInEmbeddedDialog(image);

				// 向富文本编辑框添加导入图片成功信息
				AppendTextToRichEdit(_T("导入图片成功"), RGB(0, 0, 0));
			}
			catch (...)
			{
				AfxMessageBox(L"处理图片时发生错误！");
			}
		}
		else
		{
			AfxMessageBox(L"无法加载图片！");
		}

		// 确保在函数结束前图像资源被正确释放
		image.Destroy();
	}
}


void C平移置换Dlg::OnBnClickedRadio1()
{
	// TODO: 在此添加控件通知处理程序代码
	m_bDrawRectangle = FALSE;
	m_bDrawLine = FALSE;
}
void C平移置换Dlg::OnBnClickedRadio2()
{
	// TODO: 在此添加控件通知处理程序代码
	// 允许绘制矩形
	m_bDrawRectangle = TRUE;
	m_bDrawLine = FALSE;
}
void C平移置换Dlg::OnBnClickedRadio3()
{
	// TODO: 在此添加控件通知处理程序代码
	// 允许绘制直线
	m_bDrawRectangle = FALSE;
	m_bDrawLine = TRUE;
}

void C平移置换Dlg::OnBnClickedButtonBack()
{
	// 回退操作
	if (m_bDrawRectangle)
	{
		// 回退矩形
		if (this->rectanglesCount > 0) // 使用this->引用类成员变量
		{
			// 获取要撤销的矩形信息
			Rectangles rect = this->rectanglesCollection[this->rectanglesCount - 1];

			// 向富文本编辑框添加撤销信息
			CString strInfo;
			strInfo.Format(_T("撤销一处院落，坐标为：%.2f, %.2f"), rect.x, rect.y);
			AppendTextToRichEdit(strInfo, RGB(255, 0, 0)); // 使用红色

			this->rectanglesCount--;
			if (m_pEmbeddedDialog)
			{
				m_pEmbeddedDialog->Invalidate(); // 重绘子对话框
			}
		}
	}
	else if (m_bDrawLine)
	{
		// 回退直线
		if (this->linesCount > 0) // 使用this->引用类成员变量
		{
			// 获取要撤销的直线信息
			Line line = this->linesCollection[this->linesCount - 1];

			// 向富文本编辑框添加撤销信息
			CString strInfo;
			strInfo.Format(_T("撤销一处街道，坐标为：(%.2f, %.2f), (%.2f, %.2f)"),
				line.x1, line.y1, line.x2, line.y2);
			AppendTextToRichEdit(strInfo, RGB(255, 0, 0)); // 使用红色

			this->linesCount--;
			if (m_pEmbeddedDialog)
			{
				m_pEmbeddedDialog->Invalidate(); // 重绘子对话框
			}
		}
	}
}

void C平移置换Dlg::OnStnClickedStaticEmbedded()
{
	// TODO: 在此添加控件通知处理程序代码
}

// 在平移置换Dlg.cpp中添加以下方法实现
void C平移置换Dlg::AddRectangle(double x, double y, double len, double wid)
{
	if (this->rectanglesCount < MAX_RECTANGLES)
	{
		// 使用this->引用类成员变量，明确区分
		this->rectanglesCollection[this->rectanglesCount++] = { x, y, len, wid };

		// 向富文本编辑框添加信息
		CString strInfo;
		strInfo.Format(_T("标注一处院落，坐标为：%.2f, %.2f"), x, y);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0)); // 使用绿色

		if (m_pEmbeddedDialog)
		{
			m_pEmbeddedDialog->Invalidate(); // 重绘子对话框
		}
	}
}

void C平移置换Dlg::AddLine(double x1, double y1, double x2, double y2)
{
	if (this->linesCount < MAX_LINES)
	{
		// 使用this->引用类成员变量，明确区分
		this->linesCollection[this->linesCount++] = { x1, y1, x2, y2 };

		// 向富文本编辑框添加信息
		CString strInfo;
		strInfo.Format(_T("标注一处街道，坐标为：(%.2f, %.2f), (%.2f, %.2f)"), x1, y1, x2, y2);
		AppendTextToRichEdit(strInfo, RGB(0, 0, 255)); // 使用蓝色

		if (m_pEmbeddedDialog)
		{
			m_pEmbeddedDialog->Invalidate(); // 重绘子对话框
		}
	}
}

// 在平移置换Dlg.cpp中实现
void C平移置换Dlg::AppendTextToRichEdit(const CString& strText, COLORREF color)
{
	// 获取当前文本长度
	int nTextLength = m_richEdit.GetTextLength();

	// 设置光标位置到文本末尾
	m_richEdit.SetSel(nTextLength, nTextLength);

	// 设置颜色
	CHARFORMAT cf;
	ZeroMemory(&cf, sizeof(CHARFORMAT));
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR;
	cf.crTextColor = color;
	m_richEdit.SetSelectionCharFormat(cf);

	// 插入文本
	m_richEdit.ReplaceSel(strText);

	// 添加换行符
	m_richEdit.ReplaceSel(_T("\r\n"));

	// 滚动到最后一行
	m_richEdit.LineScroll(m_richEdit.GetLineCount());
}

void C平移置换Dlg::OnBnClickedButton2()
{
	// 导出院落位置信息CSV文件
	ExportRectanglesToCSV();

	// 导出街道位置信息CSV文件
	ExportLinesToCSV();

	// 在富文本编辑框中显示导出成功信息
	AppendTextToRichEdit(_T("数据导出成功，文件保存在工作目录下"), RGB(0, 0, 0));
}

bool C平移置换Dlg::ExportRectanglesToCSV()
{
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("院落位置信息.csv");

	// 创建文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite))
	{
		CString strError;
		strError.Format(_T("无法创建文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 写入CSV文件头
		file.WriteString(_T("x,y,len,wid\n"));

		// 写入每条矩形数据
		for (int i = 0; i < rectanglesCount; i++)
		{
			CString strLine;
			strLine.Format(_T("%.2f,%.2f,%.2f,%.2f\n"),
				rectanglesCollection[i].x,
				rectanglesCollection[i].y,
				rectanglesCollection[i].len,
				rectanglesCollection[i].wid);

			file.WriteString(strLine);
		}

		// 关闭文件
		file.Close();

		// 在富文本编辑框中显示导出院落信息成功
		CString strInfo;
		strInfo.Format(_T("成功导出 %d 条院落位置信息到文件: %s"), rectanglesCount, strFilePath);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0));

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("导出院落位置信息时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

bool C平移置换Dlg::ExportLinesToCSV()
{
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("街道位置信息.csv");

	// 创建文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite))
	{
		CString strError;
		strError.Format(_T("无法创建文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 写入CSV文件头
		file.WriteString(_T("x1,y1,x2,y2\n"));

		// 写入每条直线数据
		for (int i = 0; i < linesCount; i++)
		{
			CString strLine;
			strLine.Format(_T("%.2f,%.2f,%.2f,%.2f\n"),
				linesCollection[i].x1,
				linesCollection[i].y1,
				linesCollection[i].x2,
				linesCollection[i].y2);

			file.WriteString(strLine);
		}

		// 关闭文件
		file.Close();

		// 在富文本编辑框中显示导出街道信息成功
		CString strInfo;
		strInfo.Format(_T("成功导出 %d 条街道位置信息到文件: %s"), linesCount, strFilePath);
		AppendTextToRichEdit(strInfo, RGB(0, 0, 255));

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("导出街道位置信息时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

void C平移置换Dlg::OnBnClickedButton3()
{
	// 导入院落位置信息
	if (ImportRectanglesFromCSV())
	{
		// 在富文本编辑框中显示导入成功信息
		CString strInfo;
		strInfo.Format(_T("成功导入院落位置信息，共 %d 条记录"), rectanglesCount);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0));
	}
	else
	{
		// 在富文本编辑框中显示导入失败信息
		AppendTextToRichEdit(_T("导入院落位置信息失败"), RGB(255, 0, 0));
	}
}

bool C平移置换Dlg::ImportRectanglesFromCSV()
{
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("院落位置信息.csv");

	// 打开文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 清空原有数据
		rectanglesCount = 0;

		// 读取并跳过CSV文件头
		CString strLine;
		if (file.ReadString(strLine))
		{
			// 检查CSV文件头是否符合格式要求
			if (strLine.Find(_T("x,y,len,wid")) == -1)
			{
				AfxMessageBox(_T("文件格式不正确，缺少必要的列标题"));
				file.Close();
				return false;
			}
		}

		// 逐行读取数据
		while (file.ReadString(strLine) && rectanglesCount < MAX_RECTANGLES)
		{
			// 解析CSV行
			double x, y, len, wid;
			int result = _stscanf_s(strLine, _T("%lf,%lf,%lf,%lf"), &x, &y, &len, &wid);

			if (result == 4) // 成功解析4个值
			{
				// 添加到院落集合
				rectanglesCollection[rectanglesCount].x = x;
				rectanglesCollection[rectanglesCount].y = y;
				rectanglesCollection[rectanglesCount].len = len;
				rectanglesCollection[rectanglesCount].wid = wid;

				rectanglesCount++;
			}
		}

		// 关闭文件
		file.Close();

		// 如果嵌入对话框存在，则重绘
		if (m_pEmbeddedDialog && ::IsWindow(m_pEmbeddedDialog->GetSafeHwnd()))
		{
			m_pEmbeddedDialog->Invalidate();
		}

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("导入院落位置信息时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

void C平移置换Dlg::OnBnClickedButton4()
{
	// 导入街道位置信息
	if (ImportLinesFromCSV())
	{
		// 在富文本编辑框中显示导入成功信息
		CString strInfo;
		strInfo.Format(_T("成功导入街道位置信息，共 %d 条记录"), linesCount);
		AppendTextToRichEdit(strInfo, RGB(0, 0, 255));
	}
	else
	{
		// 在富文本编辑框中显示导入失败信息
		AppendTextToRichEdit(_T("导入街道位置信息失败"), RGB(255, 0, 0));
	}
}

bool C平移置换Dlg::ImportLinesFromCSV()
{
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("街道位置信息.csv");

	// 打开文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 清空原有数据
		linesCount = 0;

		// 读取并跳过CSV文件头
		CString strLine;
		if (file.ReadString(strLine))
		{
			// 检查CSV文件头是否符合格式要求
			if (strLine.Find(_T("x1,y1,x2,y2")) == -1)
			{
				AfxMessageBox(_T("文件格式不正确，缺少必要的列标题"));
				file.Close();
				return false;
			}
		}

		// 逐行读取数据
		while (file.ReadString(strLine) && linesCount < MAX_LINES)
		{
			// 解析CSV行
			double x1, y1, x2, y2;
			int result = _stscanf_s(strLine, _T("%lf,%lf,%lf,%lf"), &x1, &y1, &x2, &y2);

			if (result == 4) // 成功解析4个值
			{
				// 添加到街道集合
				linesCollection[linesCount].x1 = x1;
				linesCollection[linesCount].y1 = y1;
				linesCollection[linesCount].x2 = x2;
				linesCollection[linesCount].y2 = y2;

				linesCount++;
			}
		}

		// 关闭文件
		file.Close();

		// 如果嵌入对话框存在，则重绘
		if (m_pEmbeddedDialog && ::IsWindow(m_pEmbeddedDialog->GetSafeHwnd()))
		{
			m_pEmbeddedDialog->Invalidate();
		}

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("导入街道位置信息时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

void C平移置换Dlg::OnBnClickedButton5()
{
	// 确保嵌入对话框存在
	if (m_pEmbeddedDialog && ::IsWindow(m_pEmbeddedDialog->GetSafeHwnd()))
	{
		// 强制重绘嵌入对话框
		m_pEmbeddedDialog->Invalidate();

		// 检查是否有院落或街道数据
		if (rectanglesCount > 0 || linesCount > 0)
		{
			// 在富文本编辑框中显示成功信息
			CString strInfo;
			strInfo.Format(_T("标注图生成成功，共 %d 处院落和 %d 处街道"), rectanglesCount, linesCount);
			AppendTextToRichEdit(strInfo, RGB(0, 0, 0));
		}
		else
		{
			// 在富文本编辑框中显示警告信息
			AppendTextToRichEdit(_T("当前没有院落或街道数据，请先导入或手动标注"), RGB(255, 165, 0));
		}
	}
	else
	{
		// 在富文本编辑框中显示错误信息
		AppendTextToRichEdit(_T("生成标注图失败，嵌入窗口无效"), RGB(255, 0, 0));
	}
}

void C平移置换Dlg::OnBnClickedButton6()
{
	// 计算所有院落到最近街道的距离
	if (CalculateAllDistances())
	{
		// 在富文本编辑框中显示计算成功信息
		CString strInfo;
		strInfo.Format(_T("成功计算 %d 个院落到最近街道的距离"), rectanglesCount);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0));

		// 在富文本框中显示部分计算结果（前5个或全部，如果少于5个）
		int showCount = min(5, rectanglesCount);
		for (int i = 0; i < showCount; i++)
		{
			CString strDistance;
			strDistance.Format(_T("院落 %d (%.2f, %.2f) 到最近街道的距离: %.2f"),
				i + 1, rectanglesCollection[i].x, rectanglesCollection[i].y, m_distanceArray[i]);
			AppendTextToRichEdit(strDistance, RGB(0, 0, 0));
		}

		// 如果有更多数据，显示省略信息
		if (rectanglesCount > showCount)
		{
			AppendTextToRichEdit(_T("... 更多数据已计算但未显示"), RGB(128, 128, 128));
		}
	}
	else
	{
		// 在富文本编辑框中显示计算失败信息
		AppendTextToRichEdit(_T("计算院落到街道的距离失败"), RGB(255, 0, 0));
	}
}
bool C平移置换Dlg::ExportDistancesToCSV()
{
	// 如果距离未计算，则无法导出
	if (!m_bDistanceCalculated)
	{
		AfxMessageBox(_T("距离数据尚未计算，请先计算再导出"));
		return false;
	}

	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("院落离街道的距离.csv");

	// 创建文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeCreate | CFile::modeWrite | CFile::typeText))
	{
		CString strError;
		strError.Format(_T("无法创建文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 写入CSV文件头
		file.WriteString(_T("id,x,y,distance\n"));
		//file.Flush();
		// 写入每个院落的距离数据
		for (int i = 0; i < rectanglesCount; i++)
		{
			CString strLine;
			strLine.Format(_T("%d,%.2f,%.2f,%.2f\n"),
				i + 1,
				rectanglesCollection[i].x,
				rectanglesCollection[i].y,
				m_distanceArray[i]);

			file.WriteString(strLine);
		}

		// 关闭文件
		file.Close();

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("导出距离数据时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

void C平移置换Dlg::OnBnClickedButton7()
{
	// 导出距离数据到CSV文件
	if (ExportDistancesToCSV())
	{
		// 在富文本编辑框中显示导出成功信息
		CString strInfo;
		strInfo.Format(_T("成功导出 %d 个院落到最近街道的距离数据"), rectanglesCount);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0));

		// 获取应用程序的工作目录
		TCHAR szPath[MAX_PATH];
		GetModuleFileName(NULL, szPath, MAX_PATH);

		// 提取路径部分
		CString strPath = szPath;
		int nPos = strPath.ReverseFind(_T('\\'));
		if (nPos > 0)
			strPath = strPath.Left(nPos + 1);

		// 显示文件保存位置
		CString strFilePath = strPath + _T("院落离街道的距离.csv");
		CString strFileInfo;
		strFileInfo.Format(_T("文件已保存至: %s"), strFilePath);
		AppendTextToRichEdit(strFileInfo, RGB(0, 0, 128));
	}
	else
	{
		// 在富文本编辑框中显示导出失败信息
		AppendTextToRichEdit(_T("导出距离数据失败"), RGB(255, 0, 0));
	}
}

double C平移置换Dlg::CalculatePointToLineDistance(double px, double py, double x1, double y1, double x2, double y2)
{
	// 计算线段长度的平方
	double lineLength2 = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);

	// 如果线段长度为0（即起点和终点重合），则直接计算点到起点的距离
	if (lineLength2 == 0.0)
	{
		return sqrt((px - x1) * (px - x1) + (py - y1) * (py - y1));
	}

	// 计算点到线段的投影比例
	double t = ((px - x1) * (x2 - x1) + (py - y1) * (y2 - y1)) / lineLength2;

	// 如果投影点在线段外，则计算点到线段端点的距离
	if (t < 0.0)
	{
		return sqrt((px - x1) * (px - x1) + (py - y1) * (py - y1)); // 到起点的距离
	}
	else if (t > 1.0)
	{
		return sqrt((px - x2) * (px - x2) + (py - y2) * (py - y2)); // 到终点的距离
	}

	// 计算投影点坐标
	double projX = x1 + t * (x2 - x1);
	double projY = y1 + t * (y2 - y1);

	// 计算点到投影点的距离
	return sqrt((px - projX) * (px - projX) + (py - projY) * (py - projY));
}
bool C平移置换Dlg::CalculateAllDistances()
{
	// 如果没有院落或街道数据，则无法计算
	if (rectanglesCount <= 0)
	{
		AfxMessageBox(_T("没有院落数据，无法计算距离"));
		return false;
	}

	if (linesCount <= 0)
	{
		AfxMessageBox(_T("没有街道数据，无法计算距离"));
		return false;
	}

	// 循环计算每个院落到最近街道的距离
	for (int i = 0; i < rectanglesCount; i++)
	{
		double minDistance = DBL_MAX; // 初始化为最大值

		// 获取院落中心点坐标
		double centerX = rectanglesCollection[i].x;
		double centerY = rectanglesCollection[i].y;

		// 计算该院落到每条街道的距离，取最小值
		for (int j = 0; j < linesCount; j++)
		{
			double distance = CalculatePointToLineDistance(
				centerX, centerY,
				linesCollection[j].x1, linesCollection[j].y1,
				linesCollection[j].x2, linesCollection[j].y2
			);

			// 更新最小距离
			if (distance < minDistance)
			{
				minDistance = distance;
			}
		}

		// 保存结果
		m_distanceArray[i] = minDistance;
	}

	// 标记距离已计算
	m_bDistanceCalculated = true;

	return true;
}

//第一问生成方案
void C平移置换Dlg::OnBnClickedButton8()
{
	// 向富文本框添加开始执行的提示
	AppendTextToRichEdit(_T("开始执行Python脚本..."), RGB(0, 0, 255));

	// 获取当前程序路径
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取目录路径
	CString strPath(szPath);
	int nPos = strPath.ReverseFind('\\');
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建Python脚本和CSV文件的完整路径
	CString strPyFilePath = strPath + _T("第一问代码.py");
	CString strCsvFile1 = strPath + _T("附件一：老城街区地块信息.csv");
	CString strCsvFile2 = strPath + _T("院落离街道的距离.csv");

	// 检查文件是否存在
	if (!PathFileExists(strPyFilePath))
	{
		AppendTextToRichEdit(_T("错误：Python脚本文件不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile1))
	{
		AppendTextToRichEdit(_T("错误：CSV文件1不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile2))
	{
		AppendTextToRichEdit(_T("错误：CSV文件2不存在！"), RGB(255, 0, 0));
		return;
	}

	// 获取正确的 Python 解释器路径
	CString strPythonPath;
	bool bPythonFound = false;

	// 方法1: 使用环境变量中设置的 Python 路径
	TCHAR szPythonHome[MAX_PATH] = { 0 };
	DWORD dwSize = MAX_PATH;
	if (GetEnvironmentVariable(_T("PYTHONHOME"), szPythonHome, dwSize))
	{
		strPythonPath = CString(szPythonHome) + _T("\\python.exe");
		if (PathFileExists(strPythonPath))
		{
			bPythonFound = true;
			AppendTextToRichEdit(_T("使用环境变量PYTHONHOME中的Python路径: ") + strPythonPath, RGB(0, 128, 0));
		}
	}

	// 如果环境变量中没有找到有效路径，尝试使用指定路径
	if (!bPythonFound)
	{
		// 方法2: 指定已知的 Python 安装路径
		strPythonPath = _T("C:\\Users\\asus\\AppData\\Local\\Programs\\Python\\Python39\\python.exe");

		if (PathFileExists(strPythonPath))
		{
			bPythonFound = true;
			AppendTextToRichEdit(_T("使用指定的Python路径: ") + strPythonPath, RGB(0, 128, 0));
		}
		else
		{
			// 方法3: 尝试在PATH环境变量中查找python
			TCHAR szPythonPath[MAX_PATH] = { 0 };
			if (SearchPath(NULL, _T("python.exe"), NULL, MAX_PATH, szPythonPath, NULL) > 0)
			{
				strPythonPath = szPythonPath;
				bPythonFound = true;
				AppendTextToRichEdit(_T("在系统PATH中找到Python: ") + strPythonPath, RGB(0, 128, 0));
			}
			else
			{
				// 输出错误信息
				AppendTextToRichEdit(_T("错误：环境变量未设置或Python未安装！"), RGB(255, 0, 0));
				AppendTextToRichEdit(_T("请确保安装了Python并正确设置了环境变量。"), RGB(255, 0, 0));
				return; // 退出函数，不再继续执行
			}
		}
	}

	AppendTextToRichEdit(_T("找到所有文件，开始执行..."), RGB(0, 128, 0));

	// 构建命令行，使用找到的 Python 解释器
	CString strCmd;
	strCmd.Format(_T("\"%s\" \"%s\" \"%s\" \"%s\""), strPythonPath, strPyFilePath, strCsvFile1, strCsvFile2);

	// 创建管道以捕获输出
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE hReadPipe, hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		AppendTextToRichEdit(_T("错误：无法创建管道！"), RGB(255, 0, 0));
		return;
	}

	// 设置进程启动信息
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	// 创建进程运行Python脚本
	BOOL bRet = CreateProcess(NULL, strCmd.GetBuffer(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	strCmd.ReleaseBuffer();

	if (!bRet)
	{
		DWORD dwError = GetLastError();
		CString strError;
		strError.Format(_T("错误：无法启动Python解释器！错误代码：%d"), dwError);

		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		AppendTextToRichEdit(strError, RGB(255, 0, 0));
		return;
}

	// 关闭不需要的写句柄
	CloseHandle(hWritePipe);

	// 读取Python脚本的输出
	char buffer[4096];
	DWORD bytesRead;

	// 循环读取输出直到进程结束
	while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead)
	{
		buffer[bytesRead] = '\0';

		// 将ANSI转换为Unicode (如果是Unicode版本的MFC)
#ifdef UNICODE
		int nLen = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0);
		WCHAR* wszBuffer = new WCHAR[nLen + 1];
		MultiByteToWideChar(CP_ACP, 0, buffer, -1, wszBuffer, nLen);

		// 按行分割输出内容
		CString strOutput = wszBuffer;
		delete[] wszBuffer;

		// 处理可能的多行输出
		int pos = 0;
		CString line;
		while ((line = strOutput.Tokenize(_T("\r\n"), pos)).GetLength() > 0)
		{
			// 使用您已有的函数显示输出
			AppendTextToRichEdit(line, RGB(0, 0, 0));
		}
#else
	// 非Unicode版本
		CString strOutput = buffer;

		// 处理可能的多行输出
		int pos = 0;
		CString line;
		while ((line = strOutput.Tokenize("\r\n", pos)).GetLength() > 0)
		{
			AppendTextToRichEdit(line, RGB(0, 0, 0));
		}
#endif
	}

	// 等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 获取进程退出码
	DWORD dwExitCode;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	// 清理
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hReadPipe);

	CString strExitInfo;
	strExitInfo.Format(_T("脚本执行完成，退出码：%d"), dwExitCode);
	AppendTextToRichEdit(strExitInfo, RGB(0, 128, 0));
}
bool C平移置换Dlg::ProcessRelocationData(const CString& filePath)
{
	// 打开文件
	CStdioFile file;
	if (!file.Open(filePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开文件: %s"), filePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 初始化院落空置信息数组
		memset(beforeVacancyInfoCollection, 0, sizeof(beforeVacancyInfoCollection));
		memset(afterVacancyInfoCollection, 0, sizeof(afterVacancyInfoCollection));

		// 读取并跳过CSV文件头
		CString strLine;
		if (!file.ReadString(strLine))
		{
			AfxMessageBox(_T("文件格式不正确，无法读取文件头"));
			file.Close();
			return false;
		}

		// 检查CSV文件头是否包含必要的列
		if (strLine.Find(_T("Id")) == -1 || strLine.Find(_T("newId")) == -1)
		{
			AfxMessageBox(_T("文件格式不正确，缺少必要的列（Id或newId）"));
			file.Close();
			return false;
		}

		// 创建两个临时数组，用于记录每个地块的居住状态
		// 0表示空置，1表示有人居住
		int* beforeBlockStatus = new int[MAX_RECTANGLES];
		int* afterBlockStatus = new int[MAX_RECTANGLES];

		// 初始化数组
		memset(beforeBlockStatus, 0, sizeof(int) * MAX_RECTANGLES);
		memset(afterBlockStatus, 0, sizeof(int) * MAX_RECTANGLES);

		// 用于临时记录每个院落的总地块数
		int* beforeTotalBlocks = new int[MAX_RECTANGLES];
		int* afterTotalBlocks = new int[MAX_RECTANGLES];

		// 初始化总地块数数组
		memset(beforeTotalBlocks, 0, sizeof(int) * MAX_RECTANGLES);
		memset(afterTotalBlocks, 0, sizeof(int) * MAX_RECTANGLES);

		// 逐行读取数据
		int lineCount = 0;
		while (file.ReadString(strLine) && lineCount < 10000) // 设置一个合理的上限
		{
			lineCount++;

			// 解析CSV行
			int id, newId;

			// 使用简单的CSV解析（假设字段之间用逗号分隔）
			int curPos = 0;
			CString token;

			// 读取第一列 (Id)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue; // 如果行解析错误，跳过此行
			id = _ttoi(token);

			// 读取第二列 (newId)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			newId = _ttoi(token);

			// 检查ID有效性
			if (id <= 0 || id > MAX_RECTANGLES || newId <= 0 || newId > MAX_RECTANGLES)
				continue;

			// 更新地块状态：原地块变为空置(0)，新地块变为有人居住(1)
			beforeBlockStatus[id - 1] = 0; // 原地块变为空置
			afterBlockStatus[newId - 1] = 1; // 新地块变为有人居住

			// 更新总地块数
			beforeTotalBlocks[id - 1]++;
			afterTotalBlocks[newId - 1]++;

			// 在富文本编辑框中显示处理进度（每100行显示一次）
			if (lineCount % 100 == 0)
			{
				CString strProgress;
				strProgress.Format(_T("已处理 %d 行数据..."), lineCount);
				AppendTextToRichEdit(strProgress, RGB(128, 128, 128));
			}
		}

		// 现在计算每个院落的非空置块数和总块数
		for (int i = 0; i < MAX_RECTANGLES; i++)
		{
			// 更新"之前"的院落空置信息
			beforeVacancyInfoCollection[i].totalBlocks = beforeTotalBlocks[i];
			if (beforeBlockStatus[i] == 1) // 如果地块有人居住
			{
				beforeVacancyInfoCollection[i].nonVacantBlocks++;
			}

			// 更新"之后"的院落空置信息
			afterVacancyInfoCollection[i].totalBlocks = afterTotalBlocks[i];
			if (afterBlockStatus[i] == 1) // 如果地块有人居住
			{
				afterVacancyInfoCollection[i].nonVacantBlocks++;
			}
		}

		// 释放临时数组
		delete[] beforeBlockStatus;
		delete[] afterBlockStatus;
		delete[] beforeTotalBlocks;
		delete[] afterTotalBlocks;

		// 关闭文件
		file.Close();

		// 显示处理结果摘要
		CString strSummary;
		int beforeNonVacant = 0, afterNonVacant = 0;
		int beforeTotal = 0, afterTotal = 0;

		for (int i = 0; i < MAX_RECTANGLES; i++)
		{
			beforeNonVacant += beforeVacancyInfoCollection[i].nonVacantBlocks;
			afterNonVacant += afterVacancyInfoCollection[i].nonVacantBlocks;
			beforeTotal += beforeVacancyInfoCollection[i].totalBlocks;
			afterTotal += afterVacancyInfoCollection[i].totalBlocks;
		}

		strSummary.Format(_T("处理完成，搬迁前：共 %d 个地块，其中 %d 个非空置；搬迁后：共 %d 个地块，其中 %d 个非空置"),
			beforeTotal, beforeNonVacant, afterTotal, afterNonVacant);
		AppendTextToRichEdit(strSummary, RGB(0, 128, 0));

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("处理搬迁数据时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

//问题二的
bool C平移置换Dlg::ProcessBeforeVacancyInfo2()
{
	// 这个函数现在只是一个包装，实际处理由ProcessRelocationData完成
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("方案二搬迁方案.csv");

	// 检查文件是否存在
	if (!PathFileExists(strFilePath))
	{
		AfxMessageBox(_T("搬迁方案文件不存在！"));
		return false;
	}

	// 调用新的处理函数
	return ProcessRelocationData(strFilePath);
}
bool C平移置换Dlg::ProcessAfterVacancyInfo2()
{
	// 这个函数现在也是一个包装，与ProcessBeforeVacancyInfo同样调用ProcessRelocationData
	// 但由于ProcessRelocationData已经同时处理了"之前"和"之后"的数据
	// 所以这里只需要确认数据已经被处理即可

	// 检查是否有数据已被处理
	bool hasData = false;
	for (int i = 0; i < MAX_RECTANGLES; i++)
	{
		if (afterVacancyInfoCollection[i].totalBlocks > 0)
		{
			hasData = true;
			break;
		}
	}

	// 如果没有数据，尝试重新处理
	if (!hasData)
	{
		return ProcessBeforeVacancyInfo2(); // 两个函数现在是等效的
	}

	return true; // 已有数据，不需要重新处理
}

//问题三的
bool C平移置换Dlg::ProcessBeforeVacancyInfo3()
{
	// 这个函数现在只是一个包装，实际处理由ProcessRelocationData完成
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("方案三搬迁方案.csv");

	// 检查文件是否存在
	if (!PathFileExists(strFilePath))
	{
		AfxMessageBox(_T("搬迁方案文件不存在！"));
		return false;
	}

	// 调用新的处理函数
	return ProcessRelocationData(strFilePath);
}
bool C平移置换Dlg::ProcessAfterVacancyInfo3()
{
	// 这个函数现在也是一个包装，与ProcessBeforeVacancyInfo同样调用ProcessRelocationData
	// 但由于ProcessRelocationData已经同时处理了"之前"和"之后"的数据
	// 所以这里只需要确认数据已经被处理即可

	// 检查是否有数据已被处理
	bool hasData = false;
	for (int i = 0; i < MAX_RECTANGLES; i++)
	{
		if (afterVacancyInfoCollection[i].totalBlocks > 0)
		{
			hasData = true;
			break;
		}
	}

	// 如果没有数据，尝试重新处理
	if (!hasData)
	{
		return ProcessBeforeVacancyInfo3(); // 两个函数现在是等效的
	}

	return true; // 已有数据，不需要重新处理
}

//第二问
void C平移置换Dlg::OnBnClickedButton9()
{
	// 向富文本框添加开始执行的提示
	AppendTextToRichEdit(_T("开始执行Python脚本..."), RGB(0, 0, 255));
	AppendTextToRichEdit(_T("本项执行时间根据计算机性能以及问题规模在数分钟到数十分钟不等，请耐心等待..."), RGB(0, 0, 255));
	// 获取当前程序路径
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取目录路径
	CString strPath(szPath);
	int nPos = strPath.ReverseFind('\\');
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建Python脚本和CSV文件的完整路径
	CString strPyFilePath = strPath + _T("第二问代码.py");
	CString strCsvFile1 = strPath + _T("问题一最终搬迁方案表-提供修缮补偿.csv");
	CString strCsvFile2 = strPath + _T("adjoin.csv");
	CString strCsvFile3 = strPath + _T("附件一：老城街区地块信息.csv");
	
	// 检查文件是否存在
	if (!PathFileExists(strPyFilePath))
	{
		AppendTextToRichEdit(_T("错误：Python脚本文件不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile1))
	{
		AppendTextToRichEdit(_T("错误：CSV文件1不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile2))
	{
		AppendTextToRichEdit(_T("错误：CSV文件2不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile3))
	{
		AppendTextToRichEdit(_T("错误：CSV文件3不存在！"), RGB(255, 0, 0));
		return;
	}

	// 获取正确的 Python 解释器路径
	CString strPythonPath;
	bool bPythonFound = false;

	// 方法1: 使用环境变量中设置的 Python 路径
	TCHAR szPythonHome[MAX_PATH] = { 0 };
	DWORD dwSize = MAX_PATH;
	if (GetEnvironmentVariable(_T("PYTHONHOME"), szPythonHome, dwSize))
	{
		strPythonPath = CString(szPythonHome) + _T("\\python.exe");
		if (PathFileExists(strPythonPath))
		{
			bPythonFound = true;
			AppendTextToRichEdit(_T("使用环境变量PYTHONHOME中的Python路径: ") + strPythonPath, RGB(0, 128, 0));
		}
	}

	// 如果环境变量中没有找到有效路径，尝试使用指定路径
	if (!bPythonFound)
	{
		// 方法2: 指定已知的 Python 安装路径
		strPythonPath = _T("C:\\Users\\asus\\AppData\\Local\\Programs\\Python\\Python39\\python.exe");

		if (PathFileExists(strPythonPath))
		{
			bPythonFound = true;
			AppendTextToRichEdit(_T("使用指定的Python路径: ") + strPythonPath, RGB(0, 128, 0));
		}
		else
		{
			// 方法3: 尝试在PATH环境变量中查找python
			TCHAR szPythonPath[MAX_PATH] = { 0 };
			if (SearchPath(NULL, _T("python.exe"), NULL, MAX_PATH, szPythonPath, NULL) > 0)
			{
				strPythonPath = szPythonPath;
				bPythonFound = true;
				AppendTextToRichEdit(_T("在系统PATH中找到Python: ") + strPythonPath, RGB(0, 128, 0));
			}
			else
			{
				// 输出错误信息
				AppendTextToRichEdit(_T("错误：环境变量未设置或Python未安装！"), RGB(255, 0, 0));
				AppendTextToRichEdit(_T("请确保安装了Python并正确设置了环境变量。"), RGB(255, 0, 0));
				return; // 退出函数，不再继续执行
			}
		}
	}

	AppendTextToRichEdit(_T("找到所有文件，开始执行..."), RGB(0, 128, 0));

	// 构建命令行，使用找到的 Python 解释器
	CString strCmd;
	strCmd.Format(_T("\"%s\" \"%s\" \"%s\" \"%s\" \"%s\""), strPythonPath, strPyFilePath, strCsvFile1, strCsvFile2,strCsvFile3);

	// 创建管道以捕获输出
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE hReadPipe, hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		AppendTextToRichEdit(_T("错误：无法创建管道！"), RGB(255, 0, 0));
		return;
	}

	// 设置进程启动信息
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	// 创建进程运行Python脚本
	BOOL bRet = CreateProcess(NULL, strCmd.GetBuffer(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	strCmd.ReleaseBuffer();

	if (!bRet)
	{
		DWORD dwError = GetLastError();
		CString strError;
		strError.Format(_T("错误：无法启动Python解释器！错误代码：%d"), dwError);

		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		AppendTextToRichEdit(strError, RGB(255, 0, 0));
		return;
	}

	// 关闭不需要的写句柄
	CloseHandle(hWritePipe);

	// 读取Python脚本的输出
	char buffer[4096];
	DWORD bytesRead;

	// 循环读取输出直到进程结束
	while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead)
	{
		buffer[bytesRead] = '\0';

		// 将ANSI转换为Unicode (如果是Unicode版本的MFC)
#ifdef UNICODE
		int nLen = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0);
		WCHAR* wszBuffer = new WCHAR[nLen + 1];
		MultiByteToWideChar(CP_ACP, 0, buffer, -1, wszBuffer, nLen);

		// 按行分割输出内容
		CString strOutput = wszBuffer;
		delete[] wszBuffer;

		// 处理可能的多行输出
		int pos = 0;
		CString line;
		while ((line = strOutput.Tokenize(_T("\r\n"), pos)).GetLength() > 0)
		{
			// 使用您已有的函数显示输出
			AppendTextToRichEdit(line, RGB(0, 0, 0));
		}
#else
	// 非Unicode版本
		CString strOutput = buffer;

		// 处理可能的多行输出
		int pos = 0;
		CString line;
		while ((line = strOutput.Tokenize("\r\n", pos)).GetLength() > 0)
		{
			AppendTextToRichEdit(line, RGB(0, 0, 0));
		}
#endif
	}

	// 等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 获取进程退出码
	DWORD dwExitCode;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	// 清理
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hReadPipe);

	CString strExitInfo;
	strExitInfo.Format(_T("脚本执行完成，退出码：%d"), dwExitCode);
	AppendTextToRichEdit(strExitInfo, RGB(0, 128, 0));

	// 提示开始导入
	AppendTextToRichEdit(_T("开始导入搬迁方案..."), RGB(0, 0, 255));

	// 获取应用程序的工作目录
	//TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	strPath = szPath;
	nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("问题二搬迁方案.csv");

	// 检查文件是否存在
	if (!PathFileExists(strFilePath))
	{
		AppendTextToRichEdit(_T("错误：搬迁方案文件不存在！"), RGB(255, 0, 0));
		return;
	}

	// 导入搬迁方案
	if (ImportRelocationPlan(strFilePath))
	{
		// 成功导入方案
		AppendTextToRichEdit(_T("搬迁方案导入成功！"), RGB(0, 128, 0));
	}
	else
	{
		// 导入方案失败
		AppendTextToRichEdit(_T("搬迁方案导入失败！"), RGB(255, 0, 0));
	}
}

void C平移置换Dlg::OnBnClickedButton12()
{
	// 向富文本框添加开始执行的提示
	AppendTextToRichEdit(_T("开始执行Python脚本..."), RGB(0, 0, 255));
	AppendTextToRichEdit(_T("本项执行时间根据计算机性能以及问题规模在数分钟到数十分钟不等，请耐心等待..."), RGB(0, 0, 255));
	// 获取当前程序路径
	TCHAR szPath[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取目录路径
	CString strPath(szPath);
	int nPos = strPath.ReverseFind('\\');
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建Python脚本和CSV文件的完整路径
	CString strPyFilePath = strPath + _T("第三问代码.py");
	CString strCsvFile1 = strPath + _T("问题一最终搬迁方案表-提供修缮补偿.csv");
	CString strCsvFile2 = strPath + _T("adjoin.csv");
	CString strCsvFile3 = strPath + _T("附件一：老城街区地块信息.csv");

	// 检查文件是否存在
	if (!PathFileExists(strPyFilePath))
	{
		AppendTextToRichEdit(_T("错误：Python脚本文件不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile1))
	{
		AppendTextToRichEdit(_T("错误：CSV文件1不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile2))
	{
		AppendTextToRichEdit(_T("错误：CSV文件2不存在！"), RGB(255, 0, 0));
		return;
	}

	if (!PathFileExists(strCsvFile3))
	{
		AppendTextToRichEdit(_T("错误：CSV文件3不存在！"), RGB(255, 0, 0));
		return;
	}

	// 获取正确的 Python 解释器路径
	CString strPythonPath;
	bool bPythonFound = false;

	// 方法1: 使用环境变量中设置的 Python 路径
	TCHAR szPythonHome[MAX_PATH] = { 0 };
	DWORD dwSize = MAX_PATH;
	if (GetEnvironmentVariable(_T("PYTHONHOME"), szPythonHome, dwSize))
	{
		strPythonPath = CString(szPythonHome) + _T("\\python.exe");
		if (PathFileExists(strPythonPath))
		{
			bPythonFound = true;
			AppendTextToRichEdit(_T("使用环境变量PYTHONHOME中的Python路径: ") + strPythonPath, RGB(0, 128, 0));
		}
	}

	// 如果环境变量中没有找到有效路径，尝试使用指定路径
	if (!bPythonFound)
	{
		// 方法2: 指定已知的 Python 安装路径
		strPythonPath = _T("C:\\Users\\asus\\AppData\\Local\\Programs\\Python\\Python39\\python.exe");

		if (PathFileExists(strPythonPath))
		{
			bPythonFound = true;
			AppendTextToRichEdit(_T("使用指定的Python路径: ") + strPythonPath, RGB(0, 128, 0));
		}
		else
		{
			// 方法3: 尝试在PATH环境变量中查找python
			TCHAR szPythonPath[MAX_PATH] = { 0 };
			if (SearchPath(NULL, _T("python.exe"), NULL, MAX_PATH, szPythonPath, NULL) > 0)
			{
				strPythonPath = szPythonPath;
				bPythonFound = true;
				AppendTextToRichEdit(_T("在系统PATH中找到Python: ") + strPythonPath, RGB(0, 128, 0));
			}
			else
			{
				// 输出错误信息
				AppendTextToRichEdit(_T("错误：环境变量未设置或Python未安装！"), RGB(255, 0, 0));
				AppendTextToRichEdit(_T("请确保安装了Python并正确设置了环境变量。"), RGB(255, 0, 0));
				return; // 退出函数，不再继续执行
			}
		}
	}

	AppendTextToRichEdit(_T("找到所有文件，开始执行..."), RGB(0, 128, 0));

	// 构建命令行，使用找到的 Python 解释器
	CString strCmd;
	strCmd.Format(_T("\"%s\" \"%s\" \"%s\" \"%s\" \"%s\""), strPythonPath, strPyFilePath, strCsvFile1, strCsvFile2, strCsvFile3);

	// 创建管道以捕获输出
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE hReadPipe, hWritePipe;
	if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0))
	{
		AppendTextToRichEdit(_T("错误：无法创建管道！"), RGB(255, 0, 0));
		return;
	}

	// 设置进程启动信息
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;

	PROCESS_INFORMATION pi;
	ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

	// 创建进程运行Python脚本
	BOOL bRet = CreateProcess(NULL, strCmd.GetBuffer(), NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
	strCmd.ReleaseBuffer();

	if (!bRet)
	{
		DWORD dwError = GetLastError();
		CString strError;
		strError.Format(_T("错误：无法启动Python解释器！错误代码：%d"), dwError);

		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		AppendTextToRichEdit(strError, RGB(255, 0, 0));
		return;
	}

	// 关闭不需要的写句柄
	CloseHandle(hWritePipe);

	// 读取Python脚本的输出
	char buffer[4096];
	DWORD bytesRead;

	// 循环读取输出直到进程结束
	while (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead)
	{
		buffer[bytesRead] = '\0';

		// 将ANSI转换为Unicode (如果是Unicode版本的MFC)
#ifdef UNICODE
		int nLen = MultiByteToWideChar(CP_ACP, 0, buffer, -1, NULL, 0);
		WCHAR* wszBuffer = new WCHAR[nLen + 1];
		MultiByteToWideChar(CP_ACP, 0, buffer, -1, wszBuffer, nLen);

		// 按行分割输出内容
		CString strOutput = wszBuffer;
		delete[] wszBuffer;

		// 处理可能的多行输出
		int pos = 0;
		CString line;
		while ((line = strOutput.Tokenize(_T("\r\n"), pos)).GetLength() > 0)
		{
			// 使用您已有的函数显示输出
			AppendTextToRichEdit(line, RGB(0, 0, 0));
		}
#else
	// 非Unicode版本
		CString strOutput = buffer;

		// 处理可能的多行输出
		int pos = 0;
		CString line;
		while ((line = strOutput.Tokenize("\r\n", pos)).GetLength() > 0)
		{
			AppendTextToRichEdit(line, RGB(0, 0, 0));
		}
#endif
	}

	// 等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 获取进程退出码
	DWORD dwExitCode;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	// 清理
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	CloseHandle(hReadPipe);

	CString strExitInfo;
	strExitInfo.Format(_T("脚本执行完成，退出码：%d"), dwExitCode);
	AppendTextToRichEdit(strExitInfo, RGB(0, 128, 0));

	// 提示开始导入
	AppendTextToRichEdit(_T("开始导入搬迁方案..."), RGB(0, 0, 255));

	// 获取应用程序的工作目录
	//TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	strPath = szPath;
	nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("问题三搬迁方案.csv");

	// 检查文件是否存在
	if (!PathFileExists(strFilePath))
	{
		AppendTextToRichEdit(_T("错误：搬迁方案文件不存在！"), RGB(255, 0, 0));
		return;
	}

	// 导入搬迁方案
	if (ImportRelocationPlan(strFilePath))
	{
		// 成功导入方案
		AppendTextToRichEdit(_T("搬迁方案导入成功！"), RGB(0, 128, 0));
	}
	else
	{
		// 导入方案失败
		AppendTextToRichEdit(_T("搬迁方案导入失败！"), RGB(255, 0, 0));
	}
}
bool C平移置换Dlg::ProcessVacancyInfo(const CString& fileName)
{
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + fileName;

	// 打开文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 初始化院落空置信息数组
		memset(beforeVacancyInfoCollection, 0, sizeof(beforeVacancyInfoCollection));
		memset(afterVacancyInfoCollection, 0, sizeof(afterVacancyInfoCollection));

		// 读取并跳过CSV文件头
		CString strLine;
		if (!file.ReadString(strLine))
		{
			AfxMessageBox(_T("文件格式不正确，无法读取文件头"));
			file.Close();
			return false;
		}

		// 逐行读取数据
		while (file.ReadString(strLine))
		{
			// 解析CSV行
			int col2Value, col3Value, col4Value, col5Value;

			// 使用简单的CSV解析（假设字段之间用逗号分隔且没有嵌套逗号）
			int curPos = 0;
			CString token;

			// 跳过第1列
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue; // 如果行解析错误，跳过此行

			// 读取第2列
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			col2Value = _ttoi(token);

			// 读取第3列
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			col3Value = _ttoi(token);

			// 读取第4列
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			col4Value = _ttoi(token);

			// 读取第5列
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			col5Value = _ttoi(token);

			// 更新"之前"院落空置信息 - 使用第2列和第4列
			if (col4Value > 0 && col4Value <= MAX_RECTANGLES)
			{
				int index = col4Value - 1; // 数组下标从0开始，ID从1开始
				beforeVacancyInfoCollection[index].totalBlocks++; // 总块数加1

				if (col2Value == 1)
				{
					beforeVacancyInfoCollection[index].nonVacantBlocks++; // 非空置块数加1
				}
			}

			// 更新"之后"院落空置信息 - 使用第3列和第5列
			if (col5Value > 0 && col5Value <= MAX_RECTANGLES)
			{
				int index = col5Value - 1; // 数组下标从0开始，ID从1开始
				afterVacancyInfoCollection[index].totalBlocks++; // 总块数加1

				if (col3Value != 0)
				{
					afterVacancyInfoCollection[index].nonVacantBlocks++; // 非空置块数加1
				}
			}
		}

		// 关闭文件
		file.Close();
		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("处理院落空置信息时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}


// 在cpp文件中实现
bool C平移置换Dlg::IsBeforeCourtYardVacant(int index) const
{
	if (index >= 0 && index < MAX_RECTANGLES)
	{
		// 如果非空置块数为0，则院落视为空置
		return beforeVacancyInfoCollection[index].nonVacantBlocks == 0;
	}
	return false;
}

bool C平移置换Dlg::IsAfterCourtYardVacant(int index) const
{
	if (index >= 0 && index < MAX_RECTANGLES)
	{
		// 如果非空置块数为0，则院落视为空置
		return afterVacancyInfoCollection[index].nonVacantBlocks == 0;
	}
	return false;
}

bool C平移置换Dlg::ProcessOriginalVacancyInfoFromCSV()
{
	// 获取应用程序的工作目录
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);

	// 提取路径部分
	CString strPath = szPath;
	int nPos = strPath.ReverseFind(_T('\\'));
	if (nPos > 0)
		strPath = strPath.Left(nPos + 1);

	// 构建文件完整路径
	CString strFilePath = strPath + _T("附件一：老城街区地块信息.csv");

	// 检查文件是否存在
	if (!PathFileExists(strFilePath))
	{
		CString strError;
		strError.Format(_T("无法找到文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	// 打开文件
	CStdioFile file;
	if (!file.Open(strFilePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开文件: %s"), strFilePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 初始化院落空置信息数组
		memset(beforeVacancyInfoCollection, 0, sizeof(beforeVacancyInfoCollection));

		// 读取并跳过CSV文件头
		CString strLine;
		if (!file.ReadString(strLine))
		{
			AfxMessageBox(_T("文件格式不正确，无法读取文件头"));
			file.Close();
			return false;
		}

		// 检查CSV文件头是否包含必要的列
		if (strLine.Find(_T("PlotID")) == -1 || strLine.Find(_T("YardID")) == -1 || strLine.Find(_T("IsInhabited")) == -1)
		{
			AfxMessageBox(_T("文件格式不正确，缺少必要的列（PlotID、YardID或IsInhabited）"));
			file.Close();
			return false;
		}

		// 逐行读取数据
		int lineCount = 0;
		while (file.ReadString(strLine) && lineCount < 10000) // 设置一个合理的上限
		{
			lineCount++;

			// 解析CSV行
			int plotID, yardID, isInhabited;

			// 使用简单的CSV解析（假设字段之间用逗号分隔）
			int curPos = 0;
			CString token;

			// 读取第一列 (PlotID)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue; // 如果行解析错误，跳过此行
			plotID = _ttoi(token);

			// 读取第二列 (YardID)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			yardID = _ttoi(token);

			// 跳过中间的三列 (PlotArea, YardArea, PlotDirection)
			for (int i = 0; i < 3; i++)
			{
				token = strLine.Tokenize(_T(","), curPos);
				if (curPos == -1) break;
			}

			if (curPos == -1) continue; // 如果解析出错，跳过此行

			// 读取第六列 (IsInhabited)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			isInhabited = _ttoi(token);

			// 检查ID有效性
			if (yardID <= 0 || yardID > MAX_RECTANGLES)
				continue;

			// 院落ID从1开始，数组下标从0开始，因此需要-1
			int index = yardID - 1;

			// 更新院落空置信息
			beforeVacancyInfoCollection[index].totalBlocks++; // 总块数加1

			if (isInhabited == 1)
			{
				beforeVacancyInfoCollection[index].nonVacantBlocks++; // 非空置块数加1
			}

			// 在富文本编辑框中显示处理进度（每100行显示一次）
			if (lineCount % 100 == 0)
			{
				CString strProgress;
				strProgress.Format(_T("已处理 %d 行数据..."), lineCount);
				AppendTextToRichEdit(strProgress, RGB(128, 128, 128));
			}
		}

		// 关闭文件
		file.Close();

		// 显示处理结果摘要
		CString strSummary;
		int nonVacantYards = 0, totalYards = 0;

		for (int i = 0; i < MAX_RECTANGLES; i++)
		{
			if (beforeVacancyInfoCollection[i].totalBlocks > 0)
			{
				totalYards++;
				if (beforeVacancyInfoCollection[i].nonVacantBlocks > 0)
				{
					nonVacantYards++;
				}
			}
		}

		strSummary.Format(_T("成功从CSV文件加载原始院落空置信息，共 %d 个院落，其中 %d 个有人居住"),
			totalYards, nonVacantYards);
		AppendTextToRichEdit(strSummary, RGB(0, 128, 0));

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("处理原始院落空置信息时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}
// 从CSV文件导入搬迁方案
bool C平移置换Dlg::ImportRelocationPlan(const CString& filePath)
{
	// 打开文件
	CStdioFile file;
	if (!file.Open(filePath, CFile::modeRead))
	{
		CString strError;
		strError.Format(_T("无法打开文件: %s"), filePath);
		AfxMessageBox(strError);
		return false;
	}

	try
	{
		// 清空现有方案
		m_relocationPlanCount = 0;

		// 读取并跳过CSV文件头
		CString strLine;
		if (!file.ReadString(strLine))
		{
			AfxMessageBox(_T("文件格式不正确，无法读取文件头"));
			file.Close();
			return false;
		}

		// 逐行读取数据
		while (file.ReadString(strLine) && m_relocationPlanCount < 1000)
		{
			// 解析CSV行
			int fromId, toId;

			// 使用简单的CSV解析
			int curPos = 0;
			CString token;

			// 读取第一列 (fromId)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue; // 如果行解析错误，跳过此行
			fromId = _ttoi(token);

			// 读取第二列 (toId)
			token = strLine.Tokenize(_T(","), curPos);
			if (curPos == -1) continue;
			toId = _ttoi(token);

			// 保存到搬迁方案数组
			m_relocationPlan[m_relocationPlanCount].fromId = fromId;
			m_relocationPlan[m_relocationPlanCount].toId = toId;
			m_relocationPlanCount++;
		}

		// 关闭文件
		file.Close();

		// 显示导入的搬迁方案数量
		CString strInfo;
		strInfo.Format(_T("成功导入 %d 条搬迁方案记录"), m_relocationPlanCount);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0));

		return true;
	}
	catch (CFileException* e)
	{
		e->Delete();
		file.Close();

		CString strError;
		strError.Format(_T("导入搬迁方案时发生错误"));
		AfxMessageBox(strError);
		return false;
	}
}

// 基于搬迁方案更新afterVacancyInfoCollection
void C平移置换Dlg::UpdateAfterVacancyInfo()
{
	// 先复制beforeVacancyInfoCollection到afterVacancyInfoCollection
	// 这样可以保留总块数等基础信息
	memcpy(afterVacancyInfoCollection, beforeVacancyInfoCollection, sizeof(CourtyardVacancyInfo) * MAX_RECTANGLES);

	// 然后根据搬迁方案更新afterVacancyInfoCollection
	for (int i = 0; i < m_relocationPlanCount; i++)
	{
		int fromId = m_relocationPlan[i].fromId;
		int toId = m_relocationPlan[i].toId;

		// 确保ID在有效范围内
		if (fromId > 0 && fromId <= MAX_RECTANGLES && toId > 0 && toId <= MAX_RECTANGLES)
		{
			// 搬迁前的院落ID对应的非空置块数减1（因为有人搬走了）
			if (afterVacancyInfoCollection[fromId - 1].nonVacantBlocks > 0)
				afterVacancyInfoCollection[fromId - 1].nonVacantBlocks--;

			// 搬迁后的院落ID对应的非空置块数加1（因为有人搬入了）
			afterVacancyInfoCollection[toId - 1].nonVacantBlocks++;

			// 确保非空置块数不超过总块数
			if (afterVacancyInfoCollection[toId - 1].nonVacantBlocks > afterVacancyInfoCollection[toId - 1].totalBlocks)
				afterVacancyInfoCollection[toId - 1].totalBlocks = afterVacancyInfoCollection[toId - 1].nonVacantBlocks;
		}
	}
}

void C平移置换Dlg::OnBnClickedButton10()
{
	// 首先从CSV文件读取并处理院落空置信息
	if (!ProcessOriginalVacancyInfoFromCSV())
	{
		AppendTextToRichEdit(_T("无法加载原始院落空置信息，请检查文件是否存在"), RGB(255, 0, 0));
		return;
	}

	// 检查是否有搬迁方案
	if (m_relocationPlanCount > 0)
	{
		// 基于搬迁方案更新afterVacancyInfoCollection
		UpdateAfterVacancyInfo();

		// 显示更新成功信息
		CString strInfo;
		strInfo.Format(_T("已基于 %d 条搬迁方案更新搬迁后院落空置状态"), m_relocationPlanCount);
		AppendTextToRichEdit(strInfo, RGB(0, 128, 0));
	}
	else
	{
		AppendTextToRichEdit(_T("警告：未找到搬迁方案，无法更新搬迁后的院落空置状态"), RGB(255, 165, 0));
	}

	// 首先检查是否有院落数据
	if (rectanglesCount <= 0)
	{
		AppendTextToRichEdit(_T("没有院落数据，无法进行颜色填充"), RGB(255, 0, 0));
		return;
	}

	// 检查是否有空置信息数据
	bool hasVacancyData = false;
	for (int i = 0; i < rectanglesCount; i++)
	{
		if (beforeVacancyInfoCollection[i].totalBlocks > 0)
		{
			hasVacancyData = true;
			break;
		}
	}

	if (!hasVacancyData)
	{
		AppendTextToRichEdit(_T("搬迁前的院落空置数据为空"), RGB(255, 165, 0));
		return;
	}

	// 设置矩形填充模式为"之前"
	m_rectFillMode = RECT_FILL_BEFORE;

	// 重绘嵌入对话框
	if (m_pEmbeddedDialog && ::IsWindow(m_pEmbeddedDialog->GetSafeHwnd()))
	{
		m_pEmbeddedDialog->Invalidate();

		// 在富文本编辑框中显示操作信息
		AppendTextToRichEdit(_T("已根据搬迁前院落空置信息进行着色"), RGB(0, 128, 0));

		// 显示一些填充比例的示例数据
		int showCount = min(5, rectanglesCount);
		for (int i = 0; i < showCount; i++)
		{
			if (beforeVacancyInfoCollection[i].totalBlocks > 0)
			{
				double ratio = static_cast<double>(beforeVacancyInfoCollection[i].nonVacantBlocks) /
					beforeVacancyInfoCollection[i].totalBlocks;

				CString strInfo;
				strInfo.Format(_T("院落 %d: 非空置块数 %d / 总块数 %d = 填充比例 %.2f"),
					i + 1,
					beforeVacancyInfoCollection[i].nonVacantBlocks,
					beforeVacancyInfoCollection[i].totalBlocks,
					ratio);

				AppendTextToRichEdit(strInfo, RGB(128, 128, 128));
			}
		}

		// 如果有更多数据，显示省略提示
		if (rectanglesCount > showCount)
		{
			AppendTextToRichEdit(_T("... 更多院落数据未显示"), RGB(128, 128, 128));
		}
	}
	else
	{
		AppendTextToRichEdit(_T("无法绘制，嵌入窗口无效"), RGB(255, 0, 0));
	}
}


void C平移置换Dlg::OnBnClickedButton11()
{
	// 首先检查是否有院落数据
	if (rectanglesCount <= 0)
	{
		AppendTextToRichEdit(_T("没有院落数据，无法进行颜色填充"), RGB(255, 0, 0));
		return;
	}

	// 检查是否有空置信息数据
	bool hasVacancyData = false;
	for (int i = 0; i < rectanglesCount; i++)
	{
		if (afterVacancyInfoCollection[i].totalBlocks > 0)
		{
			hasVacancyData = true;
			break;
		}
	}

	if (!hasVacancyData)
	{
		AppendTextToRichEdit(_T("缺少搬迁后的院落空置数据，请先导入或处理数据"), RGB(255, 165, 0));
		return;
	}

	// 设置矩形填充模式为"之后"
	m_rectFillMode = RECT_FILL_AFTER;

	// 重绘嵌入对话框
	if (m_pEmbeddedDialog && ::IsWindow(m_pEmbeddedDialog->GetSafeHwnd()))
	{
		m_pEmbeddedDialog->Invalidate();

		// 在富文本编辑框中显示操作信息
		AppendTextToRichEdit(_T("已根据搬迁后院落空置信息进行着色"), RGB(0, 128, 0));

		// 显示一些填充比例的示例数据
		int showCount = min(5, rectanglesCount);
		for (int i = 0; i < showCount; i++)
		{
			if (afterVacancyInfoCollection[i].totalBlocks > 0)
			{
				double ratio = static_cast<double>(afterVacancyInfoCollection[i].nonVacantBlocks) /
					afterVacancyInfoCollection[i].totalBlocks;

				CString strInfo;
				strInfo.Format(_T("院落 %d: 非空置块数 %d / 总块数 %d = 填充比例 %.2f"),
					i + 1,
					afterVacancyInfoCollection[i].nonVacantBlocks,
					afterVacancyInfoCollection[i].totalBlocks,
					ratio);

				AppendTextToRichEdit(strInfo, RGB(128, 128, 128));
			}
		}

		// 如果有更多数据，显示省略提示
		if (rectanglesCount > showCount)
		{
			AppendTextToRichEdit(_T("... 更多院落数据未显示"), RGB(128, 128, 128));
		}
	}
	else
	{
		AppendTextToRichEdit(_T("无法绘制，嵌入窗口无效"), RGB(255, 0, 0));
	}
}
double C平移置换Dlg::GetVacancyFillRatio(int index, bool isBeforeData) const
{
	// 确保索引有效
	if (index < 0 || index >= MAX_RECTANGLES)
		return 0.0;

	// 获取对应的空置信息
	const CourtyardVacancyInfo& info = isBeforeData ?
		beforeVacancyInfoCollection[index] : afterVacancyInfoCollection[index];

	// 如果总块数为0，返回0（表示全空）
	if (info.totalBlocks == 0)
		return 0.0;

	// 计算非空块占比
	return static_cast<double>(info.nonVacantBlocks) / info.totalBlocks;
}

//
//// 获取颜色填充比例（0表示全空，1表示全满）
//double C平移置换Dlg::GetVacancyFillRatio(int index, bool isBeforeData) const
//{
//	if (index < 0 || index >= MAX_RECTANGLES)
//		return 0.0;
//
//	const CourtyardVacancyInfo& info = isBeforeData ?
//		beforeVacancyInfoCollection[index] : afterVacancyInfoCollection[index];
//
//	// 如果总块数为0，返回0（表示全空）
//	if (info.totalBlocks == 0)
//		return 0.0;
//
//	// 计算非空块占比
//	return static_cast<double>(info.nonVacantBlocks) / info.totalBlocks;
//}