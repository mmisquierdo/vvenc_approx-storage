#include "approx.h"

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::start_level(int64_t level/* = 0*/){ // 1 parameters
	return 0;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::end_level(){ // 0 parameters
	return 1;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::add_approx(void * const start_address, void const * const end_address, const int64_t bufferId, const int64_t configurationId, const size_t dataSizeInBytes, const bool isPrecise /*= false*/) //6 parameters
{
	return 2;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::remove_approx(void * const start_address, void const * const end_address, const bool giveAwayRecords/*= true*/)	//3 parameters
{
	return 3;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::next_period(const bool isLinearIncrement /*= true*/, const int64_t orNewSetValue /*= std::numeric_limits<int64_t>::min()*/)	// 2 parameters
{
	return 4;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::enable_global_injection() // 0 parameters
{
	return 5;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::disable_global_injection() // 0 parameters
{
	return 6;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::disable_access_instrumentation() // 0 parameters
{
	return 7;
}

uint8_t __attribute__((noinline, optimize("O0"))) ApproxSS::enable_access_instrumentation() // 0 parameters
{
	return 8;
}
