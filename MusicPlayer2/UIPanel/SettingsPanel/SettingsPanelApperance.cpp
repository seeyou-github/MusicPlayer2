#include "stdafx.h"
#include "SettingsPanelApperance.h"
#include "MusicPlayerDlg.h"
#include "CPlayerUIHelper.h"

#include "UIElement/CombinedElement/ToggleSettingGroup.h"
#include "UIElement/Text.h"
#include "UIElement/Button.h"
#include "UIElement/Slider.h"

CSettingsPanelApperance::CSettingsPanelApperance(std::shared_ptr<UiElement::Panel> root_element)
    : CSettingsPanelTab(root_element)
{
}

void CSettingsPanelApperance::Init()
{
    dard_mode_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("darkMode");
    dard_mode_btn->GetToggleBtn()->BindBool(&theApp.m_app_setting_data.dark_mode);
    dard_mode_btn->GetToggleBtn()->SetClickedTrigger([&](UiElement::AbstractToggleButton* sender) {
        //点击深色模式时使用主窗口的ID_DARK_MODE命令，由于ID_DARK_MODE中也会对theApp.m_app_setting_data.dark_mode取非，因此这里再取一次非
        theApp.m_app_setting_data.dark_mode = !theApp.m_app_setting_data.dark_mode;
        theApp.m_pMainWnd->SendMessage(WM_COMMAND, ID_DARK_MODE);
    });

    show_spectrum_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("showSpectrum");
    ConnectToggleTrigger(show_spectrum_btn, m_data.show_spectrum);
    show_album_cover_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("showAlbumCover");
    ConnectToggleTrigger(show_album_cover_btn, m_data.show_album_cover);
    round_corder_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("roundCornerStyle");
    ConnectToggleTrigger(round_corder_btn, m_data.button_round_corners);
    enable_bckground_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("enableBackground");
    ConnectToggleTrigger(enable_bckground_btn, m_data.enable_background);
    show_statusbar_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("showStatusbar");
    ConnectToggleTrigger(show_statusbar_btn, m_data.always_show_statusbar);
    show_notify_icon_btn = m_root_element->FindElement<UiElement::ToggleSettingGroup>("showNotifyIcon");
    ConnectToggleTrigger(show_notify_icon_btn, m_data.show_notify_icon);
    use_standard_titlebar = m_root_element->FindElement<UiElement::ToggleSettingGroup>("showStandardTitlebar");
    ConnectToggleTrigger(use_standard_titlebar, m_data.show_window_frame);

    auto* titlabar_btn_settings = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnSettings");
    titlabar_btn_settings->BindBool(&theApp.m_app_setting_data.show_settings_btn_in_titlebar);
    auto* titlabar_btn_skin = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnSkin");
    titlabar_btn_skin->BindBool(&theApp.m_app_setting_data.show_skin_btn_in_titlebar);
    auto* titlabar_btn_dark_mode = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnDarkMode");
    titlabar_btn_dark_mode->BindBool(&theApp.m_app_setting_data.show_dark_light_btn_in_titlebar);
    auto* titlabar_btn_mini_mode = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnMiniMode");
    titlabar_btn_mini_mode->BindBool(&theApp.m_app_setting_data.show_minimode_btn_in_titlebar);
    auto* titlabar_btn_full_screen = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnFullScreen");
    titlabar_btn_full_screen->BindBool(&theApp.m_app_setting_data.show_fullscreen_btn_in_titlebar);
    auto* titlabar_btn_minimize = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnMinimize");
    titlabar_btn_minimize->BindBool(&theApp.m_app_setting_data.show_minimize_btn_in_titlebar);
    auto* titlabar_btn_maximize = m_root_element->FindElement<UiElement::ToggleButton>("titlebarBtnMaximize");
    titlabar_btn_maximize->BindBool(&theApp.m_app_setting_data.show_maximize_btn_in_titlebar);

    ui_refresh_interfal_value = m_root_element->FindElement<UiElement::Text>("uiRefreshIntervalValue");
    UiElement::Button* interval_up_btn = m_root_element->FindElement<UiElement::Button>("intervalUpBtn");
    interval_up_btn->SetClickedTrigger([&](UiElement::Button* sender) {
        UpdateSettingsData();
        OnUiIntervalChanged(true);
        OnSettingsChanged();
    });
    UiElement::Button* interval_down_btn = m_root_element->FindElement<UiElement::Button>("intervalDownBtn");
    interval_down_btn->SetClickedTrigger([&](UiElement::Button* sender) {
        UpdateSettingsData();
        OnUiIntervalChanged(false);
        OnSettingsChanged();
    });

    song_list_font_size_value = m_root_element->FindElement<UiElement::Text>("songListFontSizeValue");
    song_list_text_color_value = m_root_element->FindElement<UiElement::Text>("songListTextColorValue");
    song_list_font_size_slider = m_root_element->FindElement<UiElement::Slider>("songListFontSizeSlider");
    song_list_font_size_slider->BindIntValue(&m_data.song_list_font_size);
    song_list_font_size_slider->SetPosChangedTrigger([&](UiElement::Slider* sender) {
        OnSongListFontSizeChanged();
    });
    song_list_text_color_btn = m_root_element->FindElement<UiElement::Button>("songListTextColorBtn");
    song_list_text_color_btn->SetClickedTrigger([&](UiElement::Button* sender) {
        OnSongListTextColorClicked();
    });
    song_list_text_color_theme_btn = m_root_element->FindElement<UiElement::Button>("songListTextColorThemeBtn");
    song_list_text_color_theme_btn->SetClickedTrigger([&](UiElement::Button* sender) {
        OnSongListTextColorThemeClicked();
    });

}

