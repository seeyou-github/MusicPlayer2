#pragma once
#include "UIElement/UIElement.h"
namespace UiElement
{
    //进度条
    class ProgressBar : public Element
    {
    public:
        virtual void Draw() override;

        virtual bool LButtonUp(CPoint point) override;
        virtual bool LButtonDown(CPoint point) override;
        virtual bool RButtonUp(CPoint point) override;
        virtual bool MouseMove(CPoint point) override;
        virtual bool MouseLeave() override;
        virtual bool SetCursor() override;
        virtual void HideTooltip() override;
        virtual void FromXmlNode(tinyxml2::XMLElement* xml_node);

        bool hover() const;
        CRect GetProgressRect() const { return btn.rect; }

    protected:
        struct CustomColor
        {
            bool set{};
            COLORREF color{};
        };

        bool show_play_time{};
        bool play_time_both_side{};
        bool auto_color{};
        int bar_height{};
        CustomColor progress_back_color;
        CustomColor progress_color;
        CustomColor time_color;
        CPlayerUIBase::UIButton btn;

    private:
        bool last_hover{};
    };
}

