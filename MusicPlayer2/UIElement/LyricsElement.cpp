#include "stdafx.h"
#include "LyricsElement.h"
#include "Rectangle.h"
#include "TinyXml2Helper.h"
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
void UiElement::Lyrics::Draw()
{
    CalculateRect();


    bool big_font{ ui->IsDrawLargeIcon() };
    CFont* lyric_font = &theApp.m_font_set.lyric.GetFont(big_font);
    CFont* lyric_tr_font = &theApp.m_font_set.lyric_translate.GetFont(big_font);

    if (font_size_set)
    {
        CCommon::SetNumRange(font_size, 5, 72);
        if (custom_font_size != font_size
            || custom_font_name != theApp.m_lyric_setting_data.lyric_font.name
            || custom_font_style.ToInt() != theApp.m_lyric_setting_data.lyric_font.style.ToInt())
        {
            FontInfo lyric_font_info{ theApp.m_lyric_setting_data.lyric_font };
            lyric_font_info.size = font_size;
            custom_lyric_font.SetFont(lyric_font_info);

            FontInfo translate_font_info{ lyric_font_info };
            translate_font_info.size = font_size - 1;
            custom_lyric_translate_font.SetFont(translate_font_info);

            custom_font_size = font_size;
            custom_font_name = lyric_font_info.name;
            custom_font_style = lyric_font_info.style;
        }
        lyric_font = &custom_lyric_font.GetFont(big_font);
        lyric_tr_font = &custom_lyric_translate_font.GetFont(big_font);
    }
    else if (use_default_font)
    {
        lyric_font = &theApp.m_font_set.GetFontBySize(font_size).GetFont(big_font);
        lyric_tr_font = &theApp.m_font_set.GetFontBySize(font_size - 1).GetFont(big_font);
    }

    // Lyrics inside a rectangle use the rectangle background.
    ui->DrawLyrics(rect, lyric_font, lyric_tr_font, (!no_background && !IsParentRectangle()), show_song_info,
        text_color_set ? &text_color : nullptr, playing_text_color_set ? &playing_text_color : nullptr);

    Element::Draw();
}

void UiElement::Lyrics::ClearRect()
{
    rect = CRect();
}

bool UiElement::Lyrics::RButtonUp(CPoint point)
{
    if (context_menu_enable && rect.PtInRect(point))
    {
        CPoint point1;
        GetCursorPos(&point1);
        theApp.m_menu_mgr.GetMenu(MenuMgr::MainAreaLrcMenu)->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, theApp.m_pMainWnd);
        return true;
    }
    return false;
}

bool UiElement::Lyrics::IsParentRectangle() const
{
    const Element* ele{ this };
    while (ele != nullptr && ele->Parent() != nullptr)
    {
        if (dynamic_cast<const Rectangle*>(ele) != nullptr)
            return true;
        ele = ele->Parent();
    }
    return false;
}

void UiElement::Lyrics::FromXmlNode(tinyxml2::XMLElement* xml_node)
{
    Element::FromXmlNode(xml_node);
    CTinyXml2Helper::GetElementAttributeBool(xml_node, "no_background", no_background);
    CTinyXml2Helper::GetElementAttributeBool(xml_node, "use_default_font", use_default_font);
    font_size_set = !std::string(CTinyXml2Helper::ElementAttribute(xml_node, "font_size")).empty();
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "font_size", font_size);
    text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "text_color"), text_color);
    playing_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "playing_text_color"), playing_text_color);
    if (!playing_text_color_set)
        playing_text_color_set = ParseXmlColor(CTinyXml2Helper::ElementAttribute(xml_node, "current_text_color"), playing_text_color);
    CTinyXml2Helper::GetElementAttributeBool(xml_node, "show_song_info", show_song_info);
    CTinyXml2Helper::GetElementAttributeBool(xml_node, "context_menu_enable", context_menu_enable);
}
