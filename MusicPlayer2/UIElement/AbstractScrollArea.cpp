#include "stdafx.h"
#include "AbstractScrollArea.h"
#include "Layout.h"
#include "TinyXml2Helper.h"
#include "Common.h"
#include <cstdlib>

void UiElement::AbstractScrollArea::Draw()
{
    CalculateRect();

    const int SCROLLBAR_WIDTH{ ui->DPI(scrollbar_width_config) };           //滚动条的宽度
    const int SCROLLBAR_WIDTH_NARROW{ ui->DPI(scrollbar_narrow_width_config) };     //鼠标未指向滚动条时的宽度
    const int MIN_SCROLLBAR_LENGTH{ ui->DPI(scrollbar_min_handle_length_config) };      //滚动条的最小长度

    //计算滚动区域的矩形区域
    RestrictOffset();
    int scroll_area_height = GetScrollAreaHeight();
    bool show_scroll_bar = scroll_area_height > rect.Height();
    m_scroll_area_rect = rect;
    if (show_scroll_bar)
        m_scroll_area_rect.right -= SCROLLBAR_WIDTH;
    m_client_area_rect = m_scroll_area_rect;
    m_scroll_area_rect.bottom = m_scroll_area_rect.top + scroll_area_height;
    m_scroll_area_rect.MoveToY(m_scroll_area_rect.top - scroll_offset);

    DrawScrollArea();

    //绘制滚动条
    scrollbar_rect = rect;
    scrollbar_rect.left = scrollbar_rect.right - SCROLLBAR_WIDTH;
    if (hover || mouse_pressed || scrollbar_handle_pressed)
    {
        CRect scrollbar_rect_hover = scrollbar_rect;
        if (!scrollbar_hover && !scrollbar_handle_pressed)  //如果鼠标没有指向也没有拖动滚动条，滚动条以更小的宽度显示
        {
            scrollbar_rect_hover.left = scrollbar_rect_hover.left + (scrollbar_rect_hover.Width() - SCROLLBAR_WIDTH_NARROW) / 2;
            scrollbar_rect_hover.right = scrollbar_rect_hover.left + SCROLLBAR_WIDTH_NARROW;
        }

        auto drawRect = [&](CRect _rect, COLORREF color, BYTE _alpha) {
            if (theApp.m_app_setting_data.button_round_corners)
                ui->GetDrawer().DrawRoundRect(_rect, color, ui->DPI(4), _alpha);
            else
                ui->GetDrawer().FillAlphaRect(_rect, color, _alpha, true);
        };

        //开始绘制滚动条
        if (show_scroll_bar)
        {
            //填充滚动条背景
            BYTE background_alpha;
            if (!ui->IsDrawBackgroundAlpha())
                background_alpha = 255;
            else if (theApp.m_app_setting_data.dark_mode)
                background_alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency) / 2;
            else
                background_alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency) * 2 / 3;

            if (scrollbar_hover || scrollbar_handle_pressed)
                drawRect(scrollbar_rect_hover, GetScrollbarBackgroundColor(), background_alpha);

            //画滚动条把手
            //计算滚动条的长度
            int scroll_handle_length{ rect.Height() * rect.Height() / m_scroll_area_rect.Height() };
            scroll_handle_length_comp = 0;
            if (scroll_handle_length < MIN_SCROLLBAR_LENGTH)
            {
                scroll_handle_length_comp = MIN_SCROLLBAR_LENGTH - scroll_handle_length;
                scroll_handle_length = MIN_SCROLLBAR_LENGTH;
            }
            //根据播放列表偏移量计算滚动条的位置
            int scroll_pos{ (rect.Height() - scroll_handle_length_comp) * scroll_offset / m_scroll_area_rect.Height() };
            scrollbar_handle_rect = scrollbar_rect_hover;
            scrollbar_handle_rect.top = scrollbar_rect_hover.top + scroll_pos;
            scrollbar_handle_rect.bottom = scrollbar_handle_rect.top + scroll_handle_length;
            //限制滚动条把手的位置
            if (scrollbar_handle_rect.top < scrollbar_rect.top)
                scrollbar_handle_rect.MoveToY(scrollbar_rect.top);
            if (scrollbar_handle_rect.bottom > scrollbar_rect.bottom)
                scrollbar_handle_rect.MoveToY(scrollbar_rect.bottom - scrollbar_handle_rect.Height());
            //滚动条把手的颜色
            COLORREF scrollbar_handle_color{ GetScrollbarHandleColor() };
            if (scrollbar_handle_pressed)
                scrollbar_handle_color = GetScrollbarHandlePressedColor();
            else if (scrollbar_hover)
                scrollbar_handle_color = GetScrollbarHandleHoverColor();
            //滚动条把手的不透明度
            BYTE scrollbar_handle_alpha{ 255 };
            if (ui->IsDrawBackgroundAlpha())
                scrollbar_handle_alpha = ALPHA_CHG(theApp.m_app_setting_data.background_transparency);
            //绘制滚动条把手
            drawRect(scrollbar_handle_rect, scrollbar_handle_color, scrollbar_handle_alpha);
        }
    }
}

