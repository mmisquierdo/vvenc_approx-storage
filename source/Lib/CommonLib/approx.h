#ifndef APPROX_INSTRUMENTATION_H
#define APPROX_INSTRUMENTATION_H

//parameters named with a single letter are not actually used, they just serve to avoid a Pin bug.

#include <cstdint>
#include <limits>

namespace ApproxSS {
	int64_t __attribute__((optimize("O0"))) start_level(const int64_t level = 0); //1 parameters

	int64_t __attribute__((optimize("O0"))) end_level(); //0 parameters

	int64_t __attribute__((optimize("O0"))) add_approx(void * const start_address, void const * const end_address, const int64_t bufferId, const int64_t configurationId, const uint64_t dataSizeInBytes, const int64_t isPrecise = false); //6 parameters

	int64_t __attribute__((optimize("O0"))) remove_approx(void * const start_address, void const * const end_address, const bool giveAwayRecords = true); //3 parameters

	int64_t __attribute__((optimize("O0"))) next_period(const int64_t isLinearIncrement = true, const int64_t orNewSetValue = std::numeric_limits<int64_t>::min()); // 2 parameters

	int64_t __attribute__((optimize("O0"))) enable_global_injection(int64_t a1 = 0, int64_t a2 = 0, int64_t a3 = 0, int64_t a4 = 0); //4 parameters

	int64_t __attribute__((optimize("O0"))) disable_global_injection(int64_t a1 = 0, int64_t a2 = 0, int64_t a3 = 0, int64_t a4 = 0, int64_t a5 = 0); //5 parameters

	int64_t __attribute__((optimize("O0"))) disable_access_instrumentation(int64_t a1 = 0, int64_t a2 = 0, int64_t a3 = 0, int64_t a4 = 0, int64_t a5 = 0, int64_t a6 = 0, int64_t a7 = 0); //7 parameters

	int64_t __attribute__((optimize("O0"))) enable_access_instrumentation(int64_t a1 = 0, int64_t a2 = 0, int64_t a3 = 0, int64_t a4 = 0, int64_t a5 = 0, int64_t a6 = 0, int64_t a7 = 0, int64_t a8 = 0); //8 parameters
}

#endif /* APPROX_INSTRUMENTATION_H */