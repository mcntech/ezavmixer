#include "bitreader.h"
#include "h265parser.h"


#define av_log(...)
#define AVERROR_INVALIDDATA -1

#define FF_PROFILE_HEVC_MAIN                        1
#define FF_PROFILE_HEVC_MAIN_10                     2
#define FF_PROFILE_HEVC_MAIN_STILL_PICTURE          3
#define FF_PROFILE_HEVC_REXT                        4

typedef struct HEVCWindow {
    unsigned int left_offset;
    unsigned int right_offset;
    unsigned int top_offset;
    unsigned int bottom_offset;
} HEVCWindow;

typedef struct VUI {
    //AVRational sar;

    int overscan_info_present_flag;
    int overscan_appropriate_flag;

    int video_signal_type_present_flag;
    int video_format;
    int video_full_range_flag;
    int colour_description_present_flag;
    uint8_t colour_primaries;
    uint8_t transfer_characteristic;
    uint8_t matrix_coeffs;

    int chroma_loc_info_present_flag;
    int chroma_sample_loc_type_top_field;
    int chroma_sample_loc_type_bottom_field;
    int neutra_chroma_indication_flag;

    int field_seq_flag;
    int frame_field_info_present_flag;

    int default_display_window_flag;
    HEVCWindow def_disp_win;

    int vui_timing_info_present_flag;
    uint32_t vui_num_units_in_tick;
    uint32_t vui_time_scale;
    int vui_poc_proportional_to_timing_flag;
    int vui_num_ticks_poc_diff_one_minus1;
    int vui_hrd_parameters_present_flag;

    int bitstream_restriction_flag;
    int tiles_fixed_structure_flag;
    int motion_vectors_over_pic_boundaries_flag;
    int restricted_ref_pic_lists_flag;
    int min_spatial_segmentation_idc;
    int max_bytes_per_pic_denom;
    int max_bits_per_min_cu_denom;
    int log2_max_mv_length_horizontal;
    int log2_max_mv_length_vertical;
} VUI;

typedef struct PTLCommon {
    uint8_t profile_space;
    uint8_t tier_flag;
    uint8_t profile_idc;
    uint8_t profile_compatibility_flag[32];
    uint8_t level_idc;
    uint8_t progressive_source_flag;
    uint8_t interlaced_source_flag;
    uint8_t non_packed_constraint_flag;
    uint8_t frame_only_constraint_flag;
} PTLCommon;

typedef struct PTL {
    PTLCommon general_ptl;
    PTLCommon sub_layer_ptl[HEVC_MAX_SUB_LAYERS];

    uint8_t sub_layer_profile_present_flag[HEVC_MAX_SUB_LAYERS];
    uint8_t sub_layer_level_present_flag[HEVC_MAX_SUB_LAYERS];
} PTL;

typedef struct HEVCVPS {
    uint8_t vps_temporal_id_nesting_flag;
    int vps_max_layers;
    int vps_max_sub_layers; ///< vps_max_temporal_layers_minus1 + 1

    PTL ptl;
    int vps_sub_layer_ordering_info_present_flag;
    unsigned int vps_max_dec_pic_buffering[HEVC_MAX_SUB_LAYERS];
    unsigned int vps_num_reorder_pics[HEVC_MAX_SUB_LAYERS];
    unsigned int vps_max_latency_increase[HEVC_MAX_SUB_LAYERS];
    int vps_max_layer_id;
    int vps_num_layer_sets; ///< vps_num_layer_sets_minus1 + 1
    uint8_t vps_timing_info_present_flag;
    uint32_t vps_num_units_in_tick;
    uint32_t vps_time_scale;
    uint8_t vps_poc_proportional_to_timing_flag;
    int vps_num_ticks_poc_diff_one; ///< vps_num_ticks_poc_diff_one_minus1 + 1
    int vps_num_hrd_parameters;

    uint8_t data[4096];
    int data_size;
} HEVCVPS;

typedef struct ScalingList {
    /* This is a little wasteful, since sizeID 0 only needs 8 coeffs,
     * and size ID 3 only has 2 arrays, not 6. */
    uint8_t sl[4][6][64];
    uint8_t sl_dc[2][6];
} ScalingList;

typedef struct ShortTermRPS {
    unsigned int num_negative_pics;
    int num_delta_pocs;
    int rps_idx_num_delta_pocs;
    int32_t delta_poc[32];
    uint8_t used[32];
} ShortTermRPS;

typedef struct LongTermRPS {
    int     poc[32];
    uint8_t used[32];
    uint8_t nb_refs;
} LongTermRPS;

