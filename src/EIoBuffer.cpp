/*
 * EIoBuffer.cpp
 *
 *  Created on: 2016-1-11
 *      Author: cxxjava@163.com
 */

#include "../inc/EIoBuffer.hh"

namespace efc {
namespace naf {

EIoBuffer::~EIoBuffer() {
	if (derived_) {
		delete buf_;
	}
}

EIoBuffer::EIoBuffer(int capacity) :
		derived_(true), autoExpand_(false), autoShrink_(false), recapacityAllowed_(
				true) {
	buf_ = EIOByteBuffer::allocate(capacity);
	minimumCapacity_ = capacity;
}

EIoBuffer::EIoBuffer(EIOByteBuffer* newbuf, int minimumCapacity) :
		derived_(true), autoExpand_(false), autoShrink_(false), recapacityAllowed_(
				false) {
	buf_ = newbuf;
	minimumCapacity_ = minimumCapacity;
}

EIoBuffer* EIoBuffer::allocate(int capacity) {
	return new EIoBuffer(capacity);
}

EIoBuffer* EIoBuffer::wrap(void* address, int capacity, int offset) {
	EIOByteBuffer* buf = EIOByteBuffer::wrap(address, capacity, offset);
	return new EIoBuffer(buf, capacity);
}

EIoBuffer* EIoBuffer::duplicate() {
	recapacityAllowed_ = false;
	return new EIoBuffer(buf_->duplicate(), buf_->capacity());
}

EIoBuffer* EIoBuffer::slice() {
	recapacityAllowed_ = false;
	return new EIoBuffer(buf_->slice(), buf_->capacity());
}

void EIoBuffer::free() {
	// Do nothing
}

EIOByteBuffer* EIoBuffer::buf() {
	return buf_;
}

boolean EIoBuffer::isDerived() {
	return derived_;
}

boolean EIoBuffer::isReadOnly() {
	return buf_->isReadOnly();
}

int EIoBuffer::remaining() {
	return buf_->limit() - buf_->position();
}

boolean EIoBuffer::hasRemaining() {
	return buf_->limit() > buf_->position();
}

int EIoBuffer::position() {
	return buf_->position();
}

EIoBuffer* EIoBuffer::position(int newPosition) {
	autoExpand(newPosition, 0);
	buf_->position(newPosition);
	return this;
}

int EIoBuffer::limit() {
	return buf_->limit();
}

EIoBuffer* EIoBuffer::limit(int newLimit) {
	autoExpand(newLimit, 0);
	buf_->limit(newLimit);
	return this;
}

EIoBuffer* EIoBuffer::mark() {
	buf_->mark();
	return this;
}

EIoBuffer* EIoBuffer::flip() {
	buf_->flip();
	return this;
}

EIoBuffer* EIoBuffer::reset() {
	buf_->reset();
	return this;
}

EIoBuffer* EIoBuffer::clear() {
	buf_->clear();
	return this;
}

int EIoBuffer::minimumCapacity() {
	return minimumCapacity_;
}

EIoBuffer* EIoBuffer::minimumCapacity(int minimumCapacity) {
	if (minimumCapacity < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__);
	}
	minimumCapacity_ = minimumCapacity;
	return this;
}

int EIoBuffer::compareTo(EIoBuffer* that) {
	int n = this->position() + EMath::min(this->remaining(), that->remaining());
	for (int i = this->position(), j = that->position(); i < n; i++, j++) {
		byte v1 = buf_->get(i);
		byte v2 = that->buf()->get(j);
		if (v1 == v2) {
			continue;
		}
		if (v1 < v2) {
			return -1;
		}

		return +1;
	}
	return this->remaining() - that->remaining();
}

EIoBuffer* EIoBuffer::autoExpand(int expectedRemaining) {
	if (isAutoExpand()) {
		expand(expectedRemaining, true);
	}
	return this;
}

EIoBuffer* EIoBuffer::autoExpand(int pos, int expectedRemaining) {
	if (isAutoExpand()) {
		expand(pos, expectedRemaining, true);
	}
	return this;
}

int EIoBuffer::capacity() {
	return buf_->capacity();
}

EIoBuffer* EIoBuffer::capacity(int newCapacity) {
	if (!recapacityAllowed_) {
		throw EIllegalStateException(__FILE__, __LINE__, "Derived buffers and their parent can't be expanded.");
	}

	// Allocate a new buffer and transfer all settings to it.
	if (newCapacity > capacity()) {
		// Expand:
		//// Save the state.
		int pos = buf_->position();
		int limit = buf_->limit();
		int mark = buf_->markValue();
		//ByteOrder bo = order();

		//// Reallocate.
		EIOByteBuffer* oldBuf = buf();
		EIOByteBuffer* newBuf = EIOByteBuffer::allocate(newCapacity);
		oldBuf->clear();
		newBuf->put(oldBuf);
		buf_ = newBuf;
		delete oldBuf; //!

		//// Restore the state.
		buf_->limit(limit);
		if (mark >= 0) {
			buf_->position(mark);
			buf_->mark();
		}
		buf_->position(pos);
		//buf().order(bo);
	}

	return this;
}

boolean EIoBuffer::isAutoExpand() {
	return autoExpand_ && recapacityAllowed_;
}

EIoBuffer* EIoBuffer::setAutoExpand(boolean autoExpand) {
	if (!recapacityAllowed_) {
		throw EIllegalStateException(__FILE__, __LINE__, "Derived buffers and their parent can't be expanded.");
	}
	autoExpand_ = autoExpand;
	return this;
}

boolean EIoBuffer::isAutoShrink() {
	return autoShrink_ && recapacityAllowed_;
}

EIoBuffer* EIoBuffer::setAutoShrink(boolean autoShrink) {
	if (!recapacityAllowed_) {
		throw EIllegalStateException(__FILE__, __LINE__, "Derived buffers and their parent can't be shrinked.");
	}
	autoShrink_ = autoShrink;
	return this;
}

EIoBuffer* EIoBuffer::expand(int expectedRemaining) {
	return expand(position(), expectedRemaining, false);
}

EIoBuffer* EIoBuffer::expand(int pos, int expectedRemaining) {
	return expand(pos, expectedRemaining, false);
}

EIoBuffer* EIoBuffer::expand(int pos, int expectedRemaining, boolean autoExpand) {
	if (!recapacityAllowed_) {
		throw EIllegalStateException(__FILE__, __LINE__, "Derived buffers and their parent can't be expanded.");
	}

	int end = pos + expectedRemaining;
	int newCapacity;

	if (autoExpand) {
		newCapacity = EIoBuffer::normalizeCapacity(end);
	} else {
		newCapacity = end;
	}
	if (newCapacity > capacity()) {
		// The buffer needs expansion.
		capacity(newCapacity);
	}

	if (end > limit()) {
		// We call limit() directly to prevent StackOverflowError
		buf_->limit(end);
	}
	return this;
}

EIoBuffer* EIoBuffer::shrink() {
	if (!recapacityAllowed_) {
		throw EIllegalStateException(__FILE__, __LINE__, "Derived buffers and their parent can't be expanded.");
	}

	int position = buf_->position();
	int capacity = buf_->capacity();
	int limit = buf_->limit();

	if (capacity == limit) {
		return this;
	}

	unsigned newCapacity = capacity;
	int minCapacity = EMath::max(minimumCapacity_, limit);

	for (;;) {
		if (newCapacity >> 1 < minCapacity) {
			break;
		}

		newCapacity >>= 1;

		if (minCapacity == 0) {
			break;
		}
	}

	newCapacity = EMath::max(minCapacity, newCapacity);

	if (newCapacity == capacity) {
		return this;
	}

	// Shrink and compact:
	//// Save the state.
	//ByteOrder bo = order();

	//// Reallocate.
	EIOByteBuffer* oldBuf = buf_;
	EIOByteBuffer* newBuf = EIOByteBuffer::allocate(newCapacity);
	oldBuf->position(0);
	oldBuf->limit(limit);
	newBuf->put(oldBuf);
	buf_ = newBuf;
	delete oldBuf;

	//// Restore the state.
	buf_->position(position);
	buf_->limit(limit);
	//buf().order(bo);

	return this;
}

int EIoBuffer::markValue() {
	return buf_->markValue();
}

EIoBuffer* EIoBuffer::sweep() {
	clear();
	return fillAndReset(0, remaining());
}

EIoBuffer* EIoBuffer::sweep(byte value) {
	clear();
	return fillAndReset(value, remaining());
}

EIoBuffer* EIoBuffer::rewind() {
	buf_->rewind();
	return this;
}

EIoBuffer* EIoBuffer::compact() {
	buf_->compact();
	return this;
}

EIoBuffer* EIoBuffer::skip(int size) {
	autoExpand(size);
	return position(position() + size);
}

EIoBuffer* EIoBuffer::fill(byte value, int size) {
	autoExpand(size);
	eso_memset((char*)buf_->current(), value, size);
	position(position() + size);
	return this;
}

int EIoBuffer::indexOf(byte b) {
	int beginPos = position();
	int lim = limit();

	for (int i = beginPos; i < lim; i++) {
		if (get(i) == b) {
			return i;
		}
	}

	return -1;
}

EIoBuffer* EIoBuffer::fillAndReset(byte value, int size) {
	autoExpand(size);
	int pos = position();
	ON_FINALLY_NOTHROW(
		position(pos);
	) {
		fill(value, size);
    }}
	return this;
}

