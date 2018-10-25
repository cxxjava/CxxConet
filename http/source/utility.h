#pragma once

#include <strings.h>

#include <chrono>
#include <cstdint>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "./time.h"

namespace efc {
namespace naf {
namespace Http {

/**
 * Utility class for formatting dates given a strftime style format string.
 */
class DateFormatter {
public:
	DateFormatter(const std::string& format_string) :
			format_string_(format_string) {
	}

	/**
	 * @return std::string representing the GMT/UTC time based on the input time.
	 */
	std::string fromTime(const SystemTime& time);

	/**
	 * @return std::string representing the current GMT/UTC time based on the format string.
	 */
	std::string now();

private:
	std::string fromTimeT(time_t time);

	std::string format_string_;
};

/**
 * Production implementation of SystemTimeSource that returns the current time.
 */
class ProdSystemTimeSource: public SystemTimeSource {
public:
	// SystemTimeSource
	SystemTime currentTime() override {return std::chrono::system_clock::now();}

	static ProdSystemTimeSource instance_;
};

/**
 * Production implementation of MonotonicTimeSource that returns the current time.
 */
class ProdMonotonicTimeSource: public MonotonicTimeSource {
public:
	// MonotonicTimeSource
	MonotonicTime currentTime() override {return std::chrono::steady_clock::now();}

	static ProdMonotonicTimeSource instance_;
};

/**
 * Class used for creating non-copying std::istream's. See InputConstMemoryStream below.
 */
class ConstMemoryStreamBuffer: public std::streambuf {
public:
	ConstMemoryStreamBuffer(const char* data, size_t size);
};

/**
 * std::istream class similar to std::istringstream, except that it provides a view into a region of
 * constant memory. It can be more efficient than std::istringstream because it doesn't copy the
 * provided string.
 *
 * See https://stackoverflow.com/a/13059195/4447365.
 */
class InputConstMemoryStream: public virtual ConstMemoryStreamBuffer,
		public std::istream {
public:
	InputConstMemoryStream(const char* data, size_t size);
};

/**
 * Utility class for date/time helpers.
 */
class DateUtil {
public:
	/**
	 * @return whether a time_point contains a valid, not default constructed time.
	 */
	static bool timePointValid(SystemTime time_point);

	/**
	 * @return whether a time_point contains a valid, not default constructed time.
	 */
	static bool timePointValid(MonotonicTime time_point);
};

/**
 * Utility routines for working with strings.
 */
class StringUtil {
public:
	static const char WhitespaceChars[];

	/**
	 * Convert a string to an unsigned long, checking for error.
	 * @param return true if successful, false otherwise.
	 */
	static bool atoul(const char* str, uint64_t& out, int base = 10);

	/**
	 * Convert a string to a long, checking for error.
	 * @param return true if successful, false otherwise.
	 */
	static bool atol(const char* str, int64_t& out, int base = 10);

	/**
	 * Perform a case insensitive compare of 2 strings.
	 * @param lhs supplies string 1.
	 * @param rhs supplies string 2.
	 * @return < 0, 0, > 0 depending on the comparison result.
	 */
	static int caseInsensitiveCompare(const char* lhs, const char* rhs) {
		return strcasecmp(lhs, rhs);
	}

	/**
	 * Convert an unsigned integer to a base 10 string as fast as possible.
	 * @param out supplies the string to fill.
	 * @param out_len supplies the length of the output buffer. Must be >= MIN_ITOA_OUT_LEN.
	 * @param i supplies the number to convert.
	 * @return the size of the string, not including the null termination.
	 */
	static constexpr size_t MIN_ITOA_OUT_LEN = 21;
	static uint32_t itoa(char* out, size_t out_len, uint64_t i);

	/**
	 * Size-bounded string copying and concatenation
	 */
	static size_t strlcpy(char* dst, const char* src, size_t size);

	/**
	 * Join elements of a vector into a string delimited by delimiter.
	 * @param source supplies the strings to join.
	 * @param delimiter supplies the delimiter to join them together.
	 * @return string combining elements of `source` with `delimiter` in between each element.
	 */
	static std::string join(const std::vector<std::string>& source,
			const std::string& delimiter);

	/**
	 * Version of substr() that operates on a start and end index instead of a start index and a
	 * length.
	 */
	static std::string subspan(const std::string& source, size_t start,
			size_t end);

	/**
	 * Escape strings for logging purposes. Returns a copy of the string with
	 * \n, \r, \t, and " (double quote) escaped.
	 * @param source supplies the string to escape.
	 * @return escaped string.
	 */
	static std::string escape(const std::string& source);

	/**
	 * @return true if @param source ends with @param end.
	 */
	static bool endsWith(const std::string& source, const std::string& end);

	/**
	 * @param case_sensitive determines if the compare is case sensitive
	 * @return true if @param source starts with @param start and ignores cases.
	 */
	static bool startsWith(const char* source, const std::string& start,
			bool case_sensitive = true);

	/**
	 * Provide a default value for a string if empty.
	 * @param s string.
	 * @param default_value replacement for s if empty.
	 * @return s is !s.empty() otherwise default_value.
	 */
	static const std::string& nonEmptyStringOrDefault(const std::string& s,
			const std::string& default_value);
};

/**
 * Utilities for finding primes
 */
class Primes {
public:
	/**
	 * Determines whether x is prime.
	 */
	static bool isPrime(uint32_t x);

	/**
	 * Finds the next prime number larger than x.
	 */
	static uint32_t findPrimeLargerThan(uint32_t x);
};

/**
 * Computes running standard-deviation using Welford's algorithm:
 * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
 */
class WelfordStandardDeviation {
public:
	/**
	 * Accumulates a new value into the standard deviation.
	 * @param newValue the new value
	 */
	void update(double newValue);

	/**
	 * @return double the computed mean value.
	 */
	double mean() const {
		return mean_;
	}

	/**
	 * @return uint64_t the number of times update() was called
	 */
	uint64_t count() const {
		return count_;
	}

	/**
	 * @return double the standard deviation.
	 */
	double computeStandardDeviation() const;

private:
	double computeVariance() const;

	uint64_t count_ { 0 };
	double mean_ { 0 };
	double m2_ { 0 };
};

} // namespace Http
} // namespace naf
} // namespace efc
