#pragma once

#include <InputObserver.hpp>
#include <string>
#include <vector>

/*
	Publisher implementation (here Observable) from Observer Design Pattern modified to prioritize Subscribers (here Observers).
	Order, in which observers are added, is the order that decides which observer gets input.
	First observer to be added takes precedence to get input.
*/
class InputObservable
{
protected:
	std::vector<InputObserver*> m_observers;

public:
	void notify(const std::string& input);

	void add(InputObserver* obs);
};