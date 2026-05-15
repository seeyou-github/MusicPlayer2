// OptionsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "OptionsDlg.h"


// COptionsDlg 对话框

IMPLEMENT_DYNAMIC(COptionsDlg, CBaseDialog)

COptionsDlg::COptionsDlg(CWnd* pParent /*=NULL*/)
	: CBaseDialog(IDD_OPTIONS_DIALOG, pParent)
{

}

COptionsDlg::~COptionsDlg()
{
}

CString COptionsDlg::GetDialogName() const
{
    return _T("OptionsDlg");
}

bool COptionsDlg::InitializeControls()
{
    wstring temp;
    temp = theApp.m_str_table.LoadText(L"TITLE_OPT");
    SetWindowTextW(temp.c_str());
    temp = theApp.m_str_table.LoadText(L"TXT_APPLY");
    SetDlgItemTextW(IDC_APPLY_BUTTON, temp.c_str());

    RepositionTextBasedControls({
        { CtrlTextInfo::R1, IDOK, CtrlTextInfo::W32 },
        { CtrlTextInfo::R2, IDCANCEL, CtrlTextInfo::W32 },
        { CtrlTextInfo::R3, IDC_APPLY_BUTTON, CtrlTextInfo::W32 }
        });
    return true;
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	CBaseDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_OPTIONS_TAB, m_tab);
}


BEGIN_MESSAGE_MAP(COptionsDlg, CBaseDialog)
	ON_BN_CLICKED(IDC_APPLY_BUTTON, &COptionsDlg::OnBnClickedApplyButton)
	ON_WM_DESTROY()
    ON_WM_GETMINMAXINFO()
    ON_WM_SIZE()
    ON_MESSAGE(WM_TAB_WINDOW_REQUIRED, &COptionsDlg::OnTabWindowRequired)
END_MESSAGE_MAP()


// COptionsDlg 消息处理程序


BOOL COptionsDlg::OnInitDialog()
{
	CBaseDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化

    SetIcon(IconMgr::IconType::IT_Setting, FALSE);

	//保存子对话框
	m_tab_vect.push_back(&m_tab1_dlg);
	m_tab_vect.push_back(&m_tab2_dlg);
	m_tab_vect.push_back(&m_tab3_dlg);
	m_tab_vect.push_back(&m_tab4_dlg);
	m_tab_vect.push_back(&m_media_lib_dlg);
	m_tab_vect.push_back(&m_tab5_dlg);
    m_tab_height.assign(m_tab_vect.size(), 0);
    m_tab_created.assign(m_tab_vect.size(), false);

	//添加对话框
    m_tab.AddWindowPlaceholder(theApp.m_str_table.LoadText(L"TITLE_OPT_LRC").c_str(), IconMgr::IconType::IT_Lyric);
    m_tab.AddWindowPlaceholder(theApp.m_str_table.LoadText(L"TITLE_OPT_APC").c_str(), IconMgr::IconType::IT_Skin);
    m_tab.AddWindowPlaceholder(theApp.m_str_table.LoadText(L"TITLE_OPT_DATA").c_str(), IconMgr::IconType::IT_Setting);
    m_tab.AddWindowPlaceholder(theApp.m_str_table.LoadText(L"TITLE_OPT_PLAY").c_str(), IconMgr::IconType::IT_Play);
    m_tab.AddWindowPlaceholder(theApp.m_str_table.LoadText(L"TITLE_OPT_MEDIA_LIB").c_str(), IconMgr::IconType::IT_Media_Lib);
    m_tab.AddWindowPlaceholder(theApp.m_str_table.LoadText(L"TITLE_OPT_HOT_KEY").c_str(), IconMgr::IconType::IT_Key_Board);

    m_tab.SetItemSize(CSize(theApp.DPI(60), theApp.DPI(24)));
    m_tab.AdjustTabWindowSize();

    if (m_tab_selected < 0 || m_tab_selected >= m_tab.GetItemCount())
        m_tab_selected = 0;
    CreateTabPage(m_tab_selected);
	m_tab.SetCurTab(m_tab_selected);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void COptionsDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类
    for (size_t i{}; i < m_tab_vect.size(); ++i)
    {
        if (IsTabCreated(static_cast<int>(i)))
            m_tab_vect[i]->GetDataFromUi();
    }

	CBaseDialog::OnOK();
}


void COptionsDlg::OnBnClickedApplyButton()
{
	// TODO: 在此添加控件通知处理程序代码
    for (size_t i{}; i < m_tab_vect.size(); ++i)
    {
        if (IsTabCreated(static_cast<int>(i)))
            m_tab_vect[i]->GetDataFromUi();
    }

	::SendMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_SETTINGS_APPLIED, (WPARAM)this, 0);

    for (size_t i{}; i < m_tab_vect.size(); ++i)
    {
        if (IsTabCreated(static_cast<int>(i)))
            m_tab_vect[i]->ApplyDataToUi();
    }
}


void COptionsDlg::OnDestroy()
{
	CBaseDialog::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	m_tab_selected = m_tab.GetCurSel();
}


void COptionsDlg::OnSize(UINT nType, int cx, int cy)
{
    CBaseDialog::OnSize(nType, cx, cy);
    if (nType != SIZE_MINIMIZED)
    {
        //为每个子窗口更新滚动信息
        for (size_t i = 0; i < m_tab_vect.size(); i++)
        {
            if (IsTabCreated(static_cast<int>(i)))
                m_tab_vect[i]->SetScrollbarInfo(m_tab.m_tab_rect.Height(), m_tab_height[i]);
        }
    }
}

bool COptionsDlg::CreateTabPage(int index)
{
    if (index < 0 || index >= static_cast<int>(m_tab_vect.size()))
        return false;
    if (IsTabCreated(index))
        return true;

    UINT dialog_id{};
    switch (index)
    {
    case 0: dialog_id = IDD_LYRIC_SETTING_DIALOG; break;
    case 1: dialog_id = IDD_APPEREANCE_SETTING_DLG; break;
    case 2: dialog_id = IDD_DATA_SETTINGS_DIALOG; break;
    case 3: dialog_id = IDD_PLAY_SETTING_DIALOG; break;
    case 4: dialog_id = IDD_MEDIA_LIB_SETTING_DIALOG; break;
    case 5: dialog_id = IDD_HOT_KEY_SETTINGS_DIALOG; break;
    default: return false;
    }

    CTabDlg* tab = m_tab_vect[index];
    if (tab == nullptr)
        return false;
    if (tab->GetSafeHwnd() == NULL && !tab->Create(dialog_id))
        return false;

    CRect rect;
    tab->GetWindowRect(rect);
    m_tab_height[index] = rect.Height();
    m_tab.SetTabWindow(index, tab);
    m_tab_created[index] = true;
    SetCreatedTabScrollbarInfo(index);
    return true;
}

void COptionsDlg::SetCreatedTabScrollbarInfo(int index)
{
    if (IsTabCreated(index))
        m_tab_vect[index]->SetScrollbarInfo(m_tab.m_tab_rect.Height(), m_tab_height[index]);
}

bool COptionsDlg::IsTabCreated(int index) const
{
    return index >= 0
        && index < static_cast<int>(m_tab_created.size())
        && m_tab_created[index]
        && m_tab_vect[index] != nullptr
        && m_tab_vect[index]->GetSafeHwnd() != NULL;
}

LRESULT COptionsDlg::OnTabWindowRequired(WPARAM wParam, LPARAM lParam)
{
    if (reinterpret_cast<CTabCtrlEx*>(lParam) == &m_tab)
        CreateTabPage(static_cast<int>(wParam));
    return 0;
}
