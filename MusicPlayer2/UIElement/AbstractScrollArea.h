#pragma once
#include "UIElement/UIElement.h"
namespace UiElement
{
    //滚动区域
    class AbstractScrollArea : public Element
    {
    public:
        virtual void Draw() override;
        virtual void DrawScrollArea() = 0;
        CRect GetScrollAreaRect() const;

        virtual bool LButtonUp(CPoint point) override;
        virtual bool LButtonDown(CPoint point) override;
        virtual bool MouseMove(CPoint point) override;
        virtual bool MouseWheel(int delta, CPoint point) override;
        virtual bool MouseLeave() override;
        virtual bool GlobalLButtonUp(CPoint point) override;
        virtual bool GlobalMouseMove(CPoint point) override;
        virtual void FromXmlNode(tinyxml2::XMLElement* xml_node) override;

        virtual int GetScrollAreaHeight() = 0;
        void RestrictOffset();             //将滚动区域偏移量限制在正确的范围

    protected:
        CRect m_scroll_area_rect;       //滚动区域的矩形区域
        CRect m_client_area_rect;       //获取客户区域的矩形（不包含滚动条，不包含控件外区域）

        bool mouse_pressed{ };          //鼠标左键是否按下
        bool hover{};                   //指标指向播放列表区域
        CPoint mouse_pos;               //鼠标指向的区域
        CPoint mouse_pressed_pos;       //鼠标按下时的位置
        int mouse_pressed_offset{};     //鼠标按下时播放列表的位移
        int scroll_offset{};            //当前播放列表滚动的位移
        CRect scrollbar_rect{};         //滚动条的位置
        CRect scrollbar_handle_rect;    //滚动条把手的位置
        bool scrollbar_hover{};         //鼠标指向滚动条
        bool scrollbar_handle_pressed{};    //滚动条把手被按下
        int scroll_handle_length_comp{};    //计算滚动条把手长度时的补偿量

    private:
        struct ScrollbarColor
        {
            bool set{};
            COLORREF color{};
        };

        void BeginScrollbarCapture();
        void EndScrollbarCapture();
        bool IsScrollbarDragging() const;
        bool ParseScrollbarColor(tinyxml2::XMLElement* xml_node, const char* attr_name, ScrollbarColor& color);
        COLORREF GetScrollbarBackgroundColor() const;
        COLORREF GetScrollbarHandleColor() const;
        COLORREF GetScrollbarHandleHoverColor() const;
        COLORREF GetScrollbarHandlePressedColor() const;

        bool last_hover{};
        bool scrollbar_mouse_captured{};
        int scrollbar_width_config{ 10 };
        int scrollbar_narrow_width_config{ 6 };
        int scrollbar_min_handle_length_config{ 16 };
        ScrollbarColor scrollbar_background_color;
        ScrollbarColor scrollbar_handle_color;
        ScrollbarColor scrollbar_handle_hover_color;
        ScrollbarColor scrollbar_handle_pressed_color;
    };
}

