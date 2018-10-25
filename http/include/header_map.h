#pragma once

#include "Efc.hh"

#include <string.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>

namespace efc {
namespace naf {
namespace Http {

/**
 * Wrapper for a lower case string used in header operations to generally avoid needless case
 * insensitive compares.
 */
class LowerCaseString {
public:
	LowerCaseString(LowerCaseString&& rhs) : string_(std::move(rhs.string_)) {}
	LowerCaseString(const LowerCaseString& rhs) : string_(rhs.string_) {}
	explicit LowerCaseString(const std::string& new_string) : string_(new_string) { lower(); }
	explicit LowerCaseString(std::string&& new_string, bool convert = true)
      : string_(std::move(new_string)) {
		if (convert) {
			lower();
		}
	}

	const std::string& get() const { return string_; }
	bool operator==(const LowerCaseString& rhs) const { return string_ == rhs.string_; }

private:
	void lower() { std::transform(string_.begin(), string_.end(), string_.begin(), tolower); }

	std::string string_;
};

/**
 * This is a string implementation for use in header processing. It is heavily optimized for
 * performance. It supports 3 different types of storage and can switch between them:
 * 1) A reference.
 * 2) Interned string.
 * 3) Heap allocated storage.
 */
class HeaderString {
public:
	enum class Type {
		Inline, Reference, Dynamic
	};

	/**
	 * Default constructor. Sets up for inline storage.
	 */
	HeaderString();

	/**
	 * Constructor for a string reference.
	 * @param ref_value MUST point to data that will live beyond the lifetime of any request/response
	 *        using the string (since a codec may optimize for zero copy).
	 */
	explicit HeaderString(const LowerCaseString& ref_value);

	/**
	 * Constructor for a string reference.
	 * @param ref_value MUST point to data that will live beyond the lifetime of any request/response
	 *        using the string (since a codec may optimize for zero copy).
	 */
	explicit HeaderString(const std::string& ref_value);

	HeaderString(HeaderString&& move_value);
	~HeaderString();

	/**
	 * Append data to an existing string. If the string is a reference string the reference data is
	 * not copied.
	 */
	void append(const char* data, uint32_t size);

	/**
	 * @return the modifiable backing buffer (either inline or heap allocated).
	 */
	char* buffer() {return buffer_.dynamic_;}

	/**
	 * @return a null terminated C string.
	 */
	const char* c_str() const {return buffer_.ref_;}

	/**
	 * @return a std::string.
	 */
	std::string getString() const {return std::string(buffer_.ref_, string_length_);}

	/**
	 * Return the string to a default state. Reference strings are not touched. Both inline/dynamic
	 * strings are reset to zero size.
	 */
	void clear();

	/**
	 * @return whether the string is empty or not.
	 */
	bool empty() const {return string_length_ == 0;}

	/**
	 * @return whether a substring exists in the string.
	 */
	bool find(const char* str) const {return strstr(c_str(), str);}

	/**
	 * HeaderString is in token list form, each token separated by commas or whitespace,
	 * see https://www.w3.org/Protocols/rfc2616/rfc2616-sec2.html#sec2.1 for more information,
	 * header field value's case sensitivity depends on each header.
	 * @return whether contains token in case insensitive manner.
	 */
	bool caseInsensitiveContains(const char* token) const {
		// Avoid dead loop if token argument is empty.
		const int n = strlen(token);
		if (n == 0) {
			return false;
		}

		// Find token substring, skip if it's partial of other token.
		const char* tokens = c_str();
		for (const char* p = tokens; (p = strcasestr(p, token)); p += n) {
			if ((p == tokens || *(p - 1) == ' ' || *(p - 1) == ',') &&
			(*(p + n) == '\0' || *(p + n) == ' ' || *(p + n) == ',')) {
				return true;
			}
		}

		return false;
	}

	/**
	 * Set the value of the string by copying data into it. This overwrites any existing string.
	 */
	void setCopy(const char* data, uint32_t size);

