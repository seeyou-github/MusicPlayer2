#include "stdafx.h"
#include "MusicPlayer2.h"
#include "ColorSettingDlg.h"
#include "CPlayerUIHelper.h"
#include "MusicPlayerDlg.h"

namespace
{
    constexpr int ID_COLOR_BUTTON_BASE{ 30000 };
    constexpr int ID_THEME_PRESET_BASE{ 30100 };
    constexpr int ID_THEME_PRESET_END{ ID_THEME_PRESET_BASE + 20 };
    constexpr int ID_THEME_MORE_COLOR{ 30200 };
    constexpr int ID_THEME_FOLLOW_SYSTEM{ 30201 };
    constexpr int ID_SONG_LIST_TEXT_THEME{ 30202 };
    constexpr int ID_SONG_LIST_PLAYING_TEXT_THEME{ 30203 };
    constexpr int ID_TEXT_GRADIENT_COMBO{ 30204 };
    constexpr int ID_HIGHLIGHT_GRADIENT_COMBO{ 30205 };
    constexpr int ID_DESKTOP_LYRIC_DEFAULT_STYLE{ 30206 };

    constexpr int LABEL_LEFT{ 20 };
    constexpr int LABEL_RIGHT{ 250 };
    constexpr int PREVIEW_LEFT{ 270 };
    constexpr int PREVIEW_RIGHT{ 295 };
    constexpr int BUTTON_LEFT{ 320 };
    constexpr int BUTTON_RIGHT{ 410 };
    constexpr int EXTRA_LEFT{ 430 };
    constexpr int EXTRA_RIGHT{ 560 };
}

IMPLEMENT_DYNAMIC(CColorSettingDlg, CTabDlg)

CColorSettingDlg::CColorSettingDlg(CWnd* pParent)
    : CTabDlg(IDD_COLOR_SETTING_DIALOG, pParent)
{
}

CColorSettingDlg::~CColorSettingDlg()
{
}

BEGIN_MESSAGE_MAP(CColorSettingDlg, CTabDlg)
    ON_COMMAND_RANGE(ID_COLOR_BUTTON_BASE, ID_COLOR_BUTTON_BASE + 99, &CColorSettingDlg::OnSelectColor)
    ON_CONTROL_RANGE(STN_CLICKED, ID_THEME_PRESET_BASE, ID_THEME_PRESET_END, &CColorSettingDlg::OnThemePreset)
    ON_BN_CLICKED(ID_THEME_MORE_COLOR, &CColorSettingDlg::OnBnClickedMoreThemeColor)
    ON_BN_CLICKED(ID_THEME_FOLLOW_SYSTEM, &CColorSettingDlg::OnBnClickedFollowSystemColor)
    ON_BN_CLICKED(ID_SONG_LIST_TEXT_THEME, &CColorSettingDlg::OnBnClickedSongListTextColorTheme)
    ON_BN_CLICKED(ID_SONG_LIST_PLAYING_TEXT_THEME, &CColorSettingDlg::OnBnClickedSongListPlayingTextColorTheme)
    ON_CBN_SELCHANGE(ID_TEXT_GRADIENT_COMBO, &CColorSettingDlg::OnCbnSelchangeDesktopLyricGradient)
    ON_CBN_SELCHANGE(ID_HIGHLIGHT_GRADIENT_COMBO, &CColorSettingDlg::OnCbnSelchangeDesktopLyricGradient)
    ON_BN_CLICKED(ID_DESKTOP_LYRIC_DEFAULT_STYLE, &CColorSettingDlg::OnBnClickedDefaultStyle)
    ON_COMMAND(ID_LYRIC_DEFAULT_STYLE1, &CColorSettingDlg::OnLyricDefaultStyle1)
    ON_COMMAND(ID_LYRIC_DEFAULT_STYLE2, &CColorSettingDlg::OnLyricDefaultStyle2)
    ON_COMMAND(ID_LYRIC_DEFAULT_STYLE3, &CColorSettingDlg::OnLyricDefaultStyle3)
    ON_COMMAND(ID_LYRIC_DEFAULT_STYLE1_MODIFY, &CColorSettingDlg::OnLyricDefaultStyle1Modify)
    ON_COMMAND(ID_LYRIC_DEFAULT_STYLE2_MODIFY, &CColorSettingDlg::OnLyricDefaultStyle2Modify)
    ON_COMMAND(ID_LYRIC_DEFAULT_STYLE3_MODIFY, &CColorSettingDlg::OnLyricDefaultStyle3Modify)
    ON_COMMAND(ID_RESTORE_DEFAULT_STYLE, &CColorSettingDlg::OnRestoreDefaultStyle)
