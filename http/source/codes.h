#pragma once

#include <chrono>
#include <cstdint>
#include <string>

#include "../include/codes.h"
#include "../include/header_map.h"

namespace efc {
namespace naf {
namespace Http {

/**
 * General utility routines for HTTP codes.
 */
class CodeUtility {
public:

	/**
	 * Convert an HTTP response code to a descriptive string.
	 * @param code supplies the code to convert.
	 * @return const char* the string.
	 */
	static const char* toString(Code code);

	static bool is1xx(uint64_t code) {
		return code >= 100 && code < 200;
	}
	static bool is2xx(uint64_t code) {
		return code >= 200 && code < 300;
	}
	static bool is3xx(uint64_t code) {
		return code >= 300 && code < 400;
	}
	static bool is4xx(uint64_t code) {
		return code >= 400 && code < 500;
	}
	static bool is5xx(uint64_t code) {
		return code >= 500 && code < 600;
	}

	static bool isGatewayError(uint64_t code) {
		return code >= 502 && code < 505;
	}

	static std::string groupStringForResponseCode(Code response_code);
};

} // namespace http
} // namespace naf
} // namespace efc