	/**
	 * Set the value of the string to an integer. This overwrites any existing string.
	 */
	void setInteger(uint64_t value);

	/**
	 * Set the value of the string to a string reference.
	 * @param ref_value MUST point to data that will live beyond the lifetime of any request/response
	 *        using the string (since a codec may optimize for zero copy).
	 */
	void setReference(const std::string& ref_value);

	/**
	 * @return the size of the string, not including the null terminator.
	 */
	uint32_t size() const {return string_length_;}

	/**
	 * @return the type of backing storage for the string.
	 */
	Type type() const {return type_;}

	bool operator==(const char* rhs) const {return 0 == strcmp(c_str(), rhs);}
	bool operator!=(const char* rhs) const {return 0 != strcmp(c_str(), rhs);}

private:
	union {
		char* dynamic_;
		const char* ref_;
	}buffer_;

	union {
		char inline_buffer_[128];
		uint32_t dynamic_capacity_;
	};

	void freeDynamic();

	uint32_t string_length_;
	Type type_;
};

/**
 * Encapsulates an individual header entry (including both key and value).
 */
class HeaderEntry {
public:
	virtual ~HeaderEntry() {
	}

	/**
	 * @return the header key.
	 */
	virtual const HeaderString& key() const = 0;

	/**
	 * Set the header value by copying data into it.
	 */
	virtual void value(const char* value, uint32_t size) = 0;

	/**
	 * Set the header value by copying data into it.
	 */
	virtual void value(const std::string& value) = 0;

	/**
	 * Set the header value by copying an integer into it.
	 */
	virtual void value(uint64_t value) = 0;

	/**
	 * Set the header value by copying the value in another header entry.
	 */
	virtual void value(const HeaderEntry& header) = 0;

	/**
	 * @return the header value.
	 */
	virtual const HeaderString& value() const = 0;

