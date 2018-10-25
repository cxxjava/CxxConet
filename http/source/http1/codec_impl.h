#pragma once

#include "http_parser/http_parser.h"

//#include "../../../inc/EHttpSession.hh"

#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <string>

#include "../../include/codec.h"

#include "../buffer_impl.h"
#include "../to_lower_table.h"
#include "../codes.h"
#include "../header_map_impl.h"

namespace efc {
namespace naf {

class EHttpSession;

namespace Http {
namespace Http1 {

class Utility {
public:
	/**
	 * Get the response status from the response headers.
	 * @param headers supplies the headers to get the status from.
	 * @return uint64_t the response code or throws an exception if the headers are invalid.
	 */
	static uint64_t getResponseStatus(const HeaderMap& headers);
};

class ConnectionImpl;

/**
 * Base class for HTTP/1.1 request and response encoders.
 */
class StreamEncoderImpl: public StreamEncoder, public Stream/*,
 public StreamCallbackHelper*/{
public:
	// Http::StreamEncoder
	void encode100ContinueHeaders(const HeaderMap& headers) override;
	void encodeHeaders(const HeaderMap& headers, bool end_stream) override;
	void encodeData(Buffer::Instance& data, bool end_stream) override;
	void encodeTrailers(const HeaderMap& trailers) override;

	void resetStream(StreamResetReason reason) override;

protected:
	StreamEncoderImpl(ConnectionImpl& connection) : connection_(connection) {}

	static const std::string CRLF;
	static const std::string LAST_CHUNK;

	ConnectionImpl& connection_;

private:
	/**
	 * Called to encode an individual header.
	 * @param key supplies the header to encode.
	 * @param key_size supplies the byte size of the key.
	 * @param value supplies the value to encode.
	 * @param value_size supplies the byte size of the value.
	 */
	void encodeHeader(const char* key, uint32_t key_size, const char* value, uint32_t value_size);

	/**
	 * Called to finalize a stream encode.
	 */
	void endEncode();

	bool chunk_encoding_ {true};
	bool processing_100_continue_ {false};
};

/**
 * HTTP/1.1 response encoder.
 */
class ResponseStreamEncoderImpl: public StreamEncoderImpl {
public:
	ResponseStreamEncoderImpl(ConnectionImpl& connection) :
			StreamEncoderImpl(connection) {
	}

	bool startedResponse() {
		return started_response_;
	}

	// Http::StreamEncoder
	void encodeHeaders(const HeaderMap& headers, bool end_stream) override;

private:
	bool started_response_ {};
};

/**
 * HTTP/1.1 request encoder.
 */
class RequestStreamEncoderImpl: public StreamEncoderImpl {
public:
	RequestStreamEncoderImpl(ConnectionImpl& connection) :
			StreamEncoderImpl(connection) {
	}

	bool headRequest() {
		return head_request_;
	}

	// Http::StreamEncoder
	void encodeHeaders(const HeaderMap& headers, bool end_stream) override;

private:
	bool head_request_ {};
};

/**
 * Base class for HTTP/1.1 client and server connections.
 */
class ConnectionImpl: public virtual Connection {
public:
	/**
	 * @return ESocketSession& the backing network connection.
	 */
	sp<EHttpSession> connection() {
		return connection_;
	}

	/**
	 * Called when the active encoder has completed encoding the outbound half of the stream.
	 */
	virtual void onEncodeComplete() = 0;

	/**
	 * Called when resetStream() has been called on an active stream. In HTTP/1.1 the only
	 * valid operation after this point is for the connection to get blown away, but we will not
	 * fire any more callbacks in case some stack has to unwind.
	 */
	void onResetStreamBase(StreamResetReason reason);

	/**
	 * Flush all pending output from encoding.
	 */
	void flushOutput();

	void addCharToBuffer(char c);
	void addIntToBuffer(uint64_t i);

	Buffer::OwnedImpl& buffer() {
		return output_buffer_;
	}

	void copyToBuffer(const char* data, uint64_t length);

	// Http::Connection
	void dispatch(sp<EIoBuffer>& data) override;
	void goAway() override {} // Called during connection manager drain flow
	Protocol protocol() override {return protocol_;}

	virtual bool supports_http_10() {return false;}

protected:
	ConnectionImpl(sp<EHttpSession>& connection, http_parser_type type);

	bool resetStreamCalled() {return reset_stream_called_;}