int EIoBuffer::normalizeCapacity(int requestedCapacity) {
	if (requestedCapacity < 0) {
		return EInteger::MAX_VALUE;
	}

	int newCapacity = EInteger::highestOneBit(requestedCapacity);
	newCapacity <<= (newCapacity < requestedCapacity ? 1 : 0);

	return newCapacity < 0 ? EInteger::MAX_VALUE : newCapacity;
}


EStringBase EIoBuffer::toString() {
	EStringBase buf("IoBuffer");
	buf.append("[pos=");
	buf.append(position());
	buf.append(" lim=");
	buf.append(limit());
	buf.append(" cap=");
	buf.append(capacity());
	buf.append(": ");
	//buf.append(getHexDump(16));
	int n = buf_->remaining();
	char* hexs = eso_new_bytes2hexstr((es_uint8_t*)(buf_->current()), ES_MIN(16, n));
	buf.append(hexs);
	delete hexs;
	buf.append(']');
	return buf;
}

void* EIoBuffer::address() {
	return buf_->address();
}

void* EIoBuffer::current() {
	return buf_->current();
}

EString EIoBuffer::getHexdump(int lengthLimit) {
	int n = buf_->remaining();
	char* hexs = eso_new_bytes2hexstr((es_uint8_t*)(buf_->current()), ES_MIN(lengthLimit, n));
	EString s(hexs);
	delete hexs;
	return s;
}