	/**
	 * @return the header value.
	 */
	virtual HeaderString& value() = 0;

private:
	void value(const char*); // Do not allow auto conversion to std::string
}
;

/**
 * The following defines all headers that Envoy allows direct access to inside of the header map.
 * In practice, these are all headers used during normal Envoy request flow processing. This allows
 * O(1) access to these headers without even a hash lookup.
 */
#define ALL_INLINE_HEADERS(HEADER_FUNC)                                                            \
  HEADER_FUNC(AcceptEncoding)                                                                      \
  HEADER_FUNC(AccessControlRequestHeaders)                                                         \
  HEADER_FUNC(AccessControlRequestMethod)                                                          \
  HEADER_FUNC(AccessControlAllowOrigin)                                                            \
  HEADER_FUNC(AccessControlAllowHeaders)                                                           \
  HEADER_FUNC(AccessControlAllowMethods)                                                           \
  HEADER_FUNC(AccessControlAllowCredentials)                                                       \
  HEADER_FUNC(AccessControlExposeHeaders)                                                          \
  HEADER_FUNC(AccessControlMaxAge)                                                                 \
  HEADER_FUNC(Authorization)                                                                       \
  HEADER_FUNC(CacheControl)                                                                        \
  HEADER_FUNC(ClientTraceId)                                                                       \
  HEADER_FUNC(Connection)                                                                          \
  HEADER_FUNC(ContentEncoding)                                                                     \
  HEADER_FUNC(ContentLength)                                                                       \
  HEADER_FUNC(ContentType)                                                                         \
  HEADER_FUNC(Date)                                                                                \
  HEADER_FUNC(Etag)                                                                                \
  HEADER_FUNC(Expect)                                                                              \
  HEADER_FUNC(ForwardedClientCert)                                                                 \
  HEADER_FUNC(ForwardedFor)                                                                        \
  HEADER_FUNC(ForwardedProto)                                                                      \
  HEADER_FUNC(GrpcAcceptEncoding)                                                                  \
  HEADER_FUNC(GrpcMessage)                                                                         \
  HEADER_FUNC(GrpcStatus)                                                                          \
  HEADER_FUNC(Host)                                                                                \
  HEADER_FUNC(KeepAlive)                                                                           \
  HEADER_FUNC(LastModified)                                                                        \
  HEADER_FUNC(Method)                                                                              \
  HEADER_FUNC(Origin)                                                                              \
  HEADER_FUNC(OtSpanContext)                                                                       \
  HEADER_FUNC(Path)                                                                                \
  HEADER_FUNC(ProxyConnection)                                                                     \
  HEADER_FUNC(Referer)                                                                             \
  HEADER_FUNC(RequestId)                                                                           \
  HEADER_FUNC(Scheme)                                                                              \
  HEADER_FUNC(Server)                                                                              \
  HEADER_FUNC(Status)                                                                              \
  HEADER_FUNC(TE)                                                                                  \
  HEADER_FUNC(TransferEncoding)                                                                    \
  HEADER_FUNC(Upgrade)                                                                             \
  HEADER_FUNC(UserAgent)                                                                           \
  HEADER_FUNC(Vary)                                                                                \
  HEADER_FUNC(XB3TraceId)                                                                          \
  HEADER_FUNC(XB3SpanId)                                                                           \
  HEADER_FUNC(XB3ParentSpanId)                                                                     \
  HEADER_FUNC(XB3Sampled)                                                                          \
  HEADER_FUNC(XB3Flags)

/**
 * The following functions are defined for each inline header above. E.g., for ContentLength we
 * have:
 *
 * ContentLength() -> returns the header entry if it exists or nullptr.
 * insertContentLength() -> inserts the header if it does not exist, and returns a reference to it.
 * removeContentLength() -> removes the header if it exists.
 */
#define DEFINE_INLINE_HEADER(name)                                                                 \
  virtual const HeaderEntry* name() const = 0;                                                    \
  virtual HeaderEntry* name() = 0;                                                                \
  virtual HeaderEntry& insert##name() = 0;                                                        \
  virtual void remove##name() = 0;

/**
 * Wraps a set of HTTP headers.
 */
class HeaderMap {
public:
	virtual ~HeaderMap() {
	}

	ALL_INLINE_HEADERS(DEFINE_INLINE_HEADER)

	/**
	 * Add a reference header to the map. Both key and value MUST point to data that will live beyond
	 * the lifetime of any request/response using the string (since a codec may optimize for zero
	 * copy). Nothing will be copied.
	 *
	 * Calling addReference multiple times for the same header will result in multiple headers being
	 * present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to add; it WILL NOT be copied.
	 * @param value specifies the value of the header to add; it WILL NOT be copied.
	 */
	virtual void addReference(const LowerCaseString& key,
			const std::string& value) = 0;

	/**
	 * Add a header with a reference key to the map. The key MUST point to data that will live beyond
	 * the lifetime of any request/response using the string (since a codec may optimize for zero
	 * copy). The value will be copied.
	 *
	 * Calling addReferenceKey multiple times for the same header will result in multiple headers
	 * being present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to add; it WILL NOT be copied.
	 * @param value specifies the value of the header to add; it WILL be copied.
	 */
	virtual void addReferenceKey(const LowerCaseString& key,
			uint64_t value) = 0;

	/**
	 * Add a header with a reference key to the map. The key MUST point to point to data that will
	 * live beyond the lifetime of any request/response using the string (since a codec may optimize
	 * for zero copy). The value will be copied.
	 *
	 * Calling addReferenceKey multiple times for the same header will result in multiple headers
	 * being present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to add; it WILL NOT be copied.
	 * @param value specifies the value of the header to add; it WILL be copied.
	 */
	virtual void addReferenceKey(const LowerCaseString& key,
			const std::string& value) = 0;

	/**
	 * Add a header by copying both the header key and the value.
	 *
	 * Calling addCopy multiple times for the same header will result in multiple headers being
	 * present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to add; it WILL be copied.
	 * @param value specifies the value of the header to add; it WILL be copied.
	 */
	virtual void addCopy(const LowerCaseString& key, uint64_t value) = 0;

