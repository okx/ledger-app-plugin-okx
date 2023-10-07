#include "okx_plugin.h"

void handle_finalize(void *parameters) {
    ethPluginFinalize_t *msg = (ethPluginFinalize_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    msg->uiType = ETH_UI_TYPE_GENERIC;

    // Set the total number of screen you will need.
    msg->numScreens = 0;
    if (context->selectorIndex == OKX_EARN_SWAP) {
        // no tokenout
        msg->numScreens = msg->numScreens + context->earn_token_in_count;
        msg->numScreens = msg->numScreens + context->earn_token_out_count;
    } else if (context->selectorIndex == OKX_DEX_UNX_SWAP_BY_ORDER_ID ||
               context->selectorIndex == OKX_DEX_UNI_SWAPV3_SWAP_TO ||
               context->selectorIndex == OKX_DEX_PMMV2_SWAP ||
               context->selectorIndex == OKX_DEX_SMART_SWAP_BY_ORDER_ID ||
               context->selectorIndex == OKX_DEX_BRIDGE_TO_V2 ||
               context->selectorIndex == OKX_DEX_SWAP_BRIDGE_TO_V2) {
        msg->numScreens = 2;
    }
    // EDIT THIS: set `tokenLookup1` (and maybe `tokenLookup2`) to point to
    // token addresses you will info for (such as decimals, ticker...).
    // msg->tokenLookup1 = context->token_received;
    msg->result = ETH_PLUGIN_RESULT_OK;
}
