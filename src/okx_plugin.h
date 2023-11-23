#pragma once

#include "eth_internals.h"
#include "eth_plugin_interface.h"
#include <string.h>

// Number of selectors defined in this plugin. Should match the enum `selector_t`.
// EDIT THIS: Put in the number of selectors your plugin is going to support.
#define NUM_SELECTORS 7

// Name of the plugin.
#define PLUGIN_NAME "OKX Web3"

// Enumeration of the different selectors possible.
// Should follow the exact same order as the array declared in main.c
typedef enum {
    OKX_EARN_SWAP = 0,
    OKX_DEX_UNX_SWAP_BY_ORDER_ID,
    OKX_DEX_UNI_SWAPV3_SWAP_TO,
    OKX_DEX_PMMV2_SWAP,
    OKX_DEX_SMART_SWAP_BY_ORDER_ID,
    OKX_DEX_BRIDGE_TO_V2,
    OKX_DEX_SWAP_BRIDGE_TO_V2,
} selector_t;

// Enumeration used to parse the smart contract data.
// EDIT THIS: Adapt the parameter names here.
typedef enum {
    UNEXPECTED_PARAMETER = 0,
    EARN_STAKE,
    DEX_UNX_SWAP_BY_ORDER_ID,
    DEX_UNI_SWAPV3_SWAP_TO,
    DEX_PMMV2_SWAP,
    DEX_SMART_SWAP_BY_ORDER_ID,
    DEX_BRIDGE_TO_V2,
    DEX_SWAP_BRIDGE_TO_V2,
} parameter;

// EDIT THIS: Rename `OKX` to be the same as the one initialized in `main.c`.
extern const uint32_t OKX_SELECTORS[NUM_SELECTORS];

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
// EDIT THIS: This struct is used by your plugin to save the parameters you parse. You
// will need to adapt this struct to your plugin.
typedef struct context_t {
    // For display.
    char ticker[MAX_TICKER_LEN];
    uint8_t decimals;
    uint8_t token_found;

    // For parsing data.
    uint8_t next_param;  // Set to be the next param we expect to parse.
    uint16_t offset;     // Offset at which the array or struct starts.
    bool go_to_offset;   // If set, will force the parsing to iterate through parameters until
                         // `offset` is reached.

    // For both parsing and display.
    selector_t selectorIndex;

    // For OKX
    const char *operation;                          // operation name
    uint16_t earn_token_in_count;             // earn token in count
    uint16_t earn_token_out_count_offset;     // earn token out offset
    uint16_t earn_token_out_count;            // earn token out count
    uint16_t earn_calls_selector_offset;      // earn calls selector offset
    uint8_t token_in_amount1[INT256_LENGTH];  // in amount or receive amount
    uint8_t token_in_amount2[INT256_LENGTH];
    uint8_t token_out_amount1[INT256_LENGTH];
    uint8_t token_out_amount2[INT256_LENGTH];
} context_t;

// Piece of code that will check that the above structure is not bigger than 5 * 32. Do not remove
// this check.
_Static_assert(sizeof(context_t) <= 5 * 32, "Structure of parameters too big.");

void handle_provide_parameter(void *parameters);
void handle_query_contract_ui(void *parameters);
void handle_init_contract(void *parameters);
void handle_finalize(void *parameters);
void handle_provide_token(void *parameters);
void handle_query_contract_id(void *parameters);