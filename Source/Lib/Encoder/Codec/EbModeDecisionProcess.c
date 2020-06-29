/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include <stdlib.h>

#include "EbUtility.h"
#include "EbModeDecisionProcess.h"
#include "EbLambdaRateTables.h"

#if MEM_OPT_PALETTE
int svt_av1_allow_palette(int allow_palette, BlockSize sb_type);
#endif
static void mode_decision_context_dctor(EbPtr p) {
    ModeDecisionContext *obj = (ModeDecisionContext *)p;
#if SB64_MEM_OPT
    uint32_t block_max_count_sb = (obj->sb_size == MAX_SB_SIZE) ? BLOCK_MAX_COUNT_SB_128 :
                                                                  BLOCK_MAX_COUNT_SB_64;
#endif
    for (int cd = 0; cd < MAX_PAL_CAND; cd++)
        if (obj->palette_cand_array[cd].color_idx_map)
            EB_FREE_ARRAY(obj->palette_cand_array[cd].color_idx_map);
    for (uint32_t cand_index = 0; cand_index < MODE_DECISION_CANDIDATE_MAX_COUNT; ++cand_index) {
#if !MEM_OPT_PALETTE
        if (obj->fast_candidate_ptr_array[cand_index]->palette_info.color_idx_map)
#endif
#if SB64_MEM_OPT
            for (uint32_t coded_leaf_index = 0; coded_leaf_index < block_max_count_sb;
#else
            for (uint32_t coded_leaf_index = 0; coded_leaf_index < BLOCK_MAX_COUNT_SB_128;
#endif
                 ++coded_leaf_index)
                if (obj->md_blk_arr_nsq[coded_leaf_index].palette_info.color_idx_map)
                    EB_FREE_ARRAY(obj->md_blk_arr_nsq[coded_leaf_index].palette_info.color_idx_map);
#if !MEM_OPT_PALETTE
        EB_FREE_ARRAY(obj->fast_candidate_ptr_array[cand_index]->palette_info.color_idx_map);
#endif
    }
    EB_FREE_ARRAY(obj->ref_best_ref_sq_table);
    EB_FREE_ARRAY(obj->ref_best_cost_sq_table);
    EB_FREE_ARRAY(obj->above_txfm_context);
    EB_FREE_ARRAY(obj->left_txfm_context);
#if NO_ENCDEC //SB128_TODO to upgrade
    int coded_leaf_index;
#if SB64_MEM_OPT
    for (coded_leaf_index = 0; coded_leaf_index < obj->sb_size; ++coded_leaf_index) {
#else
    for (coded_leaf_index = 0; coded_leaf_index < BLOCK_MAX_COUNT_SB_128; ++coded_leaf_index) {
#endif
        EB_DELETE(obj->md_blk_arr_nsq[coded_leaf_index].recon_tmp);
        EB_DELETE(obj->md_blk_arr_nsq[coded_leaf_index].coeff_tmp);
    }
#endif
    EB_DELETE_PTR_ARRAY(obj->candidate_buffer_ptr_array, MAX_NFL_BUFF);
    EB_FREE_ARRAY(obj->candidate_buffer_tx_depth_1->candidate_ptr);
    EB_DELETE(obj->candidate_buffer_tx_depth_1);
    EB_FREE_ARRAY(obj->candidate_buffer_tx_depth_2->candidate_ptr);
    EB_DELETE(obj->candidate_buffer_tx_depth_2);
    EB_DELETE(obj->trans_quant_buffers_ptr);
    if (obj->hbd_mode_decision > EB_8_BIT_MD) EB_FREE_ALIGNED_ARRAY(obj->cfl_temp_luma_recon16bit);
    if (obj->hbd_mode_decision != EB_10_BIT_MD) EB_FREE_ALIGNED_ARRAY(obj->cfl_temp_luma_recon);
    if (obj->is_md_rate_estimation_ptr_owner) EB_FREE_ARRAY(obj->md_rate_estimation_ptr);
    EB_FREE_ARRAY(obj->fast_candidate_array);
    EB_FREE_ARRAY(obj->fast_candidate_ptr_array);
    EB_FREE_ARRAY(obj->fast_cost_array);
    EB_FREE_ARRAY(obj->full_cost_array);
    EB_FREE_ARRAY(obj->full_cost_skip_ptr);
    EB_FREE_ARRAY(obj->full_cost_merge_ptr);

#if CLEAN_UP_SB_DATA_3
    if (obj->md_local_blk_unit) {
        if (obj->hbd_mode_decision > EB_8_BIT_MD) {
            EB_FREE_ARRAY(obj->md_local_blk_unit[0].neigh_left_recon_16bit[0]);
            EB_FREE_ARRAY(obj->md_local_blk_unit[0].neigh_top_recon_16bit[0]);
        }
        if (obj->hbd_mode_decision != EB_10_BIT_MD) {
            EB_FREE_ARRAY(obj->md_local_blk_unit[0].neigh_left_recon[0]);
            EB_FREE_ARRAY(obj->md_local_blk_unit[0].neigh_top_recon[0]);
        }
    }
    if (obj->md_blk_arr_nsq) {
        EB_FREE_ARRAY(obj->md_blk_arr_nsq[0].av1xd);
    }
#else
    if (obj->md_blk_arr_nsq) {
        EB_FREE_ARRAY(obj->md_blk_arr_nsq[0].av1xd);
        if (obj->hbd_mode_decision > EB_8_BIT_MD) {
            EB_FREE_ARRAY(obj->md_blk_arr_nsq[0].neigh_left_recon_16bit[0]);
            EB_FREE_ARRAY(obj->md_blk_arr_nsq[0].neigh_top_recon_16bit[0]);
        }
        if (obj->hbd_mode_decision != EB_10_BIT_MD) {
            EB_FREE_ARRAY(obj->md_blk_arr_nsq[0].neigh_left_recon[0]);
            EB_FREE_ARRAY(obj->md_blk_arr_nsq[0].neigh_top_recon[0]);
        }
    }
#endif
    EB_FREE_ARRAY(obj->md_local_blk_unit);

    EB_FREE_ARRAY(obj->md_blk_arr_nsq);
    EB_FREE_ARRAY(obj->md_ep_pipe_sb);
#if DEPTH_PART_CLEAN_UP
    EB_FREE_ARRAY(obj->mdc_sb_array);
#endif
#if UNIFY_TXT
    for (uint32_t txt_itr = 0; txt_itr < TX_TYPES; ++txt_itr) {
        EB_DELETE(obj->recon_coeff_ptr[txt_itr]);
        EB_DELETE(obj->recon_ptr[txt_itr]);
    }
#endif
#if CAND_MEM_OPT
    EB_DELETE(obj->prediction_ptr_temp);
    EB_DELETE(obj->cfl_temp_prediction_ptr);
    EB_DELETE(obj->residual_quant_coeff_ptr);
#endif

#if MEM_OPT_MD_BUF_DESC
    EB_DELETE(obj->temp_residual_ptr);
    EB_DELETE(obj->temp_recon_ptr);
#endif
}

/******************************************************
 * Mode Decision Context Constructor
 ******************************************************/
EbErrorType mode_decision_context_ctor(ModeDecisionContext *context_ptr, EbColorFormat color_format,
#if SB64_MEM_OPT
                                       uint8_t sb_size,
#endif
                                       EbFifo *mode_decision_configuration_input_fifo_ptr,
                                       EbFifo *mode_decision_output_fifo_ptr,
                                       uint8_t enable_hbd_mode_decision, uint8_t cfg_palette) {
    uint32_t buffer_index;
    uint32_t cand_index;

#if SB64_MEM_OPT
    uint32_t block_max_count_sb = (sb_size == MAX_SB_SIZE) ? BLOCK_MAX_COUNT_SB_128 :
                                                             BLOCK_MAX_COUNT_SB_64;
    context_ptr->sb_size = sb_size;
#endif

    (void)color_format;

    context_ptr->dctor             = mode_decision_context_dctor;
    context_ptr->hbd_mode_decision = enable_hbd_mode_decision;

    // Input/Output System Resource Manager FIFOs
    context_ptr->mode_decision_configuration_input_fifo_ptr =
        mode_decision_configuration_input_fifo_ptr;
    context_ptr->mode_decision_output_fifo_ptr = mode_decision_output_fifo_ptr;

    // Cfl scratch memory
    if (context_ptr->hbd_mode_decision > EB_8_BIT_MD)
#if SB64_MEM_OPT
        EB_MALLOC_ALIGNED(context_ptr->cfl_temp_luma_recon16bit, sizeof(uint16_t) * sb_size * sb_size);
#else
        EB_MALLOC_ALIGNED(context_ptr->cfl_temp_luma_recon16bit, sizeof(uint16_t) * 128 * 128);
#endif
    if (context_ptr->hbd_mode_decision != EB_10_BIT_MD)
#if SB64_MEM_OPT
        EB_MALLOC_ALIGNED(context_ptr->cfl_temp_luma_recon, sizeof(uint8_t) * sb_size * sb_size);
#else
        EB_MALLOC_ALIGNED(context_ptr->cfl_temp_luma_recon, sizeof(uint8_t) * 128 * 128);
#endif
    // MD rate Estimation tables
    EB_MALLOC_ARRAY(context_ptr->md_rate_estimation_ptr, 1);
    context_ptr->is_md_rate_estimation_ptr_owner = EB_TRUE;

#if SB64_MEM_OPT
    EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit, block_max_count_sb);
    EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq, block_max_count_sb);
    EB_MALLOC_ARRAY(context_ptr->md_ep_pipe_sb, block_max_count_sb);
#else
    EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit, BLOCK_MAX_COUNT_SB_128);
    EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq, BLOCK_MAX_COUNT_SB_128);
    EB_MALLOC_ARRAY(context_ptr->md_ep_pipe_sb, BLOCK_MAX_COUNT_SB_128);
