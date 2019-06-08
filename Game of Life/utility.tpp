#pragma once

template<typename Function>
inline void thread_sync::sync(Function on_sync)
{
	_single_sync(_data->count1);
	on_sync();
	_single_sync(_data->count2);
}
