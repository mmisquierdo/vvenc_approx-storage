#ifndef APPROX_INSTRUMENTATION_H
#define APPROX_INSTRUMENTATION_H

//parameters named with a single letter are not actually used, they just serve to avoid a Pin bug.

#include <cstdint>

namespace ApproxSS {

	int __attribute__((optimize("O0"))) start_level(int64_t level = 0); //1 parameters

	int __attribute__((optimize("O0"))) end_level(); //0 parameters

	int __attribute__((optimize("O0"))) add_approx(void * const start_address, void const * const end_address, const int64_t bufferId, const int64_t configurationId, const uint32_t dataSizeInBytes); //5 parameters

	int __attribute__((optimize("O0"))) remove_approx(void * const start_address, void const * const end_address, const bool giveAwayRecords = true); //3 parameters

	int __attribute__((optimize("O0"))) next_period(const uint64_t period = 0, int b = 0); // 2 parameters

	int __attribute__((optimize("O0"))) enable_global_injection(int a = 0, int b = 0, int c = 0, int d = 0); //4 parameters

	int __attribute__((optimize("O0"))) disable_global_injection(int a = 0, int b = 0, int c = 0, int d = 0, int e = 0, int f = 0); //6 parameters

	int __attribute__((optimize("O0"))) disable_access_instrumentation(int a = 0, int b = 0, int c = 0, int d = 0, int e = 0, int f = 0, int g = 0); //7 parameters

	int __attribute__((optimize("O0"))) enable_access_instrumentation(int a = 0, int b = 0, int c = 0, int d = 0, int e = 0, int f = 0, int g = 0, int h = 0); //8 parameters

}

#endif /* APPROX_INSTRUMENTATION_H */