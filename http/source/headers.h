#pragma once

#include <string>

#include "../include/header_map.h"
#include "./const_singleton.h"

namespace efc {
namespace naf {
namespace Http {

/**
 * Constant HTTP headers and values. All lower case.
 */
class HeaderValues {
public:
	const LowerCaseString Accept{"accept"};
	const LowerCaseString AcceptEncoding{"accept-encoding"};
	const LowerCaseString AccessControlRequestHeaders{"access-control-request-headers"};
	const LowerCaseString AccessControlRequestMethod{"access-control-request-method"};
	const LowerCaseString AccessControlAllowOrigin{"access-control-allow-origin"};
	const LowerCaseString AccessControlAllowHeaders{"access-control-allow-headers"};
	const LowerCaseString AccessControlAllowMethods{"access-control-allow-methods"};
	const LowerCaseString AccessControlExposeHeaders{"access-control-expose-headers"};
	const LowerCaseString AccessControlMaxAge{"access-control-max-age"};
	const LowerCaseString AccessControlAllowCredentials{"access-control-allow-credentials"};
	const LowerCaseString Authorization{"authorization"};
	const LowerCaseString CacheControl{"cache-control"};
	const LowerCaseString ClientTraceId{"x-client-trace-id"};
	const LowerCaseString Connection{"connection"};
	const LowerCaseString ContentEncoding{"content-encoding"};
	const LowerCaseString ContentLength{"content-length"};
	const LowerCaseString ContentType{"content-type"};
	const LowerCaseString Cookie{"cookie"};
	const LowerCaseString Date{"date"};
	const LowerCaseString Etag{"etag"};
	const LowerCaseString Expect{"expect"};
	const LowerCaseString ForwardedClientCert{"x-forwarded-client-cert"};
	const LowerCaseString ForwardedFor{"x-forwarded-for"};
	const LowerCaseString ForwardedProto{"x-forwarded-proto"};
	const LowerCaseString GrpcMessage{"grpc-message"};
	const LowerCaseString GrpcStatus{"grpc-status"};
	const LowerCaseString GrpcAcceptEncoding{"grpc-accept-encoding"};
	const LowerCaseString Host{":authority"};
	const LowerCaseString HostLegacy{"host"};
	const LowerCaseString KeepAlive{"keep-alive"};
	const LowerCaseString LastModified{"last-modified"};
	const LowerCaseString Location{"location"};
	const LowerCaseString Method{":method"};
	const LowerCaseString Origin{"origin"};
	const LowerCaseString OtSpanContext{"x-ot-span-context"};
	const LowerCaseString Path{":path"};
	const LowerCaseString ProxyConnection{"proxy-connection"};
	const LowerCaseString Referer{"referer"};
	const LowerCaseString RequestId{"x-request-id"};
	const LowerCaseString Scheme{":scheme"};
	const LowerCaseString Server{"server"};
	const LowerCaseString SetCookie{"set-cookie"};
	const LowerCaseString Status{":status"};
	const LowerCaseString TransferEncoding{"transfer-encoding"};
	const LowerCaseString TE{"te"};
	const LowerCaseString Upgrade{"upgrade"};
	const LowerCaseString UserAgent{"user-agent"};
	const LowerCaseString Vary{"vary"};
	const LowerCaseString XB3TraceId{"x-b3-traceid"};
	const LowerCaseString XB3SpanId{"x-b3-spanid"};
	const LowerCaseString XB3ParentSpanId{"x-b3-parentspanid"};
	const LowerCaseString XB3Sampled{"x-b3-sampled"};
	const LowerCaseString XB3Flags{"x-b3-flags"};
	const LowerCaseString XContentTypeOptions{"x-content-type-options"};
	const LowerCaseString XSquashDebug{"x-squash-debug"};

	struct {
		const std::string Close{"close"};
		const std::string KeepAlive{"keep-alive"};
		const std::string Upgrade{"upgrade"};
	} ConnectionValues;

	struct {
		const std::string WebSocket{"websocket"};
	} UpgradeValues;

	struct {
		const std::string NoCacheMaxAge0{"no-cache, max-age=0"};
		const std::string NoTransform{"no-transform"};
	} CacheControlValues;

	struct {
		const std::string Text{"text/plain"};
		const std::string TextUtf8{"text/plain; charset=UTF-8"}; // TODO(jmarantz): fold this into Text
		const std::string Html{"text/html; charset=UTF-8"};
		const std::string Grpc{"application/grpc"};
		const std::string GrpcWeb{"application/grpc-web"};
		const std::string GrpcWebProto{"application/grpc-web+proto"};
		const std::string GrpcWebText{"application/grpc-web-text"};
		const std::string GrpcWebTextProto{"application/grpc-web-text+proto"};
		const std::string Json{"application/json"};
	} ContentTypeValues;

	struct {
		const std::string _100Continue{"100-continue"};
	} ExpectValues;

	struct {
		const std::string Get{"GET"};
		const std::string Head{"HEAD"};
		const std::string Post{"POST"};
		const std::string Options{"OPTIONS"};
	} MethodValues;

	struct {
		const std::string Http{"http"};
		const std::string Https{"https"};
	} SchemeValues;

	struct {
		const std::string Chunked{"chunked"};
		const std::string Deflate{"deflate"};
		const std::string Gzip{"gzip"};
	} TransferEncodingValues;

	struct {
		const std::string Default{"identity,deflate,gzip"};
	} GrpcAcceptEncodingValues;

	struct {
		const std::string Trailers{"trailers"};
	} TEValues;

	struct {
		const std::string Nosniff{"nosniff"};
	} XContentTypeOptionValues;

	struct {
		const std::string True{"true"};
	} CORSValues;

	struct {
		const std::string Http10String{"HTTP/1.0"};
		const std::string Http11String{"HTTP/1.1"};
		const std::string Http2String{"HTTP/2"};
	} ProtocolStrings;

	struct {
		const std::string Gzip{"gzip"};
		const std::string Identity{"identity"};
		const std::string Wildcard{"*"};
	} AcceptEncodingValues;

	struct {
		const std::string Gzip{"gzip"};
	} ContentEncodingValues;

	struct {
		const std::string AcceptEncoding{"Accept-Encoding"};
		const std::string Wildcard{"*"};
	} VaryValues;
};

typedef ConstSingleton<HeaderValues> Headers;

} // namespace Http
} // namespace naf
} // namespace efc