byte EIoBuffer::get() {
	return buf_->get();
}

EIoBuffer* EIoBuffer::put(byte b) {
	autoExpand(1);
	buf_->put(b);
	return this;
}

byte EIoBuffer::get(int index) {
	return buf_->get(index);
}

EIoBuffer* EIoBuffer::put(int index, byte b) {
	autoExpand(index, 1);
	buf_->put(index, b);
	return this;
}

EIoBuffer* EIoBuffer::get(void* dst, int size, int length) {
	ES_ASSERT(dst);
	buf_->get(dst, size, length);
	return this;
}

EIoBuffer* EIoBuffer::get(EA<byte>* dst) {
	int size = dst->length() * sizeof(byte);
	buf_->get(dst->address(), size, size);
	return this;
}

EIoBuffer* EIoBuffer::getSlice(int index, int length) {
	if (length < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("length: %d", length).c_str());
	}

	int pos = position();
	int lim = limit();

	if (index > lim) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("index: %d", index).c_str());
	}

	int endIndex = index + length;

	if (endIndex > lim) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("index + length (%d) is greater than limit (%d).",
				endIndex, lim).c_str());
	}

	clear();
	limit(endIndex);
	position(index);

	EIoBuffer* newBuf = slice();
	limit(lim);
	position(pos);

	return newBuf;
}