CRect UiElement::AbstractScrollArea::GetScrollAreaRect() const
{
    return m_scroll_area_rect;
}

bool UiElement::AbstractScrollArea::LButtonUp(CPoint point)
{
    bool was_scrollbar_dragging = IsScrollbarDragging();
    mouse_pressed = false;
    scrollbar_handle_pressed = false;
    EndScrollbarCapture();
    if (was_scrollbar_dragging)
        return true;
    if (rect.PtInRect(point))
        return Element::LButtonUp(point);
    else
        return false;
}

bool UiElement::AbstractScrollArea::LButtonDown(CPoint point)
{
    bool rtn = false;
    //点击了控件区域
    if (rect.PtInRect(point))
    {
        //点击了滚动条区域
        if (scrollbar_rect.PtInRect(point))
        {
            //点击了滚动条把手区域
            if (scrollbar_handle_rect.PtInRect(point))
            {
                scrollbar_handle_pressed = true;
                BeginScrollbarCapture();
            }
            //点击了滚动条空白区域
            else
            {
                mouse_pressed = false;
            }
            rtn = true;
        }
        //点击了滚动区域
        else
        {
            if (Element::LButtonDown(point))
                return true;
            mouse_pressed = true;
        }
        mouse_pressed_offset = scroll_offset;
        mouse_pressed_pos = point;
    }
    //点击了控件外
    else
    {
        mouse_pressed = false;
    }
    return rtn;
}

bool UiElement::AbstractScrollArea::MouseMove(CPoint point)
{
    if (rect.IsRectEmpty())
        return false;

    if (IsScrollbarDragging() && (GetKeyState(VK_LBUTTON) & 0x8000) == 0)
    {
        scrollbar_handle_pressed = false;
        EndScrollbarCapture();
        return true;
    }

    mouse_pos = point;
    hover = rect.PtInRect(point);
    if (last_hover && !hover)
    {
        Element::MouseLeave();
    }
    last_hover = hover;

    scrollbar_hover = scrollbar_rect.PtInRect(point);
    if (scrollbar_handle_pressed)
    {
        int delta_scrollbar_offset = mouse_pressed_pos.y - point.y;  //滚动条移动的距离
        //将滚动条移动的距离转换成播放列表的位移
        int scroll_area_height = rect.Height() - scroll_handle_length_comp;
        if (scroll_area_height > 0)
        {
            int delta_playlist_offset = delta_scrollbar_offset * m_scroll_area_rect.Height() / scroll_area_height;
            scroll_offset = mouse_pressed_offset - delta_playlist_offset;
        }
        return true;
    }
    else if (mouse_pressed)
    {
        scroll_offset = mouse_pressed_offset + (mouse_pressed_pos.y - point.y);
        return true;
    }
    else
    {
        if (Element::MouseMove(point))
            return true;
    }

    return false;
}

bool UiElement::AbstractScrollArea::MouseWheel(int delta, CPoint point)
{
    if (rect.PtInRect(point))
    {
        scroll_offset += (-delta * ui->DPI(60) / 120);  //120为鼠标滚轮一行时delta的值
        return true;
    }
    return false;
}

bool UiElement::AbstractScrollArea::MouseLeave()
{
    hover = false;
    mouse_pressed = false;
    scrollbar_hover = false;
    if (!scrollbar_mouse_captured)
        scrollbar_handle_pressed = false;
    return Element::MouseLeave();
}

bool UiElement::AbstractScrollArea::GlobalLButtonUp(CPoint point)
{
    if (IsScrollbarDragging())
    {
        AbstractScrollArea::LButtonUp(point);
        return true;
    }
    return Element::GlobalLButtonUp(point);
}