typedef struct HEVCSPS {
    unsigned vps_id;
    int chroma_format_idc;
    uint8_t separate_colour_plane_flag;

    HEVCWindow output_window;

    HEVCWindow pic_conf_win;

    int bit_depth;
    int bit_depth_chroma;
    int pixel_shift;
    //enum AVPixelFormat pix_fmt;

    unsigned int log2_max_poc_lsb;
    int pcm_enabled_flag;

    int max_sub_layers;
    struct {
        int max_dec_pic_buffering;
        int num_reorder_pics;
        int max_latency_increase;
    } temporal_layer[HEVC_MAX_SUB_LAYERS];
    uint8_t temporal_id_nesting_flag;

    VUI vui;
    PTL ptl;

    uint8_t scaling_list_enable_flag;
    ScalingList scaling_list;

    unsigned int nb_st_rps;
    ShortTermRPS st_rps[HEVC_MAX_SHORT_TERM_REF_PIC_SETS];

    uint8_t amp_enabled_flag;
    uint8_t sao_enabled;

    uint8_t long_term_ref_pics_present_flag;
    uint16_t lt_ref_pic_poc_lsb_sps[HEVC_MAX_LONG_TERM_REF_PICS];
    uint8_t used_by_curr_pic_lt_sps_flag[HEVC_MAX_LONG_TERM_REF_PICS];
    uint8_t num_long_term_ref_pics_sps;

    struct {
        uint8_t bit_depth;
        uint8_t bit_depth_chroma;
        unsigned int log2_min_pcm_cb_size;
        unsigned int log2_max_pcm_cb_size;
        uint8_t loop_filter_disable_flag;
    } pcm;
    uint8_t sps_temporal_mvp_enabled_flag;
    uint8_t sps_strong_intra_smoothing_enable_flag;

    unsigned int log2_min_cb_size;
    unsigned int log2_diff_max_min_coding_block_size;
    unsigned int log2_min_tb_size;
    unsigned int log2_max_trafo_size;
    unsigned int log2_ctb_size;
    unsigned int log2_min_pu_size;

    int max_transform_hierarchy_depth_inter;
    int max_transform_hierarchy_depth_intra;

    int transform_skip_rotation_enabled_flag;
    int transform_skip_context_enabled_flag;
    int implicit_rdpcm_enabled_flag;
    int explicit_rdpcm_enabled_flag;
    int intra_smoothing_disabled_flag;
    int persistent_rice_adaptation_enabled_flag;

    ///< coded frame dimension in various units
    int width;
    int height;
    int ctb_width;
    int ctb_height;
    int ctb_size;
    int min_cb_width;
    int min_cb_height;
    int min_tb_width;
    int min_tb_height;
    int min_pu_width;
    int min_pu_height;
    int tb_mask;

    int hshift[3];
    int vshift[3];

    int qp_bd_offset;

    uint8_t data[4096];
    int data_size;
} HEVCSPS;

typedef struct HEVCPPS {
    unsigned int sps_id; ///< seq_parameter_set_id

    uint8_t sign_data_hiding_flag;

    uint8_t cabac_init_present_flag;

    int num_ref_idx_l0_default_active; ///< num_ref_idx_l0_default_active_minus1 + 1
    int num_ref_idx_l1_default_active; ///< num_ref_idx_l1_default_active_minus1 + 1
    int pic_init_qp_minus26;

    uint8_t constrained_intra_pred_flag;
    uint8_t transform_skip_enabled_flag;

    uint8_t cu_qp_delta_enabled_flag;
    int diff_cu_qp_delta_depth;

    int cb_qp_offset;
    int cr_qp_offset;
    uint8_t pic_slice_level_chroma_qp_offsets_present_flag;
    uint8_t weighted_pred_flag;
    uint8_t weighted_bipred_flag;
    uint8_t output_flag_present_flag;
    uint8_t transquant_bypass_enable_flag;

    uint8_t dependent_slice_segments_enabled_flag;
    uint8_t tiles_enabled_flag;
    uint8_t entropy_coding_sync_enabled_flag;

    int num_tile_columns;   ///< num_tile_columns_minus1 + 1
    int num_tile_rows;      ///< num_tile_rows_minus1 + 1
    uint8_t uniform_spacing_flag;
    uint8_t loop_filter_across_tiles_enabled_flag;

    uint8_t seq_loop_filter_across_slices_enabled_flag;

    uint8_t deblocking_filter_control_present_flag;
    uint8_t deblocking_filter_override_enabled_flag;
    uint8_t disable_dbf;
    int beta_offset;    ///< beta_offset_div2 * 2
    int tc_offset;      ///< tc_offset_div2 * 2

    uint8_t scaling_list_data_present_flag;
    ScalingList scaling_list;

    uint8_t lists_modification_present_flag;
    int log2_parallel_merge_level; ///< log2_parallel_merge_level_minus2 + 2
    int num_extra_slice_header_bits;
    uint8_t slice_header_extension_present_flag;
    uint8_t log2_max_transform_skip_block_size;
    uint8_t cross_component_prediction_enabled_flag;
    uint8_t chroma_qp_offset_list_enabled_flag;
    uint8_t diff_cu_chroma_qp_offset_depth;
    uint8_t chroma_qp_offset_list_len_minus1;
    int8_t  cb_qp_offset_list[6];
    int8_t  cr_qp_offset_list[6];
    uint8_t log2_sao_offset_scale_luma;
    uint8_t log2_sao_offset_scale_chroma;

    // Inferred parameters
    unsigned int *column_width;  ///< ColumnWidth
    unsigned int *row_height;    ///< RowHeight
    unsigned int *col_bd;        ///< ColBd
    unsigned int *row_bd;        ///< RowBd
    int *col_idxX;

    int *ctb_addr_rs_to_ts; ///< CtbAddrRSToTS
    int *ctb_addr_ts_to_rs; ///< CtbAddrTSToRS
    int *tile_id;           ///< TileId
    int *tile_pos_rs;       ///< TilePosRS
    int *min_tb_addr_zs;    ///< MinTbAddrZS
    int *min_tb_addr_zs_tab;///< MinTbAddrZS

    uint8_t data[4096];
    int data_size;
} HEVCPPS;

/*
typedef struct HEVCParamSets {
    AVBufferRef *vps_list[HEVC_MAX_VPS_COUNT];
    AVBufferRef *sps_list[HEVC_MAX_SPS_COUNT];
    AVBufferRef *pps_list[HEVC_MAX_PPS_COUNT];

    */
/* currently active parameter sets *//*

    const HEVCVPS *vps;
    const HEVCSPS *sps;
    const HEVCPPS *pps;
} HEVCParamSets;
*/

