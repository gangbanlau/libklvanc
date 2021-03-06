/*
 * Copyright (c) 2017 Kernel Labs Inc. All Rights Reserved
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

#include <stdio.h>
#include <stdlib.h>
#include <libklvanc/vanc.h>

/* Normally we don't use a global, but we know our test harness will never be
   multi-threaded, and this is a really easy way to get the results out of the
   callback for comparison */
static uint16_t vancResult[16384];
static size_t vancResultCount;
static int passCount = 0;
static int failCount = 0;

#define SHOW_DETAIL 1

/* CALLBACKS for message notification */
static int cb_eia_708(void *callback_context, struct klvanc_context_s *ctx,
		      struct klvanc_packet_eia_708b_s *pkt)
{
	int ret = -1;

#ifdef SHOW_DETAIL
	/* Have the library display some debug */
	printf("Asking libklvanc to dump a struct\n");
	ret = klvanc_dump_EIA_708B(ctx, pkt);
	if (ret != 0) {
		fprintf(stderr, "Error dumping EIA-708 packet!\n");
		return -1;
	}
#endif

	uint16_t *words;
	uint16_t wordCount;
	ret = klvanc_convert_EIA_708B_to_words(pkt, &words, &wordCount);
	if (ret != 0) {
		fprintf(stderr, "Failed to convert 104 to words: %d\n", ret);
		return -1;
	}

	memcpy(vancResult, words, wordCount * sizeof(uint16_t));
	vancResultCount = wordCount;
	free(words);

	return 0;
}

static struct klvanc_callbacks_s callbacks =
{
	.eia_708b		= cb_eia_708,
};
/* END - CALLBACKS for message notification */

static unsigned char test1[] = {
	0x00, 0x00, 0x03, 0xff, 0x03, 0xff, 0x01, 0x61, 0x01, 0x01, 0x01, 0x52, 0x02, 0x96, 0x02, 0x69, 0x01, 0x52, 0x01, 0x4f, 0x02, 0x77, 0x01, 0xbc, 0x02, 0x95, 0x02, 0x72, 0x01, 0xf4, 0x02, 0xfc, 0x01, 0x80, 0x01, 0x80, 0x01, 0xfd, 0x01, 0x80, 0x01, 0x80, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x02, 0xfa, 0x02, 0x00, 0x02, 0x00, 0x01, 0x73, 0x02, 0xd1, 0x01, 0xe0, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x00, 0x02, 0x74, 0x01, 0xbc, 0x02, 0x95, 0x01, 0xbc, 0x01, 0xb4

};

static int test_eia_708(struct klvanc_context_s *ctx, const uint8_t *buf, size_t bufSize)
{
	int numWords = bufSize / 2;
	int mismatch = 0;

	printf("\nParsing a new EIA-708 VANC packet......\n");
	uint16_t *arr = malloc(bufSize);
	if (arr == NULL)
		return -1;

	for (int i = 0; i < numWords; i++) {
		arr[i] = buf[i * 2] << 8 | buf[i * 2 + 1];
	}

	printf("Original Input\n");
	for (int i = 0; i < numWords; i++) {
		printf("%04x ", arr[i]);
	}
	printf("\n");

	int ret = klvanc_packet_parse(ctx, 13, arr, numWords);

	printf("Final output\n");
	for (int i = 0; i < vancResultCount; i++) {
		printf("%04x ", vancResult[i]);
	}
	printf("\n");

	for (int i = 0; i < vancResultCount; i++) {
		if (arr[i] != vancResult[i]) {
			fprintf(stderr, "Mismatch starting at offset 0x%02x\n", i);
			mismatch = 1;
			break;
		}
	}

	free(arr);

	if (mismatch) {
		printf("Printing mismatched structure:\n");
		failCount++;
		ret = klvanc_packet_parse(ctx, 13, vancResult, vancResultCount);
	} else {
		printf("Original and generated versions match!\n");
		passCount++;
	}

	return ret;
}

int eia708_main(int argc, char *argv[])
{
	struct klvanc_context_s *ctx;
	int ret;

	if (klvanc_context_create(&ctx) < 0) {
		fprintf(stderr, "Error initializing library context\n");
		exit(1);
	}
#ifdef SHOW_DETAIL
	ctx->verbose = 1;
#endif
	ctx->callbacks = &callbacks;
	printf("Library initialized.\n");

	ret = test_eia_708(ctx, test1, sizeof(test1));
	if (ret < 0)
		fprintf(stderr, "EIA-708B failed to parse\n");

	klvanc_context_destroy(ctx);
	printf("Library destroyed.\n");

	printf("Final result: PASS: %d/%d, Failures: %d\n",
	       passCount, passCount + failCount, failCount);
	return 0;
}
