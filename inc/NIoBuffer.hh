/*
 * NIoBuffer.hh
 *
 *  Created on: 2016-1-11
 *      Author: cxxjava@163.com
 */

#ifndef NIOBUFFER_HH_
#define NIOBUFFER_HH_

#include "Efc.hh"

namespace efc {
namespace naf {

/**
 * A byte buffer used by eanf applications.
 * <p>
 * This is a replacement for {@link ByteBuffer}. Please refer to
 * {@link ByteBuffer} documentation for preliminary usage.
 *
 * <h2>Allocation</h2>
 * <p>
 *   You can allocate a new heap buffer.
 *
 *   <pre>
 *     IoBuffer buf = IoBuffer.allocate(1024, false);
 *   </pre>
 *
 *   You can also allocate a new direct buffer:
 *
 *   <pre>
 *     IoBuffer buf = IoBuffer.allocate(1024, true);
 *   </pre>
 *
 *   or you can set the default buffer type.
 *
 *   <pre>
 *     // Allocate heap buffer by default.
 *     IoBuffer.setUseDirectBuffer(false);
 *
 *     // A new heap buffer is returned.
 *     IoBuffer buf = IoBuffer.allocate(1024);
 *   </pre>
 *
 * <h2>Wrapping existing NIO buffers and arrays</h2>
 * <p>
 *   This class provides a few <tt>wrap(...)</tt> methods that wraps any NIO
 *   buffers and byte arrays.
 *
 * <h2>AutoExpand</h2>
 * <p>
 *   Writing variable-length data using NIO <tt>ByteBuffers</tt> is not really
 *   easy, and it is because its size is fixed at allocation. {@link IoBuffer} introduces
 *   the <tt>autoExpand</tt> property. If <tt>autoExpand</tt> property is set to true,
 *   you never get a {@link BufferOverflowException} or
 *   an {@link IndexOutOfBoundsException} (except when index is negative). It
 *   automatically expands its capacity. For instance:
 *
 *   <pre>
 *     String greeting = messageBundle.getMessage(&quot;hello&quot;);
 *     IoBuffer buf = IoBuffer.allocate(16);
 *     // Turn on autoExpand (it is off by default)
 *     buf.setAutoExpand(true);
 *     buf.putString(greeting, utf8encoder);
 *   </pre>
 *
 *   The underlying {@link ByteBuffer} is reallocated by {@link IoBuffer} behind
 *   the scene if the encoded data is larger than 16 bytes in the example above.
 *   Its capacity will double, and its limit will increase to the last position
 *   the string is written.
 *
 * <h2>AutoShrink</h2>
 * <p>
 *   You might also want to decrease the capacity of the buffer when most of the
 *   allocated memory area is not being used. {@link IoBuffer} provides
 *   <tt>autoShrink</tt> property to take care of this issue. If
 *   <tt>autoShrink</tt> is turned on, {@link IoBuffer} halves the capacity of the
 *   buffer when {@link #compact()} is invoked and only 1/4 or less of the current
 *   capacity is being used.
 * <p>
 *   You can also call the {@link #shrink()} method manually to shrink the capacity of the
 *   buffer.
 * <p>
 *   The underlying {@link ByteBuffer} is reallocated by the {@link IoBuffer} behind
 *   the scene, and therefore {@link #buf()} will return a different
 *   {@link ByteBuffer} instance once capacity changes. Please also note
 *   that the {@link #compact()} method or the {@link #shrink()} method
 *   will not decrease the capacity if the new capacity is less than the
 *   {@link #minimumCapacity()} of the buffer.
 *
 * <h2>Derived Buffers</h2>
 * <p>
 *   Derived buffers are the buffers which were created by the {@link #duplicate()},
 *   {@link #slice()}, or {@link #asReadOnlyBuffer()} methods. They are useful especially
 *   when you broadcast the same messages to multiple {@link IoSession}s. Please
 *   note that the buffer derived from and its derived buffers are not
 *   auto-expandable nor auto-shrinkable. Trying to call
 *   {@link #setAutoExpand(boolean)} or {@link #setAutoShrink(boolean)} with
 *   <tt>true</tt> parameter will raise an {@link IllegalStateException}.
 *
 * <h2>Changing Buffer Allocation Policy</h2>
 * <p>
 *   The {@link IoBufferAllocator} interface lets you override the default buffer
 *   management behavior. There are two allocators provided out-of-the-box:
 *   <ul>
 *     <li>{@link SimpleBufferAllocator} (default)</li>
 *     <li>{@link CachedBufferAllocator}</li>
 *   </ul>
 *   You can implement your own allocator and use it by calling
 *   {@link #setAllocator(IoBufferAllocator)}.
 *
 */

class NIoBuffer: public EComparable<NIoBuffer*>, public ESynchronizeable {
public:
	virtual ~NIoBuffer();