#endif

    // Fast Candidate Array
    EB_MALLOC_ARRAY(context_ptr->fast_candidate_array, MODE_DECISION_CANDIDATE_MAX_COUNT);

    EB_MALLOC_ARRAY(context_ptr->fast_candidate_ptr_array, MODE_DECISION_CANDIDATE_MAX_COUNT);

    for (cand_index = 0; cand_index < MODE_DECISION_CANDIDATE_MAX_COUNT; ++cand_index) {
        context_ptr->fast_candidate_ptr_array[cand_index] =
            &context_ptr->fast_candidate_array[cand_index];
        context_ptr->fast_candidate_ptr_array[cand_index]->md_rate_estimation_ptr =
            context_ptr->md_rate_estimation_ptr;
#if MEM_OPT_PALETTE
            context_ptr->fast_candidate_ptr_array[cand_index]->palette_info = NULL;
#else
        if (cfg_palette)
            EB_MALLOC_ARRAY(
                context_ptr->fast_candidate_ptr_array[cand_index]->palette_info.color_idx_map,
                MAX_PALETTE_SQUARE);
        else
            context_ptr->fast_candidate_ptr_array[cand_index]->palette_info.color_idx_map = NULL;
#endif
    }
    for (int cd = 0; cd < MAX_PAL_CAND; cd++)
        if (cfg_palette)
            EB_MALLOC_ARRAY(context_ptr->palette_cand_array[cd].color_idx_map, MAX_PALETTE_SQUARE);
        else
            context_ptr->palette_cand_array[cd].color_idx_map = NULL;
    // Transform and Quantization Buffers
#if SB64_MEM_OPT
    EB_NEW(context_ptr->trans_quant_buffers_ptr, eb_trans_quant_buffers_ctor, sb_size);
