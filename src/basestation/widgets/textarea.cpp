/*
	This widget uses substantial portions of the original NanoGUI text area
	with Enhancements provided by Binghamton University Rover Team

	NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
	The widget drawing code is based on the NanoVG demo application
	by Mikko Mononen.

	All rights reserved. The original NanoGUI license is listed at the bottom
	of the header file.
*/


#include "textarea.hpp"

#include <nanogui/opengl.h>
#include <nanogui/theme.h>
#include <nanogui/screen.h>
#include <nanogui/vscrollpanel.h>

TextArea::TextArea(Widget *parent) : Widget(parent),
  m_foreground_color(nanogui::Color(0, 0)), m_background_color(nanogui::Color(0, 0)),
  m_selection_color(.5f, 1.f), m_font("sans"), m_offset(0),
  m_max_size(0), m_padding(0), m_selectable(true), m_autoscroll(true),
  m_selection_start(-1), m_selection_end(-1) { }

void TextArea::append(const std::string& text) {
	std::lock_guard lock(append_queue_lock);
	append_queue.append(text);
}

void TextArea::compute_append() {
	std::lock_guard lock(append_queue_lock);
	if (append_queue.empty()) return;
    NVGcontext *ctx = screen()->nvg_context();

    nvgFontSize(ctx, font_size());
    nvgFontFace(ctx, m_font.c_str());

    const char *str = append_queue.c_str();
    do {
        const char *begin = str;

        while (*str != 0 && *str != '\n')
            str++;

        std::string line(begin, str);
        if (line.empty())
            continue;
        int width = nvgTextBounds(ctx, 0, 0, line.c_str(), nullptr, nullptr);
        m_blocks.push_back(Block { m_offset, width, line, m_foreground_color });

        m_offset.x() += width;
        m_max_size = max(m_max_size, m_offset);
        if (*str == '\n') {
            m_offset = nanogui::Vector2i(0, m_offset.y() + font_size());
            m_max_size = max(m_max_size, m_offset);
        }
    } while (*str++ != 0);
	append_queue.clear();

    nanogui::VScrollPanel *vscroll = dynamic_cast<nanogui::VScrollPanel *>(m_parent);
    if (vscroll) {
        vscroll->perform_layout(ctx);
        if (m_autoscroll) vscroll->set_scroll(1.0F);
    }
}

void TextArea::clear() {
	std::lock_guard lock(clear_text_lock);
    m_blocks.clear();
    m_offset = m_max_size = 0;
    m_selection_start = m_selection_end = -1;
}

bool TextArea::keyboard_event(int key, int /* scancode */, int action, int modifiers) {
    if (m_selectable && focused()) {
        if (key == GLFW_KEY_C && modifiers == SYSTEM_COMMAND_MOD && action == GLFW_PRESS &&
            m_selection_start != -1 && m_selection_end != -1) {
			std::lock_guard lock(clear_text_lock);
            nanogui::Vector2i start = m_selection_start, end = m_selection_end;
            if (start.x() > end.x() || (start.x() == end.x() && start.y() > end.y()))
                std::swap(start, end);

            std::string str;
            const int max_glyphs = 1024;
            NVGglyphPosition glyphs[max_glyphs + 1];
            for (int i = start.x(); i <= end.x(); ++i) {
                if (i > start.x() && m_blocks[i].offset.y() != m_blocks[i-1].offset.y())
                    str += '\n';

                const Block &block = m_blocks[i];
                NVGcontext *ctx = screen()->nvg_context();
                int nglyphs = nvgTextGlyphPositions(ctx, block.offset.x(), block.offset.y(),
                                                    block.text.c_str(), nullptr, glyphs, max_glyphs);
                glyphs[nglyphs].str = block.text.c_str() + block.text.length();

                if (i == start.x() && i == end.x())
                    str += std::string(glyphs[start.y()].str, glyphs[end.y()].str);
                else if (i == start.x())
                    str += std::string(glyphs[start.y()].str, glyphs[nglyphs].str);
                else if (i == end.x())
                    str += std::string(glyphs[0].str, glyphs[end.y()].str);
                else
                    str += m_blocks[i].text;
            }
            glfwSetClipboardString(screen()->glfw_window(), str.c_str());
            return true;
        }
    }
    return false;
}

nanogui::Vector2i TextArea::preferred_size(NVGcontext *) const {
    return m_max_size + m_padding * 2;
}

