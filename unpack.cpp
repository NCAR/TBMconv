

void decode(uint8_t *in_buffer, char *out_buffer, size_t length)
{
	int i, j;
	uint32_t buf[3];

	for (j = 0, i = 0; i < length ; i += 12 /* 3*32/8 */ , j += 16) {
		buf[0] = (((uint32_t) in_buffer [i+ 0]) << 24) |
		         (((uint32_t) in_buffer [i+ 1]) << 16) |
		         (((uint32_t) in_buffer [i+ 2]) <<  8) |
		         (((uint32_t) in_buffer [i+ 3]) <<  0);
		buf[1] = (((uint32_t) in_buffer [i+ 4]) << 24) |
		         (((uint32_t) in_buffer [i+ 5]) << 16) |
		         (((uint32_t) in_buffer [i+ 6]) <<  8) |
		         (((uint32_t) in_buffer [i+ 7]) <<  0);
		buf[2] = (((uint32_t) in_buffer [i+ 8]) << 24) |
		         (((uint32_t) in_buffer [i+ 9]) << 16) |
		         (((uint32_t) in_buffer [i+10]) <<  8) |
		         (((uint32_t) in_buffer [i+11]) <<  0);
		out_buffer[j+15] = buf[2] & 0x3F ; buf[2] >>= 6;
		out_buffer[j+14] = buf[2] & 0x3F ; buf[2] >>= 6;
		out_buffer[j+13] = buf[2] & 0x3F ; buf[2] >>= 6;
		out_buffer[j+12] = buf[2] & 0x3F ; buf[2] >>= 6;
		out_buffer[j+11] = buf[2] & 0x3F ; buf[2] >>= 6;
		out_buffer[j+10] = (buf[2] & 0x3) | ((buf[1] & 0xF) << 2); buf[1] >>= 4;
		out_buffer[j+ 9] = buf[1] & 0x3F ; buf[1] >>= 6;
		out_buffer[j+ 8] = buf[1] & 0x3F ; buf[1] >>= 6;
		out_buffer[j+ 7] = buf[1] & 0x3F ; buf[1] >>= 6;
		out_buffer[j+ 6] = buf[1] & 0x3F ; buf[1] >>= 6;
		out_buffer[j+ 5] = (buf[1] & 0xF) | ((buf[0] & 0x3) << 4); buf[0] >>= 2;
		out_buffer[j+ 4] = buf[0] & 0x3F ; buf[0] >>= 6;
		out_buffer[j+ 3] = buf[0] & 0x3F ; buf[0] >>= 6;
		out_buffer[j+ 2] = buf[0] & 0x3F ; buf[0] >>= 6;
		out_buffer[j+ 1] = buf[0] & 0x3F ; buf[0] >>= 6;
		out_buffer[j+ 0] = buf[0] & 0x3F ; buf[0] >>= 6;
	}
}