	/**
	 * Returns the direct or heap buffer which is capable to store the specified
	 * amount of bytes.
	 *
	 * @param capacity the capacity of the buffer
	 * @return a IoBuffer which can hold up to capacity bytes
	 *
	 * @see #setUseDirectBuffer(boolean)
	 */
	static NIoBuffer* allocate(int capacity=512);

	/**
	 * Wraps the specified byte array into enaf heap buffer. We just wrap the
	 * bytes starting from offset up to offset + length.  Note that
	 * the byte array is not copied, so any modification done on it will
	 * be visible by both sides.
	 *
	 * @param address The byte array to wrap
	 * @param capacity The number of bytes to store
	 * @param offset The starting point in the byte array
	 * @return a heap IoBuffer containing the selected part of the byte array
	 */
	static NIoBuffer* wrap(void* address, int capacity, int offset=0);

	/**
	 * @see ByteBuffer#duplicate()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* duplicate();

	/**
	 * @see ByteBuffer#slice()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* slice();

	/**
	 * Declares this buffer and all its derived buffers are not used anymore so
	 * that it can be reused by some {@link IoBufferAllocator} implementations.
	 * It is not mandatory to call this method, but you might want to invoke
	 * this method for maximum performance.
	 */
	virtual void free();

	/**
	 * @return the underlying NIO {@link ByteBuffer} instance.
	 */
	virtual EIOByteBuffer* buf();

	/**
	 * @return <tt>true</tt> if and only if this buffer is derived from another
	 * buffer via one of the {@link #duplicate()}, {@link #slice()} or
	 * {@link #asReadOnlyBuffer()} methods.
	 */
	virtual boolean isDerived();

	/**
	 * @see ByteBuffer#isReadOnly()
	 *
	 * @return <tt>true</tt> if the buffer is readOnly
	 */
	virtual boolean isReadOnly();

	/**
	 * @return the minimum capacity of this buffer which is used to determine
	 * the new capacity of the buffer shrunk by the {@link #compact()} and
	 * {@link #shrink()} operation. The default value is the initial capacity of
	 * the buffer.
	 */
	virtual int minimumCapacity();

	/**
	 * Sets the minimum capacity of this buffer which is used to determine the
	 * new capacity of the buffer shrunk by {@link #compact()} and
	 * {@link #shrink()} operation. The default value is the initial capacity of
	 * the buffer.
	 *
	 * @param minimumCapacity the wanted minimum capacity
	 * @return the underlying NIO {@link ByteBuffer} instance.
	 */
	virtual NIoBuffer* minimumCapacity(int minimumCapacity);

	/**
	 * @see ByteBuffer#capacity()
	 *
	 * @return the buffer capacity
	 */
	virtual int capacity();

	/**
	 * Increases the capacity of this buffer. If the new capacity is less than
	 * or equal to the current capacity, this method returns the original buffer.
	 * If the new capacity is greater than the current capacity, the buffer is
	 * reallocated while retaining the position, limit, mark and the content of
	 * the buffer.
	 * <br>
	 * Note that the IoBuffer is replaced, it's not copied.
	 * <br>
	 * Assuming a buffer contains N bytes, its position is 0 and its current capacity is C,
	 * here are the resulting buffer if we set the new capacity to a value V &lt; C and V &gt; C :
	 *
	 * <pre>
	 *  Initial buffer :
	 *
	 *   0       L          C
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *   ^       ^          ^
	 *   |       |          |
	 *  pos    limit     capacity
	 *
	 * V &lt;= C :
	 *
	 *   0       L          C
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *   ^       ^          ^
	 *   |       |          |
	 *  pos    limit   newCapacity
	 *
	 * V &gt; C :
	 *
	 *   0       L          C            V
	 *  +--------+-----------------------+
	 *  |XXXXXXXX|          :            |
	 *  +--------+-----------------------+
	 *   ^       ^          ^            ^
	 *   |       |          |            |
	 *  pos    limit   oldCapacity  newCapacity
	 *
	 *  The buffer has been increased.
	 *
	 * </pre>
	 *
	 * @param newCapacity the wanted capacity
	 * @return the underlying NIO {@link ByteBuffer} instance.
	 */
	virtual NIoBuffer* capacity(int newCapacity);

