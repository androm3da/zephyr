/*
 * Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <hvx.h>

LOG_MODULE_REGISTER(hvx_test, LOG_LEVEL_INF);

/* Test scenarios */
#ifndef HVX_TEST_SCENARIO
#define HVX_TEST_SCENARIO 1
#endif

/**
 * @brief Test HVX availability and basic initialization
 */
static void test_hvx_availability(void)
{
	printf("=== HVX Availability Test ===\n");

	if (hvx_is_available()) {
		printf("✓ HVX is available on this hardware\n");
	} else {
		printf("✗ HVX is not available on this hardware\n");
		return;
	}

	printf("HVX availability test completed\n\n");
}

/**
 * @brief Test HVX context allocation and deallocation
 */
static void test_hvx_context_management(void)
{
	int ret;
	struct hvx_context *ctx;

	printf("=== HVX Context Management Test ===\n");

	/* Test context allocation */
	ret = hvx_context_alloc();
	if (ret == 0) {
		printf("✓ HVX context allocated successfully\n");
	} else {
		printf("✗ HVX context allocation failed: %d\n", ret);
		return;
	}

	/* Get current context */
	ctx = hvx_get_current_context();
	if (ctx != NULL) {
		printf("✓ HVX context retrieved: context_num=%d\n", ctx->context_num);
	} else {
		printf("✗ Failed to retrieve HVX context\n");
	}

	/* Test context save/restore */
	ret = hvx_context_save();
	if (ret == 0) {
		printf("✓ HVX context saved successfully\n");
	} else {
		printf("✗ HVX context save failed: %d\n", ret);
	}

	ret = hvx_context_restore();
	if (ret == 0) {
		printf("✓ HVX context restored successfully\n");
	} else {
		printf("✗ HVX context restore failed: %d\n", ret);
	}

	/* Free context */
	hvx_context_free();
	printf("✓ HVX context freed\n");

	printf("HVX context management test completed\n\n");
}

/**
 * @brief Test HVX enable/disable functionality
 */
static void test_hvx_enable_disable(void)
{
	int ret;

	printf("=== HVX Enable/Disable Test ===\n");

	/* Enable HVX */
	ret = hvx_enable();
	if (ret == 0) {
		printf("✓ HVX enabled successfully\n");
	} else {
		printf("✗ HVX enable failed: %d\n", ret);
		return;
	}

	/* Verify context is allocated */
	struct hvx_context *ctx = hvx_get_current_context();
	if (ctx != NULL) {
		printf("✓ HVX context automatically allocated: context_num=%d\n",
		       ctx->context_num);
	} else {
		printf("✗ HVX context not allocated after enable\n");
	}

	/* Disable HVX */
	hvx_disable();
	printf("✓ HVX disabled successfully\n");

	printf("HVX enable/disable test completed\n\n");
}

/**
 * @brief Test multiple HVX context allocation (stress test)
 */
static void test_hvx_multiple_contexts(void)
{
	int ret;
	int allocated_contexts = 0;

	printf("=== HVX Multiple Contexts Test ===\n");

	/* Try to allocate maximum number of contexts */
	for (int i = 0; i < CONFIG_HEXAGON_HVX_MAX_CONTEXTS + 2; i++) {
		ret = hvx_context_alloc();
		if (ret == 0) {
			allocated_contexts++;
			printf("✓ Context %d allocated\n", i);
		} else {
			printf("✗ Context %d allocation failed: %d\n", i, ret);
			break;
		}
	}

	printf("Total contexts allocated: %d (max=%d)\n",
	       allocated_contexts, CONFIG_HEXAGON_HVX_MAX_CONTEXTS);

	/* Free all contexts */
	for (int i = 0; i < allocated_contexts; i++) {
		hvx_context_free();
	}

	printf("All contexts freed\n");
	printf("HVX multiple contexts test completed\n\n");
}

/**
 * @brief Simulate HVX vector operations (placeholder)
 */
static void test_hvx_vector_operations(void)
{
	int ret;

	printf("=== HVX Vector Operations Test ===\n");

	/* Enable HVX */
	ret = hvx_enable();
	if (ret != 0) {
		printf("✗ Failed to enable HVX for vector operations: %d\n", ret);
		return;
	}

	printf("✓ HVX enabled for vector operations\n");

	/* TODO: Add actual HVX vector operations when compiler support is available
	 * For now, we just test the infrastructure
	 */

	/* Simulate some work that would use HVX vectors */
	printf("Simulating HVX vector operations...\n");
	k_msleep(10); /* Simulate work */

	/* Save context to test the save mechanism */
	ret = hvx_context_save();
	if (ret == 0) {
		printf("✓ HVX context saved after operations\n");
	} else {
		printf("✗ HVX context save failed: %d\n", ret);
	}

	hvx_disable();
	printf("✓ HVX disabled after operations\n");

	printf("HVX vector operations test completed\n\n");
}

int main(void)
{
	printf("=== Hexagon HVX Test Application ===\n");
	printf("Testing HVX functionality for Hexagon architecture\n");
	printf("Test scenario: %d\n\n", HVX_TEST_SCENARIO);

	/* Always test basic availability first */
	test_hvx_availability();

	if (!hvx_is_available()) {
		printf("HVX not available, skipping remaining tests\n");
		return 0;
	}

	switch (HVX_TEST_SCENARIO) {
	case 1:
		test_hvx_context_management();
		break;
	case 2:
		test_hvx_enable_disable();
		break;
	case 3:
		test_hvx_multiple_contexts();
		break;
	case 4:
		test_hvx_vector_operations();
		break;
	default:
		/* Run all tests */
		test_hvx_context_management();
		test_hvx_enable_disable();
		test_hvx_multiple_contexts();
		test_hvx_vector_operations();
		break;
	}

	printf("=== HVX Test Application Completed ===\n");
	return 0;
}
