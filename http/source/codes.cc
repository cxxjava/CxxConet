#include "./codes.h"
#include "./enum_to_int.h"

#include <string>

namespace efc {
namespace naf {
namespace Http {

std::string CodeUtility::groupStringForResponseCode(Code response_code) {
	if (CodeUtility::is2xx(enumToInt(response_code))) {
		return "2xx";
	} else if (CodeUtility::is3xx(enumToInt(response_code))) {
		return "3xx";
	} else if (CodeUtility::is4xx(enumToInt(response_code))) {
		return "4xx";
	} else if (CodeUtility::is5xx(enumToInt(response_code))) {
		return "5xx";
	} else {
		return "";
	}
}

const char* CodeUtility::toString(Code code) {
	// clang-format off
	switch (code) {
	// 1xx
	case Code::Continue:
		return "Continue";

		// 2xx
	case Code::OK:
		return "OK";
	case Code::Created:
		return "Created";
	case Code::Accepted:
		return "Accepted";
	case Code::NonAuthoritativeInformation:
		return "Non-Authoritative Information";
	case Code::NoContent:
		return "No Content";
	case Code::ResetContent:
		return "Reset Content";
	case Code::PartialContent:
		return "Partial Content";
	case Code::MultiStatus:
		return "Multi-Status";
	case Code::AlreadyReported:
		return "Already Reported";
	case Code::IMUsed:
		return "IM Used";

		// 3xx
	case Code::MultipleChoices:
		return "Multiple Choices";
	case Code::MovedPermanently:
		return "Moved Permanently";
	case Code::Found:
		return "Found";
	case Code::SeeOther:
		return "See Other";
	case Code::NotModified:
		return "Not Modified";
	case Code::UseProxy:
		return "Use Proxy";
	case Code::TemporaryRedirect:
		return "Temporary Redirect";
	case Code::PermanentRedirect:
		return "Permanent Redirect";

		// 4xx
	case Code::BadRequest:
		return "Bad Request";
	case Code::Unauthorized:
		return "Unauthorized";
	case Code::PaymentRequired:
		return "Payment Required";
	case Code::Forbidden:
		return "Forbidden";
	case Code::NotFound:
		return "Not Found";
	case Code::MethodNotAllowed:
		return "Method Not Allowed";
	case Code::NotAcceptable:
		return "Not Acceptable";
	case Code::ProxyAuthenticationRequired:
		return "Proxy Authentication Required";
	case Code::RequestTimeout:
		return "Request Timeout";
	case Code::Conflict:
		return "Conflict";
	case Code::Gone:
		return "Gone";
	case Code::LengthRequired:
		return "Length Required";
	case Code::PreconditionFailed:
		return "Precondition Failed";
	case Code::PayloadTooLarge:
		return "Payload Too Large";
	case Code::URITooLong:
		return "URI Too Long";
	case Code::UnsupportedMediaType:
		return "Unsupported Media Type";
	case Code::RangeNotSatisfiable:
		return "Range Not Satisfiable";
	case Code::ExpectationFailed:
		return "Expectation Failed";
	case Code::MisdirectedRequest:
		return "Misdirected Request";
	case Code::UnprocessableEntity:
		return "Unprocessable Entity";
	case Code::Locked:
		return "Locked";
	case Code::FailedDependency:
		return "Failed Dependency";
	case Code::UpgradeRequired:
		return "Upgrade Required";
	case Code::PreconditionRequired:
		return "Precondition Required";
	case Code::TooManyRequests:
		return "Too Many Requests";
	case Code::RequestHeaderFieldsTooLarge:
		return "Request Header Fields Too Large";

		// 5xx
	case Code::InternalServerError:
		return "Internal Server Error";
	case Code::NotImplemented:
		return "Not Implemented";
	case Code::BadGateway:
		return "Bad Gateway";
	case Code::ServiceUnavailable:
		return "Service Unavailable";
	case Code::GatewayTimeout:
		return "Gateway Timeout";
	case Code::HTTPVersionNotSupported:
		return "HTTP Version Not Supported";
	case Code::VariantAlsoNegotiates:
		return "Variant Also Negotiates";
	case Code::InsufficientStorage:
		return "Insufficient Storage";
	case Code::LoopDetected:
		return "Loop Detected";
	case Code::NotExtended:
		return "Not Extended";
	case Code::NetworkAuthenticationRequired:
		return "Network Authentication Required";
	}
	// clang-format on

	return "Unknown";
}

} // namespace http
} // namespace naf
} // namespace efc