#else
    EB_NEW(context_ptr->trans_quant_buffers_ptr, eb_trans_quant_buffers_ctor);
#endif

    // Cost Arrays
    EB_MALLOC_ARRAY(context_ptr->fast_cost_array, MAX_NFL_BUFF);
    EB_MALLOC_ARRAY(context_ptr->full_cost_array, MAX_NFL_BUFF);
    EB_MALLOC_ARRAY(context_ptr->full_cost_skip_ptr, MAX_NFL_BUFF);
    EB_MALLOC_ARRAY(context_ptr->full_cost_merge_ptr, MAX_NFL_BUFF);
#if !MEM_OPT_MD_BUF_DESC
    // Candidate Buffers
    EB_ALLOC_PTR_ARRAY(context_ptr->candidate_buffer_ptr_array, MAX_NFL_BUFF);
    for (buffer_index = 0; buffer_index < MAX_NFL_BUFF; ++buffer_index) {
#if !SB64_MEM_OPT
        EB_NEW(context_ptr->candidate_buffer_ptr_array[buffer_index],
               mode_decision_candidate_buffer_ctor,
               context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT,
               &(context_ptr->fast_cost_array[buffer_index]),
               &(context_ptr->full_cost_array[buffer_index]),
               &(context_ptr->full_cost_skip_ptr[buffer_index]),
               &(context_ptr->full_cost_merge_ptr[buffer_index]));
#else
        EB_NEW(context_ptr->candidate_buffer_ptr_array[buffer_index],
               mode_decision_candidate_buffer_ctor,
               context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT,
               sb_size,
               &(context_ptr->fast_cost_array[buffer_index]),
               &(context_ptr->full_cost_array[buffer_index]),
               &(context_ptr->full_cost_skip_ptr[buffer_index]),
               &(context_ptr->full_cost_merge_ptr[buffer_index]));
#endif
    }
#endif

#if !SB64_MEM_OPT
    EB_NEW(context_ptr->candidate_buffer_tx_depth_1,
           mode_decision_scratch_candidate_buffer_ctor,
           context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT);
#else
    EB_NEW(context_ptr->candidate_buffer_tx_depth_1,
           mode_decision_scratch_candidate_buffer_ctor,
           sb_size,
           context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT);
#endif

    EB_ALLOC_PTR_ARRAY(context_ptr->candidate_buffer_tx_depth_1->candidate_ptr, 1);

#if !SB64_MEM_OPT
    EB_NEW(context_ptr->candidate_buffer_tx_depth_2,
           mode_decision_scratch_candidate_buffer_ctor,
           context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT);
#else
    EB_NEW(context_ptr->candidate_buffer_tx_depth_2,
           mode_decision_scratch_candidate_buffer_ctor,
           sb_size,
           context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT);
#endif

    EB_ALLOC_PTR_ARRAY(context_ptr->candidate_buffer_tx_depth_2->candidate_ptr, 1);

#if CLEAN_UP_SB_DATA_3
    context_ptr->md_local_blk_unit[0].neigh_left_recon[0] = NULL;
    context_ptr->md_local_blk_unit[0].neigh_top_recon[0] = NULL;
    context_ptr->md_local_blk_unit[0].neigh_left_recon_16bit[0] = NULL;
    context_ptr->md_local_blk_unit[0].neigh_top_recon_16bit[0] = NULL;
    uint16_t sz = sizeof(uint16_t);
    if (context_ptr->hbd_mode_decision > EB_8_BIT_MD) {
#if SB64_MEM_OPT
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_left_recon_16bit[0],
            block_max_count_sb * sb_size * 3 * sz);
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_top_recon_16bit[0],
            block_max_count_sb * sb_size * 3 * sz);
#else
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_left_recon_16bit[0],
            BLOCK_MAX_COUNT_SB_128 * 128 * 3 * sz);
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_top_recon_16bit[0],
            BLOCK_MAX_COUNT_SB_128 * 128 * 3 * sz);
#endif
    }
    if (context_ptr->hbd_mode_decision != EB_10_BIT_MD) {
#if SB64_MEM_OPT
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_left_recon[0],
            block_max_count_sb * sb_size * 3);
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_top_recon[0],
            block_max_count_sb * sb_size * 3);
#else
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_left_recon[0],
            BLOCK_MAX_COUNT_SB_128 * 128 * 3);
        EB_MALLOC_ARRAY(context_ptr->md_local_blk_unit[0].neigh_top_recon[0],
            BLOCK_MAX_COUNT_SB_128 * 128 * 3);
#endif
    }
#if CLEAN_UP_SB_DATA_7
    uint32_t coded_leaf_index;
#else
    uint32_t coded_leaf_index, txb_index;
#endif

