#include "okx_plugin.h"

// Invest
const u_int8_t okx_selector_deposit[] = {0xc4, 0x1a, 0x3b, 0xe8};               // 0xc41a3be8;
const u_int8_t okx_selector_deposit_and_stake[] = {0xb5, 0xf5, 0x34, 0xde};     // 0xb5f534de;
const u_int8_t okx_selector_stake[] = {0x36, 0x5f, 0xa1, 0xb1};                 // 0x365fa1b1;

// Redeem
const u_int8_t okx_selector_unstake[] = {0x3c, 0xd9, 0x0f, 0x2d};               // 0x3cd90f2d;
const u_int8_t okx_selector_unstake_and_withdraw[] = {0xad, 0x54, 0x93, 0x2d};  // 0xad54932d;
const u_int8_t okx_selector_withdraw[] = {0x8c, 0xfb, 0x1b, 0xc3};              // 0x8cfb1bc3;

// Claim
const u_int8_t okx_selector_claim_reward[] = {0xd9, 0x23, 0x8f, 0x08};          // 0xd9238f08;

static void handle_okx_earn_stake(ethPluginProvideParameter_t *msg, context_t *context) {
    if (context->go_to_offset) {
        if (msg->parameterOffset != context->offset + SELECTOR_SIZE) {
            PRINTF("-- OKX PLUGIN ************************************* go to offset return -- %d\n", context->offset);
            return;
        }
        context->go_to_offset = false;
    }
    switch (context->next_param) {
        case EARN_STAKE:
            // find call data selector line
            if (msg->parameterOffset == PARAMETER_LENGTH*1 + SELECTOR_SIZE)
            {
                u_int16_t calls_offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
                context->earn_calls_selector_offset = calls_offset / PARAMETER_LENGTH + 5;
                PRINTF("-- OKX PLUGIN ************************************* EARN_STAKE calls selector offset: %d\n", context->earn_calls_selector_offset);
            }

            // parse selector
            if (context->earn_calls_selector_offset != 0 && msg->parameterOffset == PARAMETER_LENGTH*context->earn_calls_selector_offset + SELECTOR_SIZE)
            {
                u_int8_t selector[SELECTOR_SIZE];
                copy_parameter(selector, msg->parameter, SELECTOR_SIZE);
                if (memcmp(selector, okx_selector_deposit, SELECTOR_SIZE) == 0 ||
                    memcmp(selector, okx_selector_deposit_and_stake, SELECTOR_SIZE) == 0 ||
                    memcmp(selector, okx_selector_stake, SELECTOR_SIZE) == 0)
                {
                    context->operation = "Invest";
                }else if (memcmp(selector, okx_selector_unstake, SELECTOR_SIZE) == 0 ||
                          memcmp(selector, okx_selector_unstake_and_withdraw, SELECTOR_SIZE) == 0 ||
                          memcmp(selector, okx_selector_withdraw, SELECTOR_SIZE) == 0)
                {
                    context->operation = "Redeem";
                }else if (memcmp(selector, okx_selector_claim_reward, SELECTOR_SIZE) == 0)
                {
                    context->operation = "Claim";
                }
            }
            
            // find token in/out count line
            if (msg->parameterOffset == PARAMETER_LENGTH*4 + SELECTOR_SIZE)
            {
                // get token in count
                context->earn_token_in_count = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
                PRINTF("-- OKX PLUGIN ************************************* EARN_STAKE token in count: %d\n", context->earn_token_in_count);
                // token out line number，5 is the initial number, a fixed value.
                u_int16_t token_out_count_line = 5 + context->earn_token_in_count * 3;
                PRINTF("-- OKX PLUGIN ************************************* EARN_STAKE token in count line: %d\n", token_out_count_line);
                context->earn_token_out_count_offset = token_out_count_line;
            }

            // parse token in data
            if (context->earn_token_in_count != 0)
            {
                // get token in data
                if (msg->parameterOffset > PARAMETER_LENGTH*5 + SELECTOR_SIZE &&
                    msg->parameterOffset <= PARAMETER_LENGTH*(5 + context->earn_token_in_count * 3) + SELECTOR_SIZE)
                {
                    u_int16_t current_line = (msg->parameterOffset - SELECTOR_SIZE) / PARAMETER_LENGTH;
                    u_int16_t current_sub_line = (current_line - 5) % 3;
                    if (current_sub_line == 1) { // token amount
                        u_int16_t line = (current_line - 5) / 3;
                        if (line == 0)
                        {
                            copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
                        }else if (line == 1)
                        {
                            copy_parameter(context->token_in_amount2, msg->parameter, sizeof(context->token_in_amount2));
                        }
                    }
                    PRINTF("-- OKX PLUGIN ************************************* TOKEN IN DATA current line -- %d\n", current_line);
                }
            }

            // token out data
            if (context->earn_token_out_count_offset != 0 && msg->parameterOffset == PARAMETER_LENGTH*context->earn_token_out_count_offset + SELECTOR_SIZE)
            {
                context->earn_token_out_count = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
                PRINTF("-- OKX PLUGIN ************************************* Token out count: %d\n", context->earn_token_out_count);
            }

            // parse token out data
            if (context->earn_token_out_count_offset != 0 && context->earn_token_out_count != 0)
            {
                // get token out data
                if (msg->parameterOffset > PARAMETER_LENGTH*context->earn_token_out_count_offset + SELECTOR_SIZE 
                && msg->parameterOffset <= PARAMETER_LENGTH*(context->earn_token_out_count_offset + context->earn_token_out_count * 2) + SELECTOR_SIZE)
                {
                    u_int16_t current_line = (msg->parameterOffset - SELECTOR_SIZE) / PARAMETER_LENGTH;
                    u_int16_t current_sub_line = (current_line - context->earn_token_out_count_offset) % 2;
                    if (current_sub_line == 0) { // token contract amount
                        u_int16_t line = (current_line - context->earn_token_out_count_offset) / 2;

                        PRINTF("-- OKX PLUGIN ************************************* TOKEN OUT DATA Line -- %d\n", (current_line - context->earn_token_out_count_offset));

                        if (line == 1)
                        {
                            copy_parameter(context->token_out_amount1, msg->parameter, sizeof(context->token_out_amount1));
                        }else if (line == 2)
                        {
                            copy_parameter(context->token_out_amount2, msg->parameter, sizeof(context->token_out_amount2));
                        }
                    }
                    PRINTF("-- OKX PLUGIN ************************************* TOKEN OUT DATA current line -- %d\n", current_line);
                    PRINTF("-- OKX PLUGIN ************************************* TOKEN OUT DATA current sub line -- %d\n", current_sub_line);
                }
            }
            context->next_param = EARN_STAKE;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_okx_dex_unx_swap_by_order_id(ethPluginProvideParameter_t *msg, context_t *context) {
    if (context->go_to_offset) {
        if (msg->parameterOffset != context->offset + SELECTOR_SIZE) {
            PRINTF("-- OKX PLUGIN ************************************* go to offset return -- %d\n", context->offset);
            return;
        }
        context->go_to_offset = false;
    }
    switch (context->next_param) {
        case DEX_UNX_SWAP_BY_ORDER_ID:
            // find minReturn line
            if (msg->parameterOffset == PARAMETER_LENGTH*2 + SELECTOR_SIZE)
            {
                PRINTF("-- OKX PLUGIN ************************************* DEX_UNX_SWAP_BY_ORDER_ID -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
                copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
            }
            context->next_param = DEX_UNX_SWAP_BY_ORDER_ID;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_okx_dex_uni_swapv3_swap_to(ethPluginProvideParameter_t *msg, context_t *context) {
    switch (context->next_param) {
        case DEX_UNI_SWAPV3_SWAP_TO:
            // find minReturn line
            if (msg->parameterOffset == PARAMETER_LENGTH*2 + SELECTOR_SIZE)
            {
                PRINTF("-- OKX PLUGIN ************************************* DEX_UNI_SWAPV3_SWAP_TO -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
                copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
            }
            context->next_param = DEX_UNI_SWAPV3_SWAP_TO;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_okx_dex_pmmv2_swap(ethPluginProvideParameter_t *msg, context_t *context) {
    switch (context->next_param) {
        case DEX_PMMV2_SWAP:
            // find minReturn line
            if (msg->parameterOffset == PARAMETER_LENGTH*2 + SELECTOR_SIZE)
            {
                PRINTF("-- OKX PLUGIN ************************************* DEX_PMMV2_SWAP -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
                copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
            }
            context->next_param = DEX_PMMV2_SWAP;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_okx_dex_smart_swap_by_order_id(ethPluginProvideParameter_t *msg, context_t *context) {
    switch (context->next_param) {
        case DEX_SMART_SWAP_BY_ORDER_ID:
            // find minReturn line
            if (msg->parameterOffset == PARAMETER_LENGTH*4 + SELECTOR_SIZE)
            {
                PRINTF("-- OKX PLUGIN ************************************* DEX_SMART_SWAP_BY_ORDER_ID -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
                copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
            }
            context->next_param = DEX_SMART_SWAP_BY_ORDER_ID;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_okx_dex_bridge_to_v2(ethPluginProvideParameter_t *msg, context_t *context) {
    switch (context->next_param) {
        case DEX_BRIDGE_TO_V2:
            // find minReturn line
            if (msg->parameterOffset == PARAMETER_LENGTH*5 + SELECTOR_SIZE)
            {
                PRINTF("-- OKX PLUGIN ************************************* DEX_BRIDGE_TO_V2 -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
                copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
            }
            context->next_param = DEX_BRIDGE_TO_V2;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_okx_dex_swap_bridge_to_v2(ethPluginProvideParameter_t *msg, context_t *context) {
    switch (context->next_param) {
        case DEX_SWAP_BRIDGE_TO_V2:
            PRINTF("-- OKX PLUGIN ************************************* DEX_SWAP_BRIDGE_TO_V2 -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
            // find minReturn line
            if (msg->parameterOffset == PARAMETER_LENGTH*6 + SELECTOR_SIZE)
            {
                PRINTF("-- OKX PLUGIN ************************************* DEX_SWAP_BRIDGE_TO_V2 -- %.*H\n", PARAMETER_LENGTH, msg->parameter);
                copy_parameter(context->token_in_amount1, msg->parameter, sizeof(context->token_in_amount1));
            }
            context->next_param = DEX_SWAP_BRIDGE_TO_V2;
            break;
        case UNEXPECTED_PARAMETER:
            PRINTF("-- OKX PLUGIN ************************************* UNEXPECTED_PARAMETER --\n");
            break;
        // Keep this
        default:
            PRINTF("-- OKX PLUGIN ************************************* Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

void handle_provide_parameter(void *parameters) {
    ethPluginProvideParameter_t *msg = (ethPluginProvideParameter_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;
    // We use `%.*H`: it's a utility function to print bytes. You first give
    // the number of bytes you wish to print (in this case, `PARAMETER_LENGTH`) and then
    // the address (here `msg->parameter`).
    PRINTF("plugin provide parameter: offset %d\nBytes: %.*H\n",
           msg->parameterOffset,
           PARAMETER_LENGTH,
           msg->parameter);

    msg->result = ETH_PLUGIN_RESULT_OK;

    // EDIT THIS: adapt the cases and the names of the functions.
    switch (context->selectorIndex) {
        case OKX_EARN_SWAP:
            handle_okx_earn_stake(msg, context);
            break;
        case OKX_DEX_UNX_SWAP_BY_ORDER_ID:
            handle_okx_dex_unx_swap_by_order_id(msg, context);
            break;
        case OKX_DEX_UNI_SWAPV3_SWAP_TO:
            handle_okx_dex_uni_swapv3_swap_to(msg, context);
            break;
        case OKX_DEX_PMMV2_SWAP:
            handle_okx_dex_pmmv2_swap(msg, context);
            break;
        case OKX_DEX_SMART_SWAP_BY_ORDER_ID:
            handle_okx_dex_smart_swap_by_order_id(msg, context);
            break;
        case OKX_DEX_BRIDGE_TO_V2:
            handle_okx_dex_bridge_to_v2(msg, context);
            break;
        case OKX_DEX_SWAP_BRIDGE_TO_V2:
            handle_okx_dex_swap_bridge_to_v2(msg, context);
            break;
        default:
            PRINTF("Selector Index not supported: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}