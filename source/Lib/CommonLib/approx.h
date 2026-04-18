#ifndef APPROX_INSTRUMENTATION_H
#define APPROX_INSTRUMENTATION_H

#include <cstdint>
#include <limits>
#include <string>

namespace ApproxSS {
	uint8_t __attribute__((noinline, optimize("O0"))) start_level(const int64_t level = 0); //1 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) end_level(); //0 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) add_approx(void * const start_address, void const * const end_address, const int64_t bufferId, const int64_t configurationId, const size_t dataSizeInBytes, const bool isPrecise = false); //6 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) remove_approx(void * const start_address, void const * const end_address, const bool giveAwayRecords = true); //3 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) next_period(const bool isLinearIncrement = true, const int64_t orNewSetValue = std::numeric_limits<int64_t>::min()); // 2 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) enable_global_injection(); //0 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) disable_global_injection(); //0 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) disable_access_instrumentation(); //0 parameters

	uint8_t __attribute__((noinline, optimize("O0"))) enable_access_instrumentation(); //0 parameters
}

#endif /* APPROX_INSTRUMENTATION_H */