#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

#include "../../inc/EIoBuffer.hh"

namespace efc {
namespace naf {
namespace Http {
namespace Buffer {

typedef ELinkedList<sp<EIoBuffer> > LinkedBuffer;

/**
 * A basic buffer abstraction.
 */
class Instance {
public:
	virtual ~Instance() {
	}

	virtual void add(sp<EIoBuffer>& buf) = 0;

	/**
	 * Copy data into the buffer.
	 * @param data supplies the data address.
	 * @param size supplies the data size.
	 */
	virtual void add(const void* data, uint64_t size) = 0;

	/**
	 * Copy a string into the buffer.
	 * @param data supplies the string to copy.
	 */
	virtual void add(const std::string& data) = 0;

	/**
	 * Drain data from the buffer.
	 * @param size supplies the length of data to drain.
	 */
	virtual void clear() = 0;

	/**
	 * @return uint64_t the total length of the buffer (not necessarily contiguous in memory).
	 */
	virtual uint64_t length() = 0;

	/**
	 * Move a buffer into this buffer. As little copying is done as possible.
	 * @param rhs supplies the buffer to move.
	 */
	virtual void move(Instance& rhs) = 0;

	/**
	 * Move a portion of a buffer into this buffer. As little copying is done as possible.
	 * @param rhs supplies the buffer to move.
	 * @param length supplies the amount of data to move.
	 */
	virtual void move(Instance& rhs, uint64_t length) = 0;
};

} // namespace buffer
} // namespace http
} // namespace naf
} // namespace efc
