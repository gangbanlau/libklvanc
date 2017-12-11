/*
 * Copyright (c) 2016 Kernel Labs Inc. All Rights Reserved
 *
 * Address: Kernel Labs Inc., PO Box 745, St James, NY. 11780
 * Contact: sales@kernellabs.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <libklvanc/vanc.h>

#include "core-private.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *klvanc_afd_to_string(enum klvanc_payload_afd_e afd)
{
	switch(afd) {
	case AFD_BOX_16x9_TOP:
		return "AFD_BOX_16x9_TOP";
	case AFD_BOX_14x9_TOP:
		return "AFD_BOX_14x9_TOP";
	case AFD_BOX_16x9_CENTER:
		return "AFD_16x9_CENTER";
	case AFD_FULL_FRAME:
		return "AFD_FULL_FRAME";
	case AFD_16x9_CENTER:
		return "AFD_16x9_CENTER";
	case AFD_14x9_CENTER:
		return "AFD_14x9_CENTER";
	case AFD_4x3_WITH_ALTERNATIVE_14x9_CENTER:
		return "AFD_4x3_WITH_ALTERNATIVE_14x9_CENTER";
	case AFD_16x9_WITH_ALTERNATIVE_14x9_CENTER:
		return "AFD_16x9_WITH_ALTERNATIVE_14x9_CENTER";
	case AFD_16x9_WITH_ALTERNATIVE_4x3_CENTER:
		return "AFD_16x9_WITH_ALTERNATIVE_4x3_CENTER";
	default:
        	return "AFD_UNDEFINED";
	}
}

static enum klvanc_payload_afd_e afd_raw_to_enum(unsigned char afd)
{
	switch(afd) {
	case 0x02:
		return AFD_BOX_16x9_TOP;
	case 0x03:
		return AFD_BOX_14x9_TOP;
	case 0x04:
		return AFD_BOX_16x9_CENTER;
	case 0x08:
		return AFD_FULL_FRAME;
	case 0x0a:
		return AFD_16x9_CENTER;
	case 0x0b:
		return AFD_14x9_CENTER;
	case 0x0d:
		return AFD_4x3_WITH_ALTERNATIVE_14x9_CENTER;
	case 0x0e:
		return AFD_16x9_WITH_ALTERNATIVE_14x9_CENTER;
	case 0x0f:
		return AFD_16x9_WITH_ALTERNATIVE_4x3_CENTER;
	default:
		return AFD_UNDEFINED;
	}
}

static unsigned char afd_enum_to_raw(enum klvanc_payload_afd_e afd)
{
	switch(afd) {
	case AFD_BOX_16x9_TOP:
		return 0x02;
	case AFD_BOX_14x9_TOP:
		return 0x03;
	case AFD_BOX_16x9_CENTER:
		return 0x04;
	case AFD_FULL_FRAME:
		return 0x08;
	case AFD_16x9_CENTER:
		return 0x0a;
	case AFD_14x9_CENTER:
		return 0x0b;
	case AFD_4x3_WITH_ALTERNATIVE_14x9_CENTER:
		return 0x0d;
	case AFD_16x9_WITH_ALTERNATIVE_14x9_CENTER:
		return 0x0e;
	case AFD_16x9_WITH_ALTERNATIVE_4x3_CENTER:
		return 0x0f;
	default:
		return 0x00;
	}
}

const char *klvanc_aspectRatio_to_string(enum klvanc_payload_aspect_ratio_e ar)
{
	switch(ar) {
	case ASPECT_4x3:
		return "ASPECT_4x3";
	case ASPECT_16x9:
		return "ASPECT_16x9";
	default:
        	return "ASPECT_UNDEFINED";
	}
}

int klvanc_dump_AFD(struct klvanc_context_s *ctx, void *p)
{
	if (ctx->verbose)
		PRINT_DEBUG("%s()\n", __func__);

	struct klvanc_packet_afd_s *pkt = p;

	PRINT_DEBUG("%s() AFD: %s Aspect Ratio: %s Flags: 0x%x Value1: 0x%x Value2: 0x%x\n", __func__,
		    klvanc_afd_to_string(pkt->afd),
		    klvanc_aspectRatio_to_string(pkt->aspectRatio),
		    pkt->barDataFlags,
		    pkt->barDataValue[0],
		    pkt->barDataValue[1]);

	return KLAPI_OK;
}

int parse_AFD(struct klvanc_context_s *ctx,
			      struct klvanc_packet_header_s *hdr, void **pp)
{
	if (ctx->verbose)
		PRINT_DEBUG("%s()\n", __func__);

	struct klvanc_packet_afd_s *pkt = calloc(1, sizeof(*pkt));
	if (!pkt)
		return -ENOMEM;

	memcpy(&pkt->hdr, hdr, sizeof(*hdr));
	unsigned char afd = (sanitizeWord(hdr->payload[0]) >> 3) & 0x0f;

	pkt->afd = afd_raw_to_enum(afd);

	if (sanitizeWord(hdr->payload[0]) & 0x04)
		pkt->aspectRatio = ASPECT_16x9;
	else
		pkt->aspectRatio = ASPECT_4x3;

	pkt->barDataFlags = sanitizeWord(hdr->payload[3]) >> 4;
	pkt->barDataValue[0]  = sanitizeWord(hdr->payload[4]) << 8;
	pkt->barDataValue[0] |= sanitizeWord(hdr->payload[5]);
	pkt->barDataValue[1]  = sanitizeWord(hdr->payload[6]) << 8;
	pkt->barDataValue[1] |= sanitizeWord(hdr->payload[7]);

	if (ctx->callbacks && ctx->callbacks->afd)
		ctx->callbacks->afd(ctx->callback_context, ctx, pkt);

	*pp = pkt;
	return KLAPI_OK;
}

int klvanc_create_AFD(struct klvanc_packet_afd_s **pkt)
{
	struct klvanc_packet_afd_s *p = calloc(1, sizeof(*p));
	if (p == NULL)
		return -ENOMEM;

	*pkt = p;
	return 0;
}

void klvanc_destroy_AFD(struct klvanc_packet_afd_s *pkt)
{
	free(pkt);
}

int klvanc_set_AFD_val(struct klvanc_packet_afd_s *pkt, unsigned char val)
{
	pkt->afd = afd_raw_to_enum(val);
	if (pkt->afd == AFD_UNDEFINED)
		return 1;
	else
		return 0;
}

int klvanc_convert_AFD_to_packetBytes(struct klvanc_packet_afd_s *pkt, uint8_t **bytes, uint16_t *byteCount)
{
	unsigned char afd;

	if (!pkt || !bytes) {
		return -1;
	}

	*bytes = malloc(255);
	if (*bytes == NULL)
		return -ENOMEM;

	/* Serialize the AFD struct into a binary blob */
	struct klbs_context_s *bs = klbs_alloc();
	klbs_write_set_buffer(bs, *bytes, 255);

	afd = afd_enum_to_raw(pkt->afd) << 3;
	if (pkt->aspectRatio == ASPECT_16x9)
		afd |= 0x04;

	klbs_write_bits(bs, afd, 8);
	klbs_write_bits(bs, 0x00, 8); /* Reserved */
	klbs_write_bits(bs, 0x00, 8); /* Reserved */
	klbs_write_bits(bs, 0x00, 8); /* Bar Data Flags */
	klbs_write_bits(bs, 0x00, 8); /* Bar Data Value 1 */
	klbs_write_bits(bs, 0x00, 8); /* Bar Data Value 2 */

	klbs_write_buffer_complete(bs);

	*byteCount = klbs_get_byte_count(bs);
	klbs_free(bs);

	return 0;
}

int klvanc_convert_AFD_to_words(struct klvanc_packet_afd_s *pkt, uint16_t **words, uint16_t *wordCount)
{
	uint8_t *buf;
	uint16_t byteCount;
	int ret;

	ret = klvanc_convert_AFD_to_packetBytes(pkt, &buf, &byteCount);
	if (ret != 0)
		return ret;

	/* Create the final array of VANC bytes (with correct DID/SDID,
	   checksum, etc) */
	klvanc_sdi_create_payload(0x05, 0x41, buf, byteCount, words, wordCount, 10);

	free(buf);

	return 0;
}