typedef struct AVCodecContext {
    int dummy;
} AVCodecContext;


#if 0
static void decode_sublayer_hrd(cBitReader *gb, unsigned int nb_cpb,
                                int subpic_params_present)
{
    int i;

    for (i = 0; i < nb_cpb; i++) {
        gb->ue(); // bit_rate_value_minus1
        gb->ue(); // cpb_size_value_minus1

        if (subpic_params_present) {
            gb->ue(); // cpb_size_du_value_minus1
            gb->ue(); // bit_rate_du_value_minus1
        }
        skip_bits1(gb); // cbr_flag
    }
}
#endif

#if 0
static int decode_hrd(cBitReader *gb, int common_inf_present,
                      int max_sublayers)
{
    int nal_params_present = 0, vcl_params_present = 0;
    int subpic_params_present = 0;
    int i;

    if (common_inf_present) {
        nal_params_present = get_bits1(gb);
        vcl_params_present = get_bits1(gb);

        if (nal_params_present || vcl_params_present) {
            subpic_params_present = get_bits1(gb);

            if (subpic_params_present) {
                skip_bits(gb, 8); // tick_divisor_minus2
                skip_bits(gb, 5); // du_cpb_removal_delay_increment_length_minus1
                skip_bits(gb, 1); // sub_pic_cpb_params_in_pic_timing_sei_flag
                skip_bits(gb, 5); // dpb_output_delay_du_length_minus1
            }

            skip_bits(gb, 4); // bit_rate_scale
            skip_bits(gb, 4); // cpb_size_scale

            if (subpic_params_present)
                skip_bits(gb, 4);  // cpb_size_du_scale

            skip_bits(gb, 5); // initial_cpb_removal_delay_length_minus1
            skip_bits(gb, 5); // au_cpb_removal_delay_length_minus1
            skip_bits(gb, 5); // dpb_output_delay_length_minus1
        }
    }

    for (i = 0; i < max_sublayers; i++) {
        int low_delay = 0;
        unsigned int nb_cpb = 1;
        int fixed_rate = get_bits1(gb);

        if (!fixed_rate)
            fixed_rate = get_bits1(gb);

        if (fixed_rate)
            gb->ue();  // elemental_duration_in_tc_minus1
        else
            low_delay = get_bits1(gb);

        if (!low_delay) {
            nb_cpb = gb->ue() + 1;
            if (nb_cpb < 1 || nb_cpb > 32) {
                av_log(NULL, AV_LOG_ERROR, "nb_cpb %d invalid\n", nb_cpb);
                return AVERROR_INVALIDDATA;
            }
        }

        if (nal_params_present)
            decode_sublayer_hrd(gb, nb_cpb, subpic_params_present);
        if (vcl_params_present)
            decode_sublayer_hrd(gb, nb_cpb, subpic_params_present);
    }
    return 0;
}
#endif

static int decode_profile_tier_level(cBitReader *gb, AVCodecContext *avctx,
                                     PTLCommon *ptl) {
    int i;

    if (gb->get_bits_left() < 2 + 1 + 5 + 32 + 4 + 16 + 16 + 12)
        return -1;

    ptl->profile_space = gb->u(2);
    ptl->tier_flag = gb->u(1);
    ptl->profile_idc = gb->u(5);
    if (ptl->profile_idc == FF_PROFILE_HEVC_MAIN) {
        av_log(avctx, AV_LOG_DEBUG, "Main profile bitstream\n");
    } else if (ptl->profile_idc == FF_PROFILE_HEVC_MAIN_10) {
        av_log(avctx, AV_LOG_DEBUG, "Main 10 profile bitstream\n");
    } else if (ptl->profile_idc == FF_PROFILE_HEVC_MAIN_STILL_PICTURE) {
        av_log(avctx, AV_LOG_DEBUG, "Main Still Picture profile bitstream\n");
    } else if (ptl->profile_idc == FF_PROFILE_HEVC_REXT) {
        av_log(avctx, AV_LOG_DEBUG, "Range Extension profile bitstream\n");
    } else {
            av_log(avctx, AV_LOG_WARNING, "Unknown HEVC profile: %d\n", ptl->profile_idc);
    }
    for (i = 0; i < 32; i++) {
        ptl->profile_compatibility_flag[i] = gb->u(1);

        if (ptl->profile_idc == 0 && i > 0 && ptl->profile_compatibility_flag[i])
            ptl->profile_idc = i;
    }
    ptl->progressive_source_flag    = gb->u(1);
    ptl->interlaced_source_flag     = gb->u(1);
    ptl->non_packed_constraint_flag = gb->u(1);
    ptl->frame_only_constraint_flag = gb->u(1);

    gb->u( 16); // XXX_reserved_zero_44bits[0..15]
    gb->u( 16); // XXX_reserved_zero_44bits[16..31]
    gb->u( 12); // XXX_reserved_zero_44bits[32..43]

    return 0;
}

