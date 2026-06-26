#include "InputSystem.h"
#include "Windows.h"
#include "InputListener.h"

InputSystem::InputSystem()
{
}

InputSystem::~InputSystem()
{
}

void InputSystem::update()
{
	POINT current_mouse_pos = {};
	::GetCursorPos(&current_mouse_pos);

	if (m_first_time)
	{
		m_old_mouse_pos = POINT(current_mouse_pos.x, current_mouse_pos.y);
		m_first_time = false;
	}

	if (current_mouse_pos.x != m_old_mouse_pos.x || current_mouse_pos.y != m_old_mouse_pos.y)
	{
		std::map<InputListener*, InputListener*>::iterator it = m_map_listeners.begin();

		while (it != m_map_listeners.end())
		{

			it->second->onMouseMove(Point(current_mouse_pos.x - m_old_mouse_pos.x, current_mouse_pos.y - m_old_mouse_pos.y));
			it++;
		}
	}

	m_old_mouse_pos = POINT(current_mouse_pos.x, current_mouse_pos.y);

	if (::GetKeyboardState(m_keys_state))
	{
		for (unsigned int i = 0; i < 256; i++)
		{
			//Key is Down
			if (m_keys_state[i] & 0x80)
			{
				std::map<InputListener*, InputListener*>::iterator it = m_map_listeners.begin();

				while (it != m_map_listeners.end())
				{
					if (i == VK_LBUTTON)
					{
						if (m_keys_state[i] != m_old_keys_state[i])
							it->second->onLeftMouseDown(Point(current_mouse_pos.x, current_mouse_pos.y));
					}
					else if (i == VK_RBUTTON)
					{
						if (m_keys_state[i] != m_old_keys_state[i])
							it->second->onRightMouseDown(Point(current_mouse_pos.x, current_mouse_pos.y));
					}
					else
						it->second->onKeyDown(i);
					it++;
				}
			}
			//Key is Up
			else
			{
				if (m_old_keys_state[i] & 0x80)
				{
					std::map<InputListener*, InputListener*>::iterator it = m_map_listeners.begin();

					while (it != m_map_listeners.end())
					{
						if (i == VK_LBUTTON)
						{
							if (m_keys_state[i] != m_old_keys_state[i])
								it->second->onLeftMouseUp(Point(current_mouse_pos.x, current_mouse_pos.y));
						}
						else if (i == VK_RBUTTON)
						{
							if (m_keys_state[i] != m_old_keys_state[i])
								it->second->onRightMouseUp(Point(current_mouse_pos.x, current_mouse_pos.y));
						}
						else
							it->second->onKeyUp(i);
						it++;
					}
				}
			}
			
		}

		::memcpy(m_old_keys_state, m_keys_state, sizeof(unsigned char) * 256);
	}
}

void InputSystem::addListener(InputListener* listener)
{
	m_map_listeners[listener] = listener;
}

void InputSystem::removeListener(InputListener* listener)
{
	std::map<InputListener*, InputListener*>::iterator it = m_map_listeners.find(listener);

	if (it != m_map_listeners.end())
	{
		m_map_listeners.erase(it);
	}
}

InputSystem* InputSystem::get()
{
	static InputSystem system;
	return &system;
}