#if SB64_MEM_OPT
    for (coded_leaf_index = 0; coded_leaf_index < block_max_count_sb; ++coded_leaf_index) {
#else
    for (coded_leaf_index = 0; coded_leaf_index < BLOCK_MAX_COUNT_SB_128; ++coded_leaf_index) {
#endif
        for (int i = 0; i < 3; i++) {
#if SB64_MEM_OPT
            size_t offset = (coded_leaf_index * sb_size * 3 + i * sb_size) * sz;
#else
            size_t offset = (coded_leaf_index * 128 * 3 + i * 128) * sz;
#endif
            context_ptr->md_local_blk_unit[coded_leaf_index].neigh_left_recon_16bit[i] =
                context_ptr->md_local_blk_unit[0].neigh_left_recon_16bit[0] + offset;
            context_ptr->md_local_blk_unit[coded_leaf_index].neigh_top_recon_16bit[i] =
                context_ptr->md_local_blk_unit[0].neigh_top_recon_16bit[0] + offset;
        }
        for (int i = 0; i < 3; i++) {
#if SB64_MEM_OPT
            size_t offset = coded_leaf_index * sb_size * 3 + i * sb_size;
#else
            size_t offset = coded_leaf_index * 128 * 3 + i * 128;
#endif
            context_ptr->md_local_blk_unit[coded_leaf_index].neigh_left_recon[i] =
                context_ptr->md_local_blk_unit[0].neigh_left_recon[0] + offset;
            context_ptr->md_local_blk_unit[coded_leaf_index].neigh_top_recon[i] =
                context_ptr->md_local_blk_unit[0].neigh_top_recon[0] + offset;
        }
    }
#endif

    context_ptr->md_blk_arr_nsq[0].av1xd                     = NULL;
#if !CLEAN_UP_SB_DATA_3
    context_ptr->md_blk_arr_nsq[0].neigh_left_recon[0]       = NULL;
    context_ptr->md_blk_arr_nsq[0].neigh_top_recon[0]        = NULL;
    context_ptr->md_blk_arr_nsq[0].neigh_left_recon_16bit[0] = NULL;
    context_ptr->md_blk_arr_nsq[0].neigh_top_recon_16bit[0]  = NULL;
#endif
#if SB64_MEM_OPT
    EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq[0].av1xd, block_max_count_sb);
#else
    EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq[0].av1xd, BLOCK_MAX_COUNT_SB_128);
#endif

#if DEPTH_PART_CLEAN_UP
    EB_MALLOC_ARRAY(context_ptr->mdc_sb_array, 1);
#endif

#if !CLEAN_UP_SB_DATA_3
    uint16_t sz = sizeof(uint16_t);
    if (context_ptr->hbd_mode_decision > EB_8_BIT_MD) {
        EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq[0].neigh_left_recon_16bit[0],
                        BLOCK_MAX_COUNT_SB_128 * 128 * 3 * sz);
        EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq[0].neigh_top_recon_16bit[0],
                        BLOCK_MAX_COUNT_SB_128 * 128 * 3 * sz);
    }
    if (context_ptr->hbd_mode_decision != EB_10_BIT_MD) {
        EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq[0].neigh_left_recon[0],
                        BLOCK_MAX_COUNT_SB_128 * 128 * 3);
        EB_MALLOC_ARRAY(context_ptr->md_blk_arr_nsq[0].neigh_top_recon[0],
                        BLOCK_MAX_COUNT_SB_128 * 128 * 3);
    }
    uint32_t coded_leaf_index, txb_index;
