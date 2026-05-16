#include "stdafx.h"
#include "BottomLyrics.h"
#include "TinyXml2Helper.h"
#include "Player.h"
#include "UserUi.h"
#include "StackElement.h"
#include <cstdlib>

namespace
{
    bool ParseXmlColor(const std::string& str_color, COLORREF& color)
    {
        if (str_color.empty())
            return false;

        std::string color_text = str_color;
        int base = 10;
        if (color_text[0] == '#')
        {
            color_text.erase(0, 1);
            base = 16;
        }
        else if (color_text.size() > 2 && color_text[0] == '0' && (color_text[1] == 'x' || color_text[1] == 'X'))
        {
            color_text.erase(0, 2);
            base = 16;
        }

        char* end_ptr{};
        unsigned long value = std::strtoul(color_text.c_str(), &end_ptr, base);
        if (end_ptr == color_text.c_str() || *end_ptr != '\0' || value > 0xFFFFFF)
            return false;

        color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
        return true;
    }
}

void UiElement::BottomLyrics::Draw()
{
    CalculateRect();
    if (rect.Height() < ui->DPI(8) || theApp.m_lyric_setting_data.show_desktop_lyric || IsPlayDetailPageShown() || !CPlayer::GetInstance().IsPlaying())
    {
        current_scroll_info.Reset();
        next_scroll_info.Reset();
        double_line_flag = 0;
        return;
    }

    FontInfo lyric_font_info{ theApp.m_lyric_setting_data.lyric_font };
    CCommon::SetNumRange(font_size, 5, 72);
    if (custom_font_size != font_size
        || custom_font_name != lyric_font_info.name
        || custom_font_style.ToInt() != lyric_font_info.style.ToInt())
    {
        lyric_font_info.size = font_size;
        custom_lyric_font.SetFont(lyric_font_info);
        custom_font_size = font_size;
        custom_font_name = lyric_font_info.name;
        custom_font_style = lyric_font_info.style;
    }

    CUIDrawer& drawer = ui->GetDrawer();
    CFont* old_font = drawer.SetFont(&custom_lyric_font.GetFont(ui->IsDrawLargeIcon()));
    DrawAreaGuard guard(&drawer, rect);

    CRect draw_rect{ rect };
    draw_rect.DeflateRect(ui->DPI(2), 0);
    const bool draw_single_line = single_line || draw_rect.Height() < ui->DPI(font_size * 3);
    const int line_height = draw_single_line ? draw_rect.Height() : draw_rect.Height() / 2;
    CRect current_rect{ draw_rect };
    current_rect.bottom = current_rect.top + line_height;
    CRect next_rect{ draw_rect };
    next_rect.top = current_rect.bottom;

    std::wstring current_text;
    std::wstring next_text;
    int progress{};

    if (CPlayerUIHelper::IsMidiLyric())
    {
        current_text = CPlayer::GetInstance().GetMidiLyric();
    }
    else if (CPlayer::GetInstance().m_Lyrics.IsEmpty())
    {
        current_scroll_info.Reset();
        next_scroll_info.Reset();
    }
    else
    {
        static const std::wstring& empty_lyric = theApp.m_str_table.LoadText(L"UI_LYRIC_EMPTY_LINE");
        const bool karaoke{ theApp.m_lyric_setting_data.lyric_karaoke_disp };
        const bool ignore_blank{ theApp.m_lyric_setting_data.donot_show_blank_lines };
        CLyrics& lyrics{ CPlayer::GetInstance().m_Lyrics };
        CPlayTime time{ CPlayer::GetInstance().GetCurrentPosition() };
        CLyrics::Lyric current_lyric{ lyrics.GetLyric(time, false, ignore_blank, karaoke) };
        CLyrics::Lyric next_lyric{ lyrics.GetLyric(time, true, ignore_blank, karaoke) };
        progress = lyrics.GetLyricProgress(time, ignore_blank, karaoke, [&drawer](const std::wstring& str) { return drawer.GetTextExtent(str.c_str()).cx; });

        current_text = current_lyric.text.empty() ? empty_lyric : current_lyric.text;
        next_text = next_lyric.text.empty() ? empty_lyric : next_lyric.text;
    }

    if (draw_single_line)
    {
        DrawCurrentLyric(draw_rect, current_text, progress);
    }
    else
    {
        bool switch_flag{ double_line_flag > 5000 };
        switch_flag ^= (double_line_flag % 5000) > progress;
        double_line_flag = switch_flag ? 10000 + progress : progress;

        if (!switch_flag)
        {
            DrawCurrentLyric(current_rect, current_text, progress);
            DrawNextLyric(next_rect, next_text, next_scroll_info);
        }
        else
        {
            DrawNextLyric(current_rect, next_text, next_scroll_info);
            DrawCurrentLyric(next_rect, current_text, progress);
        }
    }

    drawer.SetFont(old_font);
    Element::Draw();
}

