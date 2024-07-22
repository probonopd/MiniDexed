#ifndef _H_UTILS_
#define _H_UTILS_

#include <cstdint>

class ArpUtils {
public:
	ArpUtils();
	~ArpUtils();
	void quicksort(uint8_t arr[][2], int l, int r);
private:
	void swap(uint8_t *a, uint8_t *b);
};

#endif //_H_UTILS_
