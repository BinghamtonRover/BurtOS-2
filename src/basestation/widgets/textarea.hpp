/*
	This widget uses substantial portions of the original NanoGUI text area
	with Enhancements provided by Binghamton University Rover Team

	NanoGUI was developed by Wenzel Jakob <wenzel.jakob@epfl.ch>.
	The widget drawing code is based on the NanoVG demo application
	by Mikko Mononen.

	All rights reserved. The original NanoGUI license is listed at the bottom
	of this file.
*/


#pragma once

#include <nanogui/widget.h>
#include <cstdio>
#include <sstream>
#include <mutex>


// Multi-line read-only text widget, ideal for displaying
// log messages etc.
//
// Appended text can use different colors, but the font size is
// fixed for the entire widget.
// 
// Notable Enhancements:
// - thread safety for append
class TextArea : public nanogui::Widget {
public:
	TextArea(Widget *parent);

	/// Set the used font
	void set_font(const std::string &font) { m_font = font; }

	/// Return the used font
	const std::string &font() const { return m_font; }

	/// Set the foreground color (applies to all subsequently added text)
	void set_foreground_color(const nanogui::Color &color) {
		m_foreground_color = color;
	}

	/// Return the foreground color (applies to all subsequently added text)
	const nanogui::Color &foreground_color() const {
		return m_foreground_color;
	}

	/// Set the widget's background color (a global property)
	void set_background_color(const nanogui::Color &background_color) {
		m_background_color = background_color;
	}

	/// Return the widget's background color (a global property)
	const nanogui::Color &background_color() const {
		return m_background_color;
	}
	//
	/// Set the widget's selection color (a global property)
	void set_selection_color(const nanogui::Color &selection_color) {
		m_selection_color = selection_color;
	}

	/// Return the widget's selection color (a global property)
	const nanogui::Color &selection_color() const {
		return m_selection_color;
	}

	/// Set the amount of padding to add around the text
	void set_padding(int padding) { m_padding = padding; }

	/// Return the amount of padding that is added around the text
	int padding() const { return m_padding; }

	/// Set whether the text can be selected using the mouse
	void set_selectable(int selectable) { m_selectable = selectable; }

	/// Return whether the text can be selected using the mouse
	int is_selectable() const { return m_selectable; }

	/// Append text at the end of the widget
	void append(const std::string& text);

	/// Append a line of text at the bottom
	void append_line(const std::string &text) {
		append(text + "\n");
	}

	/// Clear all current contents
	void clear();

	/* Widget implementation */
	virtual void draw(NVGcontext *ctx) override;
	virtual nanogui::Vector2i preferred_size(NVGcontext *ctx) const override;
	virtual bool mouse_button_event(const nanogui::Vector2i &p, int button, bool down,
									int modifiers) override;
	virtual bool mouse_drag_event(const nanogui::Vector2i &p, const nanogui::Vector2i &rel, int button,
								  int modifiers) override;
	virtual bool keyboard_event(int key, int scancode, int action, int modifiers) override;

protected:
	nanogui::Vector2i position_to_block(const nanogui::Vector2i &pos) const;
	nanogui::Vector2i block_to_position(const nanogui::Vector2i &pos) const;

	void compute_append();

protected:
	struct Block {
		nanogui::Vector2i offset;
		int width;
		std::string text;
		nanogui::Color color;
	};

	std::vector<Block> m_blocks;
	nanogui::Color m_foreground_color;
	nanogui::Color m_background_color;
	nanogui::Color m_selection_color;
	std::string m_font;
	nanogui::Vector2i m_offset, m_max_size;
	int m_padding;
	bool m_selectable;
	nanogui::Vector2i m_selection_start;
	nanogui::Vector2i m_selection_end;
	
	std::string append_queue;
	std::mutex append_queue_lock;
	std::mutex clear_text_lock;
};

/*
	NanoGUI Software License

	Copyright (c) 2017 Wenzel Jakob <wenzel.jakob@epfl.ch>, All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	list of conditions and the following disclaimer.

	2. Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

	3. Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software
	without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
	FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	You are under no obligation whatsoever to provide any bug fixes, patches, or
	upgrades to the features, functionality or performance of the source code
	("Enhancements") to anyone; however, if you choose to make your Enhancements
	available either publicly, or directly to the author of this software, without
	imposing a separate written license agreement for such Enhancements, then you
	hereby grant the following license: a non-exclusive, royalty-free perpetual
	license to install, use, modify, prepare derivative works, incorporate into
	other computer software, distribute, and sublicense such enhancements or
	derivative works thereof, in binary and source code form.
*/