#pragma once

#include <string>

/*
	Subscriber implementation (here Observer) from Observer Design Pattern.
*/
class InputObserver
{
private:
	std::string m_input;
	bool m_wants_input;
	bool m_has_input;

public:
	InputObserver();
	
	void update(const std::string& str);

	void set_wants_input(bool b);

	bool wants_input() const;

	bool has_input() const;

	std::string get_input();
};