	sp<EHttpSession> connection_;
	http_parser parser_;
	HeaderMapPtr deferred_end_stream_headers_;
	Http::Code error_code_ {Http::Code::BadRequest};

private:
	enum class HeaderParsingState {Field, Value, Done};

	/**
	 * Called in order to complete an in progress header decode.
	 */
	void completeLastHeader();

	/**
	 * Dispatch a memory span.
	 * @param slice supplies the start address.
	 * @len supplies the lenght of the span.
	 */
	size_t dispatchSlice(const char* slice, size_t len);

	/**
	 * Called when a request/response is beginning. A base routine happens first then a virtual
	 * dispatch is invoked.
	 */
	void onMessageBeginBase();
	virtual void onMessageBegin() = 0;

	/**
	 * Called when URL data is received.
	 * @param data supplies the start address.
	 * @param lenth supplies the length.
	 */
	virtual void onUrl(const char* data, size_t length) = 0;

	/**
	 * Called when header field data is received.
	 * @param data supplies the start address.
	 * @param length supplies the length.
	 */
	void onHeaderField(const char* data, size_t length);

	/**
	 * Called when header value data is received.
	 * @param data supplies the start address.
	 * @param length supplies the length.
	 */
	void onHeaderValue(const char* data, size_t length);

	/**
	 * Called when headers are complete. A base routine happens first then a virtual disaptch is
	 * invoked.
	 * @return 0 if no error, 1 if there should be no body.
	 */
	int onHeadersCompleteBase();
	virtual int onHeadersComplete(HeaderMapImplPtr&& headers) = 0;

	/**
	 * Called when body data is received.
	 * @param data supplies the start address.
	 * @param length supplies the length.
	 */
	virtual void onBody(const char* data, size_t length) = 0;

	/**
	 * Called when the request/response is complete.
	 */
	virtual void onMessageComplete() = 0;

	/**
	 * @see onResetStreamBase().
	 */
	virtual void onResetStream(StreamResetReason reason) = 0;

	/**
	 * Send a protocol error response to remote.
	 */
	virtual void sendProtocolError() = 0;

	static http_parser_settings settings_;
	static const ToLowerTable& toLowerTable();

	HeaderMapImplPtr current_header_map_;
	HeaderParsingState header_parsing_state_ {HeaderParsingState::Field};
	HeaderString current_header_field_;
	HeaderString current_header_value_;
	bool reset_stream_called_ {};

	Buffer::OwnedImpl output_buffer_;

	sp<EIoBuffer> reserved_iovec_;
	Protocol protocol_ {Protocol::Http11};
};

/**
 * Implementation of Http::ServerConnection for HTTP/1.1.
 */
class ServerConnectionImpl: public ServerConnection, public ConnectionImpl {
public:
	ServerConnectionImpl(sp<EHttpSession>& connection,
			ServerConnectionCallbacks& callbacks, Http1Settings settings);

	virtual bool supports_http_10() override {return codec_settings_.accept_http_10_;}

private:
	/**
	 * An active HTTP/1.1 request.
	 */
	struct ActiveRequest {
		ActiveRequest(ConnectionImpl& connection) :
				response_encoder_(connection) {
		}

		HeaderString request_url_;
		StreamDecoder* request_decoder_ { };
		ResponseStreamEncoderImpl response_encoder_;bool remote_complete_ { };
	};

	/**
	 * Manipulate the request's first line, parsing the url and converting to a relative path if
	 * neccessary. Compute Host / :authority headers based on 7230#5.7 and 7230#6
	 *
	 * @param is_connect true if the request has the CONNECT method
	 * @param headers the request's headers
	 * @throws CodecProtocolException on an invalid url in the request line
	 */
	void handlePath(HeaderMapImpl& headers, unsigned int method);

	// ConnectionImpl
	void onEncodeComplete() override;
	void onMessageBegin() override;
	void onUrl(const char* data, size_t length) override;
	int onHeadersComplete(HeaderMapImplPtr&& headers) override;
	void onBody(const char* data, size_t length) override;
	void onMessageComplete() override;
	void onResetStream(StreamResetReason reason) override;
	void sendProtocolError() override;

	ServerConnectionCallbacks& callbacks_;
	std::unique_ptr<ActiveRequest> active_request_;
	Http1Settings codec_settings_;
};

} // namespace http1
} // namespace http
} // namespace naf
} // namespace efc