#endif
#if SB64_MEM_OPT
    for (coded_leaf_index = 0; coded_leaf_index < block_max_count_sb; ++coded_leaf_index) {
#else
    for (coded_leaf_index = 0; coded_leaf_index < BLOCK_MAX_COUNT_SB_128; ++coded_leaf_index) {
#endif
#if !CLEAN_UP_SB_DATA_7
        for (txb_index = 0; txb_index < TRANSFORM_UNIT_MAX_COUNT; ++txb_index)
            context_ptr->md_blk_arr_nsq[coded_leaf_index].txb_array[txb_index].txb_index =
                txb_index;
#endif
#if !CLEAN_UP_SB_DATA_3
        const BlockGeom *blk_geom = get_blk_geom_mds(coded_leaf_index);
        UNUSED(blk_geom);
#endif
        context_ptr->md_blk_arr_nsq[coded_leaf_index].av1xd =
            context_ptr->md_blk_arr_nsq[0].av1xd + coded_leaf_index;
        context_ptr->md_blk_arr_nsq[coded_leaf_index].segment_id = 0;
#if !CLEAN_UP_SB_DATA_3
        for (int i = 0; i < 3; i++) {
            size_t offset = (coded_leaf_index * 128 * 3 + i * 128) * sz;
            context_ptr->md_blk_arr_nsq[coded_leaf_index].neigh_left_recon_16bit[i] =
                context_ptr->md_blk_arr_nsq[0].neigh_left_recon_16bit[0] + offset;
            context_ptr->md_blk_arr_nsq[coded_leaf_index].neigh_top_recon_16bit[i] =
                context_ptr->md_blk_arr_nsq[0].neigh_top_recon_16bit[0] + offset;
        }
        for (int i = 0; i < 3; i++) {
            size_t offset = coded_leaf_index * 128 * 3 + i * 128;
            context_ptr->md_blk_arr_nsq[coded_leaf_index].neigh_left_recon[i] =
                context_ptr->md_blk_arr_nsq[0].neigh_left_recon[0] + offset;
            context_ptr->md_blk_arr_nsq[coded_leaf_index].neigh_top_recon[i] =
                context_ptr->md_blk_arr_nsq[0].neigh_top_recon[0] + offset;
        }
#endif
#if MEM_OPT_PALETTE
        const BlockGeom *blk_geom = get_blk_geom_mds(coded_leaf_index);
        if (svt_av1_allow_palette(cfg_palette, blk_geom->bsize))
#else
        if (cfg_palette)
#endif
            EB_MALLOC_ARRAY(
                context_ptr->md_blk_arr_nsq[coded_leaf_index].palette_info.color_idx_map,
                MAX_PALETTE_SQUARE);
        else
            context_ptr->md_blk_arr_nsq[coded_leaf_index].palette_info.color_idx_map = NULL;
#if NO_ENCDEC //SB128_TODO to upgrade
        {
            EbPictureBufferDescInitData init_data;

            init_data.buffer_enable_mask = PICTURE_BUFFER_DESC_FULL_MASK;
            init_data.max_width          = SB_STRIDE_Y;
            init_data.max_height         = SB_STRIDE_Y;
            init_data.bit_depth          = EB_32BIT;
            init_data.color_format       = EB_YUV420;
            init_data.left_padding       = 0;
            init_data.right_padding      = 0;
            init_data.top_padding        = 0;
            init_data.bot_padding        = 0;
            init_data.split_mode         = EB_FALSE;

            EB_NEW(context_ptr->md_blk_arr_nsq[coded_leaf_index].coeff_tmp,
                   eb_picture_buffer_desc_ctor,
                   (EbPtr)&init_data);

            init_data.buffer_enable_mask = PICTURE_BUFFER_DESC_FULL_MASK;
            init_data.max_width          = SB_STRIDE_Y;
            init_data.max_height         = SB_STRIDE_Y;
            init_data.bit_depth          = EB_8BIT;
            init_data.color_format       = EB_YUV420;
            init_data.left_padding       = 0;
            init_data.right_padding      = 0;
            init_data.top_padding        = 0;
            init_data.bot_padding        = 0;
            init_data.split_mode         = EB_FALSE;

            EB_NEW(context_ptr->md_blk_arr_nsq[coded_leaf_index].recon_tmp,
                   eb_picture_buffer_desc_ctor,
                   (EbPtr)&init_data);
        }
#endif
    }
    EB_MALLOC_ARRAY(context_ptr->ref_best_cost_sq_table, MAX_REF_TYPE_CAND);
    EB_MALLOC_ARRAY(context_ptr->ref_best_ref_sq_table, MAX_REF_TYPE_CAND);
#if SB64_MEM_OPT
    EB_MALLOC_ARRAY(context_ptr->above_txfm_context, (sb_size >> MI_SIZE_LOG2));
    EB_MALLOC_ARRAY(context_ptr->left_txfm_context, (sb_size >> MI_SIZE_LOG2));
#else
    EB_MALLOC_ARRAY(context_ptr->above_txfm_context, (MAX_SB_SIZE >> MI_SIZE_LOG2));
    EB_MALLOC_ARRAY(context_ptr->left_txfm_context, (MAX_SB_SIZE >> MI_SIZE_LOG2));
#endif

#if  CAND_MEM_OPT
    EbPictureBufferDescInitData thirty_two_width_picture_buffer_desc_init_data;
    EbPictureBufferDescInitData picture_buffer_desc_init_data;

#if SB64_MEM_OPT
    picture_buffer_desc_init_data.max_width = sb_size;
    picture_buffer_desc_init_data.max_height = sb_size;
#else
    picture_buffer_desc_init_data.max_width = MAX_SB_SIZE;
    picture_buffer_desc_init_data.max_height = MAX_SB_SIZE;
#endif
    picture_buffer_desc_init_data.bit_depth = context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT;
    picture_buffer_desc_init_data.color_format = EB_YUV420;
    picture_buffer_desc_init_data.buffer_enable_mask = PICTURE_BUFFER_DESC_FULL_MASK;
    picture_buffer_desc_init_data.left_padding = 0;
    picture_buffer_desc_init_data.right_padding = 0;
    picture_buffer_desc_init_data.top_padding = 0;
    picture_buffer_desc_init_data.bot_padding = 0;
    picture_buffer_desc_init_data.split_mode = EB_FALSE;

#if SB64_MEM_OPT
    thirty_two_width_picture_buffer_desc_init_data.max_width = sb_size;
    thirty_two_width_picture_buffer_desc_init_data.max_height = sb_size;
#else
    thirty_two_width_picture_buffer_desc_init_data.max_width = MAX_SB_SIZE;
    thirty_two_width_picture_buffer_desc_init_data.max_height = MAX_SB_SIZE;
#endif
    thirty_two_width_picture_buffer_desc_init_data.bit_depth = EB_32BIT;
    thirty_two_width_picture_buffer_desc_init_data.color_format = EB_YUV420;
    thirty_two_width_picture_buffer_desc_init_data.buffer_enable_mask =
        PICTURE_BUFFER_DESC_FULL_MASK;
    thirty_two_width_picture_buffer_desc_init_data.left_padding = 0;
    thirty_two_width_picture_buffer_desc_init_data.right_padding = 0;
    thirty_two_width_picture_buffer_desc_init_data.top_padding = 0;
    thirty_two_width_picture_buffer_desc_init_data.bot_padding = 0;
    thirty_two_width_picture_buffer_desc_init_data.split_mode = EB_FALSE;

#if UNIFY_TXT
    for (uint32_t txt_itr = 0; txt_itr < TX_TYPES; ++txt_itr) {
        EB_NEW(context_ptr->recon_coeff_ptr[txt_itr],
            eb_picture_buffer_desc_ctor,
            (EbPtr)&thirty_two_width_picture_buffer_desc_init_data);
        EB_NEW(context_ptr->recon_ptr[txt_itr],
            eb_picture_buffer_desc_ctor,
            (EbPtr)&picture_buffer_desc_init_data);
    }
#endif
    EB_NEW(context_ptr->residual_quant_coeff_ptr,
        eb_picture_buffer_desc_ctor,
        (EbPtr)&thirty_two_width_picture_buffer_desc_init_data);

    EB_NEW(context_ptr->prediction_ptr_temp,
        eb_picture_buffer_desc_ctor,
        (EbPtr)&picture_buffer_desc_init_data);

    EB_NEW(context_ptr->cfl_temp_prediction_ptr,
        eb_picture_buffer_desc_ctor,
        (EbPtr)&picture_buffer_desc_init_data);

#endif

#if MEM_OPT_MD_BUF_DESC
    EbPictureBufferDescInitData double_width_picture_buffer_desc_init_data;
    double_width_picture_buffer_desc_init_data.max_width          = sb_size;
    double_width_picture_buffer_desc_init_data.max_height         = sb_size;
    double_width_picture_buffer_desc_init_data.bit_depth          = EB_16BIT;
    double_width_picture_buffer_desc_init_data.color_format       = EB_YUV420;
    double_width_picture_buffer_desc_init_data.buffer_enable_mask = PICTURE_BUFFER_DESC_FULL_MASK;
    double_width_picture_buffer_desc_init_data.left_padding       = 0;
    double_width_picture_buffer_desc_init_data.right_padding      = 0;
    double_width_picture_buffer_desc_init_data.top_padding        = 0;
    double_width_picture_buffer_desc_init_data.bot_padding        = 0;
    double_width_picture_buffer_desc_init_data.split_mode         = EB_FALSE;

    // The temp_recon_ptr and temp_residual_ptr will be shared by all candidates
    // If you want to do something with residual or recon, you need to create one
    EB_NEW(context_ptr->temp_recon_ptr,
           eb_picture_buffer_desc_ctor,
           (EbPtr)&picture_buffer_desc_init_data);
    EB_NEW(context_ptr->temp_residual_ptr,
           eb_picture_buffer_desc_ctor,
           (EbPtr)&double_width_picture_buffer_desc_init_data);

    // Candidate Buffers
    EB_ALLOC_PTR_ARRAY(context_ptr->candidate_buffer_ptr_array, MAX_NFL_BUFF);
#if MEM_OPT_UV_MODE
    for (buffer_index = 0; buffer_index < MAX_NFL_BUFF_Y; ++buffer_index) {
        EB_NEW(context_ptr->candidate_buffer_ptr_array[buffer_index],
               mode_decision_candidate_buffer_ctor,
               context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT,
               sb_size,
               PICTURE_BUFFER_DESC_FULL_MASK,
               context_ptr->temp_residual_ptr,
               context_ptr->temp_recon_ptr,
               &(context_ptr->fast_cost_array[buffer_index]),
               &(context_ptr->full_cost_array[buffer_index]),
               &(context_ptr->full_cost_skip_ptr[buffer_index]),
               &(context_ptr->full_cost_merge_ptr[buffer_index]));
    }

    for (buffer_index = MAX_NFL_BUFF_Y; buffer_index < MAX_NFL_BUFF; ++buffer_index) {
        EB_NEW(context_ptr->candidate_buffer_ptr_array[buffer_index],
               mode_decision_candidate_buffer_ctor,
               context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT,
               sb_size,
               PICTURE_BUFFER_DESC_CHROMA_MASK,
               context_ptr->temp_residual_ptr,
               context_ptr->temp_recon_ptr,
               &(context_ptr->fast_cost_array[buffer_index]),
               &(context_ptr->full_cost_array[buffer_index]),
               &(context_ptr->full_cost_skip_ptr[buffer_index]),
               &(context_ptr->full_cost_merge_ptr[buffer_index]));
    }
#else
    for (buffer_index = 0; buffer_index < MAX_NFL_BUFF; ++buffer_index) {
        EB_NEW(context_ptr->candidate_buffer_ptr_array[buffer_index],
               mode_decision_candidate_buffer_ctor,
               context_ptr->hbd_mode_decision ? EB_10BIT : EB_8BIT,
               sb_size,
               context_ptr->temp_residual_ptr,
               context_ptr->temp_recon_ptr,
               &(context_ptr->fast_cost_array[buffer_index]),
               &(context_ptr->full_cost_array[buffer_index]),
               &(context_ptr->full_cost_skip_ptr[buffer_index]),
               &(context_ptr->full_cost_merge_ptr[buffer_index]));
    }
#endif
#endif
    return EB_ErrorNone;
}

