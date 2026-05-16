#pragma once
#include "UIElement/UIElement.h"
namespace UiElement
{
    // Lyrics element.
    class Lyrics : public Element
    {
    public:
        virtual void Draw() override;
        virtual void ClearRect() override;
        virtual bool RButtonUp(CPoint point) override;
        virtual void FromXmlNode(tinyxml2::XMLElement* xml_node);

    protected:
        bool IsParentRectangle() const;

        bool no_background = false;
        bool use_default_font = false;
        bool font_size_set = false;
        int font_size{ 9 };
        UIFont custom_lyric_font;
        UIFont custom_lyric_translate_font;
        int custom_font_size{};
        std::wstring custom_font_name;
        FontStyle custom_font_style;
        bool show_song_info = false;
        bool context_menu_enable = true;
    };
}

