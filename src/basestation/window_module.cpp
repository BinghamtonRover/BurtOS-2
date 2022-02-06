#include "window_module.hpp"
#include <nanogui/icons.h>
#include <nanogui/vector.h>

using namespace module::gui;

WindowModule::WindowModule(
	const std::string& caption, unsigned int width, unsigned int height,
	const WindowModuleParams& p)
	: nanogui::Screen(
		  nanogui::Vector2i(width, height), caption, p.resizable, p.fullscreen,
		  true, true, true, p.gl_major, p.gl_minor)
{
}

WindowModule::~WindowModule(){
	nanogui::Screen::set_visible(false);
}

nanogui::Window* WindowModule::create_subwindow(const std::string& title) {
    const int this_window_index = m_subwindows.windows.size();

    auto w = new SubWindow(*this, this_window_index, this, title);
    m_subwindows.windows.push_back(w);

    create_manager_ui();

    auto items = m_subwindows.ui_combo->items();
    auto items_short = m_subwindows.ui_combo->items_short();

    items.push_back(title);
    items_short.push_back(title.substr(0,50)); //how much of title should be visible

    const auto old_selected = m_subwindows.ui_combo->selected_index();

    m_subwindows.ui_combo->set_items(items, items_short);
    m_subwindows.ui_combo->set_selected_index(old_selected > 0 ? old_selected : 0);

    w->button_panel()
    ->add<nanogui::Button>("", FA_ANGLE_DOWN)
    ->set_callback([this, this_window_index]() { 
        m_subwindows.minimize(this_window_index); });

    // w->button_panel()
    // ->add<nanogui::Button>("", FA_WINDOW_CLOSE)
    // ->set_callback([this, this_window_index]() { 
    //     m_subWindows.close(this_window_index); });

    w->center();
    for (int i = 0; i < static_cast<int>(m_subwindows.windows.size()); i++)
        m_subwindows.restore(i);

    return w;
}

void WindowModule::create_manager_ui() {
    constexpr int WIDTH = 100;
	constexpr int HEIGHT = 40;
    constexpr int HIDEN_WIDTH = 70;
	constexpr int HIDEN_HEIGHT = 70;
    if (m_subwindows.ui) return;

    auto& w = m_subwindows.ui;
    w = new nanogui::Window(this, "Manager");

    w->set_layout(new nanogui::GroupLayout());

    w->set_size({WIDTH, HEIGHT});
    w->set_position({10,10});
    nanogui::Theme* mod_theme = new nanogui::Theme(screen()->nvg_context());
    mod_theme->m_window_header_height = 20;
    w->set_theme(mod_theme);
    auto stack_panels = w->add<nanogui::StackedWidget>();
    auto page1 = new nanogui::Widget(nullptr);
    auto page2 = new nanogui::Widget(nullptr);

    stack_panels->add_child(0, page1);
    stack_panels->add_child(1, page2);
    stack_panels->set_selected_index(0);

    page1->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1));
	page2->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1));

    auto unhide_button =
		page2->add<nanogui::Button>("", FA_CHEVRON_CIRCLE_RIGHT);
	unhide_button->set_callback([=]() {
		stack_panels->set_selected_index(0);
		w->set_fixed_size({0, 0});
		this->perform_layout();
	});
    unhide_button->set_tooltip("Unhide this panel");

	m_subwindows.ui_combo = page1->add<nanogui::ComboBox>();

	m_subwindows.ui_combo->set_callback([this](int index) {
		m_subwindows.restore(index);
		m_subwindows.set_focused(index);
	});


    auto pn = page1->add<nanogui::Widget>();
	pn->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Horizontal, nanogui::Alignment::Fill, 4, 4));

	auto minimize_button = pn->add<nanogui::Button>("Current", FA_ANGLE_DOWN);
	minimize_button->set_callback([this]() {
		m_subwindows.minimize(m_subwindows.ui_combo->selected_index());
	});
	minimize_button->set_tooltip("Minimize");

    auto maximize_button = pn->add<nanogui::Button>("Current", FA_ANGLE_UP);
	maximize_button->set_callback([this]() {
		m_subwindows.restore(m_subwindows.ui_combo->selected_index());
	});
	maximize_button->set_tooltip("Restore selected");

	auto minimize_all = pn->add<nanogui::Button>("All", FA_ANGLE_DOUBLE_DOWN);
	minimize_all->set_callback([this]() {
		for (int i = 0; i < static_cast<int>(m_subwindows.windows.size()); i++)
			m_subwindows.minimize(i);
	});
	minimize_all->set_tooltip("Minimize all");

	auto maximize_all = pn->add<nanogui::Button>("All", FA_ANGLE_DOUBLE_UP);
	maximize_all->set_callback([this]() {
		for (int i = 0; i < static_cast<int>(m_subwindows.windows.size()); i++)
			m_subwindows.restore(i);
	});
	maximize_all->set_tooltip("Restore all");

	auto hide_button =
		pn->add<nanogui::Button>("", FA_MINUS_SQUARE);
	hide_button->set_callback([=]() {
		stack_panels->set_selected_index(1);
		w->set_fixed_size({HIDEN_WIDTH, HIDEN_HEIGHT});
		this->perform_layout();
	});
	hide_button->set_tooltip("Hide this panel");


}

void WindowModule::SubWindows::on_subwindow_focused(int index) {
	const int n = static_cast<int>(ui_combo->items().size());
	if (index >= 0 && index < n) ui_combo->set_selected_index(index);
}

void WindowModule::SubWindows::minimize(int index) {
	if (index < 0 || index > static_cast<int>(windows.size())) return;
	auto w = windows.at(index);

	w->set_visible(false);
	parent.perform_layout();
}

// void WindowModule::SubWindows::close(int index) {
//     if (index < 0 || index > static_cast<int>(windows.size())) return;
//     auto w = windows.at(index);


// }

void WindowModule::SubWindows::restore(int index) {
	if (index < 0 || index > static_cast<int>(windows.size())) return;
	auto w = windows.at(index);

	w->set_visible(true);
	parent.perform_layout();
}

void WindowModule::SubWindows::set_focused(int index)
{
	const int n = static_cast<int>(windows.size());
	if (index >= 0 && index < n) windows.at(index)->request_focus();
}