#include "stdafx.h"
#include "CommonData.h"
#include "MusicPlayer2.h"

bool UIData::ShowWindowMenuBar() const
{
    return show_menu_bar && theApp.m_app_setting_data.show_window_frame && !full_screen;
}

bool UIData::ShowUiMenuBar() const
{
    return show_menu_bar && !theApp.m_app_setting_data.show_window_frame && !full_screen;
}

wstring LyricSettingData::AbsoluteLyricPath() const
{
    wstring absolute_path = CCommon::GetTemplatePath() + L"lyrices\\";
    CCommon::CreateDir(absolute_path);
    return absolute_path;
}

wstring ApperanceSettingData::AbsoluteAlbumCoverPath() const
{
    wstring absolute_path = CCommon::GetTemplatePath() + L"cover\\";
    CCommon::CreateDir(absolute_path);
    return absolute_path;
}

int ApperanceSettingData::TitleDisplayItem() const
{
    int value{};
    if (show_minimize_btn_in_titlebar)
        value |= (1 << 0);
    if (show_maximize_btn_in_titlebar)
        value |= (1 << 1);
    if (show_minimode_btn_in_titlebar)
        value |= (1 << 2);
    if (show_fullscreen_btn_in_titlebar)
        value |= (1 << 3);
    if (show_skin_btn_in_titlebar)
        value |= (1 << 4);
    if (show_settings_btn_in_titlebar)
        value |= (1 << 5);
    return value;
}

void FontSet::Init(LPCTSTR font_name)
{
    for (int font_size{ FONT_SIZE_MIN }; font_size <= FONT_SIZE_MAX; font_size++)
    {
        fonts[font_size].SetFont(font_size, font_name);
    }
    dlg.SetFont(9, font_name);
}

UIFont& FontSet::GetFontBySize(int font_size)
{
    if (font_size < FONT_SIZE_MIN)
        font_size = FONT_SIZE_MIN;
    if (font_size > FONT_SIZE_MAX)
        font_size = FONT_SIZE_MAX;
    auto iter = fonts.find(font_size);
    if (iter != fonts.end())
        return iter->second;
    return dlg;
}

int CalculateSongListItemHeight(int font_size)
{
    CCommon::SetNumRange(font_size, 15, 30);

    int text_height{ MulDiv(font_size, 96, 72) };
    HDC hDC{ ::GetDC(HWND_DESKTOP) };
    if (hDC != nullptr)
    {
        HGDIOBJ hFont{ theApp.m_font_set.GetFontBySize(font_size).GetFont().GetSafeHandle() };
        HGDIOBJ old_font{};
        if (hFont != nullptr)
            old_font = ::SelectObject(hDC, hFont);

        TEXTMETRIC text_metric{};
        if (::GetTextMetrics(hDC, &text_metric))
        {
            const int dpi_y{ ::GetDeviceCaps(hDC, LOGPIXELSY) };
            text_height = ::MulDiv(text_metric.tmHeight, 96, dpi_y);
        }

        if (old_font != nullptr)
            ::SelectObject(hDC, old_font);
        ::ReleaseDC(HWND_DESKTOP, hDC);
    }

    return text_height + CONSTVAL::SONG_LIST_TEXT_VERTICAL_PADDING * 2;
}

std::wstring FontInfo::GetFontInfoString() const
{
    wstring str = name + L", " + std::to_wstring(size) + L"pt";
    wstring font_style;
    if (style.bold || style.italic)
        str.push_back(L',');
    if (style.bold)
        str += L' ' + theApp.m_str_table.LoadText(L"TIP_OPT_LRC_FONT_INFO_BOLD");
    if (style.italic)
        str += L' ' + theApp.m_str_table.LoadText(L"TIP_OPT_LRC_FONT_INFO_ITALIC");

    return str;
}