EIoBuffer* EIoBuffer::getSlice(int length) {
	if (length < 0) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("length: %d", length).c_str());
	}
	int pos = position();
	int lim = limit();
	int nextPos = pos + length;
	if (lim < nextPos) {
		throw EIllegalArgumentException(__FILE__, __LINE__, EString::formatOf("position + length (%d) is greater than limit (%d).",
				nextPos, lim).c_str());
	}

	limit(pos + length);
	EIoBuffer* newBuf = slice();
	position(nextPos);
	limit(lim);
	return newBuf;
}

EIoBuffer* EIoBuffer::put(EIOByteBuffer* src) {
	autoExpand(src->remaining());
	buf_->put(src);
	return this;
}

EIoBuffer* EIoBuffer::put(EIoBuffer* src) {
	return put(src->buf());
}

EIoBuffer* EIoBuffer::put(const void* src, int length) {
	ES_ASSERT(src);
	autoExpand(length);
	buf_->put(src, length);
	return this;
}

EIoBuffer* EIoBuffer::put(EA<byte>* src) {
	int size = src->length() * sizeof(byte);
	autoExpand(size);
	buf_->put(src->address(), size);
	return this;
}

char EIoBuffer::getChar() {
	return (char) (buf_->get());
}

EIoBuffer* EIoBuffer::putChar(char value) {
	autoExpand(1);
	buf_->put(value);
	return this;
}

char EIoBuffer::getChar(int index) {
	return (char) (buf_->get(index));
}

EIoBuffer* EIoBuffer::putChar(int index, char value) {
	autoExpand(index, 1);
	buf_->put(index, value);
	return this;
}

short EIoBuffer::getShort() {
	short n = EStream::readShort(buf_->current());
	buf_->skip(2);
	return n;
}

EIoBuffer* EIoBuffer::putShort(short value) {
	autoExpand(2);
	EStream::writeShort(buf_->current(), value);
	buf_->skip(2);
	return this;
}

short EIoBuffer::getShort(int index) {
	byte s[2];
	s[0] = buf_->get(index);
	s[1] = buf_->get(index + 1);
	return EStream::readShort(s);
}

EIoBuffer* EIoBuffer::putShort(int index, short value) {
	autoExpand(index, 2);
	EStream::writeShort(((char*)buf_->current()) + index, value);
	return this;
}

int EIoBuffer::getInt() {
	int n = EStream::readInt(buf_->current());
	buf_->skip(4);
	return n;
}

EIoBuffer* EIoBuffer::putInt(int value) {
	autoExpand(4);
	EStream::writeInt(buf_->current(), value);
	buf_->skip(4);
	return this;
}

int EIoBuffer::getInt(int index) {
	byte s[4];
	s[0] = buf_->get(index);
	s[1] = buf_->get(index + 1);
	s[2] = buf_->get(index + 2);
	s[3] = buf_->get(index + 3);
	return EStream::readInt(s);
}

EIoBuffer* EIoBuffer::putInt(int index, int value) {
	autoExpand(index, 4);
	EStream::writeInt(((char*)buf_->current()) + index, value);
	return this;
}

llong EIoBuffer::getLLong() {
	llong n = EStream::readLLong(buf_->current());
	buf_->skip(8);
	return n;
}

EIoBuffer* EIoBuffer::putLLong(llong value) {
	autoExpand(8);
	EStream::writeLLong(buf_->current(), value);
	buf_->skip(8);
	return this;
}

llong EIoBuffer::getLLong(int index) {
	byte s[8];
	s[0] = buf_->get(index);
	s[1] = buf_->get(index + 1);
	s[2] = buf_->get(index + 2);
	s[3] = buf_->get(index + 3);
	s[4] = buf_->get(index + 4);
	s[5] = buf_->get(index + 5);
	s[6] = buf_->get(index + 6);
	s[7] = buf_->get(index + 7);
	return EStream::readLLong(s);
}

