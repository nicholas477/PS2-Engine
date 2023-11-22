#include "objects/timer.hpp"
#include <algorithm>
#include <memory>

Timer::Timer(float timer_length)
{
	check(timer_length >= 0.f);

	start_time     = Engine::get_game_time();
	end_time       = start_time + timer_length;
	timer_finished = false;

	check(is_valid());
}

void Timer::tick(float deltaTime)
{
	if (Engine::get_game_time() >= end_time && !timer_finished)
	{
		timer_finished = true;
		on_timer_end(deltaTime);
	}
	else
	{
		on_timer_tick(deltaTime);
	}
}

std::vector<std::unique_ptr<Timer>>& get_managed_timers()
{
	static std::vector<std::unique_ptr<Timer>> managed_timers = []() -> auto
	{
		std::vector<std::unique_ptr<Timer>> new_timers;
		new_timers.reserve(128);
		return new_timers;
	}
	();

	return managed_timers;
}

static class TimerManager: public Tickable
{
public:
	virtual void tick(float deltatime) override
	{
		// Look over all of the timers, remove the ones that have finished
		auto& managed_timers = get_managed_timers();

		auto end = std::remove_if(managed_timers.begin(), managed_timers.end(), [](const std::unique_ptr<Timer>& elem) {
			return elem->is_finished();
		});
		managed_timers.erase(end, managed_timers.end());
	}
} _timer_manager;

class TimerLambda: public Timer
{
public:
	TimerLambda(float timer_length,
	            std::function<void(Timer*, float)> on_timer_tick,
	            std::function<void(Timer*, float)> on_timer_finish)
	    : Timer(timer_length)
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
	std::function<void(Timer*, float)> timer_tick_func;
	std::function<void(Timer*, float)> timer_finish_func;
};

void create_managed_timer_lambda(float timer_length,
                                 std::function<void(Timer*, float)> on_timer_tick,
                                 std::function<void(Timer*, float)> on_timer_finish)
{
	check(get_managed_timers().size() < 128);
	printf("timers size: %d\n", get_managed_timers().size());
	get_managed_timers().emplace_back(std::make_unique<TimerLambda>(timer_length, std::move(on_timer_tick), std::move(on_timer_finish)));
}