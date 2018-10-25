#include "./codec_impl.h"

#include "../../../inc/EHttpSession.hh"

#include <cstdint>
#include <string>

#include "../../include/buffer.h"
#include "../../include/header_map.h"

#include "../enum_to_int.h"
#include "../utility.h"
#include "../exception.h"
#include "../headers.h"

namespace efc {
namespace naf {
namespace Http {
namespace Http1 {

uint64_t Utility::getResponseStatus(const HeaderMap& headers) {
	const HeaderEntry* header = headers.Status();
	uint64_t response_code;
	if (!header
			|| !StringUtil::atoul(headers.Status()->value().c_str(),
					response_code)) {
		throw CodecClientException(__FILE__, __LINE__,
				":status must be specified and a valid unsigned long");
	}

	return response_code;
}

const std::string StreamEncoderImpl::CRLF = "\r\n";
const std::string StreamEncoderImpl::LAST_CHUNK = "0\r\n\r\n";

void StreamEncoderImpl::encodeHeader(const char* key, uint32_t key_size,
		const char* value, uint32_t value_size) {

	ES_ASSERT(key_size > 0);

	connection_.copyToBuffer(key, key_size);
	connection_.addCharToBuffer(':');
	connection_.addCharToBuffer(' ');
	connection_.copyToBuffer(value, value_size);
	connection_.addCharToBuffer('\r');
	connection_.addCharToBuffer('\n');
}

void StreamEncoderImpl::encode100ContinueHeaders(const HeaderMap& headers) {
	ES_ASSERT(headers.Status()->value() == "100");
	processing_100_continue_ = true;
	encodeHeaders(headers, false);
	processing_100_continue_ = false;
}

void StreamEncoderImpl::encodeHeaders(const HeaderMap& headers,
		bool end_stream) {
	bool saw_content_length = false;
	headers.iterate(
			[](const HeaderEntry& header, void* context) -> HeaderMap::Iterate {
				const char* key_to_use = header.key().c_str();
				uint32_t key_size_to_use = header.key().size();
				// Translate :authority -> host so that upper layers do not need to deal with this.
				if (key_size_to_use > 1 && key_to_use[0] == ':' && key_to_use[1] == 'a') {
					key_to_use = Headers::get().HostLegacy.get().c_str();
					key_size_to_use = Headers::get().HostLegacy.get().size();
				}

				// Skip all headers starting with ':' that make it here.
				if (key_to_use[0] == ':') {
					return HeaderMap::Iterate::Continue;
				}

				static_cast<StreamEncoderImpl*>(context)->encodeHeader(
						key_to_use, key_size_to_use, header.value().c_str(), header.value().size());
				return HeaderMap::Iterate::Continue;
			}, this);

	if (headers.ContentLength()) {
		saw_content_length = true;
	}

	ES_ASSERT(!headers.TransferEncoding());

	// Assume we are chunk encoding unless we are passed a content length or this is a header only
	// response. Upper layers generally should strip transfer-encoding since it only applies to
	// HTTP/1.1. The codec will infer it based on the type of response.
	if (saw_content_length) {
		chunk_encoding_ = false;
	} else {
		if (processing_100_continue_) {
			// Make sure we don't serialize chunk information with 100-Continue headers.
			chunk_encoding_ = false;
		} else if (end_stream) {
			encodeHeader(Headers::get().ContentLength.get().c_str(),
					Headers::get().ContentLength.get().size(), "0", 1);
			chunk_encoding_ = false;
		} else if (connection_.protocol() == Protocol::Http10) {
			chunk_encoding_ = false;
		} else {
			encodeHeader(Headers::get().TransferEncoding.get().c_str(),
					Headers::get().TransferEncoding.get().size(),
					Headers::get().TransferEncodingValues.Chunked.c_str(),
					Headers::get().TransferEncodingValues.Chunked.size());
			chunk_encoding_ = true;
		}
	}

	connection_.addCharToBuffer('\r');
	connection_.addCharToBuffer('\n');

	if (end_stream) {
		endEncode();
	} else {
		connection_.flushOutput();
	}
}

void StreamEncoderImpl::encodeData(Buffer::Instance& data, bool end_stream) {
	// end_stream may be indicated with a zero length data buffer. If that is the case, so not
	// atually write the zero length buffer out.
	if (data.length() > 0) {
		if (chunk_encoding_) {
			//@see: connection_.buffer().add(fmt::format("{:x}\r\n", data.length()));
			connection_.buffer().add(
					EString::formatOf("%x\r\n", data.length()).data());
		}

		connection_.buffer().move(data);

		if (chunk_encoding_) {
			connection_.buffer().add(CRLF);
		}
	}

	if (end_stream) {
		endEncode();
	} else {
		connection_.flushOutput();
	}
}

void StreamEncoderImpl::encodeTrailers(const HeaderMap&) {
	endEncode();
}

void StreamEncoderImpl::endEncode() {
	if (chunk_encoding_) {
		connection_.buffer().add(LAST_CHUNK);
	}

	connection_.flushOutput();
	connection_.onEncodeComplete();
}

void ConnectionImpl::flushOutput() {
	output_buffer_.add(reserved_iovec_);

	Buffer::LinkedBuffer& lb = output_buffer_.buffer();
	sp<EIoBuffer> buf;
	while ((buf = lb.poll()) != null) {
		connection_->write(buf);
	}
}

void ConnectionImpl::addCharToBuffer(char c) {
	reserved_iovec_->put(c);
}

void ConnectionImpl::addIntToBuffer(uint64_t i) {
	reserved_iovec_->putString(EString(i).toString().c_str());
}

void ConnectionImpl::copyToBuffer(const char* data, uint64_t length) {
	reserved_iovec_->put(data, length);
}

void StreamEncoderImpl::resetStream(StreamResetReason reason) {
	connection_.onResetStreamBase(reason);
}

static const char RESPONSE_PREFIX[] = "HTTP/1.1 ";
static const char HTTP_10_RESPONSE_PREFIX[] = "HTTP/1.0 ";

void ResponseStreamEncoderImpl::encodeHeaders(const HeaderMap& headers,
		bool end_stream) {
	started_response_ = true;
	uint64_t numeric_status = Utility::getResponseStatus(headers);

	if (connection_.protocol() == Protocol::Http10
			&& connection_.supports_http_10()) {
		connection_.copyToBuffer(HTTP_10_RESPONSE_PREFIX,
				sizeof(HTTP_10_RESPONSE_PREFIX) - 1);
	} else {
		connection_.copyToBuffer(RESPONSE_PREFIX, sizeof(RESPONSE_PREFIX) - 1);
	}
	connection_.addIntToBuffer(numeric_status);
	connection_.addCharToBuffer(' ');

	const char* status_string = CodeUtility::toString(
			static_cast<Code>(numeric_status));
	uint32_t status_string_len = strlen(status_string);
	connection_.copyToBuffer(status_string, status_string_len);

	connection_.addCharToBuffer('\r');
	connection_.addCharToBuffer('\n');

	StreamEncoderImpl::encodeHeaders(headers, end_stream);
}

static const char REQUEST_POSTFIX[] = " HTTP/1.1\r\n";

void RequestStreamEncoderImpl::encodeHeaders(const HeaderMap& headers,
		bool end_stream) {
	const HeaderEntry* method = headers.Method();
	const HeaderEntry* path = headers.Path();
	if (!method || !path) {
		throw CodecClientException(__FILE__, __LINE__,
				":method and :path must be specified");
	}

	if (method->value() == Headers::get().MethodValues.Head.c_str()) {
		head_request_ = true;
	}

	connection_.copyToBuffer(method->value().c_str(), method->value().size());
	connection_.addCharToBuffer(' ');
	connection_.copyToBuffer(path->value().c_str(), path->value().size());
	connection_.copyToBuffer(REQUEST_POSTFIX, sizeof(REQUEST_POSTFIX) - 1);

	StreamEncoderImpl::encodeHeaders(headers, end_stream);
}

http_parser_settings ConnectionImpl::settings_ {
		[](http_parser* parser) -> int {
			static_cast<ConnectionImpl*>(parser->data)->onMessageBeginBase();
			return 0;
		}, [](http_parser* parser, const char* at, size_t length) -> int {
			static_cast<ConnectionImpl*>(parser->data)->onUrl(at, length);
			return 0;
		},
		nullptr, // on_status
		[](http_parser* parser, const char* at, size_t length) -> int {
			static_cast<ConnectionImpl*>(parser->data)->onHeaderField(at, length);
			return 0;
		},
		[](http_parser* parser, const char* at, size_t length) -> int {
			static_cast<ConnectionImpl*>(parser->data)->onHeaderValue(at, length);
			return 0;
		},
		[](http_parser* parser) -> int {
			return static_cast<ConnectionImpl*>(parser->data)->onHeadersCompleteBase();
		}, [](http_parser* parser, const char* at, size_t length) -> int {
			static_cast<ConnectionImpl*>(parser->data)->onBody(at, length);
			return 0;
		}, [](http_parser* parser) -> int {
			static_cast<ConnectionImpl*>(parser->data)->onMessageComplete();
			return 0;
		}, nullptr, // on_chunk_header
		nullptr // on_chunk_complete
};

const ToLowerTable& ConnectionImpl::toLowerTable() {
	static ToLowerTable* table = new ToLowerTable();
	return *table;
}

ConnectionImpl::ConnectionImpl(sp<EHttpSession>& connection,
		http_parser_type type) :
		connection_(connection), output_buffer_() {
	reserved_iovec_ = EIoBuffer::allocate()->setAutoExpand(true);

	http_parser_init(&parser_, type);
	parser_.data = this;
}

void ConnectionImpl::completeLastHeader() {
	if (!current_header_field_.empty()) {
		toLowerTable().toLowerCase(current_header_field_.buffer(),
				current_header_field_.size());
		current_header_map_->addViaMove(std::move(current_header_field_),
				std::move(current_header_value_));
	}

	header_parsing_state_ = HeaderParsingState::Field;
	ES_ASSERT(current_header_field_.empty());
	ES_ASSERT(current_header_value_.empty());
}

void ConnectionImpl::dispatch(sp<EIoBuffer>& data) {
	// Always unpause before dispatch.
	http_parser_pause(&parser_, 0);

	ssize_t total_parsed = 0;
	if (data->remaining() > 0) {
		total_parsed += dispatchSlice(static_cast<const char*>(data->current()),
				data->remaining());
	} else {
		dispatchSlice(nullptr, 0);
	}
}

size_t ConnectionImpl::dispatchSlice(const char* slice, size_t len) {
	ssize_t rc = http_parser_execute(&parser_, &settings_, slice, len);
	if (HTTP_PARSER_ERRNO(&parser_) != HPE_OK
			&& HTTP_PARSER_ERRNO(&parser_) != HPE_PAUSED) {
		sendProtocolError();
		throw CodecProtocolException(__FILE__, __LINE__,
				"http/1.1 protocol error: "
						+ std::string(
								http_errno_name(HTTP_PARSER_ERRNO(&parser_))));
	}

	return rc;
}

void ConnectionImpl::onHeaderField(const char* data, size_t length) {
	if (header_parsing_state_ == HeaderParsingState::Done) {
		// Ignore trailers.
		return;
	}

	if (header_parsing_state_ == HeaderParsingState::Value) {
		completeLastHeader();
	}

	current_header_field_.append(data, length);
}

void ConnectionImpl::onHeaderValue(const char* data, size_t length) {
	if (header_parsing_state_ == HeaderParsingState::Done) {
		// Ignore trailers.
		return;
	}

	header_parsing_state_ = HeaderParsingState::Value;
	current_header_value_.append(data, length);
}

int ConnectionImpl::onHeadersCompleteBase() {
	completeLastHeader();
	if (!(parser_.http_major == 1 && parser_.http_minor == 1)) {
		// This is not necessarily true, but it's good enough since higher layers only care if this is
		// HTTP/1.1 or not.
		protocol_ = Protocol::Http10;
	}

	int rc = onHeadersComplete(std::move(current_header_map_));
	current_header_map_.reset();
	header_parsing_state_ = HeaderParsingState::Done;
	return rc;
}

void ConnectionImpl::onMessageBeginBase() {
	ES_ASSERT(!current_header_map_);
	current_header_map_.reset(new HeaderMapImpl());
	header_parsing_state_ = HeaderParsingState::Field;
	onMessageBegin();
}

void ConnectionImpl::onResetStreamBase(StreamResetReason reason) {
	ES_ASSERT(!reset_stream_called_);
	reset_stream_called_ = true;
	onResetStream(reason);
}

ServerConnectionImpl::ServerConnectionImpl(sp<EHttpSession>& connection,
		ServerConnectionCallbacks& callbacks, Http1Settings settings) :
		ConnectionImpl(connection, HTTP_REQUEST), callbacks_(callbacks), codec_settings_(
				settings) {
}

void ServerConnectionImpl::onEncodeComplete() {
	ES_ASSERT(active_request_);
	if (active_request_->remote_complete_) {
		// Only do this if remote is complete. If we are replying before the request is complete the
		// only logical thing to do is for higher level code to reset() / close the connection so we
		// leave the request around so that it can fire reset callbacks.
		active_request_.reset();
	}
}

void ServerConnectionImpl::handlePath(HeaderMapImpl& headers,
		unsigned int method) {
	HeaderString path(Headers::get().Path);

	bool is_connect = (method == HTTP_CONNECT);

	// The url is relative or a wildcard when the method is OPTIONS. Nothing to do here.
	if (active_request_->request_url_.c_str()[0] == '/'
			|| ((method == HTTP_OPTIONS)
					&& active_request_->request_url_.c_str()[0] == '*')) {
		headers.addViaMove(std::move(path),
				std::move(active_request_->request_url_));
		return;
	}

	// If absolute_urls and/or connect are not going be handled, copy the url and return.
	// This forces the behavior to be backwards compatible with the old codec behavior.
	if (!codec_settings_.allow_absolute_url_) {
		headers.addViaMove(std::move(path),
				std::move(active_request_->request_url_));
		return;
	}

	if (is_connect) {
		headers.addViaMove(std::move(path),
				std::move(active_request_->request_url_));
		return;
	}

	struct http_parser_url u;
	http_parser_url_init(&u);
	int result = http_parser_parse_url(active_request_->request_url_.buffer(),
			active_request_->request_url_.size(), is_connect, &u);

	if (result != 0) {
		sendProtocolError();
		throw CodecProtocolException(__FILE__, __LINE__,
				"http/1.1 protocol error: invalid url in request line, parsed invalid");
	} else {
		if ((u.field_set & (1 << UF_HOST)) == (1 << UF_HOST)
				&& (u.field_set & (1 << UF_SCHEMA)) == (1 << UF_SCHEMA)) {
			// RFC7230#5.7
			// When a proxy receives a request with an absolute-form of
			// request-target, the proxy MUST ignore the received Host header field
			// (if any) and instead replace it with the host information of the
			// request-target. A proxy that forwards such a request MUST generate a
			// new Host field-value based on the received request-target rather than
			// forward the received Host field-value.

			uint16_t authority_len = u.field_data[UF_HOST].len;

			if ((u.field_set & (1 << UF_PORT)) == (1 << UF_PORT)) {
				authority_len = authority_len + u.field_data[UF_PORT].len + 1;
			}

			// Insert the host header, this will later be converted to :authority
			std::string new_host(
					active_request_->request_url_.c_str()
							+ u.field_data[UF_HOST].off, authority_len);

			headers.insertHost().value(new_host);

			// RFC allows the absolute-uri to not end in /, but the absolute path form
			// must start with /
			if ((u.field_set & (1 << UF_PATH)) == (1 << UF_PATH)
					&& u.field_data[UF_PATH].len > 0) {
				HeaderString new_path;
				new_path.setCopy(
						active_request_->request_url_.c_str()
								+ u.field_data[UF_PATH].off,
						active_request_->request_url_.size()
								- u.field_data[UF_PATH].off);
				headers.addViaMove(std::move(path), std::move(new_path));
			} else {
				HeaderString new_path;
				new_path.setCopy("/", 1);
				headers.addViaMove(std::move(path), std::move(new_path));
			}

			active_request_->request_url_.clear();
			return;
		}
		sendProtocolError();
		throw CodecProtocolException(__FILE__, __LINE__,
				"http/1.1 protocol error: invalid url in request line");
	}
}

int ServerConnectionImpl::onHeadersComplete(HeaderMapImplPtr&& headers) {
	// Handle the case where response happens prior to request complete. It's up to upper layer code
	// to disconnect the connection but we shouldn't fire any more events since it doesn't make
	// sense.
	if (active_request_) {
		const char* method_string = http_method_str(static_cast<http_method>(parser_.method));

		// Currently, CONNECT is not supported, however; http_parser_parse_url needs to know about
		// CONNECT
		handlePath(*headers, parser_.method);
		ES_ASSERT(active_request_->request_url_.empty());

		headers->insertMethod().value(method_string, strlen(method_string));

		// Determine here whether we have a body or not. This uses the new RFC semantics where the
		// presence of content-length or chunked transfer-encoding indicates a body vs. a particular
		// method. If there is no body, we defer raising decodeHeaders() until the parser is flushed
		// with message complete. This allows upper layers to behave like HTTP/2 and prevents a proxy
		// scenario where the higher layers stream through and implicitly switch to chunked transfer
		// encoding because end stream with zero body length has not yet been indicated.
		if (parser_.flags & F_CHUNKED ||
				(parser_.content_length > 0 && parser_.content_length != ULLONG_MAX)) {
			active_request_->request_decoder_->decodeHeaders(std::move(headers), false);

			// If the connection has been closed (or is closing) after decoding headers, pause the parser
			// so we return control to the caller.
			//if (connection_.state() != ESocketSession::State::Open) {
			if (connection_->isClosed()) { //cxxjava
				http_parser_pause(&parser_, 1);
			}

		} else {
			deferred_end_stream_headers_ = std::move(headers);
		}
	}

	return 0;
}

void ServerConnectionImpl::onMessageBegin() {
	if (!resetStreamCalled()) {
		ES_ASSERT(!active_request_);
		active_request_.reset(new ActiveRequest(*this));
		active_request_->request_decoder_ = &callbacks_.newStream(
				active_request_->response_encoder_);
	}
}

void ServerConnectionImpl::onUrl(const char* data, size_t length) {
	if (active_request_) {
		active_request_->request_url_.append(data, length);
	}
}

void ServerConnectionImpl::onBody(const char* data, size_t length) {
	ES_ASSERT(!deferred_end_stream_headers_);
	if (active_request_) {
		Buffer::OwnedImpl buffer(data, length);
		active_request_->request_decoder_->decodeData(buffer, false);
	}
}

void ServerConnectionImpl::onMessageComplete() {
	if (active_request_) {
		Buffer::OwnedImpl buffer;
		active_request_->remote_complete_ = true;

		if (!!deferred_end_stream_headers_) {
			active_request_->request_decoder_->decodeHeaders(
					std::move(deferred_end_stream_headers_), true);
			deferred_end_stream_headers_.reset();
		} else {
			active_request_->request_decoder_->decodeData(buffer, true);
		}
	}

	// Always pause the parser so that the calling code can process 1 request at a time and apply
	// back pressure. However this means that the calling code needs to detect if there is more data
	// in the buffer and dispatch it again.
	http_parser_pause(&parser_, 1);
}

void ServerConnectionImpl::onResetStream(StreamResetReason reason) {
	ES_ASSERT(active_request_);
	active_request_.reset();
}

void ServerConnectionImpl::sendProtocolError() {
	// We do this here because we may get a protocol error before we have a logical stream. Higher
	// layers can only operate on streams, so there is no coherent way to allow them to send an error
	// "out of band." On one hand this is kind of a hack but on the other hand it normalizes HTTP/1.1
	// to look more like HTTP/2 to higher layers.
	if (!active_request_
			|| !active_request_->response_encoder_.startedResponse()) {
		//cxxjava
		//Buffer::OwnedImpl bad_request_response(
		//    fmt::format("HTTP/1.1 {} {}\r\ncontent-length: 0\r\nconnection: close\r\n\r\n",
		//                std::to_string(enumToInt(error_code_)), CodeUtility::toString(error_code_)));
		//connection_.write(bad_request_response, false);
		EString msg =
				EString::formatOf(
						"HTTP/1.1 %d %s\r\ncontent-length: 0\r\nconnection: close\r\n\r\n",
						enumToInt(error_code_),
						CodeUtility::toString(error_code_));
		sp<EIoBuffer> buf = EIoBuffer::wrap(msg.c_str(), msg.length());
		connection_->write(buf);
	}
}

}// namespace Http1
} // namespace Http
} // namespace naf
} // namespace efc