	/**
	 * Add a header by copying both the header key and the value.
	 *
	 * Calling addCopy multiple times for the same header will result in multiple headers being
	 * present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to add; it WILL be copied.
	 * @param value specifies the value of the header to add; it WILL be copied.
	 */
	virtual void addCopy(const LowerCaseString& key,
			const std::string& value) = 0;

	/**
	 * Set a reference header in the map. Both key and value MUST point to data that will live beyond
	 * the lifetime of any request/response using the string (since a codec may optimize for zero
	 * copy). Nothing will be copied.
	 *
	 * Calling setReference multiple times for the same header will result in only the last header
	 * being present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to set; it WILL NOT be copied.
	 * @param value specifies the value of the header to set; it WILL NOT be copied.
	 */
	virtual void setReference(const LowerCaseString& key,
			const std::string& value) = 0;

	/**
	 * Set a header with a reference key in the map. The key MUST point to point to data that will
	 * live beyond the lifetime of any request/response using the string (since a codec may optimize
	 * for zero copy). The value will be copied.
	 *
	 * Calling setReferenceKey multiple times for the same header will result in only the last header
	 * being present in the HeaderMap.
	 *
	 * @param key specifies the name of the header to set; it WILL NOT be copied.
	 * @param value specifies the value of the header to set; it WILL be copied.
	 */
	virtual void setReferenceKey(const LowerCaseString& key,
			const std::string& value) = 0;

	/**
	 * @return uint64_t the approximate size of the header map in bytes.
	 */
	virtual uint64_t byteSize() const = 0;

	/**
	 * Get a header by key.
	 * @param key supplies the header key.
	 * @return the header entry if it exists otherwise nullptr.
	 */
	virtual const HeaderEntry* get(const LowerCaseString& key) const = 0;
	virtual HeaderEntry* get(const LowerCaseString& key) = 0;

	// aliases to make iterate() and iterateReverse() callbacks easier to read
	enum class Iterate {
		Continue, Break
	};

	/**
	 * Callback when calling iterate() over a const header map.
	 * @param header supplies the header entry.
	 * @param context supplies the context passed to iterate().
	 * @return Iterate::Continue to continue iteration.
	 */
	typedef Iterate (*ConstIterateCb)(const HeaderEntry& header, void* context);

	/**
	 * Iterate over a constant header map.
	 * @param cb supplies the iteration callback.
	 * @param context supplies the context that will be passed to the callback.
	 */
	virtual void iterate(ConstIterateCb cb, void* context) const = 0;

	/**
	 * Iterate over a constant header map in reverse order.
	 * @param cb supplies the iteration callback.
	 * @param context supplies the context that will be passed to the callback.
	 */
	virtual void iterateReverse(ConstIterateCb cb, void* context) const = 0;

	enum class Lookup {
		Found, NotFound, NotSupported
	};

	/**
	 * Lookup one of the predefined inline headers (see ALL_INLINE_HEADERS below) by key.
	 * @param key supplies the header key.
	 * @param entry is set to the header entry if it exists and if key is one of the predefined inline
	 * headers; otherwise, nullptr.
	 * @return Lookup::Found if lookup was successful, Lookup::NotFound if the header entry doesn't
	 * exist, or Lookup::NotSupported if key is not one of the predefined inline headers.
	 */
	virtual Lookup lookup(const LowerCaseString& key,
			const HeaderEntry** entry) const = 0;

	/**
	 * Remove all instances of a header by key.
	 * @param key supplies the header key to remove.
	 */
	virtual void remove(const LowerCaseString& key) = 0;

	/**
	 * @return the number of headers in the map.
	 */
	virtual size_t size() const = 0;
};

typedef sp<HeaderMap> HeaderMapPtr;

} // namespace http
} // namespace naf
} // namespace efc
