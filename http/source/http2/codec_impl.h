#pragma once

//#include "../../../inc/EHttpSession.hh"

#include <cstdint>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "../../include/codec.h"

#include "../buffer_impl.h"
#include "../linked_object.h"
#include "../header_map_impl.h"

#include "nghttp2/nghttp2.h"

namespace efc {
namespace naf {

class EHttpSession;

namespace Http {
namespace Http2 {

const std::string ALPN_STRING = "h2";

// This is not the full client magic, but it's the smallest size that should be able to
// differentiate between HTTP/1 and HTTP/2.
const std::string CLIENT_MAGIC_PREFIX = "PRI * HTTP/2";

class Utility {
public:
	/**
	 * Get the response status from the response headers.
	 * @param headers supplies the headers to get the status from.
	 * @return uint64_t the response code or throws an exception if the headers are invalid.
	 */
	static uint64_t getResponseStatus(const HeaderMap& headers);

	/**
	 * Deal with https://tools.ietf.org/html/rfc7540#section-8.1.2.5
	 * @param key supplies the incoming header key.
	 * @param value supplies the incoming header value.
	 * @param cookies supplies the header string to fill if this is a cookie header that needs to be
	 *                rebuilt.
	 */
	static bool reconstituteCrumbledCookies(const HeaderString& key,
			const HeaderString& value, HeaderString& cookies);
};

/**
 * Base class for HTTP/2 client and server codecs.
 */
class ConnectionImpl: public virtual Connection {
public:
	ConnectionImpl(sp<EHttpSession>& connection, /*Stats::Scope& stats,*/
	const Http2Settings& http2_settings) :
		connection_(connection), per_stream_buffer_limit_(
				http2_settings.initial_stream_window_size_), dispatching_(
				false), raised_goaway_(false), pending_deferred_reset_(
				false) {
	}

	~ConnectionImpl();

	// Http::Connection
	void dispatch(sp<EIoBuffer>& data) override;
	void goAway() override;
	Protocol protocol() override {
		return Protocol::Http2;
	}

protected:
	/**
	 * Wrapper for static nghttp2 callback dispatchers.
	 */
	class Http2Callbacks {
	public:
		Http2Callbacks();
		~Http2Callbacks();

		const nghttp2_session_callbacks* callbacks() {return callbacks_;}

	private:
		nghttp2_session_callbacks* callbacks_;
	};

	/**
	 * Wrapper for static nghttp2 session options.
	 */
	class Http2Options {
	public:
		Http2Options();
		~Http2Options();

		const nghttp2_option* options() {return options_;}

	private:
		nghttp2_option* options_;
	};

	/**
	 * Base class for client and server side streams.
	 */
	struct StreamImpl : public StreamEncoder,
	public Stream,
	public LinkedObject<StreamImpl> {
		StreamImpl(ConnectionImpl& parent, uint32_t buffer_limit);

		StreamImpl* base() {return this;}
		ssize_t onDataSourceRead(uint64_t length, uint32_t* data_flags);
		int onDataSourceSend(const uint8_t* framehd, size_t length);
		void resetStreamWorker(StreamResetReason reason);
		static void buildHeaders(std::vector<nghttp2_nv>& final_headers, const HeaderMap& headers);
		void saveHeader(HeaderString&& name, HeaderString&& value);
		virtual void submitHeaders(const std::vector<nghttp2_nv>& final_headers,
				nghttp2_data_provider* provider) = 0;
		void submitTrailers(const HeaderMap& trailers);

		// Http::StreamEncoder
		void encode100ContinueHeaders(const HeaderMap& headers) override;
		void encodeHeaders(const HeaderMap& headers, bool end_stream) override;
		void encodeData(Buffer::Instance& data, bool end_stream) override;
		void encodeTrailers(const HeaderMap& trailers) override;

		// Http::Stream
		void resetStream(StreamResetReason reason) override;

		// Max header size of 63K. This is arbitrary but makes it easier to test since nghttp2 doesn't
		// appear to transmit headers greater than approximtely 64K (NGHTTP2_MAX_HEADERSLEN) for reasons
		// I don't fully understand.
		static const uint64_t MAX_HEADER_SIZE = 63 * 1024;

		bool buffers_overrun() const {return read_disable_count_ > 0;}

		ConnectionImpl& parent_;
		HeaderMapImplPtr headers_;
		StreamDecoder* decoder_ {};
		int32_t stream_id_ {-1};
		uint32_t unconsumed_bytes_ {0};
		uint32_t read_disable_count_ {0};
		Buffer::OwnedImpl pending_recv_data_ {};
		Buffer::OwnedImpl pending_send_data_ {};

		HeaderMapPtr pending_trailers_;
		sp<StreamResetReason> deferred_reset_; //cxxjava
		HeaderString cookies_;
		bool local_end_stream_sent_ : 1;
		bool remote_end_stream_ : 1;
		bool data_deferred_ : 1;
		bool waiting_for_non_informational_headers_ : 1;

		bool local_end_stream_ {}; //@see: codec_helper.h
	};