	/**
	 * @return <tt>true</tt> if and only if <tt>autoExpand</tt> is turned on.
	 */
	virtual boolean isAutoExpand();

	/**
	 * Turns on or off <tt>autoExpand</tt>.
	 *
	 * @param autoExpand The flag value to set
	 * @return The modified IoBuffer instance
	 */
	virtual NIoBuffer* setAutoExpand(boolean autoExpand);

	/**
	 * @return <tt>true</tt> if and only if <tt>autoShrink</tt> is turned on.
	 */
	virtual boolean isAutoShrink();

	/**
	 * Turns on or off <tt>autoShrink</tt>.
	 *
	 * @param autoShrink The flag value to set
	 * @return The modified IoBuffer instance
	 */
	virtual NIoBuffer* setAutoShrink(boolean autoShrink);

	/**
	 * Changes the capacity and limit of this buffer so this buffer get the
	 * specified <tt>expectedRemaining</tt> room from the current position. This
	 * method works even if you didn't set <tt>autoExpand</tt> to <tt>true</tt>.
	 * <br>
	 * Assuming a buffer contains N bytes, its position is P and its current capacity is C,
	 * here are the resulting buffer if we call the expand method with a expectedRemaining
	 * value V :
	 *
	 * <pre>
	 *  Initial buffer :
	 *
	 *   0       L          C
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *   ^       ^          ^
	 *   |       |          |
	 *  pos    limit     capacity
	 *
	 * ( pos + V )  &lt;= L, no change :
	 *
	 *   0       L          C
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *   ^       ^          ^
	 *   |       |          |
	 *  pos    limit   newCapacity
	 *
	 * You can still put ( L - pos ) bytes in the buffer
	 *
	 * ( pos + V ) &gt; L &amp; ( pos + V ) &lt;= C :
	 *
	 *  0        L          C
	 *  +------------+------+
	 *  |XXXXXXXX:...|      |
	 *  +------------+------+
	 *   ^           ^      ^
	 *   |           |      |
	 *  pos       newlimit  newCapacity
	 *
	 *  You can now put ( L - pos + V )  bytes in the buffer.
	 *
	 *
	 *  ( pos + V ) &gt; C
	 *
	 *   0       L          C
	 *  +-------------------+----+
	 *  |XXXXXXXX:..........:....|
	 *  +------------------------+
	 *   ^                       ^
	 *   |                       |
	 *  pos                      +-- newlimit
	 *                           |
	 *                           +-- newCapacity
	 *
	 * You can now put ( L - pos + V ) bytes in the buffer, which limit is now
	 * equals to the capacity.
	 * </pre>
	 *
	 * Note that the expecting remaining bytes starts at the current position. In all
	 * those examples, the position is 0.
	 *
	 * @param expectedRemaining The expected remaining bytes in the buffer
	 * @return The modified IoBuffer instance
	 */
	virtual NIoBuffer* expand(int expectedRemaining);

	/**
	 * Changes the capacity and limit of this buffer so this buffer get the
	 * specified <tt>expectedRemaining</tt> room from the specified
	 * <tt>position</tt>. This method works even if you didn't set
	 * <tt>autoExpand</tt> to <tt>true</tt>.
	 * Assuming a buffer contains N bytes, its position is P and its current capacity is C,
	 * here are the resulting buffer if we call the expand method with a expectedRemaining
	 * value V :
	 *
	 * <pre>
	 *  Initial buffer :
	 *
	 *      P    L          C
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *      ^    ^          ^
	 *      |    |          |
	 *     pos limit     capacity
	 *
	 * ( pos + V )  &lt;= L, no change :
	 *
	 *      P    L          C
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *      ^    ^          ^
	 *      |    |          |
	 *     pos limit   newCapacity
	 *
	 * You can still put ( L - pos ) bytes in the buffer
	 *
	 * ( pos + V ) &gt; L &amp; ( pos + V ) &lt;= C :
	 *
	 *      P    L          C
	 *  +------------+------+
	 *  |XXXXXXXX:...|      |
	 *  +------------+------+
	 *      ^        ^      ^
	 *      |        |      |
	 *     pos    newlimit  newCapacity
	 *
	 *  You can now put ( L - pos + V)  bytes in the buffer.
	 *
	 *
	 *  ( pos + V ) &gt; C
	 *
	 *      P       L          C
	 *  +-------------------+----+
	 *  |XXXXXXXX:..........:....|
	 *  +------------------------+
	 *      ^                    ^
	 *      |                    |
	 *     pos                   +-- newlimit
	 *                           |
	 *                           +-- newCapacity
	 *
	 * You can now put ( L - pos + V ) bytes in the buffer, which limit is now
	 * equals to the capacity.
	 * </pre>
	 *
	 * Note that the expecting remaining bytes starts at the current position. In all
	 * those examples, the position is P.
	 *
	 * @param position The starting position from which we want to define a remaining
	 * number of bytes
	 * @param expectedRemaining The expected remaining bytes in the buffer
	 * @return The modified IoBuffer instance
	 */
	virtual NIoBuffer* expand(int position, int expectedRemaining);

