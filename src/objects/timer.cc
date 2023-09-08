#include "objects/timer.hpp"
#include <algorithm>
#include <memory>

timer::timer(float timer_length)
{
	check(timer_length >= 0.f);

	start_time     = engine::get_game_time();
	end_time       = start_time + timer_length;
	timer_finished = false;

	check(is_valid());
}

void timer::tick(float deltaTime)
{
	if (engine::get_game_time() >= end_time && !timer_finished)
	{
		timer_finished = true;
		on_timer_end(deltaTime);
	}
	else
	{
		on_timer_tick(deltaTime);
	}
}

std::vector<std::unique_ptr<timer>>& get_managed_timers()
{
	static std::vector<std::unique_ptr<timer>> managed_timers = []() -> auto
	{
		std::vector<std::unique_ptr<timer>> new_timers;
		new_timers.reserve(128);
		return new_timers;
	}
	();

	return managed_timers;
}

static class timer_manager: public tickable
{
public:
	virtual void tick(float deltatime) override
	{
		// Look over all of the timers, remove the ones that have finished
		auto& managed_timers = get_managed_timers();

		auto end = std::remove_if(managed_timers.begin(), managed_timers.end(), [](const std::unique_ptr<timer>& elem) {
			return elem->is_finished();
		});
		managed_timers.erase(end, managed_timers.end());
	}
} _timer_manager;

class timer_lambda: public timer
{
public:
	timer_lambda(float timer_length,
	             std::function<void(timer*, float)> on_timer_tick,
	             std::function<void(timer*, float)> on_timer_finish)
	    : timer(timer_length)
	    , timer_tick_func(std::move(on_timer_tick))
	    , timer_finish_func(std::move(on_timer_finish))
	{
	}

	virtual void on_timer_tick(float deltaTime)
	{
		if (timer_tick_func)
		{
			timer_tick_func(this, deltaTime);
		}
	}

	virtual void on_timer_end(float deltaTime)
	{
		if (timer_finish_func)
		{
			timer_finish_func(this, deltaTime);
		}
	}

protected:
	std::function<void(timer*, float)> timer_tick_func;
	std::function<void(timer*, float)> timer_finish_func;
};

void create_managed_timer_lambda(float timer_length,
                                 std::function<void(timer*, float)> on_timer_tick,
                                 std::function<void(timer*, float)> on_timer_finish)
{
	check(get_managed_timers().size() < 128);
	printf("timers size: %d\n", get_managed_timers().size());
	get_managed_timers().emplace_back(std::make_unique<timer_lambda>(timer_length, std::move(on_timer_tick), std::move(on_timer_finish)));
}