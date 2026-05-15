#pragma once
#include "SettingsPanelTab.h"

#include "CommonData.h"
namespace UiElement
{
    class ToggleSettingGroup;
    class Text;
    class Slider;
    class Button;
}

class CSettingsPanelApperance : public CSettingsPanelTab
{
public:
    CSettingsPanelApperance(std::shared_ptr<UiElement::Panel> root_element);

    // 通过 CSettingsPanelTab 继承
    void Init() override;
    void UpdateSettingsData() override;
    void SettingDataToUi() override;
    void OnSettingsChanged() override;

private:
    void OnUiIntervalChanged(bool up);
    void OnSongListFontSizeChanged();
    void OnSongListTextColorClicked();
    void OnSongListTextColorThemeClicked();
    void UpdateSongListSettingText();

private:
    ApperanceSettingData m_data;
    UiElement::ToggleSettingGroup* dard_mode_btn{};
    UiElement::ToggleSettingGroup* show_spectrum_btn{};
    UiElement::ToggleSettingGroup* show_album_cover_btn{};
    UiElement::ToggleSettingGroup* round_corder_btn{};
    UiElement::ToggleSettingGroup* enable_bckground_btn{};
    UiElement::ToggleSettingGroup* show_statusbar_btn{};
    UiElement::ToggleSettingGroup* show_notify_icon_btn{};
    UiElement::ToggleSettingGroup* use_standard_titlebar{};
    UiElement::ToggleSettingGroup* show_menubar_btn{};
    UiElement::Text* ui_refresh_interfal_value{};
    UiElement::Slider* song_list_font_size_slider{};
    UiElement::Text* song_list_font_size_value{};
    UiElement::Text* song_list_text_color_value{};
    UiElement::Button* song_list_text_color_btn{};
    UiElement::Button* song_list_text_color_theme_btn{};
};