	/**
	 * Changes the capacity of this buffer so this buffer occupies as less
	 * memory as possible while retaining the position, limit and the buffer
	 * content between the position and limit.
	 * <br>
	 * <b>The capacity of the buffer never becomes less than {@link #minimumCapacity()}</b>
	 * <br>.
	 * The mark is discarded once the capacity changes.
	 * <br>
	 * Typically, a call to this method tries to remove as much unused bytes
	 * as possible, dividing by two the initial capacity until it can't without
	 * obtaining a new capacity lower than the {@link #minimumCapacity()}. For instance, if
	 * the limit is 7 and the capacity is 36, with a minimum capacity of 8,
	 * shrinking the buffer will left a capacity of 9 (we go down from 36 to 18, then from 18 to 9).
	 *
	 * <pre>
	 *  Initial buffer :
	 *
	 *  +--------+----------+
	 *  |XXXXXXXX|          |
	 *  +--------+----------+
	 *      ^    ^  ^       ^
	 *      |    |  |       |
	 *     pos   |  |    capacity
	 *           |  |
	 *           |  +-- minimumCapacity
	 *           |
	 *           +-- limit
	 *
	 * Resulting buffer :
	 *
	 *  +--------+--+-+
	 *  |XXXXXXXX|  | |
	 *  +--------+--+-+
	 *      ^    ^  ^ ^
	 *      |    |  | |
	 *      |    |  | +-- new capacity
	 *      |    |  |
	 *     pos   |  +-- minimum capacity
	 *           |
	 *           +-- limit
	 * </pre>
	 *
	 * @return The modified IoBuffer instance
	 */
	virtual NIoBuffer* shrink();

	/**
	 * @see java.nio.Buffer#position()
	 * @return The current position in the buffer
	 */
	virtual int position();

	/**
	 * @see java.nio.Buffer#position(int)
	 *
	 * @param newPosition Sets the new position in the buffer
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* position(int newPosition);

	/**
	 * @see java.nio.Buffer#limit()
	 *
	 * @return the modified IoBuffer's limit
	 */
	virtual int limit();

	/**
	 * @see java.nio.Buffer#limit(int)
	 *
	 * @param newLimit The new buffer's limit
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* limit(int newLimit);

	/**
	 * @see java.nio.Buffer#mark()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* mark();

	/**
	 * @return the position of the current mark. This method returns <tt>-1</tt>
	 * if no mark is set.
	 */
	virtual int markValue();

	/**
	 * @see java.nio.Buffer#reset()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* reset();

	/**
	 * @see java.nio.Buffer#clear()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* clear();

	/**
	 * Clears this buffer and fills its content with <tt>NUL</tt>. The position
	 * is set to zero, the limit is set to the capacity, and the mark is
	 * discarded.
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* sweep();

	/**
	 * double Clears this buffer and fills its content with <tt>value</tt>. The
	 * position is set to zero, the limit is set to the capacity, and the mark
	 * is discarded.
	 *
	 * @param value The value to put in the buffer
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* sweep(byte value);

	/**
	 * @see java.nio.Buffer#flip()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* flip();

	/**
	 * @see java.nio.Buffer#rewind()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* rewind();

	/**
	 * @see ByteBuffer#compact()
	 *
	 * @return the modified IoBuffer
	 */
	virtual NIoBuffer* compact();

