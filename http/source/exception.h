#pragma once

#include <string>

#include "Efc.hh"

#include "../include/header_map.h"

namespace efc {
namespace naf {
namespace Http {

/**
 * Indicates a non-recoverable protocol error that should result in connection termination.
 */
class CodecProtocolException : public ERuntimeException {
public:
	CodecProtocolException(const char *_file_, int _line_,
			const std::string& message) :
			ERuntimeException(_file_, _line_, message.c_str()) {
	}
};

/**
 * Raised when a response is received on a connection that did not send a request. In practice
 * this can only happen on HTTP/1.1 connections.
 */
class PrematureResponseException : public ERuntimeException {
public:
	PrematureResponseException(const char *_file_, int _line_,
			HeaderMapPtr&& headers) :
			ERuntimeException(_file_, _line_, ""), headers_(std::move(headers)) {
	}

	const HeaderMap& headers() { return *headers_; }

private:
	HeaderMapPtr headers_;
};

/**
 * Indicates a client (local) side error which should not happen.
 */
class CodecClientException : public ERuntimeException {
public:
	CodecClientException(const char *_file_, int _line_,
			const std::string& message) :
			ERuntimeException(_file_, _line_, message.c_str()) {
	}
};

} // namespace http
} // namespace naf
} // namespace efc
