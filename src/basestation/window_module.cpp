#include "window_module.hpp"
#include <nanogui/icons.h>
#include <nanogui/vector.h>

using namespace tony::gui;

// bool WindowModule::mouse_button_event(const nanogui::Vector2i& p, int button, bool down, int modifiers) {
// 	if (!Screen::mouse_button_event(p, button, down, modifiers))
// 		m_background_canvas.mouseButtonEvent(p, button, down, modifiers);

// 	return true;
// }

// bool CDisplayWindowGUI::mouseMotionEvent(
// 	const nanogui::Vector2i& p, const nanogui::Vector2i& rel, int button,
// 	int modifiers)
// {
// 	if (!Screen::mouseMotionEvent(p, rel, button, modifiers))
// 		m_background_canvas.mouseMotionEvent(p, rel, button, modifiers);

// 	return true;
// }

// bool CDisplayWindowGUI::scrollEvent(
// 	const nanogui::Vector2i& p, const nanogui::Vector2f& rel)
// {
// 	if (!Screen::scrollEvent(p, rel)) m_background_canvas.scrollEvent(p, rel);

// 	return true;
// }

// bool CDisplayWindowGUI::dropEvent(const std::vector<std::string>& filenames)
// {
// 	for (const auto& callback : m_dropFilesCallbacks)
// 	{
// 		try
// 		{
// 			if (callback(filenames)) return true;
// 		}
// 		catch (const std::exception& e)
// 		{
// 			std::cerr << "[CDisplayWindowGUI] Exception in drop file event "
// 						 "callback:\n"
// 					  << e.what() << std::endl;
// 		}
// 	}

// 	return false;
// }

// bool CDisplayWindowGUI::keyboardEvent(
// 	int key, int scancode, int action, int modifiers)
// {
// 	for (const auto& callback : m_keyboardCallbacks)
// 	{
// 		try
// 		{
// 			if (callback(key, scancode, action, modifiers)) return true;
// 		}
// 		catch (const std::exception& e)
// 		{
// 			std::cerr
// 				<< "[CDisplayWindowGUI] Exception in keyboard event callback:\n"
// 				<< e.what() << std::endl;
// 		}
// 	}

// 	if (Screen::keyboardEvent(key, scancode, action, modifiers)) return true;

// 	// Process special key events?
// 	return false;
// }




WindowModule::WindowModule(
	const std::string& caption, unsigned int width, unsigned int height,
	const WindowModuleParams& p)
	: nanogui::Screen(
		  nanogui::Vector2i(width, height), caption, p.resizable, p.fullscreen,
		  true, true, true, p.glMajor, p.glMinor)
{
}

WindowModule::~WindowModule(){
	nanogui::Screen::set_visible(false);
}

nanogui::Window* WindowModule::createManagedSubWindow(const std::string& title) {
    const int this_window_index = m_subWindows.windows.size();

    auto w = new SubWindow(*this, this_window_index, this, title);
    m_subWindows.windows.push_back(w);

    createControlUI();

    auto items = m_subWindows.uiCombo->items();
    auto items_short = m_subWindows.uiCombo->items_short();

    items.push_back(title);
    items_short.push_back(title.substr(0,50)); //how much of title should be visible

    const auto old_selected = m_subWindows.uiCombo->selected_index();

    m_subWindows.uiCombo->set_items(items, items_short);
    m_subWindows.uiCombo->set_selected_index(old_selected > 0 ? old_selected : 0);

    w->button_panel()
    ->add<nanogui::Button>("", FA_ANGLE_DOWN)
    ->set_callback([this, this_window_index]() { 
        m_subWindows.minimize(this_window_index); });

    // w->button_panel()
    // ->add<nanogui::Button>("", FA_WINDOW_CLOSE)
    // ->set_callback([this, this_window_index]() { 
    //     m_subWindows.close(this_window_index); });

    w->center();
    for (int i = 0; i < static_cast<int>(m_subWindows.windows.size()); i++)
        m_subWindows.restore(i);

    return w;
}

void WindowModule::createControlUI() {
    constexpr int WIDTH = 100, HEIGHT = 40;
    constexpr int HIDEN_WIDTH = 70, HIDEN_HEIGHT = 70;
    if (m_subWindows.ui) return;

    auto& w = m_subWindows.ui;
    w = new nanogui::Window(this, "Manager", false);

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
    stack_panels->setSelectedIndex(0);

    page1->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1));
	page2->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Vertical, nanogui::Alignment::Fill, 1, 1));

    auto unhideB =
		page2->add<nanogui::Button>("", FA_CHEVRON_CIRCLE_RIGHT);
	unhideB->set_callback([=]() {
		stack_panels->setSelectedIndex(0);
		w->set_fixed_size({0, 0});
		this->perform_layout();
	});
    unhideB->set_tooltip("Unhide this panel");

	m_subWindows.uiCombo = page1->add<nanogui::ComboBox>();

	m_subWindows.uiCombo->set_callback([this](int index) {
		m_subWindows.restore(index);
		m_subWindows.setFocused(index);
	});


    auto pn = page1->add<nanogui::Widget>();
	pn->set_layout(new nanogui::BoxLayout(
		nanogui::Orientation::Horizontal, nanogui::Alignment::Fill, 4, 4));

	auto minB = pn->add<nanogui::Button>("Current", FA_ANGLE_DOWN);
	minB->set_callback([this]() {
		m_subWindows.minimize(m_subWindows.uiCombo->selected_index());
	});
	minB->set_tooltip("Minimize");

    auto maxB = pn->add<nanogui::Button>("Current", FA_ANGLE_UP);
	maxB->set_callback([this]() {
		m_subWindows.restore(m_subWindows.uiCombo->selected_index());
	});
	maxB->set_tooltip("Restore selected");

	auto minAll = pn->add<nanogui::Button>("All", FA_ANGLE_DOUBLE_DOWN);
	minAll->set_callback([this]() {
		for (int i = 0; i < static_cast<int>(m_subWindows.windows.size()); i++)
			m_subWindows.minimize(i);
	});
	minAll->set_tooltip("Minimize all");

	auto maxAll = pn->add<nanogui::Button>("All", FA_ANGLE_DOUBLE_UP);
	maxAll->set_callback([this]() {
		for (int i = 0; i < static_cast<int>(m_subWindows.windows.size()); i++)
			m_subWindows.restore(i);
	});
	maxAll->set_tooltip("Restore all");

	auto hideB =
		pn->add<nanogui::Button>("", FA_MINUS_SQUARE);
	hideB->set_callback([=]() {
		stack_panels->setSelectedIndex(1);
		w->set_fixed_size({HIDEN_WIDTH, HIDEN_HEIGHT});
		this->perform_layout();
	});
	hideB->set_tooltip("Hide this panel");


}

void WindowModule::SubWindows::onSubWindowFocused(int index) {
	const int n = static_cast<int>(uiCombo->items().size());
	if (index >= 0 && index < n) uiCombo->set_selected_index(index);
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

void WindowModule::SubWindows::setFocused(int index)
{
	const int n = static_cast<int>(windows.size());
	if (index >= 0 && index < n) windows.at(index)->request_focus();
}