	/**
	 * Forwards the position of this buffer as the specified <code>size</code>
	 * bytes.
	 *
	 * @param size The added size
	 * @return The modified IoBuffer
	 */
	virtual NIoBuffer* skip(int size);

	/**
	 * Returns the first occurrence position of the specified byte from the
	 * current position to the current limit.
	 *
	 * @param b The byte we are looking for
	 * @return <tt>-1</tt> if the specified byte is not found
	 */
	virtual int indexOf(byte b);

	/**
	 * @see java.nio.Buffer#remaining()
	 *
	 * @return The remaining bytes in the buffer
	 */
	virtual int remaining();

	/**
	 * @see java.nio.Buffer#hasRemaining()
	 *
	 * @return <tt>true</tt> if there are some remaining bytes in the buffer
	 */
	virtual boolean hasRemaining();

	/**
	 * Dumps an {@link IoBuffer} to a hex formatted string.
	 *
	 * @param in the buffer to dump
	 * @param lengthLimit the limit at which hex dumping will stop
	 * @return a hex formatted string representation of the <i>in</i> {@link Iobuffer}.
	 */
	virtual EString getHexdump(int lengthLimit=16);

	/**
	 * {@inheritDoc}
	 */
	virtual int compareTo(NIoBuffer* that);

	/**
	 * {@inheritDoc}
	 */
	virtual EStringBase toString();

	/**
	 * Return base address.
	 */
	virtual void* address();

	/**
	 * Return current address.
	 */
	virtual void* current();

public:
	/**
     * @see ByteBuffer#get()
     *
     * @return The byte at the current position
     */
	virtual byte get();

    /**
     * @see ByteBuffer#put(byte)
     *
     * @param b The byte to put in the buffer
     * @return the modified IoBuffer

     */
	virtual NIoBuffer* put(byte b);

    /**
     * @see ByteBuffer#get(int)
     *
     * @param index The position for which we want to read a byte
     * @return the byte at the given position
     */
	virtual byte get(int index);

    /**
     * @see ByteBuffer#put(int, byte)
     *
     * @param index The position where the byte will be put
     * @param b The byte to put
     * @return the modified IoBuffer

     */
	virtual NIoBuffer* put(int index, byte b);

    /**
     * @see ByteBuffer#get(byte[], int, int)
     *
     * @param dst The destination buffer
     * @param size The destination buffer size
     * @param length The number of bytes to copy
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* get(void* dst, int size, int length);

    /**
     * @see ByteBuffer#get(byte[])
     *
     * @param dst The byte[] that will contain the read bytes
     * @return the IoBuffer
     */
	virtual NIoBuffer* get(EA<byte>* dst);

    /**
     * Get a new IoBuffer containing a slice of the current buffer
     *
     * @param index The position in the buffer
     * @param length The number of bytes to copy
     * @return the new IoBuffer
     */
	virtual NIoBuffer* getSlice(int index, int length); //!

    /**
     * Get a new IoBuffer containing a slice of the current buffer
     *
     * @param length The number of bytes to copy
     * @return the new IoBuffer
     */
	virtual NIoBuffer* getSlice(int length); //!

    /**
     * Writes the content of the specified <tt>src</tt> into this buffer.
     *
     * @param src The source ByteBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* put(EIOByteBuffer* src);

    /**
     * Writes the content of the specified <tt>src</tt> into this buffer.
     *
     * @param src The source IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* put(NIoBuffer* src);

    /**
     * @see ByteBuffer#put(byte[], int, int)
     *
     * @param src The byte[] to put
     * @param length The number of bytes to copy
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* put(const void* src, int length);

    /**
     * @see ByteBuffer#put(byte[])
     *
     * @param src The byte[] to put
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* put(EA<byte>* src);

    /**
     * @see ByteBuffer#getChar()
     *
     * @return The char at the current position
     */
	virtual char getChar();

    /**
     * @see ByteBuffer#putChar(char)
     *
     * @param value The char to put at the current position
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putChar(char value);

    /**
     * @see ByteBuffer#getChar(int)
     *
     * @param index The index in the IoBuffer where we will read a char from
     * @return the char at 'index' position
     */
	virtual char getChar(int index);

