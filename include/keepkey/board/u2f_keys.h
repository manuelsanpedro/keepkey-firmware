#ifndef __U2F_KEYS_H_INCLUDED__
#define __U2F_KEYS_H_INCLUDED__

#include <stdint.h>

static const uint8_t U2F_ATT_PRIV_KEY[] = {
	0x71, 0x26, 0xac, 0x2b, 0xf6, 0x44, 0xdc, 0x61,
	0x86, 0xad, 0x83, 0xef, 0x1f, 0xcd, 0xf1, 0x2a,
	0x57, 0xb5, 0xcf, 0xa2, 0x00, 0x0b, 0x8a, 0xd0,
	0x27, 0xe9, 0x56, 0xe8, 0x54, 0xc5, 0x0a, 0x8b
};

static const uint8_t U2F_ATT_CERT[] = {
	0x30, 0x82, 0x01, 0x18, 0x30, 0x81, 0xc0, 0x02, 0x09, 0x00, 0xb1, 0xd9,
	0x8f, 0x42, 0x64, 0x72, 0xd3, 0x2c, 0x30, 0x0a, 0x06, 0x08, 0x2a, 0x86,
	0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x30, 0x15, 0x31, 0x13, 0x30, 0x11,
	0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x0a, 0x54, 0x72, 0x65, 0x7a, 0x6f,
	0x72, 0x20, 0x55, 0x32, 0x46, 0x30, 0x1e, 0x17, 0x0d, 0x31, 0x36, 0x30,
	0x34, 0x32, 0x39, 0x31, 0x33, 0x33, 0x31, 0x35, 0x33, 0x5a, 0x17, 0x0d,
	0x32, 0x36, 0x30, 0x34, 0x32, 0x37, 0x31, 0x33, 0x33, 0x31, 0x35, 0x33,
	0x5a, 0x30, 0x15, 0x31, 0x13, 0x30, 0x11, 0x06, 0x03, 0x55, 0x04, 0x03,
	0x0c, 0x0a, 0x54, 0x72, 0x65, 0x7a, 0x6f, 0x72, 0x20, 0x55, 0x32, 0x46,
	0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x02,
	0x01, 0x06, 0x08, 0x2a, 0x86, 0x48, 0xce, 0x3d, 0x03, 0x01, 0x07, 0x03,
	0x42, 0x00, 0x04, 0xd9, 0x18, 0xbd, 0xfa, 0x8a, 0x54, 0xac, 0x92, 0xe9,
	0x0d, 0xa9, 0x1f, 0xca, 0x7a, 0xa2, 0x64, 0x54, 0xc0, 0xd1, 0x73, 0x36,
	0x31, 0x4d, 0xde, 0x83, 0xa5, 0x4b, 0x86, 0xb5, 0xdf, 0x4e, 0xf0, 0x52,
	0x65, 0x9a, 0x1d, 0x6f, 0xfc, 0xb7, 0x46, 0x7f, 0x1a, 0xcd, 0xdb, 0x8a,
	0x33, 0x08, 0x0b, 0x5e, 0xed, 0x91, 0x89, 0x13, 0xf4, 0x43, 0xa5, 0x26,
	0x1b, 0xc7, 0x7b, 0x68, 0x60, 0x6f, 0xc1, 0x30, 0x0a, 0x06, 0x08, 0x2a,
	0x86, 0x48, 0xce, 0x3d, 0x04, 0x03, 0x02, 0x03, 0x47, 0x00, 0x30, 0x44,
	0x02, 0x20, 0x24, 0x1e, 0x81, 0xff, 0xd2, 0xe5, 0xe6, 0x15, 0x36, 0x94,
	0xc3, 0x55, 0x2e, 0x8f, 0xeb, 0xd7, 0x1e, 0x89, 0x35, 0x92, 0x1c, 0xb4,
	0x83, 0x41, 0x43, 0x71, 0x1c, 0x76, 0xea, 0xee, 0xf3, 0x95, 0x02, 0x20,
	0x5f, 0x80, 0xeb, 0x10, 0xf2, 0x5c, 0xcc, 0x39, 0x8b, 0x3c, 0xa8, 0xa9,
	0xad, 0xa4, 0x02, 0x7f, 0x93, 0x13, 0x20, 0x77, 0xb7, 0xab, 0xce, 0x77,
	0x46, 0x5a, 0x27, 0xf5, 0x3d, 0x33, 0xa1, 0x1d,
};

#endif // __U2F_KEYS_H_INCLUDED__
