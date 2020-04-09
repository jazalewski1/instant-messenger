#include <InputObservable.hpp>
#include <InputObserver.hpp>
#include <string>

void InputObservable::add(InputObserver* obs)
{
	m_observers.push_back(obs);
}

void InputObservable::notify(const std::string& input)
{
	for(auto& obs : m_observers)
	{
		auto in_obs = dynamic_cast<InputObserver*>(obs);
		if(in_obs->wants_input())
		{
			in_obs->update(input);
			break;
		}
	}
}