void TextArea::draw(NVGcontext *ctx) {
	compute_append();
    nanogui::VScrollPanel *vscroll = dynamic_cast<nanogui::VScrollPanel *>(m_parent);


    std::vector<Block>::iterator start_it = m_blocks.begin(),
                                 end_it = m_blocks.end();
    if (vscroll) {
        int window_offset = -position().y(),
            window_size = vscroll->size().y();

        start_it = std::lower_bound(
            m_blocks.begin(),
            m_blocks.end(),
            window_offset,
            [&](const Block &block, int value) {
                return block.offset.y() + font_size() < value;
            }
        );

        end_it = std::upper_bound(
            m_blocks.begin(),
            m_blocks.end(),
            window_offset + window_size,
            [](int value, const Block &block) {
                return value < block.offset.y();
            }
        );
    }

    if (m_background_color.w() != 0.f) {
        nvgFillColor(ctx, m_background_color);
        nvgBeginPath(ctx);
        nvgRect(ctx, m_pos.x(), m_pos.y(), m_size.x(), m_size.y());
        nvgFill(ctx);
    }

    nanogui::Vector2i selection_end = block_to_position(m_selection_end);
    selection_end += m_pos + m_padding;
    if (m_selection_end != nanogui::Vector2i(-1)) {
        nvgBeginPath(ctx);
        nvgMoveTo(ctx, selection_end.x(), selection_end.y());
        nvgLineTo(ctx, selection_end.x(), selection_end.y() + font_size());
        nvgStrokeColor(ctx, nvgRGBA(255, 192, 0, 255));
        nvgStrokeWidth(ctx, 1.0f);
        nvgStroke(ctx);
    }

    nanogui::Vector2i selection_start = block_to_position(m_selection_start);
    selection_start += m_pos + m_padding;
    bool flip = false;
    if (selection_start.y() > selection_end.y() ||
        (selection_start.y() == selection_end.y() && selection_start.x() > selection_end.x())) {
        std::swap(selection_start, selection_end);
        flip = true;
    }
    if (m_selection_end != nanogui::Vector2i(-1) && m_selection_end != nanogui::Vector2i(-1)) {
        nvgBeginPath(ctx);
        nvgFillColor(ctx, m_selection_color);
        if (selection_end.y() == selection_start.y()) {
            nvgRect(ctx, selection_start.x(), selection_start.y(),
                    selection_end.x() - selection_start.x(),
                    font_size());
        } else {
            nvgRect(ctx, selection_start.x(), selection_start.y(),
                    m_blocks[flip ? m_selection_end.x() : m_selection_start.x()].width -
                    (selection_start.x() - m_pos.x() - m_padding),
                    font_size());
            nvgRect(ctx, m_pos.x() + m_padding, selection_end.y(),
                    selection_end.x() - m_pos.x() - m_padding, font_size());
        }
        nvgFill(ctx);
    }

    nvgFontFace(ctx, m_font.c_str());
    nvgFontSize(ctx, font_size());
    nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_TOP);

    for (auto it = start_it; it != end_it; ++it) {
        const Block &block = *it;
        nanogui::Color color = block.color;
        if (color == nanogui::Color(0, 0))
            color = m_theme->m_text_color;

        nanogui::Vector2i offset = block.offset + m_pos + m_padding;

        if (m_selection_end != nanogui::Vector2i(-1) && m_selection_end != nanogui::Vector2i(-1) &&
            offset.y() > selection_start.y() && offset.y() < selection_end.y()) {
            nvgFillColor(ctx, m_selection_color);
            nvgBeginPath(ctx);
            nvgRect(ctx, offset.x(), offset.y(), block.width, font_size());
            nvgFill(ctx);
        }


        nvgFillColor(ctx, color);
        nvgText(ctx, offset.x(), offset.y(),
                block.text.c_str(), nullptr);
    }
}

bool TextArea::mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
                                  int /* modifiers */) {
    if (down && button == GLFW_MOUSE_BUTTON_1 && m_selectable) {
        m_selection_start = m_selection_end =
            position_to_block(p - m_pos - m_padding);
        request_focus();
        return true;
    }

    return false;
}

bool TextArea::mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &/* rel */,
                                int /* button */, int /* modifiers */) {
    if (m_selection_start != -1 && m_selectable) {
        m_selection_end = position_to_block(p - m_pos - m_padding);
        return true;
    }
    return false;
}

nanogui::Vector2i TextArea::position_to_block(const nanogui::Vector2i &pos) const {
    NVGcontext *ctx = screen()->nvg_context();
    auto it = std::lower_bound(
        m_blocks.begin(),
        m_blocks.end(),
        pos.y(),
        [&](const Block &block, int value) {
            return block.offset.y() + font_size() < value;
        }
    );

    const int max_glyphs = 1024;
    NVGglyphPosition glyphs[max_glyphs];
    int selection = 0;

    if (it == m_blocks.end()) {
        if (m_blocks.empty())
            return nanogui::Vector2i(-1, 1);
        it = m_blocks.end() - 1;
        const Block &block = *it;
        selection = nvgTextGlyphPositions(ctx, block.offset.x(), block.offset.y(),
                              block.text.c_str(), nullptr, glyphs, max_glyphs);
    } else {
        for (auto it2 = it; it2 != m_blocks.end() && it2->offset.y() == it->offset.y(); ++it2) {
            const Block &block = *it2;
            nvgFontSize(ctx, font_size());
            nvgFontFace(ctx, m_font.c_str());
            int nglyphs =
                nvgTextGlyphPositions(ctx, block.offset.x(), block.offset.y(),
                                      block.text.c_str(), nullptr, glyphs, max_glyphs);

            for (int i = 0; i < nglyphs; ++i) {
                if (glyphs[i].minx + glyphs[i].maxx < pos.x() * 2)
                    selection = i + 1;
            }
        }
    }

    return nanogui::Vector2i(
        it - m_blocks.begin(),
        selection
    );
}

nanogui::Vector2i TextArea::block_to_position(const nanogui::Vector2i &pos) const {
    if (pos.x() < 0 || pos.x() >= (int) m_blocks.size())
        return nanogui::Vector2i(-1, -1);
    NVGcontext *ctx = screen()->nvg_context();
    const Block &block = m_blocks[pos.x()];
    const int max_glyphs = 1024;
    NVGglyphPosition glyphs[max_glyphs];
    nvgFontSize(ctx, font_size());
    nvgFontFace(ctx, m_font.c_str());
    int nglyphs =
        nvgTextGlyphPositions(ctx, block.offset.x(), block.offset.y(),
                              block.text.c_str(), nullptr, glyphs, max_glyphs);
    if (pos.y() == nglyphs)
        return block.offset + nanogui::Vector2i(glyphs[pos.y() - 1].maxx + 1, 0);
    else if (pos.y() > nglyphs)
        return nanogui::Vector2i(-1, -1);

    return block.offset + nanogui::Vector2i(glyphs[pos.y()].x, 0);
}
