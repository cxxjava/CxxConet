#include "./to_lower_table.h"

namespace efc {
namespace naf {
namespace Http {

ToLowerTable::ToLowerTable() {
	for (size_t c = 0; c < 256; c++) {
		table_[c] = c;
		if ((c >= 'A') && (c <= 'Z')) {
			table_[c] |= 0x20;
		}
	}
}

void ToLowerTable::toLowerCase(char* buffer, uint32_t size) const {
	for (size_t i = 0; i < size; i++) {
		buffer[i] = table_[static_cast<uint8_t>(buffer[i])];
	}
}

} // namespace http
} // namespace naf
} // namespace efc