void UiElement::BottomLyrics::ClearRect()
{
    rect = CRect();
    current_scroll_info.Reset();
    next_scroll_info.Reset();
    double_line_flag = 0;
}

void UiElement::BottomLyrics::FromXmlNode(tinyxml2::XMLElement* xml_node)
{
    Element::FromXmlNode(xml_node);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "font_size", font_size);
    CTinyXml2Helper::GetElementAttributeBool(xml_node, "single_line", single_line);
    played_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "played_text_color"), played_text_color);
    unplayed_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "unplayed_text_color"), unplayed_text_color);
    next_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "next_text_color"), next_text_color);
    if (!played_text_color_set)
        played_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "playing_text_color"), played_text_color);
    if (!unplayed_text_color_set)
        unplayed_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "text_color"), unplayed_text_color);
    if (!next_text_color_set && unplayed_text_color_set)
    {
        next_text_color = unplayed_text_color;
        next_text_color_set = true;
    }
}

bool UiElement::BottomLyrics::IsPlayDetailPageShown() const
{
    const CUserUi* user_ui = dynamic_cast<const CUserUi*>(ui);
    if (user_ui == nullptr)
        return false;

    UiElement::StackElement* stack_element = const_cast<CUserUi*>(user_ui)->FindElementInAllUi<UiElement::StackElement>("ui_big_main_stack_element");
    return stack_element != nullptr && stack_element->GetCurIndex() == 1;
}

void UiElement::BottomLyrics::DrawCurrentLyric(CRect rect, const std::wstring& text, int progress)
{
    if (text.empty())
        return;

    if (progress > 0 && progress < 1000)
        ui->GetDrawer().DrawWindowText(rect, text.c_str(), PlayedTextColor(), UnplayedTextColor(), progress, Alignment::LEFT, false);
    else if (progress >= 1000)
        ui->GetDrawer().DrawScrollText(rect, text.c_str(), PlayedTextColor(), CPlayerUIHelper::GetScrollTextPixel(), false, current_scroll_info);
    else
        ui->GetDrawer().DrawScrollText(rect, text.c_str(), UnplayedTextColor(), CPlayerUIHelper::GetScrollTextPixel(), false, current_scroll_info);
}

void UiElement::BottomLyrics::DrawNextLyric(CRect rect, const std::wstring& text, CDrawCommon::ScrollInfo& scroll_info)
{
    if (!text.empty())
        ui->GetDrawer().DrawScrollText(rect, text.c_str(), NextTextColor(), CPlayerUIHelper::GetScrollTextPixel(), false, scroll_info);
}

COLORREF UiElement::BottomLyrics::PlayedTextColor() const
{
    return played_text_color_set ? played_text_color : ui->GetUIColors().color_text;
}

COLORREF UiElement::BottomLyrics::UnplayedTextColor() const
{
    return unplayed_text_color_set ? unplayed_text_color : ui->GetUIColors().color_text_2;
}

COLORREF UiElement::BottomLyrics::NextTextColor() const
{
    return next_text_color_set ? next_text_color : ui->GetUIColors().color_text_2;
}