static int parse_ptl(cBitReader *gb, AVCodecContext *avctx,
                     PTL *ptl, int max_num_sub_layers)
{
    int i;
    if (decode_profile_tier_level(gb, avctx, &ptl->general_ptl) < 0 ||
        gb->get_bits_left() < 8 + (8*2 * (max_num_sub_layers - 1 > 0))) {
            av_log(avctx, AV_LOG_ERROR, "PTL information too short\n");
        return -1;
    }

    ptl->general_ptl.level_idc = gb->u( 8);

    for (i = 0; i < max_num_sub_layers - 1; i++) {
        ptl->sub_layer_profile_present_flag[i] = gb->u(1);
        ptl->sub_layer_level_present_flag[i]   = gb->u(1);
    }

    if (max_num_sub_layers - 1> 0)
        for (i = max_num_sub_layers - 1; i < 8; i++)
            gb->u( 2); // reserved_zero_2bits[i]
    for (i = 0; i < max_num_sub_layers - 1; i++) {
        if (ptl->sub_layer_profile_present_flag[i] &&
            decode_profile_tier_level(gb, avctx, &ptl->sub_layer_ptl[i]) < 0) {
            av_log(avctx, AV_LOG_ERROR,
                   "PTL information for sublayer %i too short\n", i);
            return -1;
        }
        if (ptl->sub_layer_level_present_flag[i]) {
            if (gb->get_bits_left() < 8) {
                av_log(avctx, AV_LOG_ERROR,
                       "Not enough data for sublayer %i level_idc\n", i);
                return -1;
            } else
                ptl->sub_layer_ptl[i].level_idc = gb->u( 8);
        }
    }

    return 0;
}


#if 0
static void decode_vui(cBitReader *gb, AVCodecContext *avctx,
                       int apply_defdispwin, HEVCSPS *sps)
{
    VUI backup_vui, *vui = &sps->vui;
    GetBitContext backup;
    int sar_present, alt = 0;

    av_log(avctx, AV_LOG_DEBUG, "Decoding VUI\n");

    sar_present = gb->u(1);
    if (sar_present) {
        uint8_t sar_idx = gb->u( 8);
        if (sar_idx < FF_ARRAY_ELEMS(vui_sar))
            vui->sar = vui_sar[sar_idx];
        else if (sar_idx == 255) {
            vui->sar.num = gb->u( 16);
            vui->sar.den = gb->u( 16);
        } else
                av_log(avctx, AV_LOG_WARNING,
                       "Unknown SAR index: %u.\n", sar_idx);
    }

    vui->overscan_info_present_flag = gb->u(1);
    if (vui->overscan_info_present_flag)
        vui->overscan_appropriate_flag = gb->u(1);

    vui->video_signal_type_present_flag = gb->u(1);
    if (vui->video_signal_type_present_flag) {
        vui->video_format                    = gb->u( 3);
        vui->video_full_range_flag           = gb->u(1);
        vui->colour_description_present_flag = gb->u(1);
        if (vui->video_full_range_flag && sps->pix_fmt == AV_PIX_FMT_YUV420P)
            sps->pix_fmt = AV_PIX_FMT_YUVJ420P;
        if (vui->colour_description_present_flag) {
            vui->colour_primaries        = gb->u( 8);
            vui->transfer_characteristic = gb->u( 8);
            vui->matrix_coeffs           = gb->u( 8);

            // Set invalid values to "unspecified"
            if (!av_color_primaries_name(vui->colour_primaries))
                vui->colour_primaries = AVCOL_PRI_UNSPECIFIED;
            if (!av_color_transfer_name(vui->transfer_characteristic))
                vui->transfer_characteristic = AVCOL_TRC_UNSPECIFIED;
            if (!av_color_space_name(vui->matrix_coeffs))
                vui->matrix_coeffs = AVCOL_SPC_UNSPECIFIED;
            if (vui->matrix_coeffs == AVCOL_SPC_RGB) {
                switch (sps->pix_fmt) {
                    case AV_PIX_FMT_YUV444P:
                        sps->pix_fmt = AV_PIX_FMT_GBRP;
                        break;
                    case AV_PIX_FMT_YUV444P10:
                        sps->pix_fmt = AV_PIX_FMT_GBRP10;
                        break;
                    case AV_PIX_FMT_YUV444P12:
                        sps->pix_fmt = AV_PIX_FMT_GBRP12;
                        break;
                }
            }
        }
    }

    vui->chroma_loc_info_present_flag = gb->u(1);
    if (vui->chroma_loc_info_present_flag) {
        vui->chroma_sample_loc_type_top_field    = gb->ue();
        vui->chroma_sample_loc_type_bottom_field = gb->ue();
    }

    vui->neutra_chroma_indication_flag = gb->u(1);
    vui->field_seq_flag                = gb->u(1);
    vui->frame_field_info_present_flag = gb->u(1);

    // Backup context in case an alternate header is detected
    memcpy(&backup, gb, sizeof(backup));
    memcpy(&backup_vui, vui, sizeof(backup_vui));
    if (gb->get_bits_left() >= 68 && show_bits_long(gb, 21) == 0x100000) {
        vui->default_display_window_flag = 0;
        av_log(avctx, AV_LOG_WARNING, "Invalid default display window\n");
    } else
        vui->default_display_window_flag = gb->u(1);

    if (vui->default_display_window_flag) {
        int vert_mult  = 1 + (sps->chroma_format_idc < 2);
        int horiz_mult = 1 + (sps->chroma_format_idc < 3);
        vui->def_disp_win.left_offset   = gb->ue() * horiz_mult;
        vui->def_disp_win.right_offset  = gb->ue() * horiz_mult;
        vui->def_disp_win.top_offset    = gb->ue() *  vert_mult;
        vui->def_disp_win.bottom_offset = gb->ue() *  vert_mult;

        if (apply_defdispwin &&
            avctx->flags2 & AV_CODEC_FLAG2_IGNORE_CROP) {
            av_log(avctx, AV_LOG_DEBUG,
                   "discarding vui default display window, "
                           "original values are l:%u r:%u t:%u b:%u\n",
                   vui->def_disp_win.left_offset,
                   vui->def_disp_win.right_offset,
                   vui->def_disp_win.top_offset,
                   vui->def_disp_win.bottom_offset);

            vui->def_disp_win.left_offset   =
            vui->def_disp_win.right_offset  =
            vui->def_disp_win.top_offset    =
            vui->def_disp_win.bottom_offset = 0;
        }
    }

    timing_info:
    vui->vui_timing_info_present_flag = gb->u(1);

    if (vui->vui_timing_info_present_flag) {
        if( gb->get_bits_left() < 66 && !alt) {
            // The alternate syntax seem to have timing info located
            // at where def_disp_win is normally located
            av_log(avctx, AV_LOG_WARNING,
                   "Strange VUI timing information, retrying...\n");
            memcpy(vui, &backup_vui, sizeof(backup_vui));
            memcpy(gb, &backup, sizeof(backup));
            alt = 1;
            goto timing_info;
        }
        vui->vui_num_units_in_tick               = gb->u(32);
        vui->vui_time_scale                      = gb->u(32);
        if (alt) {
            av_log(avctx, AV_LOG_INFO, "Retry got %"PRIu32"/%"PRIu32" fps\n",
                   vui->vui_time_scale, vui->vui_num_units_in_tick);
        }
        vui->vui_poc_proportional_to_timing_flag = gb->u(1);
        if (vui->vui_poc_proportional_to_timing_flag)
            vui->vui_num_ticks_poc_diff_one_minus1 = gb->ue();
        vui->vui_hrd_parameters_present_flag = gb->u(1);
        if (vui->vui_hrd_parameters_present_flag)
            decode_hrd(gb, 1, sps->max_sub_layers);
    }

    vui->bitstream_restriction_flag = gb->u(1);
    if (vui->bitstream_restriction_flag) {
        if (gb->get_bits_left() < 8 && !alt) {
            av_log(avctx, AV_LOG_WARNING,
                   "Strange VUI bitstream restriction information, retrying"
                           " from timing information...\n");
            memcpy(vui, &backup_vui, sizeof(backup_vui));
            memcpy(gb, &backup, sizeof(backup));
            alt = 1;
            goto timing_info;
        }
        vui->tiles_fixed_structure_flag              = gb->u(1);
        vui->motion_vectors_over_pic_boundaries_flag = gb->u(1);
        vui->restricted_ref_pic_lists_flag           = gb->u(1);
        vui->min_spatial_segmentation_idc            = gb->ue();
        vui->max_bytes_per_pic_denom                 = gb->ue();
        vui->max_bits_per_min_cu_denom               = gb->ue();
        vui->log2_max_mv_length_horizontal           = gb->ue();
        vui->log2_max_mv_length_vertical             = gb->ue();
    }

    if (gb->get_bits_left() < 1 && !alt) {
        // XXX: Alternate syntax when sps_range_extension_flag != 0?
        av_log(avctx, AV_LOG_WARNING,
               "Overread in VUI, retrying from timing information...\n");
        memcpy(vui, &backup_vui, sizeof(backup_vui));
        memcpy(gb, &backup, sizeof(backup));
        alt = 1;
        goto timing_info;
    }
}
#endif

