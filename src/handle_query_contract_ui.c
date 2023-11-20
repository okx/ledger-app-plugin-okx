#include "okx_plugin.h"

// Set UI for the "Earn" screen.
static void set_okx_earn_ui(ethQueryContractUI_t *msg, const context_t *context) {
    uint8_t screenIndex = msg->screenIndex;
    uint16_t earn_token_in_count = context->earn_token_in_count;
    uint16_t earn_token_out_count = context->earn_token_out_count;

    if (screenIndex < earn_token_in_count)  // token in
    {
        strlcpy(msg->title, "Send", msg->titleLength);
        if (screenIndex == 0) {
            const uint8_t *eth_amount = context->token_in_amount1;
            uint8_t eth_amount_size = sizeof(context->token_in_amount1);
            amountToString(context->token_in_amount1,
                           eth_amount_size,
                           WEI_TO_ETHER,
                           "Token 1",
                           msg->msg,
                           msg->msgLength);
        } else if (screenIndex == 1) {
            const uint8_t *eth_amount = context->token_in_amount2;
            uint8_t eth_amount_size = sizeof(context->token_in_amount2);
            amountToString(eth_amount,
                           eth_amount_size,
                           WEI_TO_ETHER,
                           "Token 2",
                           msg->msg,
                           msg->msgLength);
        }
    } else  // token out
    {
        strlcpy(msg->title, "Receive", msg->titleLength);
        uint8_t outIndex = screenIndex - earn_token_in_count;
        if (outIndex == 0) {
            const uint8_t *eth_amount = context->token_out_amount1;
            uint8_t eth_amount_size = sizeof(context->token_out_amount1);
            amountToString(eth_amount,
                           eth_amount_size,
                           WEI_TO_ETHER,
                           "Token 1",
                           msg->msg,
                           msg->msgLength);
        } else if (outIndex == 1) {
            const uint8_t *eth_amount = context->token_out_amount2;
            uint8_t eth_amount_size = sizeof(context->token_out_amount2);
            amountToString(eth_amount,
                           eth_amount_size,
                           WEI_TO_ETHER,
                           "Token 2",
                           msg->msg,
                           msg->msgLength);
        }
    }
}

// Set UI for the "Send" screen.
static void set_send_ui(ethQueryContractUI_t *msg, const context_t *context) {
    if (context->selectorIndex == OKX_EARN_SWAP) {
        strlcpy(msg->title, "Stake", msg->titleLength);
    } else if (context->selectorIndex == OKX_DEX_UNX_SWAP_BY_ORDER_ID) {
        strlcpy(msg->title, "Swap", msg->titleLength);
    } else {
        strlcpy(msg->title, "Send", msg->titleLength);
    }

    const uint8_t *eth_amount = msg->pluginSharedRO->txContent->value.value;
    uint8_t eth_amount_size = msg->pluginSharedRO->txContent->value.length;

    // Converts the uint256 number located in `eth_amount` to its string representation and
    // copies this to `msg->msg`.
    // amountToString(eth_amount, eth_amount_size, WEI_TO_ETHER, "ETH", msg->msg, msg->msgLength);
    amountToString(eth_amount,
                   eth_amount_size,
                   WEI_TO_ETHER,
                   msg->network_ticker,
                   msg->msg,
                   msg->msgLength);
}

// Set UI for "Receive" screen.
static void set_receive_ui(ethQueryContractUI_t *msg, const context_t *context) {
    strlcpy(msg->title, "Receive Min.", msg->titleLength);

    uint8_t decimals = context->decimals;
    const char *ticker = context->ticker;

    // If the token look up failed, use the default network ticker along with the default decimals.
    if (!context->token_found) {
        decimals = WEI_TO_ETHER;
        // ticker = msg->network_ticker;
        ticker = "";
    }

    amountToString(context->token_in_amount1,
                   sizeof(context->token_in_amount1),
                   decimals,
                   ticker,
                   msg->msg,
                   msg->msgLength);
}

void handle_query_contract_ui(void *parameters) {
    ethQueryContractUI_t *msg = (ethQueryContractUI_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    // msg->title is the upper line displayed on the device.
    // msg->msg is the lower line displayed on the device.

    // Clean the display fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    msg->result = ETH_PLUGIN_RESULT_OK;

    if (context->selectorIndex == OKX_EARN_SWAP) {  // earn
        set_okx_earn_ui(msg, context);
    } else {  // dex
        switch (msg->screenIndex) {
            case 0:
                set_send_ui(msg, context);
                break;
            case 1:
                set_receive_ui(msg, context);
                break;
            // Keep this
            default:
                PRINTF("Received an invalid screenIndex\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
        }
    }
}
