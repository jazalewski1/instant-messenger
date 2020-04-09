#include <InputObserver.hpp>
#include <string>

InputObserver::InputObserver() :
	m_input{},
	m_wants_input{false},
	m_has_input{false}
{
}

void InputObserver::set_wants_input(bool b) { m_wants_input = b; }

bool InputObserver::wants_input() const { return m_wants_input; }

bool InputObserver::has_input() const { return m_has_input; }

std::string InputObserver::get_input()
{
	m_has_input = false;
	return m_input;
}

void InputObserver::update(const std::string& str)
{
	m_input = str;
	m_has_input = true;
}