int hevc_parse_sps(HEVCSPS *sps, cBitReader *gb, unsigned int *sps_id,
                      int apply_defdispwin, /*AVBufferRef **vps_list,*/ AVCodecContext *avctx)
{
    HEVCWindow *ow;
    int ret = 0;
    int log2_diff_max_min_transform_block_size;
    int bit_depth_chroma, start, vui_present, sublayer_ordering_info;
    int i;

    // Coded parameters

    sps->vps_id = gb->u( 4);
    if (sps->vps_id >= HEVC_MAX_VPS_COUNT) {
        av_log(avctx, AV_LOG_ERROR, "VPS id out of range: %d\n", sps->vps_id);
        return AVERROR_INVALIDDATA;
    }

//    if (vps_list && !vps_list[sps->vps_id]) {
//        av_log(avctx, AV_LOG_ERROR, "VPS %d does not exist\n",
//               sps->vps_id);
//        return AVERROR_INVALIDDATA;
//    }

    sps->max_sub_layers = gb->u( 3) + 1;
    if (sps->max_sub_layers > HEVC_MAX_SUB_LAYERS) {
        av_log(avctx, AV_LOG_ERROR, "sps_max_sub_layers out of range: %d\n",
               sps->max_sub_layers);
        return AVERROR_INVALIDDATA;
    }

    sps->temporal_id_nesting_flag = gb->u( 1);

    if ((ret = parse_ptl(gb, avctx, &sps->ptl, sps->max_sub_layers)) < 0)
        return ret;

    //*sps_id = gb->ue();
    *sps_id = gb->ue();
    if (*sps_id >= HEVC_MAX_SPS_COUNT) {
        av_log(avctx, AV_LOG_ERROR, "SPS id out of range: %d\n", *sps_id);
        return AVERROR_INVALIDDATA;
    }

    sps->chroma_format_idc = gb->ue();
    if (sps->chroma_format_idc > 3U) {
        av_log(avctx, AV_LOG_ERROR, "chroma_format_idc %d is invalid\n", sps->chroma_format_idc);
        return AVERROR_INVALIDDATA;
    }

    if (sps->chroma_format_idc == 3)
        sps->separate_colour_plane_flag = gb->u(1);

    if (sps->separate_colour_plane_flag)
        sps->chroma_format_idc = 0;

    sps->width  = gb->ue();
    sps->height = gb->ue();
#if 0
/*
    if ((ret = av_image_check_size(sps->width,
                                   sps->height, 0, avctx)) < 0)
        return ret;
*/

    if (gb->u(1)) { // pic_conformance_flag
        int vert_mult  = 1 + (sps->chroma_format_idc < 2);
        int horiz_mult = 1 + (sps->chroma_format_idc < 3);
        sps->pic_conf_win.left_offset   = gb->ue() * horiz_mult;
        sps->pic_conf_win.right_offset  = gb->ue() * horiz_mult;
        sps->pic_conf_win.top_offset    = gb->ue() *  vert_mult;
        sps->pic_conf_win.bottom_offset = gb->ue() *  vert_mult;
/*
        if (avctx->flags2 & AV_CODEC_FLAG2_IGNORE_CROP) {
            av_log(avctx, AV_LOG_DEBUG,
                   "discarding sps conformance window, "
                           "original values are l:%u r:%u t:%u b:%u\n",
                   sps->pic_conf_win.left_offset,
                   sps->pic_conf_win.right_offset,
                   sps->pic_conf_win.top_offset,
                   sps->pic_conf_win.bottom_offset);

            sps->pic_conf_win.left_offset   =
            sps->pic_conf_win.right_offset  =
            sps->pic_conf_win.top_offset    =
            sps->pic_conf_win.bottom_offset = 0;
        }*/
        sps->output_window = sps->pic_conf_win;
    }

    sps->bit_depth   = gb->ue() + 8;
    bit_depth_chroma = gb->ue() + 8;
    if (sps->chroma_format_idc && bit_depth_chroma != sps->bit_depth) {
        av_log(avctx, AV_LOG_ERROR,
               "Luma bit depth (%d) is different from chroma bit depth (%d), "
                       "this is unsupported.\n",
               sps->bit_depth, bit_depth_chroma);
        return AVERROR_INVALIDDATA;
    }
    sps->bit_depth_chroma = bit_depth_chroma;

/*
    ret = map_pixel_format(avctx, sps);
    if (ret < 0)
        return ret;
*/

    sps->log2_max_poc_lsb = gb->ue() + 4;
    if (sps->log2_max_poc_lsb > 16) {
        av_log(avctx, AV_LOG_ERROR, "log2_max_pic_order_cnt_lsb_minus4 out range: %d\n",
               sps->log2_max_poc_lsb - 4);
        return AVERROR_INVALIDDATA;
    }

    sublayer_ordering_info = gb->u(1);
    start = sublayer_ordering_info ? 0 : sps->max_sub_layers - 1;
    for (i = start; i < sps->max_sub_layers; i++) {
        sps->temporal_layer[i].max_dec_pic_buffering = gb->ue() + 1;
        sps->temporal_layer[i].num_reorder_pics      = gb->ue();
        sps->temporal_layer[i].max_latency_increase  = gb->ue() - 1;
        if (sps->temporal_layer[i].max_dec_pic_buffering > (unsigned)HEVC_MAX_DPB_SIZE) {
            av_log(avctx, AV_LOG_ERROR, "sps_max_dec_pic_buffering_minus1 out of range: %d\n",
                   sps->temporal_layer[i].max_dec_pic_buffering - 1U);
            return AVERROR_INVALIDDATA;
        }
        if (sps->temporal_layer[i].num_reorder_pics > sps->temporal_layer[i].max_dec_pic_buffering - 1) {
            av_log(avctx, AV_LOG_WARNING, "sps_max_num_reorder_pics out of range: %d\n",
                   sps->temporal_layer[i].num_reorder_pics);
/*

            if (avctx->err_recognition & AV_EF_EXPLODE ||
                sps->temporal_layer[i].num_reorder_pics > HEVC_MAX_DPB_SIZE - 1) {
                return AVERROR_INVALIDDATA;
            }
*/
            sps->temporal_layer[i].max_dec_pic_buffering = sps->temporal_layer[i].num_reorder_pics + 1;
        }
    }

    if (!sublayer_ordering_info) {
        for (i = 0; i < start; i++) {
            sps->temporal_layer[i].max_dec_pic_buffering = sps->temporal_layer[start].max_dec_pic_buffering;
            sps->temporal_layer[i].num_reorder_pics      = sps->temporal_layer[start].num_reorder_pics;
            sps->temporal_layer[i].max_latency_increase  = sps->temporal_layer[start].max_latency_increase;
        }
    }

    sps->log2_min_cb_size                    = gb->ue() + 3;
    sps->log2_diff_max_min_coding_block_size = gb->ue();
    sps->log2_min_tb_size                    = gb->ue() + 2;
    log2_diff_max_min_transform_block_size   = gb->ue();
    sps->log2_max_trafo_size                 = log2_diff_max_min_transform_block_size +
                                               sps->log2_min_tb_size;

    if (sps->log2_min_cb_size < 3 || sps->log2_min_cb_size > 30) {
        av_log(avctx, AV_LOG_ERROR, "Invalid value %d for log2_min_cb_size", sps->log2_min_cb_size);
        return AVERROR_INVALIDDATA;
    }

    if (sps->log2_diff_max_min_coding_block_size > 30) {
        av_log(avctx, AV_LOG_ERROR, "Invalid value %d for log2_diff_max_min_coding_block_size", sps->log2_diff_max_min_coding_block_size);
        return AVERROR_INVALIDDATA;
    }

    if (sps->log2_min_tb_size >= sps->log2_min_cb_size || sps->log2_min_tb_size < 2) {
        av_log(avctx, AV_LOG_ERROR, "Invalid value for log2_min_tb_size");
        return AVERROR_INVALIDDATA;
    }

    if (log2_diff_max_min_transform_block_size < 0 || log2_diff_max_min_transform_block_size > 30) {
        av_log(avctx, AV_LOG_ERROR, "Invalid value %d for log2_diff_max_min_transform_block_size", log2_diff_max_min_transform_block_size);
        return AVERROR_INVALIDDATA;
    }

    sps->max_transform_hierarchy_depth_inter = gb->ue();
    sps->max_transform_hierarchy_depth_intra = gb->ue();

    sps->scaling_list_enable_flag = gb->u(1);
    if (sps->scaling_list_enable_flag) {
        set_default_scaling_list_data(&sps->scaling_list);

        if (gb->u(1)) {
            ret = scaling_list_data(gb, avctx, &sps->scaling_list, sps);
            if (ret < 0)
                return ret;
        }
    }

    sps->amp_enabled_flag = gb->u(1);
    sps->sao_enabled      = gb->u(1);

    sps->pcm_enabled_flag = gb->u(1);
    if (sps->pcm_enabled_flag) {
        sps->pcm.bit_depth   = gb->u( 4) + 1;
        sps->pcm.bit_depth_chroma = gb->u( 4) + 1;
        sps->pcm.log2_min_pcm_cb_size = gb->ue() + 3;
        sps->pcm.log2_max_pcm_cb_size = sps->pcm.log2_min_pcm_cb_size +
                                        gb->ue();
 /*       if (FFMAX(sps->pcm.bit_depth, sps->pcm.bit_depth_chroma) > sps->bit_depth) {
            av_log(avctx, AV_LOG_ERROR,
                   "PCM bit depth (%d, %d) is greater than normal bit depth (%d)\n",
                   sps->pcm.bit_depth, sps->pcm.bit_depth_chroma, sps->bit_depth);
            return AVERROR_INVALIDDATA;
        }*/

        sps->pcm.loop_filter_disable_flag = gb->u(1);
    }

    sps->nb_st_rps = gb->ue();
    if (sps->nb_st_rps > HEVC_MAX_SHORT_TERM_REF_PIC_SETS) {
        av_log(avctx, AV_LOG_ERROR, "Too many short term RPS: %d.\n",
               sps->nb_st_rps);
        return AVERROR_INVALIDDATA;
    }
    for (i = 0; i < sps->nb_st_rps; i++) {
        if ((ret = ff_hevc_decode_short_term_rps(gb, avctx, &sps->st_rps[i],
                                                 sps, 0)) < 0)
            return ret;
    }

    sps->long_term_ref_pics_present_flag = gb->u(1);
    if (sps->long_term_ref_pics_present_flag) {
        sps->num_long_term_ref_pics_sps = gb->ue();
        if (sps->num_long_term_ref_pics_sps > HEVC_MAX_LONG_TERM_REF_PICS) {
            av_log(avctx, AV_LOG_ERROR, "Too many long term ref pics: %d.\n",
                   sps->num_long_term_ref_pics_sps);
            return AVERROR_INVALIDDATA;
        }
        for (i = 0; i < sps->num_long_term_ref_pics_sps; i++) {
            sps->lt_ref_pic_poc_lsb_sps[i]       = gb->u( sps->log2_max_poc_lsb);
            sps->used_by_curr_pic_lt_sps_flag[i] = gb->u(1);
        }
    }

    sps->sps_temporal_mvp_enabled_flag          = gb->u(1);
    sps->sps_strong_intra_smoothing_enable_flag = gb->u(1);
    //sps->vui.sar = (AVRational){0, 1};
    vui_present = gb->u(1);
    if (vui_present)
        decode_vui(gb, avctx, apply_defdispwin, sps);

    if (gb->u(1)) { // sps_extension_flag
        int sps_extension_flag[1];
        for (i = 0; i < 1; i++)
            sps_extension_flag[i] = gb->u(1);
        gb->u( 7); //sps_extension_7bits = gb->u( 7);
        if (sps_extension_flag[0]) {
            int extended_precision_processing_flag;
            int high_precision_offsets_enabled_flag;
            int cabac_bypass_alignment_enabled_flag;

            sps->transform_skip_rotation_enabled_flag = gb->u(1);
            sps->transform_skip_context_enabled_flag  = gb->u(1);
            sps->implicit_rdpcm_enabled_flag = gb->u(1);

            sps->explicit_rdpcm_enabled_flag = gb->u(1);

            extended_precision_processing_flag = gb->u(1);
            if (extended_precision_processing_flag)
                    av_log(avctx, AV_LOG_WARNING,
                           "extended_precision_processing_flag not yet implemented\n");

            sps->intra_smoothing_disabled_flag       = gb->u(1);
            high_precision_offsets_enabled_flag  = gb->u(1);
            if (high_precision_offsets_enabled_flag)
                    av_log(avctx, AV_LOG_WARNING,
                           "high_precision_offsets_enabled_flag not yet implemented\n");

            sps->persistent_rice_adaptation_enabled_flag = gb->u(1);

            cabac_bypass_alignment_enabled_flag  = gb->u(1);
            if (cabac_bypass_alignment_enabled_flag)
                    av_log(avctx, AV_LOG_WARNING,
                           "cabac_bypass_alignment_enabled_flag not yet implemented\n");
        }
    }
    if (apply_defdispwin) {
        sps->output_window.left_offset   += sps->vui.def_disp_win.left_offset;
        sps->output_window.right_offset  += sps->vui.def_disp_win.right_offset;
        sps->output_window.top_offset    += sps->vui.def_disp_win.top_offset;
        sps->output_window.bottom_offset += sps->vui.def_disp_win.bottom_offset;
    }

    ow = &sps->output_window;
 /*   if (ow->left_offset >= INT_MAX - ow->right_offset     ||
        ow->top_offset  >= INT_MAX - ow->bottom_offset    ||
        ow->left_offset + ow->right_offset  >= sps->width ||
        ow->top_offset  + ow->bottom_offset >= sps->height) {
        av_log(avctx, AV_LOG_WARNING, "Invalid cropping offsets: %u/%u/%u/%u\n",
               ow->left_offset, ow->right_offset, ow->top_offset, ow->bottom_offset);
        if (avctx->err_recognition & AV_EF_EXPLODE) {
            return AVERROR_INVALIDDATA;
        }
        av_log(avctx, AV_LOG_WARNING,
               "Displaying the whole video surface.\n");
        memset(ow, 0, sizeof(*ow));
        memset(&sps->pic_conf_win, 0, sizeof(sps->pic_conf_win));
    }
*/
    // Inferred parameters
    sps->log2_ctb_size = sps->log2_min_cb_size +
                         sps->log2_diff_max_min_coding_block_size;
    sps->log2_min_pu_size = sps->log2_min_cb_size - 1;

    if (sps->log2_ctb_size > HEVC_MAX_LOG2_CTB_SIZE) {
        av_log(avctx, AV_LOG_ERROR, "CTB size out of range: 2^%d\n", sps->log2_ctb_size);
        return AVERROR_INVALIDDATA;
    }
/*    if (sps->log2_ctb_size < 4) {
        av_log(avctx,
               AV_LOG_ERROR,
               "log2_ctb_size %d differs from the bounds of any known profile\n",
               sps->log2_ctb_size);
        avpriv_request_sample(avctx, "log2_ctb_size %d", sps->log2_ctb_size);
        return AVERROR_INVALIDDATA;
    }*/

    sps->ctb_width  = (sps->width  + (1 << sps->log2_ctb_size) - 1) >> sps->log2_ctb_size;
    sps->ctb_height = (sps->height + (1 << sps->log2_ctb_size) - 1) >> sps->log2_ctb_size;
    sps->ctb_size   = sps->ctb_width * sps->ctb_height;

    sps->min_cb_width  = sps->width  >> sps->log2_min_cb_size;
    sps->min_cb_height = sps->height >> sps->log2_min_cb_size;
    sps->min_tb_width  = sps->width  >> sps->log2_min_tb_size;
    sps->min_tb_height = sps->height >> sps->log2_min_tb_size;
    sps->min_pu_width  = sps->width  >> sps->log2_min_pu_size;
    sps->min_pu_height = sps->height >> sps->log2_min_pu_size;
    sps->tb_mask       = (1 << (sps->log2_ctb_size - sps->log2_min_tb_size)) - 1;

    sps->qp_bd_offset = 6 * (sps->bit_depth - 8);

/*    if (av_mod_uintp2(sps->width, sps->log2_min_cb_size) ||
        av_mod_uintp2(sps->height, sps->log2_min_cb_size)) {
        av_log(avctx, AV_LOG_ERROR, "Invalid coded frame dimensions.\n");
        return AVERROR_INVALIDDATA;
    }*/

    if (sps->max_transform_hierarchy_depth_inter > sps->log2_ctb_size - sps->log2_min_tb_size) {
        av_log(avctx, AV_LOG_ERROR, "max_transform_hierarchy_depth_inter out of range: %d\n",
               sps->max_transform_hierarchy_depth_inter);
        return AVERROR_INVALIDDATA;
    }
    if (sps->max_transform_hierarchy_depth_intra > sps->log2_ctb_size - sps->log2_min_tb_size) {
        av_log(avctx, AV_LOG_ERROR, "max_transform_hierarchy_depth_intra out of range: %d\n",
               sps->max_transform_hierarchy_depth_intra);
        return AVERROR_INVALIDDATA;
    }
/*

    if (sps->log2_max_trafo_size > FFMIN(sps->log2_ctb_size, 5)) {
        av_log(avctx, AV_LOG_ERROR,
               "max transform block size out of range: %d\n",
               sps->log2_max_trafo_size);
        return AVERROR_INVALIDDATA;
    }
*/

    if (gb->get_bits_left() < 0) {
        av_log(avctx, AV_LOG_ERROR,
               "Overread SPS by %d bits\n", -gb->get_bits_left());
        return AVERROR_INVALIDDATA;
    }
#endif
    return 0;
}



