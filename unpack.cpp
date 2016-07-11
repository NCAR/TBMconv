
/**
 * Copyright (c) 2016, University Corporation for Atmospheric Research
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

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
