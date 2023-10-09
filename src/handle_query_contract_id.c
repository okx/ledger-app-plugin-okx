#include "okx_plugin.h"

// Sets the first screen to display.
void handle_query_contract_id(void *parameters) {
    ethQueryContractID_t *msg = (ethQueryContractID_t *) parameters;
    const context_t *context = (const context_t *) msg->pluginContext;
    // msg->name will be the upper sentence displayed on the screen.
    // msg->version will be the lower sentence displayed on the screen.

    // For the first screen, display the plugin name.
    // strlcpy(msg->name, PLUGIN_NAME, msg->nameLength);
    if (context->selectorIndex == OKX_EARN_SWAP) {
        strlcpy(msg->name, "OKX Web3 DeFi", msg->nameLength);
        if (context->operation != NULL) {
            strlcpy(msg->version, context->operation, msg->versionLength);
        } else {
            strlcpy(msg->version, "Earn", msg->versionLength);
        }
        msg->result = ETH_PLUGIN_RESULT_OK;
    } else if (context->selectorIndex == OKX_DEX_UNX_SWAP_BY_ORDER_ID ||
               context->selectorIndex == OKX_DEX_UNI_SWAPV3_SWAP_TO ||
               context->selectorIndex == OKX_DEX_PMMV2_SWAP ||
               context->selectorIndex == OKX_DEX_SMART_SWAP_BY_ORDER_ID) {
        strlcpy(msg->name, "OKX Web3 Dex", msg->nameLength);
        strlcpy(msg->version, "Swap", msg->versionLength);
        msg->result = ETH_PLUGIN_RESULT_OK;
    } else if (context->selectorIndex == OKX_DEX_BRIDGE_TO_V2 ||
               context->selectorIndex == OKX_DEX_SWAP_BRIDGE_TO_V2) {
        strlcpy(msg->name, "OKX Web3 Dex", msg->nameLength);
        strlcpy(msg->version, "Cross-chain Swap", msg->versionLength);
        msg->result = ETH_PLUGIN_RESULT_OK;
    } else {
        PRINTF("Selector index: %d not supported\n", context->selectorIndex);
        msg->result = ETH_PLUGIN_RESULT_ERROR;
    }
}