#include "./utility.h"

#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iterator>
#include <string>

#include "./exception.h"

namespace efc {
namespace naf {
namespace Http {

std::string DateFormatter::fromTime(const SystemTime& time) {
	return fromTimeT(std::chrono::system_clock::to_time_t(time));
}

std::string DateFormatter::fromTimeT(time_t time) {
	tm current_tm;
	gmtime_r(&time, &current_tm);

	std::array<char, 1024> buf;
	strftime(&buf[0], buf.size(), format_string_.c_str(), &current_tm);
	return std::string(&buf[0]);
}

std::string DateFormatter::now() {
	time_t current_time_t;
	time(&current_time_t);
	return fromTimeT(current_time_t);
}

ProdSystemTimeSource ProdSystemTimeSource::instance_;
ProdMonotonicTimeSource ProdMonotonicTimeSource::instance_;

ConstMemoryStreamBuffer::ConstMemoryStreamBuffer(const char* data,
		size_t size) {
	// std::streambuf won't modify `data`, but the interface still requires a char* for convenience,
	// so we need to const_cast.
	char* ptr = const_cast<char*>(data);

	this->setg(ptr, ptr, ptr + size);
}

InputConstMemoryStream::InputConstMemoryStream(const char* data, size_t size) :
		ConstMemoryStreamBuffer { data, size }, std::istream {
				static_cast<std::streambuf*>(this) } {
}

bool DateUtil::timePointValid(SystemTime time_point) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			time_point.time_since_epoch()).count() != 0;
}

bool DateUtil::timePointValid(MonotonicTime time_point) {
	return std::chrono::duration_cast<std::chrono::milliseconds>(
			time_point.time_since_epoch()).count() != 0;
}

const char StringUtil::WhitespaceChars[] = " \t\f\v\n\r";

bool StringUtil::atoul(const char* str, uint64_t& out, int base) {
	if (strlen(str) == 0) {
		return false;
	}

	char* end_ptr;
	errno = 0;
	out = strtoul(str, &end_ptr, base);
	if (*end_ptr != '\0' || (out == ULONG_MAX && errno == ERANGE)) {
		return false;
	} else {
		return true;
	}
}

bool StringUtil::atol(const char* str, int64_t& out, int base) {
	if (strlen(str) == 0) {
		return false;
	}

	char* end_ptr;
	errno = 0;
	out = strtol(str, &end_ptr, base);
	if (*end_ptr != '\0'
			|| ((out == LONG_MAX || out == LONG_MIN) && errno == ERANGE)) {
		return false;
	} else {
		return true;
	}
}

uint32_t StringUtil::itoa(char* out, size_t buffer_size, uint64_t i) {
	// The maximum size required for an unsigned 64-bit integer is 21 chars (including null).
	if (buffer_size < 21) {
		throw std::invalid_argument("itoa buffer too small");
	}

	char* current = out;
	do {
		*current++ = "0123456789"[i % 10];
		i /= 10;
	} while (i > 0);

	for (uint64_t i = 0, j = current - out - 1; i < j; i++, j--) {
		char c = out[i];
		out[i] = out[j];
		out[j] = c;
	}

	*current = 0;
	return current - out;
}

size_t StringUtil::strlcpy(char* dst, const char* src, size_t size) {
	strncpy(dst, src, size - 1);
	dst[size - 1] = '\0';
	return strlen(src);
}

std::string StringUtil::join(const std::vector<std::string>& source,
		const std::string& delimiter) {
	std::ostringstream buf;
	std::copy(source.begin(), source.end(),
			std::ostream_iterator<std::string>(buf, delimiter.c_str()));
	std::string ret = buf.str();
	// copy will always end with an extra delimiter, we remove it here.
	return ret.substr(0, ret.length() - delimiter.length());
}

std::string StringUtil::subspan(const std::string& source, size_t start,
		size_t end) {
	return source.substr(start, end - start);
}

std::string StringUtil::escape(const std::string& source) {
	std::string ret;

	// Prevent unnecessary allocation by allocating 2x original size.
	ret.reserve(source.length() * 2);
	for (char c : source) {
		switch (c) {
		case '\r':
			ret += "\\r";
			break;
		case '\n':
			ret += "\\n";
			break;
		case '\t':
			ret += "\\t";
			break;
		case '"':
			ret += "\\\"";
			break;
		default:
			ret += c;
			break;
		}
	}

	return ret;
}

bool StringUtil::endsWith(const std::string& source, const std::string& end) {
	if (source.length() < end.length()) {
		return false;
	}

	size_t start_position = source.length() - end.length();
	return std::equal(source.begin() + start_position, source.end(),
			end.begin());
}

bool StringUtil::startsWith(const char* source, const std::string& start,
		bool case_sensitive) {
	if (case_sensitive) {
		return strncmp(source, start.c_str(), start.size()) == 0;
	} else {
		return strncasecmp(source, start.c_str(), start.size()) == 0;
	}
}

const std::string& StringUtil::nonEmptyStringOrDefault(const std::string& s,
		const std::string& default_value) {
	return s.empty() ? default_value : s;
}

bool Primes::isPrime(uint32_t x) {
	if (x < 4) {
		return true; // eliminates special-casing 2.
	} else if ((x & 1) == 0) {
		return false; // eliminates even numbers >2.
	}

	uint32_t limit = sqrt(x);
	for (uint32_t factor = 3; factor <= limit; factor += 2) {
		if ((x % factor) == 0) {
			return false;
		}
	}
	return true;
}

uint32_t Primes::findPrimeLargerThan(uint32_t x) {
	x += (x % 2) + 1;
	while (!isPrime(x)) {
		x += 2;
	}
	return x;
}

// https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
void WelfordStandardDeviation::update(double newValue) {
	++count_;
	const double delta = newValue - mean_;
	mean_ += delta / count_;
	const double delta2 = newValue - mean_;
	m2_ += delta * delta2;
}

double WelfordStandardDeviation::computeVariance() const {
	if (count_ < 2) {
		return std::nan("");
	}
	return m2_ / (count_ - 1);
}

double WelfordStandardDeviation::computeStandardDeviation() const {
	const double variance = computeVariance();
	// It seems very difficult for variance to go negative, but from the calculation in update()
	// above, I can't quite convince myself it's impossible, so put in a guard to be sure.
	return (std::isnan(variance) || variance < 0) ?
			std::nan("") : sqrt(variance);
}

} // namespace Http
} // namespace naf
} // namespace efc