void CSettingsPanelApperance::UpdateSettingsData()
{
    m_data = theApp.m_app_setting_data;
}

void CSettingsPanelApperance::SettingDataToUi()
{
    dard_mode_btn->GetToggleBtn()->SetChecked(m_data.dark_mode);
    show_spectrum_btn->GetToggleBtn()->SetChecked(m_data.show_spectrum);
    show_album_cover_btn->GetToggleBtn()->SetChecked(m_data.show_album_cover);
    round_corder_btn->GetToggleBtn()->SetChecked(m_data.button_round_corners);
    enable_bckground_btn->GetToggleBtn()->SetChecked(m_data.enable_background);
    show_statusbar_btn->GetToggleBtn()->SetChecked(m_data.always_show_statusbar);
    show_notify_icon_btn->GetToggleBtn()->SetChecked(m_data.show_notify_icon);
    use_standard_titlebar->GetToggleBtn()->SetChecked(m_data.show_window_frame);
    ui_refresh_interfal_value->SetText(std::to_wstring(m_data.ui_refresh_interval));
    song_list_font_size_slider->SetCurPos(m_data.song_list_font_size);
    UpdateSongListSettingText();
}

void CSettingsPanelApperance::OnSettingsChanged()
{
    CMusicPlayerDlg::GetInstance()->ApplyApperanceSettings(m_data);
    CMusicPlayerDlg::GetInstance()->SettingsChanged();
}

void CSettingsPanelApperance::OnUiIntervalChanged(bool up)
{
    if (up)
        m_data.ui_refresh_interval += UI_INTERVAL_STEP;
    else
        m_data.ui_refresh_interval -= UI_INTERVAL_STEP;
    m_data.ui_refresh_interval = (m_data.ui_refresh_interval / UI_INTERVAL_STEP * UI_INTERVAL_STEP);
    CCommon::SetNumRange(m_data.ui_refresh_interval, MIN_UI_INTERVAL, MAX_UI_INTERVAL);
    ui_refresh_interfal_value->SetText(std::to_wstring(m_data.ui_refresh_interval));
}

void CSettingsPanelApperance::OnSongListFontSizeChanged()
{
    CCommon::SetNumRange(m_data.song_list_font_size, 15, 30);
    UpdateSongListSettingText();
    OnSettingsChanged();
}

void CSettingsPanelApperance::OnSongListTextColorClicked()
{
    UpdateSettingsData();
    COLORREF init_color{ m_data.song_list_custom_text_color ? m_data.song_list_text_color : CPlayerUIHelper::GetUIColors(theApp.m_app_setting_data.dark_mode).color_text };
    CColorDialog color_dlg(init_color, 0, theApp.m_pMainWnd);
    if (color_dlg.DoModal() == IDOK)
    {
        m_data.song_list_custom_text_color = true;
        m_data.song_list_text_color = color_dlg.GetColor();
        UpdateSongListSettingText();
        OnSettingsChanged();
    }
}

void CSettingsPanelApperance::OnSongListTextColorThemeClicked()
{
    UpdateSettingsData();
    m_data.song_list_custom_text_color = false;
    UpdateSongListSettingText();
    OnSettingsChanged();
}

void CSettingsPanelApperance::UpdateSongListSettingText()
{
    song_list_font_size_value->SetText(std::to_wstring(m_data.song_list_font_size));
    if (m_data.song_list_custom_text_color)
    {
        CString color_text;
        color_text.Format(_T("#%02X%02X%02X"), GetRValue(m_data.song_list_text_color), GetGValue(m_data.song_list_text_color), GetBValue(m_data.song_list_text_color));
        song_list_text_color_value->SetText(color_text.GetString());
        song_list_text_color_btn->SetText(color_text.GetString());
        song_list_text_color_theme_btn->SetEnable(true);
    }
    else
    {
        song_list_text_color_value->SetText(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_TEXT_COLOR_THEME"));
        song_list_text_color_btn->SetText(theApp.m_str_table.LoadText(L"TXT_OPT_APC_SONG_LIST_TEXT_COLOR_SELECT"));
        song_list_text_color_theme_btn->SetEnable(false);
    }
}