/**************************************************
 * Reset Mode Decision Neighbor Arrays
 *************************************************/
void reset_mode_decision_neighbor_arrays(PictureControlSet *pcs_ptr, uint16_t tile_idx) {
    uint8_t depth;
    for (depth = 0; depth < NEIGHBOR_ARRAY_TOTAL_COUNT; depth++) {
        neighbor_array_unit_reset(pcs_ptr->md_intra_luma_mode_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_intra_chroma_mode_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_mv_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_skip_flag_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_mode_type_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_leaf_depth_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->mdleaf_partition_neighbor_array[depth][tile_idx]);
        if (pcs_ptr->hbd_mode_decision != EB_10_BIT_MD) {
            neighbor_array_unit_reset(pcs_ptr->md_luma_recon_neighbor_array[depth][tile_idx]);
            neighbor_array_unit_reset(
                pcs_ptr->md_tx_depth_1_luma_recon_neighbor_array[depth][tile_idx]);
            neighbor_array_unit_reset(
                pcs_ptr->md_tx_depth_2_luma_recon_neighbor_array[depth][tile_idx]);
            neighbor_array_unit_reset(pcs_ptr->md_cb_recon_neighbor_array[depth][tile_idx]);
            neighbor_array_unit_reset(pcs_ptr->md_cr_recon_neighbor_array[depth][tile_idx]);
        }
        if (pcs_ptr->hbd_mode_decision > EB_8_BIT_MD) {
            neighbor_array_unit_reset(pcs_ptr->md_luma_recon_neighbor_array16bit[depth][tile_idx]);
            neighbor_array_unit_reset(
                pcs_ptr->md_tx_depth_1_luma_recon_neighbor_array16bit[depth][tile_idx]);
            neighbor_array_unit_reset(
                pcs_ptr->md_tx_depth_2_luma_recon_neighbor_array16bit[depth][tile_idx]);
            neighbor_array_unit_reset(pcs_ptr->md_cb_recon_neighbor_array16bit[depth][tile_idx]);
            neighbor_array_unit_reset(pcs_ptr->md_cr_recon_neighbor_array16bit[depth][tile_idx]);
        }

        neighbor_array_unit_reset(pcs_ptr->md_skip_coeff_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(
            pcs_ptr->md_luma_dc_sign_level_coeff_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(
            pcs_ptr->md_tx_depth_1_luma_dc_sign_level_coeff_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(
            pcs_ptr->md_cb_dc_sign_level_coeff_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(
            pcs_ptr->md_cr_dc_sign_level_coeff_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_txfm_context_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_inter_pred_dir_neighbor_array[depth][tile_idx]);
        neighbor_array_unit_reset(pcs_ptr->md_ref_frame_type_neighbor_array[depth][tile_idx]);

        neighbor_array_unit_reset32(pcs_ptr->md_interpolation_type_neighbor_array[depth][tile_idx]);
    }

    return;
}
#if !TPL_LA_LAMBDA_SCALING
extern void lambda_assign_low_delay(uint32_t *fast_lambda, uint32_t *full_lambda,
                                    uint32_t *fast_chroma_lambda, uint32_t *full_chroma_lambda,
                                    uint32_t *full_chroma_lambda_sao,
                                    uint8_t qp_hierarchical_position, uint8_t qp, uint8_t chroma_qp)

{
    if (qp_hierarchical_position == 0) {
        *fast_lambda            = lambda_mode_decision_ld_sad[qp];
        *fast_chroma_lambda     = lambda_mode_decision_ld_sad[qp];
        *full_lambda            = lambda_mode_decision_ld_sse[qp];
        *full_chroma_lambda     = lambda_mode_decision_ld_sse[qp];
        *full_chroma_lambda_sao = lambda_mode_decision_ld_sse[chroma_qp];
    } else { // Hierarchical postions 1, 2, 3, 4, 5
        *fast_lambda            = lambda_mode_decision_ld_sad_qp_scaling[qp];
        *fast_chroma_lambda     = lambda_mode_decision_ld_sad_qp_scaling[qp];
        *full_lambda            = lambda_mode_decision_ld_sse_qp_scaling[qp];
        *full_chroma_lambda     = lambda_mode_decision_ld_sse_qp_scaling[qp];
        *full_chroma_lambda_sao = lambda_mode_decision_ld_sse_qp_scaling[chroma_qp];
    }
}

void lambda_assign_random_access(uint32_t *fast_lambda, uint32_t *full_lambda,
                                 uint32_t *fast_chroma_lambda, uint32_t *full_chroma_lambda,
                                 uint32_t *full_chroma_lambda_sao, uint8_t qp_hierarchical_position,
                                 uint8_t qp, uint8_t chroma_qp)

{
    if (qp_hierarchical_position == 0) {
        *fast_lambda            = lambda_mode_decision_ra_sad[qp];
        *fast_chroma_lambda     = lambda_mode_decision_ra_sad[qp];
        *full_lambda            = lambda_mode_decision_ra_sse[qp];
        *full_chroma_lambda     = lambda_mode_decision_ra_sse[qp];
        *full_chroma_lambda_sao = lambda_mode_decision_ra_sse[chroma_qp];
    } else if (qp_hierarchical_position < 3) { // Hierarchical postions 1, 2

        *fast_lambda            = lambda_mode_decision_ra_sad_qp_scaling_l1[qp];
        *fast_chroma_lambda     = lambda_mode_decision_ra_sad_qp_scaling_l1[qp];
        *full_lambda            = lambda_mode_decision_ra_sse_qp_scaling_l1[qp];
        *full_chroma_lambda     = lambda_mode_decision_ra_sse_qp_scaling_l1[qp];
        *full_chroma_lambda_sao = lambda_mode_decision_ra_sse_qp_scaling_l1[chroma_qp];
    } else { // Hierarchical postions 3, 4, 5
        *fast_lambda            = lambda_mode_decision_ra_sad_qp_scaling_l3[qp];
        *fast_chroma_lambda     = lambda_mode_decision_ra_sad_qp_scaling_l3[qp];
        *full_lambda            = lambda_mode_decision_ra_sse_qp_scaling_l3[qp];
        *full_chroma_lambda     = lambda_mode_decision_ra_sse_qp_scaling_l3[qp];
        *full_chroma_lambda_sao = lambda_mode_decision_ra_sse_qp_scaling_l3[chroma_qp];
    }
}

void lambda_assign_i_slice(uint32_t *fast_lambda, uint32_t *full_lambda,
                           uint32_t *fast_chroma_lambda, uint32_t *full_chroma_lambda,
                           uint32_t *full_chroma_lambda_sao, uint8_t qp_hierarchical_position,
                           uint8_t qp, uint8_t chroma_qp)

{
    if (qp_hierarchical_position == 0) {
        *fast_lambda            = lambda_mode_decision_i_slice_sad[qp];
        *fast_chroma_lambda     = lambda_mode_decision_i_slice_sad[qp];
        *full_lambda            = lambda_mode_decision_i_slice_sse[qp];
        *full_chroma_lambda     = lambda_mode_decision_i_slice_sse[qp];
        *full_chroma_lambda_sao = lambda_mode_decision_i_slice_sse[chroma_qp];
    }
}
const EbLambdaAssignFunc lambda_assignment_function_table[4] = {
    lambda_assign_low_delay, // low delay P
    lambda_assign_low_delay, // low delay b
    lambda_assign_random_access, // Random Access
    lambda_assign_i_slice // I_SLICE
};
#endif
#if TPL_LAMBDA_IMP
// Set the lambda for each sb.
// When lambda tuning is on (blk_lambda_tuning), lambda of each block is set separately (full_lambda_md/fast_lambda_md)
// later in set_tuned_blk_lambda
#endif
void av1_lambda_assign_md(
    ModeDecisionContext   *context_ptr)
{
        context_ptr->full_lambda_md[0] = av1_lambda_mode_decision8_bit_sse[context_ptr->qp_index];
        context_ptr->fast_lambda_md[0] = av1_lambda_mode_decision8_bit_sad[context_ptr->qp_index];

        context_ptr->full_lambda_md[1] = av1lambda_mode_decision10_bit_sse[context_ptr->qp_index];
        context_ptr->fast_lambda_md[1] = av1lambda_mode_decision10_bit_sad[context_ptr->qp_index];

        context_ptr->full_lambda_md[1] *= 16;
        context_ptr->fast_lambda_md[1] *= 4;
#if TPL_LAMBDA_IMP
        context_ptr->full_sb_lambda_md[0] = context_ptr->full_lambda_md[0];
        context_ptr->full_sb_lambda_md[1] = context_ptr->full_lambda_md[1];
#endif
}
void av1_lambda_assign(uint32_t *fast_lambda, uint32_t *full_lambda, uint8_t bit_depth, uint16_t qp_index,
                       EbBool multiply_lambda) {
    if (bit_depth == 8) {
        *full_lambda = av1_lambda_mode_decision8_bit_sse[qp_index];
        *fast_lambda = av1_lambda_mode_decision8_bit_sad[qp_index];
    } else if (bit_depth == 10) {
        *full_lambda = av1lambda_mode_decision10_bit_sse[qp_index];
        *fast_lambda = av1lambda_mode_decision10_bit_sad[qp_index];
        if (multiply_lambda) {
            *full_lambda *= 16;
            *fast_lambda *= 4;
        }
    } else if (bit_depth == 12) {
        *full_lambda = av1lambda_mode_decision12_bit_sse[qp_index];
        *fast_lambda = av1lambda_mode_decision12_bit_sad[qp_index];
    } else {
        assert(bit_depth >= 8);
        assert(bit_depth <= 12);
    }

    // NM: To be done: tune lambda based on the picture type and layer.
}
const EbAv1LambdaAssignFunc av1_lambda_assignment_function_table[4] = {
    av1_lambda_assign,
    av1_lambda_assign,
    av1_lambda_assign,
    av1_lambda_assign,
};

void reset_mode_decision(SequenceControlSet *scs_ptr, ModeDecisionContext *context_ptr,
                         PictureControlSet *pcs_ptr, uint16_t tile_group_idx,
                         uint32_t segment_index) {
    FrameHeader *frm_hdr = &pcs_ptr->parent_pcs_ptr->frm_hdr;
    context_ptr->hbd_mode_decision = pcs_ptr->hbd_mode_decision;
    // QP
#if !QP2QINDEX
    uint16_t picture_qp   = pcs_ptr->parent_pcs_ptr->frm_hdr.quantization_params.base_q_idx;
    context_ptr->qp       = picture_qp;
    context_ptr->qp_index = context_ptr->qp;
    // Asuming cb and cr offset to be the same for chroma QP in both slice and pps for lambda computation
    context_ptr->chroma_qp = (uint8_t)context_ptr->qp;
#endif
    context_ptr->qp_index  = (uint8_t)frm_hdr->quantization_params.base_q_idx;
    av1_lambda_assign_md(context_ptr);
    // Reset MD rate Estimation table to initial values by copying from md_rate_estimation_array
    if (context_ptr->is_md_rate_estimation_ptr_owner) {
        context_ptr->is_md_rate_estimation_ptr_owner = EB_FALSE;
        EB_FREE_ARRAY(context_ptr->md_rate_estimation_ptr);
    }
    context_ptr->md_rate_estimation_ptr = pcs_ptr->md_rate_estimation_array;
    uint32_t cand_index;
    for (cand_index = 0; cand_index < MODE_DECISION_CANDIDATE_MAX_COUNT; ++cand_index)
        context_ptr->fast_candidate_ptr_array[cand_index]->md_rate_estimation_ptr =
            context_ptr->md_rate_estimation_ptr;

    // Reset CABAC Contexts
#if !MD_FRAME_CONTEXT_MEM_OPT
    context_ptr->coeff_est_entropy_coder_ptr = pcs_ptr->coeff_est_entropy_coder_ptr;
#endif
    // Reset Neighbor Arrays at start of new Segment / Picture
    if (segment_index == 0) {
        for (uint16_t r =
                 pcs_ptr->parent_pcs_ptr->tile_group_info[tile_group_idx].tile_group_tile_start_y;
             r < pcs_ptr->parent_pcs_ptr->tile_group_info[tile_group_idx].tile_group_tile_end_y;
             r++) {
            for (uint16_t c = pcs_ptr->parent_pcs_ptr->tile_group_info[tile_group_idx]
                                  .tile_group_tile_start_x;
                 c < pcs_ptr->parent_pcs_ptr->tile_group_info[tile_group_idx].tile_group_tile_end_x;
                 c++) {
                uint16_t tile_idx = c + r * pcs_ptr->parent_pcs_ptr->av1_cm->tiles_info.tile_cols;
                reset_mode_decision_neighbor_arrays(pcs_ptr, tile_idx);
            }
        }
        (void)scs_ptr;
    }
    return;
}

/******************************************************
 * Mode Decision Configure SB
 ******************************************************/
void mode_decision_configure_sb(ModeDecisionContext *context_ptr, PictureControlSet *pcs_ptr,
                                uint8_t sb_qp) {
#if !QP2QINDEX
    (void)pcs_ptr;
    //Disable Lambda update per SB
    context_ptr->qp = sb_qp;
    // Asuming cb and cr offset to be the same for chroma QP in both slice and pps for lambda computation

    context_ptr->chroma_qp = (uint8_t)context_ptr->qp;
#endif

    /* Note(CHKN) : when Qp modulation varies QP on a sub-SB(CU) basis,  Lamda has to change based on Cu->QP , and then this code has to move inside the CU loop in MD */

    // Lambda Assignement
    context_ptr->qp_index =
        pcs_ptr->parent_pcs_ptr->frm_hdr.delta_q_params.delta_q_present
#if QP2QINDEX
            ? sb_qp
#else
            ? (uint8_t)quantizer_to_qindex[sb_qp]
#endif
            : (uint8_t)pcs_ptr->parent_pcs_ptr->frm_hdr.quantization_params.base_q_idx;

    av1_lambda_assign_md(context_ptr);

    return;
}
