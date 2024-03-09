#pragma once

// Define a ARM_MAC_ARCH as ARM-based macs currently segfault on c++ exceptions meaning we disable these at compile time
#ifdef __APPLE__
	#ifdef __MACH__
		#if defined(__aarch64__) || defined(__arm__)
			#define ARM_MAC_ARCH 1
		#endif
	#endif
#endif