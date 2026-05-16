#include "stdafx.h"
#include "BottomLyrics.h"
#include "TinyXml2Helper.h"
#include "Player.h"
#include "UserUi.h"
#include "StackElement.h"

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
    return theApp.m_app_setting_data.bottom_lyric_played_text_color;
}

COLORREF UiElement::BottomLyrics::UnplayedTextColor() const
{
    return theApp.m_app_setting_data.bottom_lyric_unplayed_text_color;
}

COLORREF UiElement::BottomLyrics::NextTextColor() const
{
    return theApp.m_app_setting_data.bottom_lyric_next_text_color;
}
