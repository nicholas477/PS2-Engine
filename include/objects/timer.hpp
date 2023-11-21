#pragma once

#include <functional>
#include <memory>

#include "egg/assert.hpp"
#include "engine.hpp"
#include "tick.hpp"

class timer: public tickable
{
public:
	timer(float timer_length);

	virtual void tick(float deltaTime) override;

	bool is_finished() const
	{
		return timer_finished;
	}

	float get_start_time() const
	{
		return start_time;
	}

	float get_end_time() const
	{
		return end_time;
	}

	float get_time_left() const
	{
		return end_time - engine::get_game_time();
	}

	bool is_valid() const
	{
		return start_time >= 0.f && end_time >= 0.f && end_time >= start_time;
	}

	// Override these functions
	virtual void on_timer_tick(float deltaTime)
	{
	}

	virtual void on_timer_end(float deltaTime)
	{
	}

	virtual ~timer() {};

protected:
	float start_time    = -1.f;
	float end_time      = -1.f;
	bool timer_finished = false;
};


std::vector<std::unique_ptr<timer>>& get_managed_timers();

// Creates a timer that is automatically destroyed when the timer is finished
template <class T, class... Args>
void create_managed_timer(Args&&... args)
{
	check(get_managed_timers().size() < 128);
	printf("timers size: %d\n", get_managed_timers().size());

	static_assert(std::is_base_of<timer, T>::value);
	get_managed_timers().emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
}

// Creates a timer that is automatically destroyed when the timer is finished
void create_managed_timer_lambda(float timer_length,
                                 std::function<void(timer*, float)> on_timer_tick   = std::function<void(timer*, float)>(),
                                 std::function<void(timer*, float)> on_timer_finish = std::function<void(timer*, float)>());