static int extract_rbsp(const char *src,  char *dst, int length) {
    int i, si, di;


   si = di = i = 0;
    while (si + 2 < length) {
/*        // remove escapes (very rare 1:2^22)
        if (src[si + 2] > 3) {
            dst[di++] = src[si++];
            dst[di++] = src[si++];
        } else */
        if (src[si] == 0 && src[si + 1] == 0 && src[si + 2] != 0) {
            if (src[si + 2] == 3) { // escape
                dst[di++] = 0;
                dst[di++] = 0;
                si += 3;

                continue;
            }
        }

        dst[di++] = src[si++];
    }
    while (si < length)
        dst[di++] = src[si++];
    return di;
}

bool hevc_decode_nal_sps(const char *spsData, int len, int *pwidth, int *pheight) {
    HEVCSPS sps;
    AVCodecContext avctx;

    int res = 0;
    unsigned int sps_id;
    int apply_defdispwin = 0;
    char *rbsp =(char *) malloc(len * 2);
    int rbspLen = extract_rbsp(spsData, rbsp, len );

    cBitReader gb(rbsp + 2, rbspLen - 2);
    res = hevc_parse_sps(&sps, &gb, &sps_id, apply_defdispwin, /*ps->vps_list,*/ &avctx);

    if(AVERROR_INVALIDDATA != res) {
        *pwidth = sps.width;
        *pheight = sps.height;
    }
    free(rbsp);
    return true;
}
