#pragma once

template<typename Callable>
inline void thread_sync::sync(Callable on_sync)
{
	_single_sync(_data->count1);
	on_sync();
	_single_sync(_data->count2);
}