    /**
     * @see ByteBuffer#putChar(int, char)
     *
     * @param index The index in the IoBuffer where we will put a char in
     * @param value The char to put at the current position
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putChar(int index, char value);

    /**
     * @see ByteBuffer#getShort()
     *
     * @return The read short
     */
	virtual short getShort();

    /**
     * @see ByteBuffer#putShort(short)
     *
     * @param value The short to put at the current position
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putShort(short value);

    /**
     * @see ByteBuffer#getShort()
     *
     * @param index The index in the IoBuffer where we will read a short from
     * @return The read short
     */
	virtual short getShort(int index);

    /**
     * @see ByteBuffer#putShort(int, short)
     *
     * @param index The position at which the short should be written
     * @param value The short to put at the current position
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putShort(int index, short value);

    /**
     * @see ByteBuffer#getInt()
     *
     * @return The int read
     */
	virtual int getInt();

    /**
     * @see ByteBuffer#putInt(int)
     *
     * @param value The int to put at the current position
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putInt(int value);

    /**
     * @see ByteBuffer#getInt(int)
     * @param index The index in the IoBuffer where we will read an int from
     * @return the int at the given position
     */
	virtual int getInt(int index);

    /**
     * @see ByteBuffer#putInt(int, int)
     *
     * @param index The position where to put the int
     * @param value The int to put in the IoBuffer
     * @return the modified IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putInt(int index, int value);

    /**
     * @see ByteBuffer#getLong()
     *
     * @return The long at the current position
     */
	virtual llong getLLong();

    /**
     * @see ByteBuffer#putLong(int, long)
     *
     * @param value The log to put in the IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putLLong(llong value);

    /**
     * @see ByteBuffer#getLong(int)
     *
     * @param index The index in the IoBuffer where we will read a long from
     * @return the long at the given position
     */
	virtual llong getLLong(int index);

    /**
     * @see ByteBuffer#putLong(int, long)
     *
     * @param index The position where to put the long
     * @param value The long to put in the IoBuffer
     * @return the modified IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putLLong(int index, llong value);

    /**
     * @see ByteBuffer#getFloat()
     *
     * @return the float at the current position
     */
	virtual float getFloat();

    /**
     * @see ByteBuffer#putFloat(float)
     *
     * @param value The float to put in the IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putFloat(float value);

    /**
     * @see ByteBuffer#getFloat(int)
     *
     * @param index The index in the IoBuffer where we will read a float from
     * @return The float at the given position
     */
	virtual float getFloat(int index);

    /**
     * @see ByteBuffer#putFloat(int, float)
     *
     * @param index The position where to put the float
     * @param value The float to put in the IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putFloat(int index, float value);

    /**
     * @see ByteBuffer#getDouble()
     *
     * @return the double at the current position
     */
	virtual double getDouble();

    /**
     * @see ByteBuffer#putDouble(double)
     *
     * @param value The double to put at the IoBuffer current position
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putDouble(double value);

    /**
     * @see ByteBuffer#getDouble(int)
     *
     * @param index The position where to get the double from
     * @return The double at the given position
     */
	virtual double getDouble(int index);

    /**
     * @see ByteBuffer#putDouble(int, double)
     *
     * @param index The position where to put the double
     * @param value The double to put in the IoBuffer
     * @return the modified IoBuffer
     */
	virtual NIoBuffer* putDouble(int index, double value);

    // //////////////////////////////
    // String getters and putters //
    // //////////////////////////////

    /**
     * Reads a <code>NUL</code>-terminated string from this buffer using the
     * specified <code>decoder</code> and returns it. This method reads until
     * the limit of this buffer if no <tt>NUL</tt> is found.
     *
     * @return the read String
     * @exception CharacterCodingException Thrown when an error occurred while decoding the buffer
     */
	virtual EString getString();

    /**
     * Reads a <code>NUL</code>-terminated string from this buffer using the
     * specified <code>decoder</code> and returns it.
     *
     * @param fieldSize the maximum number of bytes to read
     * @return the read String
     * @exception CharacterCodingException Thrown when an error occurred while decoding the buffer
     */
	virtual EString getString(int fieldSize);

    /**
     * Writes the content of <code>in</code> into this buffer using the
     * specified <code>encoder</code>. This method doesn't terminate string with
     * <tt>NUL</tt>. You have to do it by yourself.
     *
     * @param val The CharSequence to put in the IoBuffer
     * @param encoder The CharsetEncoder to use
     * @return The modified IoBuffer
     * @throws CharacterCodingException When we have an error while decoding the String
     */
	virtual NIoBuffer* putString(const char* val);