bool UiElement::AbstractScrollArea::GlobalMouseMove(CPoint point)
{
    if (IsScrollbarDragging())
    {
        AbstractScrollArea::MouseMove(point);
        return true;
    }
    return Element::GlobalMouseMove(point);
}

void UiElement::AbstractScrollArea::FromXmlNode(tinyxml2::XMLElement* xml_node)
{
    Element::FromXmlNode(xml_node);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "scrollbar_width", scrollbar_width_config);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "scrollbar_narrow_width", scrollbar_narrow_width_config);
    CTinyXml2Helper::GetElementAttributeInt(xml_node, "scrollbar_min_handle_length", scrollbar_min_handle_length_config);
    CCommon::SetNumRange(scrollbar_width_config, 1, 64);
    CCommon::SetNumRange(scrollbar_narrow_width_config, 1, scrollbar_width_config);
    CCommon::SetNumRange(scrollbar_min_handle_length_config, 4, 256);
    ParseScrollbarColor(xml_node, "scrollbar_background_color", scrollbar_background_color);
    ParseScrollbarColor(xml_node, "scrollbar_handle_color", scrollbar_handle_color);
    ParseScrollbarColor(xml_node, "scrollbar_handle_hover_color", scrollbar_handle_hover_color);
    ParseScrollbarColor(xml_node, "scrollbar_handle_pressed_color", scrollbar_handle_pressed_color);
}

void UiElement::AbstractScrollArea::RestrictOffset()
{
    int& offset{ scroll_offset };
    if (offset < 0)
        offset = 0;
    int offset_max{ GetScrollAreaHeight() - rect.Height() };
    if (offset_max <= 0)
        offset = 0;
    else if (offset > offset_max)
        offset = offset_max;
}

void UiElement::AbstractScrollArea::BeginScrollbarCapture()
{
    CWnd* owner = ui != nullptr ? ui->GetOwner() : nullptr;
    if (owner != nullptr && owner->GetSafeHwnd() != nullptr)
    {
        owner->SetCapture();
        scrollbar_mouse_captured = true;
    }
}

void UiElement::AbstractScrollArea::EndScrollbarCapture()
{
    if (scrollbar_mouse_captured)
    {
        CWnd* owner = ui != nullptr ? ui->GetOwner() : nullptr;
        if (owner != nullptr && ::GetCapture() == owner->GetSafeHwnd())
            ::ReleaseCapture();
        scrollbar_mouse_captured = false;
    }
}

bool UiElement::AbstractScrollArea::IsScrollbarDragging() const
{
    return scrollbar_handle_pressed || scrollbar_mouse_captured;
}

bool UiElement::AbstractScrollArea::ParseScrollbarColor(tinyxml2::XMLElement* xml_node, const char* attr_name, ScrollbarColor& color)
{
    std::string str_color = CTinyXml2Helper::ElementAttribute(xml_node, attr_name);
    if (str_color.empty())
        return false;

    int base = 10;
    if (str_color[0] == '#')
    {
        str_color.erase(0, 1);
        base = 16;
    }
    else if (str_color.size() > 2 && str_color[0] == '0' && (str_color[1] == 'x' || str_color[1] == 'X'))
    {
        str_color.erase(0, 2);
        base = 16;
    }

    char* end_ptr{};
    unsigned long value = std::strtoul(str_color.c_str(), &end_ptr, base);
    if (end_ptr == str_color.c_str() || *end_ptr != '\0' || value > 0xFFFFFF)
        return false;

    color.color = RGB((value >> 16) & 0xFF, (value >> 8) & 0xFF, value & 0xFF);
    color.set = true;
    return true;
}

COLORREF UiElement::AbstractScrollArea::GetScrollbarBackgroundColor() const
{
    return scrollbar_background_color.set ? scrollbar_background_color.color : ui->GetUIColors().color_control_bar_back;
}

COLORREF UiElement::AbstractScrollArea::GetScrollbarHandleColor() const
{
    return scrollbar_handle_color.set ? scrollbar_handle_color.color : ui->GetUIColors().color_scrollbar_handle;
}

COLORREF UiElement::AbstractScrollArea::GetScrollbarHandleHoverColor() const
{
    return scrollbar_handle_hover_color.set ? scrollbar_handle_hover_color.color : ui->GetUIColors().color_button_hover;
}

COLORREF UiElement::AbstractScrollArea::GetScrollbarHandlePressedColor() const
{
    return scrollbar_handle_pressed_color.set ? scrollbar_handle_pressed_color.color : ui->GetUIColors().color_button_pressed;
}