EIoBuffer* EIoBuffer::putLLong(int index, llong value) {
	autoExpand(index, 8);
	EStream::writeLLong(((char*)buf_->current()) + index, value);
	return this;
}

float EIoBuffer::getFloat() {
	int n = getInt();
	return eso_intBits2float(n);
}

EIoBuffer* EIoBuffer::putFloat(float value) {
	autoExpand(4);
	int n = eso_float2intBits(value);
	return putInt(n);
}

float EIoBuffer::getFloat(int index) {
	int n = getInt(index);
	return eso_intBits2float(n);
}

EIoBuffer* EIoBuffer::putFloat(int index, float value) {
	autoExpand(index, 4);
	int n = eso_float2intBits(value);
	return putInt(index, n);
}

double EIoBuffer::getDouble() {
	llong n = getLLong();
	return eso_llongBits2double(n);
}

EIoBuffer* EIoBuffer::putDouble(double value) {
	autoExpand(8);
	llong n = eso_double2llongBits(value);
	return putLLong(n);
}

double EIoBuffer::getDouble(int index) {
	llong n = getLLong(index);
	return eso_llongBits2double(n);
}

EIoBuffer* EIoBuffer::putDouble(int index, double value) {
	autoExpand(index, 8);
	llong n = eso_double2llongBits(value);
	return putLLong(index, n);
}

EString EIoBuffer::getString() {
	if (!hasRemaining()) {
		return "";
	}

	const char* str = (char*)(buf_->current());
	int length = eso_strlen(str);
	length = ES_MIN(length, buf_->remaining());
	buf_->position(position() + length);
	return EString(str, 0, length);
}

EString EIoBuffer::getString(int fieldSize) {
	if (fieldSize == 0) {
		return "";
	}

	if (!hasRemaining()) {
		return "";
	}

	const char* str = (char*)(buf_->current());
	int length = eso_strlen(str);
	length = ES_MIN(ES_MIN(length, buf_->remaining()), fieldSize);
	buf_->position(position() + length);
	return EString(str, 0, length);
}

EIoBuffer* EIoBuffer::putString(const char* val) {
	if (!val) {
		return this;
	}

	int length = eso_strlen(val);
	length += 1; //+'\0'
	autoExpand(length);
	buf_->put(val, length);
	return this;
}

EIoBuffer* EIoBuffer::putString(const char* val, int fieldSize) {
	if (!val || fieldSize <= 0) {
		return this;
	}

	int length = eso_strlen(val);
	length = ES_MIN(length, fieldSize);
	autoExpand(length + 1); //+'\0'
	buf_->put(val, length);
	buf_->put((byte)'\0');
	return this;
}

EInputStream* EIoBuffer::asInputStream() {
	class InputStream: public EInputStream {
	private:
		EIoBuffer* owner;
	public:
		InputStream(EIoBuffer* o): owner(o) {
		}
		virtual long available() {
            return owner->remaining();
        }
		virtual int read() {
			if (owner->hasRemaining()) {
				return owner->get() & 0xff;
			}
			return -1;
		}
		virtual int read(void *b, int len) {
			int remaining = owner->remaining();
			if (remaining > 0) {
				int readBytes = ES_MIN(remaining, len);
				owner->get(b, len, readBytes);
				return readBytes;
			}

			return -1;
		}
		virtual long skip(long n) {
			int bytes;
			if (n > EInteger::MAX_VALUE) {
				bytes = owner->remaining();
			} else {
				bytes = ES_MIN(owner->remaining(), (int) n);
			}
			owner->skip(bytes);
			return bytes;
		}
	};

	return new InputStream(this);
}

EOutputStream* EIoBuffer::asOutputStream() {
	class OutputStream: public EOutputStream {
	private:
		EIoBuffer* owner;
	public:
		OutputStream(EIoBuffer* o): owner(o) {
		}
		virtual void write(const void *b, int len) {
			owner->put((void*)b, len);
		}
		virtual void write(int b) {
			owner->put((byte) b);
		}
	};
	return new OutputStream(this);
}

} /* namespace naf */
} /* namespace efc */