END_MESSAGE_MAP()

bool CColorSettingDlg::InitializeControls()
{
    return true;
}

BOOL CColorSettingDlg::OnInitDialog()
{
    CTabDlg::OnInitDialog();
    CreateControls();
    CRect rect;
    GetWindowRect(rect);
    SetWindowPos(nullptr, 0, 0, rect.Width(), m_content_height, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    ApplyDataToUi();
    return TRUE;
}

void CColorSettingDlg::CreateControls()
{
    m_next_color_button_id = ID_COLOR_BUTTON_BASE;
    int y{ theApp.DPI(10) };

    AddThemeControls(y);
    AddSongListColorControls(y);
    AddDesktopLyricColorControls(y);

    AddSection(theApp.m_str_table.LoadText(L"TXT_OPT_COLOR_BOTTOM_LYRICS"), y);
    AddColorRow(L"played_text_color", &m_app_data.bottom_lyric_played_text_color, y);
    AddColorRow(L"unplayed_text_color", &m_app_data.bottom_lyric_unplayed_text_color, y);
    AddColorRow(L"next_text_color", &m_app_data.bottom_lyric_next_text_color, y);
    y += theApp.DPI(8);

    AddSection(theApp.m_str_table.LoadText(L"TXT_OPT_COLOR_LYRICS"), y);
    AddColorRow(L"text_color", &m_app_data.lyric_text_color, y);
    AddColorRow(L"playing_text_color", &m_app_data.lyric_playing_text_color, y);
    y += theApp.DPI(8);

    AddSection(theApp.m_str_table.LoadText(L"TXT_OPT_COLOR_MYPLAYERLIST_TAB"), y);
    AddColorRow(L"tab_background_color", &m_app_data.my_player_list_tab_background_color, y);
    AddColorRow(L"tab_selected_background_color", &m_app_data.my_player_list_tab_selected_background_color, y);
    AddColorRow(L"tab_unselected_background_color", &m_app_data.my_player_list_tab_unselected_background_color, y);
    AddColorRow(L"tab_selected_text_color", &m_app_data.my_player_list_tab_selected_text_color, y);
    AddColorRow(L"tab_unselected_text_color", &m_app_data.my_player_list_tab_unselected_text_color, y);
    y += theApp.DPI(8);

    AddSection(theApp.m_str_table.LoadText(L"TXT_OPT_COLOR_PROGRESS_BAR"), y);
    AddColorRow(L"progress_back_color", &m_app_data.progress_back_color, y);
    AddColorRow(L"progress_color", &m_app_data.progress_color, y);
    AddColorRow(L"time_color", &m_app_data.progress_time_color, y);

    m_content_height = y + theApp.DPI(20);
}

void CColorSettingDlg::AddSection(const std::wstring& text, int& y)
{
    CreateStatic(text, CRect(theApp.DPI(12), y, theApp.DPI(560), y + theApp.DPI(18)));
    y += theApp.DPI(26);
}

void CColorSettingDlg::AddColorRow(const std::wstring& text, COLORREF* color, int& y)
{
    auto row = std::make_unique<ColorRow>();
    row->id = m_next_color_button_id++;
    row->color = color;
    row->label.Create(text.c_str(), WS_CHILD | WS_VISIBLE | SS_RIGHT, CRect(theApp.DPI(LABEL_LEFT), y + theApp.DPI(3), theApp.DPI(LABEL_RIGHT), y + theApp.DPI(19)), this);
    SetControlFont(row->label);
    row->preview.Create(std::wstring().c_str(), WS_CHILD | WS_VISIBLE | SS_NOTIFY | WS_BORDER, CRect(theApp.DPI(PREVIEW_LEFT), y, theApp.DPI(PREVIEW_RIGHT), y + theApp.DPI(18)), this, row->id);
    row->preview.SetFillColor(*color);
    row->button.Create(theApp.m_str_table.LoadText(L"TXT_OPT_COLOR_SELECT").c_str(), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        CRect(theApp.DPI(BUTTON_LEFT), y, theApp.DPI(BUTTON_RIGHT), y + theApp.DPI(20)), this, row->id);
    SetControlFont(row->button);
    m_color_rows.push_back(std::move(row));
    y += theApp.DPI(28);
}

void CColorSettingDlg::AddThemeControls(int& y)
{
    CreateStatic(theApp.m_str_table.LoadText(L"TXT_OPT_APC_COLOR_THEME"), CRect(theApp.DPI(12), y, theApp.DPI(560), y + theApp.DPI(18)));
    y += theApp.DPI(22);

    CreateStatic(theApp.m_str_table.LoadText(L"TXT_OPT_APC_COLOR_THEME"), CRect(theApp.DPI(LABEL_LEFT), y + theApp.DPI(3), theApp.DPI(LABEL_RIGHT), y + theApp.DPI(19)), WS_CHILD | WS_VISIBLE | SS_RIGHT);
    m_theme_color_preview = CreateColorPreview(m_app_data.theme_color.original_color, CRect(theApp.DPI(PREVIEW_LEFT), y, theApp.DPI(PREVIEW_RIGHT), y + theApp.DPI(18)));
    m_more_theme_color_btn = CreateButton(theApp.m_str_table.LoadText(L"TXT_OPT_APC_COLOR_MORE"),
        CRect(theApp.DPI(BUTTON_LEFT), y, theApp.DPI(BUTTON_RIGHT), y + theApp.DPI(20)), ID_THEME_MORE_COLOR);
    y += theApp.DPI(34);

    CreateStatic(theApp.m_str_table.LoadText(L"TXT_OPT_APC_COLOR_PRESET"), CRect(theApp.DPI(LABEL_LEFT), y + theApp.DPI(3), theApp.DPI(LABEL_RIGHT), y + theApp.DPI(19)), WS_CHILD | WS_VISIBLE | SS_RIGHT);
    const COLORREF preset_colors[] = {
        RGB(134, 186, 249), RGB(115, 210, 45), RGB(255, 164, 16), RGB(33, 147, 167),
        RGB(249, 153, 197), RGB(162, 161, 216), RGB(110, 110, 110)
    };
    for (int i{}; i < static_cast<int>(_countof(preset_colors)); ++i)
    {
        CColorStatic* preset = CreateColorPreview(preset_colors[i],
            CRect(theApp.DPI(PREVIEW_LEFT + i * 28), y, theApp.DPI(PREVIEW_LEFT + 24 + i * 28), y + theApp.DPI(18)),
            ID_THEME_PRESET_BASE + i);
        m_theme_presets.push_back(preset);
    }
    y += theApp.DPI(34);

    m_follow_system_color_chk = CreateButton(theApp.m_str_table.LoadText(L"TXT_OPT_APC_COLOR_FOLLOW_SYSTEM"),
        CRect(theApp.DPI(PREVIEW_LEFT), y, theApp.DPI(EXTRA_RIGHT), y + theApp.DPI(18)), ID_THEME_FOLLOW_SYSTEM,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX);
    y += theApp.DPI(34);
}

void CColorSettingDlg::AddSongListColorControls(int& y)
{
    AddSection(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_TEXT_COLOR"), y);
    AddColorRow(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_TEXT_COLOR"), &m_app_data.song_list_text_color, y);
    m_song_list_text_color_theme_chk = CreateButton(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_TEXT_COLOR_THEME"),
        CRect(theApp.DPI(EXTRA_LEFT), y - theApp.DPI(25), theApp.DPI(EXTRA_RIGHT), y - theApp.DPI(7)), ID_SONG_LIST_TEXT_THEME,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX);
    AddColorRow(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_PLAYING_TEXT_COLOR"), &m_app_data.song_list_playing_text_color, y);
    m_song_list_playing_text_color_theme_chk = CreateButton(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_TEXT_COLOR_THEME"),
        CRect(theApp.DPI(EXTRA_LEFT), y - theApp.DPI(25), theApp.DPI(EXTRA_RIGHT), y - theApp.DPI(7)), ID_SONG_LIST_PLAYING_TEXT_THEME,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX);
    y += theApp.DPI(8);
}

void CColorSettingDlg::AddDesktopLyricColorControls(int& y)
{
    AddSection(theApp.m_str_table.LoadText(L"TXT_OPT_COLOR_DESKTOP_LYRIC"), y);
    AddColorRow(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_TEXT"), &m_lyric_data.desktop_lyric_data.text_color1, y);
    AddColorRow(L"", &m_lyric_data.desktop_lyric_data.text_color2, y);
    m_text_gradient_combo = CreateCombo(CRect(theApp.DPI(EXTRA_LEFT), y - theApp.DPI(53), theApp.DPI(EXTRA_RIGHT), y - theApp.DPI(30)), ID_TEXT_GRADIENT_COMBO);
    AddColorRow(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_HIGHLIGHT"), &m_lyric_data.desktop_lyric_data.highlight_color1, y);
    AddColorRow(L"", &m_lyric_data.desktop_lyric_data.highlight_color2, y);
    m_highlight_gradient_combo = CreateCombo(CRect(theApp.DPI(EXTRA_LEFT), y - theApp.DPI(53), theApp.DPI(EXTRA_RIGHT), y - theApp.DPI(30)), ID_HIGHLIGHT_GRADIENT_COMBO);
    CreateButton(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_DEFAULT_STYLE"),
        CRect(theApp.DPI(EXTRA_LEFT), y - theApp.DPI(22), theApp.DPI(EXTRA_RIGHT), y), ID_DESKTOP_LYRIC_DEFAULT_STYLE);
    y += theApp.DPI(8);
}

void CColorSettingDlg::UpdateColorRows()
{
    for (auto& row : m_color_rows)
    {
        if (row->color != nullptr)
            row->preview.SetFillColor(*row->color);
    }
}

void CColorSettingDlg::UpdateSongListColorControls()
{
    if (m_song_list_text_color_theme_chk != nullptr)
    {
        m_song_list_text_color_theme_chk->SetCheck(!m_app_data.song_list_custom_text_color);
        if (!m_app_data.song_list_custom_text_color)
            m_color_rows[0]->preview.SetFillColor(CPlayerUIHelper::GetUIColors(m_app_data.dark_mode).color_text);
        m_color_rows[0]->button.EnableWindow(m_app_data.song_list_custom_text_color);
    }
    if (m_song_list_playing_text_color_theme_chk != nullptr)
    {
        m_song_list_playing_text_color_theme_chk->SetCheck(!m_app_data.song_list_custom_playing_text_color);
        if (!m_app_data.song_list_custom_playing_text_color)
            m_color_rows[1]->preview.SetFillColor(CPlayerUIHelper::GetUIColors(m_app_data.dark_mode).color_song_list_playing_text);
        m_color_rows[1]->button.EnableWindow(m_app_data.song_list_custom_playing_text_color);
    }
}

void CColorSettingDlg::GetDataFromUi()
{
    if (m_follow_system_color_chk != nullptr)
        m_app_data.theme_color_follow_system = (m_follow_system_color_chk->GetCheck() != 0);
    if (m_song_list_text_color_theme_chk != nullptr)
        m_app_data.song_list_custom_text_color = (m_song_list_text_color_theme_chk->GetCheck() == 0);
    if (m_song_list_playing_text_color_theme_chk != nullptr)
        m_app_data.song_list_custom_playing_text_color = (m_song_list_playing_text_color_theme_chk->GetCheck() == 0);
    if (m_text_gradient_combo != nullptr)
        m_lyric_data.desktop_lyric_data.text_gradient = m_text_gradient_combo->GetCurSel();
    if (m_highlight_gradient_combo != nullptr)
        m_lyric_data.desktop_lyric_data.highlight_gradient = m_highlight_gradient_combo->GetCurSel();
}

void CColorSettingDlg::ApplyDataToUi()
{
    if (m_theme_color_preview != nullptr)
        m_theme_color_preview->SetFillColor(m_app_data.theme_color.original_color);
    if (m_follow_system_color_chk != nullptr)
        m_follow_system_color_chk->SetCheck(m_app_data.theme_color_follow_system);
    if (m_text_gradient_combo != nullptr && m_text_gradient_combo->GetCount() == 0)
    {
        m_text_gradient_combo->AddString(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_GRADIENT_NONE").c_str());
        m_text_gradient_combo->AddString(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_GRADIENT_TWO").c_str());
        m_text_gradient_combo->AddString(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_GRADIENT_THREE").c_str());
        m_highlight_gradient_combo->AddString(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_GRADIENT_NONE").c_str());
        m_highlight_gradient_combo->AddString(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_GRADIENT_TWO").c_str());
        m_highlight_gradient_combo->AddString(theApp.m_str_table.LoadText(L"TXT_OPT_LRC_DESKTOP_LRC_COLOR_GRADIENT_THREE").c_str());
    }
    if (m_text_gradient_combo != nullptr)
        m_text_gradient_combo->SetCurSel(m_lyric_data.desktop_lyric_data.text_gradient);
    if (m_highlight_gradient_combo != nullptr)
        m_highlight_gradient_combo->SetCurSel(m_lyric_data.desktop_lyric_data.highlight_gradient);
    UpdateColorRows();
    UpdateSongListColorControls();
}

void CColorSettingDlg::SelectColor(ColorRow& row)
{
    if (row.color == nullptr)
        return;
    CColorDialog color_dlg(*row.color, 0, this);
    if (color_dlg.DoModal() == IDOK)
    {
        *row.color = color_dlg.GetColor();
        row.preview.SetFillColor(*row.color);
    }
}

void CColorSettingDlg::ClickThemeColor(COLORREF color)
{
    m_app_data.theme_color.original_color = color;
    m_app_data.theme_color_follow_system = false;
    if (m_theme_color_preview != nullptr)
        m_theme_color_preview->SetFillColor(color);
    if (m_follow_system_color_chk != nullptr)
        m_follow_system_color_chk->SetCheck(FALSE);
}

void CColorSettingDlg::ApplyDefaultLyricStyle(const LyricStyleDefaultData& style)
{
    CDesktopLyric::LyricStyleDefaultDataToLyricSettingData(style, m_lyric_data.desktop_lyric_data);
    ApplyDataToUi();
}

CWnd* CColorSettingDlg::TrackDefaultStyleMenuOwner()
{
    return this;
}

CStatic* CColorSettingDlg::CreateStatic(const std::wstring& text, const CRect& rect, DWORD style)
{
    auto wnd = std::make_unique<CStatic>();
    wnd->Create(text.c_str(), style, rect, this);
    SetControlFont(*wnd);
    CStatic* result = wnd.get();
    m_dynamic_controls.push_back(std::move(wnd));
    return result;
}

CButton* CColorSettingDlg::CreateButton(const std::wstring& text, const CRect& rect, int id, DWORD style)
{
    auto wnd = std::make_unique<CButton>();
    wnd->Create(text.c_str(), style, rect, this, id);
    SetControlFont(*wnd);
    CButton* result = wnd.get();
    m_dynamic_controls.push_back(std::move(wnd));
    return result;
}

CColorStatic* CColorSettingDlg::CreateColorPreview(COLORREF color, const CRect& rect, int id)
{
    auto wnd = std::make_unique<CColorStatic>();
    wnd->Create(std::wstring().c_str(), WS_CHILD | WS_VISIBLE | SS_NOTIFY | WS_BORDER, rect, this, id);
    wnd->SetFillColor(color);
    CColorStatic* result = wnd.get();
    m_dynamic_controls.push_back(std::move(wnd));
    return result;
}

CComboBox* CColorSettingDlg::CreateCombo(const CRect& rect, int id)
{
    auto wnd = std::make_unique<CComboBox>();
    wnd->Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL, rect, this, id);
    SetControlFont(*wnd);
    CComboBox* result = wnd.get();
    m_dynamic_controls.push_back(std::move(wnd));
    return result;
}

void CColorSettingDlg::SetControlFont(CWnd& wnd)
{
    wnd.SetFont(GetFont());
}

void CColorSettingDlg::OnSelectColor(UINT id)
{
    for (auto& row : m_color_rows)
    {
        if (row->id == static_cast<int>(id))
        {
            SelectColor(*row);
            break;
        }
    }
}

void CColorSettingDlg::OnThemePreset(UINT id)
{
    const int index = static_cast<int>(id) - ID_THEME_PRESET_BASE;
    if (index >= 0 && index < static_cast<int>(m_theme_presets.size()))
        ClickThemeColor(m_theme_presets[index]->GetFillColor());
}

void CColorSettingDlg::OnBnClickedMoreThemeColor()
{
    CColorDialog color_dlg(m_app_data.theme_color.original_color, 0, this);
    if (color_dlg.DoModal() == IDOK)
        ClickThemeColor(color_dlg.GetColor());
}

void CColorSettingDlg::OnBnClickedFollowSystemColor()
{
    GetDataFromUi();
}

void CColorSettingDlg::OnBnClickedSongListTextColorTheme()
{
    GetDataFromUi();
    if (m_app_data.song_list_custom_text_color && m_app_data.song_list_text_color == 0)
        m_app_data.song_list_text_color = CPlayerUIHelper::GetUIColors(m_app_data.dark_mode).color_text;
    UpdateSongListColorControls();
}

void CColorSettingDlg::OnBnClickedSongListPlayingTextColorTheme()
{
    GetDataFromUi();
    if (m_app_data.song_list_custom_playing_text_color && m_app_data.song_list_playing_text_color == 0)
        m_app_data.song_list_playing_text_color = CPlayerUIHelper::GetUIColors(m_app_data.dark_mode).color_song_list_playing_text;
    UpdateSongListColorControls();
}

void CColorSettingDlg::OnCbnSelchangeDesktopLyricGradient()
{
    GetDataFromUi();
}

void CColorSettingDlg::OnBnClickedDefaultStyle()
{
    CWnd* pBtn = GetDlgItem(ID_DESKTOP_LYRIC_DEFAULT_STYLE);
    if (pBtn == nullptr)
        return;
    CRect rect;
    pBtn->GetWindowRect(rect);
    CMenu* pMenu = theApp.m_menu_mgr.GetMenu(MenuMgr::OptDlrcDefStyleMenu);
    if (pMenu != nullptr)
        pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.bottom, TrackDefaultStyleMenuOwner());
}

void CColorSettingDlg::OnLyricDefaultStyle1()
{
    if (m_pDesktopLyric != nullptr)
        ApplyDefaultLyricStyle(m_pDesktopLyric->GetDefaultStyle(0));
}

void CColorSettingDlg::OnLyricDefaultStyle2()
{
    if (m_pDesktopLyric != nullptr)
        ApplyDefaultLyricStyle(m_pDesktopLyric->GetDefaultStyle(1));
}

void CColorSettingDlg::OnLyricDefaultStyle3()
{
    if (m_pDesktopLyric != nullptr)
        ApplyDefaultLyricStyle(m_pDesktopLyric->GetDefaultStyle(2));
}

void CColorSettingDlg::OnLyricDefaultStyle1Modify()
{
    if (m_pDesktopLyric == nullptr)
        return;
    LyricStyleDefaultData style_data;
    CDesktopLyric::LyricSettingDatatOLyricStyleDefaultData(m_lyric_data.desktop_lyric_data, style_data);
    m_pDesktopLyric->SetDefaultStyle(style_data, 0);
}

void CColorSettingDlg::OnLyricDefaultStyle2Modify()
{
    if (m_pDesktopLyric == nullptr)
        return;
    LyricStyleDefaultData style_data;
    CDesktopLyric::LyricSettingDatatOLyricStyleDefaultData(m_lyric_data.desktop_lyric_data, style_data);
    m_pDesktopLyric->SetDefaultStyle(style_data, 1);
}

void CColorSettingDlg::OnLyricDefaultStyle3Modify()
{
    if (m_pDesktopLyric == nullptr)
        return;
    LyricStyleDefaultData style_data;
    CDesktopLyric::LyricSettingDatatOLyricStyleDefaultData(m_lyric_data.desktop_lyric_data, style_data);
    m_pDesktopLyric->SetDefaultStyle(style_data, 2);
}

void CColorSettingDlg::OnRestoreDefaultStyle()
{
    if (m_pDesktopLyric != nullptr)
        m_pDesktopLyric->RestoreDefaultStyle();
}