	typedef std::unique_ptr<StreamImpl> StreamImplPtr;

	/**
	 * Client side stream (request).
	 */
	struct ClientStreamImpl : public StreamImpl {
		using StreamImpl::StreamImpl;

		// StreamImpl
		void submitHeaders(const std::vector<nghttp2_nv>& final_headers,
				nghttp2_data_provider* provider) override;
	};

	/**
	 * Server side stream (response).
	 */
	struct ServerStreamImpl : public StreamImpl {
		using StreamImpl::StreamImpl;

		// StreamImpl
		void submitHeaders(const std::vector<nghttp2_nv>& final_headers,
				nghttp2_data_provider* provider) override;
	};

	ConnectionImpl* base() {return this;}
	StreamImpl* getStream(int32_t stream_id);
	int saveHeader(const nghttp2_frame* frame, HeaderString&& name, HeaderString&& value);
	void sendPendingFrames();
	void sendSettings(const Http2Settings& http2_settings, bool disable_push);

	static Http2Callbacks http2_callbacks_;
	static Http2Options http2_options_;

	std::list<StreamImplPtr> active_streams_;
	nghttp2_session* session_ {};
	sp<EHttpSession> connection_;
	uint32_t per_stream_buffer_limit_;

private:
	virtual ConnectionCallbacks& callbacks() = 0;
	virtual int onBeginHeaders(const nghttp2_frame* frame) = 0;
	int onData(int32_t stream_id, const uint8_t* data, size_t len);
	int onFrameReceived(const nghttp2_frame* frame);
	int onFrameSend(const nghttp2_frame* frame);
	virtual int onHeader(const nghttp2_frame* frame, HeaderString&& name, HeaderString&& value) = 0;
	int onInvalidFrame(int error_code);
	ssize_t onSend(const uint8_t* data, size_t length);
	int onStreamClose(int32_t stream_id, uint32_t error_code);

	bool dispatching_ : 1;
	bool raised_goaway_ : 1;
	bool pending_deferred_reset_ : 1;
};

/**
 * HTTP/2 server connection codec.
 */
class ServerConnectionImpl: public ServerConnection, public ConnectionImpl {
public:
	ServerConnectionImpl(sp<EHttpSession>& connection,
			ServerConnectionCallbacks& callbacks, const Http2Settings& http2_settings);

private:
	// ConnectionImpl
	ConnectionCallbacks& callbacks() override {return callbacks_;}
	int onBeginHeaders(const nghttp2_frame* frame) override;
	int onHeader(const nghttp2_frame* frame, HeaderString&& name, HeaderString&& value) override;

	ServerConnectionCallbacks& callbacks_;
};

} // namespace Http2
} // namespace Http
} // namespace naf
} // namespace efc