    /**
     * Writes the content of <code>in</code> into this buffer as a
     * <code>NUL</code>-terminated string using the specified
     * <code>encoder</code>.
     * <p>
     * If the charset name of the encoder is UTF-16, you cannot specify odd
     * <code>fieldSize</code>, and this method will append two <code>NUL</code>s
     * as a terminator.
     * <p>
     * Please note that this method doesn't terminate with <code>NUL</code> if
     * the input string is longer than <tt>fieldSize</tt>.
     *
     * @param val The CharSequence to put in the IoBuffer
     * @param fieldSize the maximum number of bytes to write
     * @return The modified IoBuffer
     * @throws CharacterCodingException When we have an error while decoding the String
     */
	virtual NIoBuffer* putString(const char* val, int fieldSize);

	/**
	 * Fills this buffer with the specified value. This method moves buffer
	 * position forward.
	 *
	 * @param value The value to fill the IoBuffer with
	 * @param size The added size
	 * @return The modified IoBuffer
	 */
	virtual NIoBuffer* fill(byte value, int size);

	/**
	 * Fills this buffer with the specified value. This method does not change
	 * buffer position.
	 *
	 * @param value The value to fill the IoBuffer with
	 * @param size The added size
	 * @return The modified IoBuffer
	 */
	virtual NIoBuffer* fillAndReset(byte value, int size);

public:
    /**
     * @return an {@link InputStream} that reads the data from this buffer.
     * {@link InputStream#read()} returns <tt>-1</tt> if the buffer position
     * reaches to the limit.
     */
	virtual EInputStream* asInputStream(); //!

    /**
     * @return an {@link OutputStream} that appends the data into this buffer.
     * Please note that the {@link OutputStream#write(int)} will throw a
     * {@link BufferOverflowException} instead of an {@link IOException} in case
     * of buffer overflow. Please set <tt>autoExpand</tt> property by calling
     * {@link #setAutoExpand(boolean)} to prevent the unexpected runtime
     * exception.
     */
	virtual EOutputStream* asOutputStream(); //!

protected:
	EIOByteBuffer* buf_;

	/** Tells if a buffer has been created from an existing buffer */
	boolean derived_;

	/** A flag set to true if the buffer can extend automatically */
	boolean autoExpand_;

	/** A flag set to true if the buffer can shrink automatically */
	boolean autoShrink_;

	/** Tells if a buffer can be expanded */
	boolean recapacityAllowed_;// = true;

	/** The minimum number of bytes the IoBuffer can hold */
	int minimumCapacity_;

	/** A mask for a byte */
	static const long BYTE_MASK = 0xFFL;

	/** A mask for a short */
	static const long SHORT_MASK = 0xFFFFL;

	/** A mask for an int */
	static const long INT_MASK = 0xFFFFFFFFL;

	/**
	 * Creates a new instance. This is an empty constructor. It's protected,
	 * to forbid its usage by the users.
	 */
	NIoBuffer() {
		// Do nothing
	}

	NIoBuffer(int capacity);
	NIoBuffer(EIOByteBuffer* newbuf, int minimumCapacity);

	/**
	 * This method forwards the call to {@link #expand(int)} only when
	 * <tt>autoExpand</tt> property is <tt>true</tt>.
	 */
	NIoBuffer* autoExpand(int expectedRemaining);

	/**
	 * This method forwards the call to {@link #expand(int)} only when
	 * <tt>autoExpand</tt> property is <tt>true</tt>.
	 */
	NIoBuffer* autoExpand(int pos, int expectedRemaining);

	/**
	 *
	 */
	NIoBuffer* expand(int pos, int expectedRemaining, boolean autoExpand);

	/**
	 * Normalizes the specified capacity of the buffer to power of 2, which is
	 * often helpful for optimal memory usage and performance. If it is greater
	 * than or equal to {@link Integer#MAX_VALUE}, it returns
	 * {@link Integer#MAX_VALUE}. If it is zero, it returns zero.
	 *
	 * @param requestedCapacity The IoBuffer capacity we want to be able to store
	 * @return The  power of 2 strictly superior to the requested capacity
	 */
	static int normalizeCapacity(int requestedCapacity);
};

} /* namespace naf */
} /* namespace efc */
#endif /* NIOBUFFER_HH_ */
