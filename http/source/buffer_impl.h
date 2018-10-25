#pragma once

#include "Efc.hh"

#include <cstdint>
#include <string>

#include "../include/buffer.h"

namespace efc {
namespace naf {
namespace Http {
namespace Buffer {

/**
 * Wraps an allocated and owned evbuffer.
 *
 * Note that due to the internals of move() accessing buffer(), OwnedImpl is not
 * compatible with non-LibEventInstance buffers.
 */
class OwnedImpl: public Instance {
public:
	OwnedImpl();
	OwnedImpl(const std::string& data);
	OwnedImpl(const void* data, uint64_t size);

	void add(sp<EIoBuffer>& buf) override;
	void add(const void* data, uint64_t size) override;
	void add(const std::string& data) override;
	void clear() override;
	uint64_t length() override;
	void move(Instance& rhs) override;
	void move(Instance& rhs, uint64_t length) override;

	LinkedBuffer& buffer() {return buffer_;}

private:
	LinkedBuffer buffer_;

	uint32_t bufferLimit_ {0};
};

} // namespace buffer
} // namespace http
} // namespace naf
} // namespace efc
