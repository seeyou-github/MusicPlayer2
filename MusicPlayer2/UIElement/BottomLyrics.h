#pragma once
#include "UIElement/UIElement.h"

namespace UiElement
{
    // Two-line lyrics for the bottom control bar.
    class BottomLyrics : public Element
    {
    public:
        virtual void Draw() override;
        virtual void ClearRect() override;
        virtual void FromXmlNode(tinyxml2::XMLElement* xml_node) override;

    private:
        bool IsPlayDetailPageShown() const;
        void DrawCurrentLyric(CRect rect, const std::wstring& text, int progress);
        void DrawNextLyric(CRect rect, const std::wstring& text, CDrawCommon::ScrollInfo& scroll_info);
        COLORREF PlayedTextColor() const;
        COLORREF UnplayedTextColor() const;
        COLORREF NextTextColor() const;

    private:
        int font_size{ 16 };
        bool played_text_color_set{};
        bool unplayed_text_color_set{};
        bool next_text_color_set{};
        COLORREF played_text_color{};
        COLORREF unplayed_text_color{};
        COLORREF next_text_color{};
        bool single_line{};
        UIFont custom_lyric_font;
        int custom_font_size{};
        std::wstring custom_font_name;
        FontStyle custom_font_style;
        int double_line_flag{};
        CDrawCommon::ScrollInfo current_scroll_info;
        CDrawCommon::ScrollInfo next_scroll_info;
    };
}
