#pragma once
#include <memory>
#include <vector>
#include "TabDlg.h"
#include "ColorStatic.h"
#include "DesktopLyric.h"

class CColorSettingDlg : public CTabDlg
{
    DECLARE_DYNAMIC(CColorSettingDlg)

public:
    CColorSettingDlg(CWnd* pParent = NULL);
    virtual ~CColorSettingDlg();

    ApperanceSettingData m_app_data;
    LyricSettingData m_lyric_data;
    CDesktopLyric* m_pDesktopLyric{};

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_COLOR_SETTING_DIALOG };
#endif

protected:
    struct ColorRow
    {
        int id{};
        CStatic label;
        CColorStatic preview;
        CButton button;
        COLORREF* color{};
    };

    virtual bool InitializeControls() override;
    virtual void GetDataFromUi() override;
    virtual void ApplyDataToUi() override;
    virtual BOOL OnInitDialog() override;

    void CreateControls();
    void AddSection(const std::wstring& text, int& y);
    void AddColorRow(const std::wstring& text, COLORREF* color, int& y);
    void AddThemeControls(int& y);
    void AddSongListColorControls(int& y);
    void AddDesktopLyricColorControls(int& y);
    void UpdateColorRows();
    void UpdateSongListColorControls();
    void SelectColor(ColorRow& row);
    void ClickThemeColor(COLORREF color);
    void ApplyDefaultLyricStyle(const LyricStyleDefaultData& style);
    CWnd* TrackDefaultStyleMenuOwner();

    CStatic* CreateStatic(const std::wstring& text, const CRect& rect, DWORD style = WS_CHILD | WS_VISIBLE);
    CButton* CreateButton(const std::wstring& text, const CRect& rect, int id, DWORD style = WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON);
    CColorStatic* CreateColorPreview(COLORREF color, const CRect& rect, int id = 0);
    CComboBox* CreateCombo(const CRect& rect, int id);
    void SetControlFont(CWnd& wnd);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnSelectColor(UINT id);
    afx_msg void OnThemePreset(UINT id);
    afx_msg void OnBnClickedMoreThemeColor();
    afx_msg void OnBnClickedFollowSystemColor();
    afx_msg void OnBnClickedSongListTextColorTheme();
    afx_msg void OnBnClickedSongListPlayingTextColorTheme();
    afx_msg void OnCbnSelchangeDesktopLyricGradient();
    afx_msg void OnBnClickedDefaultStyle();
    afx_msg void OnLyricDefaultStyle1();
    afx_msg void OnLyricDefaultStyle2();
    afx_msg void OnLyricDefaultStyle3();
    afx_msg void OnLyricDefaultStyle1Modify();
    afx_msg void OnLyricDefaultStyle2Modify();
    afx_msg void OnLyricDefaultStyle3Modify();
    afx_msg void OnRestoreDefaultStyle();

private:
    std::vector<std::unique_ptr<CWnd>> m_dynamic_controls;
    std::vector<std::unique_ptr<ColorRow>> m_color_rows;
    CColorStatic* m_theme_color_preview{};
    std::vector<CColorStatic*> m_theme_presets;
    CButton* m_more_theme_color_btn{};
    CButton* m_follow_system_color_chk{};
    CButton* m_song_list_text_color_theme_chk{};
    CButton* m_song_list_playing_text_color_theme_chk{};
    CComboBox* m_text_gradient_combo{};
    CComboBox* m_highlight_gradient_combo{};
    int m_next_color_button_id{};
    int m_content_height{};
};
