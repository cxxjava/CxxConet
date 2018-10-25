#include "./buffer_impl.h"
#include "./macros.h"

#include "Efc.hh"

#include <cstdint>
#include <string>

namespace efc {
namespace naf {
namespace Http {
namespace Buffer {

void OwnedImpl::add(sp<EIoBuffer>& buf) {
	buf->flip();
	buffer_.add(buf);
}

void OwnedImpl::add(const void* data, uint64_t size) {
	sp<EIoBuffer> buf(EIoBuffer::allocate(size));
	buf->put(data, size);
	add(buf);
}

void OwnedImpl::add(const std::string& data) {
	add(data.c_str(), data.length());
}

void OwnedImpl::clear() {
	buffer_.clear();
}

uint64_t OwnedImpl::length() {
	uint64_t len = 0;
	for (int i = 0; i < buffer_.size(); i++) {
		len += buffer_.getAt(i)->remaining();
	}
	return len;
}

void OwnedImpl::move(Instance& rhs) {
	LinkedBuffer& that_buf = dynamic_cast<OwnedImpl&>(rhs).buffer_;
	sp<EIoBuffer> iobuf;
	while ((iobuf = that_buf.poll()) != null) {
		buffer_.add(iobuf);
	}
}

void OwnedImpl::move(Instance& rhs, uint64_t length) {
	LinkedBuffer& that_buf = dynamic_cast<OwnedImpl&>(rhs).buffer_;
	sp<EIoBuffer> iobuf;
	while (length > 0 && (iobuf = that_buf.peek()) != null) {
		if (length >= iobuf->remaining()) {
			buffer_.add(iobuf);
			that_buf.removeFirst();
			length -= iobuf->remaining();
		} else {
			add(iobuf->current(), length);
			iobuf->position(length);
		}
	}
}

OwnedImpl::OwnedImpl() :
		buffer_() {
}

OwnedImpl::OwnedImpl(const std::string& data) :
		OwnedImpl() {
	add(data);
}

OwnedImpl::OwnedImpl(const void* data, uint64_t size) :
		OwnedImpl() {
	add(data, size);
}

} // namespace buffer
} // namespace http
} // namespace naf
} // namespace efc
