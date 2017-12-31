In archive libavcodec.a:

aac_ac3_parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aac_ac3_parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000001e4 ff_aac_ac3_parse
00000000         *UND*	00000000 ff_combine_frame



aac_parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aac_parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000024 aac_parse_init
00000000 l     F .text	000000a8 aac_sync
00000000 l    d  .data.rel	00000000 .data.rel
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 ff_adts_header_parse
00000000         *UND*	00000000 ff_mpeg4audio_channels
00000000 g     O .data.rel	0000002c ff_aac_parser
00000000         *UND*	00000000 ff_aac_ac3_parse
00000000         *UND*	00000000 ff_parse_close



aacdec.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aacdec.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000030 skip_bits_long
00000030 l     F .text	00000058 get_bits
00000088 l     F .text	0000001c skip_bits
000000a4 l     F .text	00000034 get_bits1
000000d8 l     F .text	00000054 apply_independent_coupling
0000012c l     F .text	00000090 assign_pair
000001bc l     F .text	000000b4 count_paired_channels
00000270 l     F .text	00000048 push_output_configuration
000002b8 l     F .text	00000074 decode_ltp
0000032c l     F .text	00000120 apply_channel_coupling
0000044c l     F .text	0000005c latm_get_value
000004a8 l     F .text	00000068 flush
00000510 l     F .text	00000144 windowing_and_mdct_ltp
00000654 l     F .text	00000140 apply_ltp
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	000000a4 aac_decode_close
00000794 l     F .text	00000478 decode_ics_info
00000013 l       .rodata.str1.1	00000000 .LC1
00000000 l       .rodata.str1.1	00000000 .LC0
00000060 l       .rodata.str1.1	00000000 .LC2
00000080 l       .rodata.str1.1	00000000 .LC3
000000a6 l       .rodata.str1.1	00000000 .LC4
000000cd l       .rodata.str1.1	00000000 .LC5
00000c0c l     F .text	0000021c decode_tns
0000010c l       .rodata.str1.1	00000000 .LC6
00000e28 l     F .text	00000130 apply_dependent_coupling
0000013d l       .rodata.str1.1	00000000 .LC7
00000f58 l     F .text	000000e4 frame_configure_elements
0000103c l     F .text	0000031c imdct_and_windowing_960
00001358 l     F .text	000000e0 imdct_and_windowing_ld
00001438 l     F .text	000001c8 update_ltp
00001600 l     F .text	00000304 imdct_and_windowing
00001904 l     F .text	00000344 imdct_and_windowing_eld
000000a4 l     F .text.unlikely	0000062c aac_static_table_init
00001c48 l     F .text	000000d0 set_default_channel_config
00000174 l       .rodata.str1.1	00000000 .LC8
000001a0 l       .rodata.str1.1	00000000 .LC9
00001d18 l     F .text	00000824 output_configure
00000243 l       .rodata.str1.1	00000000 .LC10
00000259 l       .rodata.str1.1	00000000 .LC11
0000253c l     F .text	00000410 get_che
00000274 l       .rodata.str1.1	00000000 .LC14
00000283 l       .rodata.str1.1	00000000 .LC15
0000026c l       .rodata.str1.1	00000000 .LC12
00000270 l       .rodata.str1.1	00000000 .LC13
00000294 l       .rodata.str1.1	00000000 .LC16
000002eb l       .rodata.str1.1	00000000 .LC17
0000294c l     F .text	0000007c pop_output_configuration
000029c8 l     F .text	00000308 spectral_to_sample.isra.18
00000342 l       .rodata.str1.1	00000000 .LC18
00002cd0 l     F .text	000002c4 apply_tns
00002f94 l     F .text	0000003c show_bits.constprop.28
00002fd0 l     F .text	000000d4 decode_channel_map
00000381 l       .rodata.str1.1	00000000 .LC21
00000361 l       .rodata.str1.1	00000000 .LC19
0000037f l       .rodata.str1.1	00000000 .LC20
000030a4 l     F .text	0000026c decode_pce.isra.20
0000039e l       .rodata.str1.1	00000000 .LC22
0000040d l       .rodata.str1.1	00000000 .LC23
00003310 l     F .text	00000298 decode_ga_specific_config
0000044a l       .rodata.str1.1	00000000 .LC24
00000464 l       .rodata.str1.1	00000000 .LC25
00000483 l       .rodata.str1.1	00000000 .LC26
000035a8 l     F .text	000002f0 decode_audio_specific_config_gb
00000495 l       .rodata.str1.1	00000000 .LC29
000004b5 l       .rodata.str1.1	00000000 .LC30
000004df l       .rodata.str1.1	00000000 .LC31
000004ed l       .rodata.str1.1	00000000 .LC32
00000494 l       .rodata.str1.1	00000000 .LC28
0000048f l       .rodata.str1.1	00000000 .LC27
0000051e l       .rodata.str1.1	00000000 .LC33
00003898 l     F .text	0000022c latm_decode_audio_specific_config
00000535 l       .rodata.str1.1	00000000 .LC34
0000054b l       .rodata.str1.1	00000000 .LC35
00003ac4 l     F .text	000000b8 decode_audio_specific_config.constprop.26
00000561 l       .rodata.str1.1	00000000 .LC36
000006d0 l     F .text.unlikely	000003cc aac_decode_init
00000a9c l     F .text.unlikely	0000002c latm_decode_init
00003b7c l     F .text	00000300 apply_prediction.isra.5
00003e7c l     F .text	00001668 decode_ics.constprop.29
00000588 l       .rodata.str1.1	00000000 .LC37
0000059b l       .rodata.str1.1	00000000 .LC38
000005df l       .rodata.str1.1	00000000 .LC39
00000609 l       .rodata.str1.1	00000000 .LC40
0000067b l       .rodata.str1.1	00000000 .LC41
000006de l       .rodata.str1.1	00000000 .LC42
000006fe l       .rodata.str1.1	00000000 .LC43
0000072f l       .rodata.str1.1	00000000 .LC44
00000733 l       .rodata.str1.1	00000000 .LC45
00000759 l       .rodata.str1.1	00000000 .LC46
000054e4 l     F .text	000004a8 decode_cpe
00000779 l       .rodata.str1.1	00000000 .LC47
0000598c l     F .text	0000020c aac_decode_er_frame
00000796 l       .rodata.str1.1	00000000 .LC48
000007ba l       .rodata.str1.1	00000000 .LC49
000007e2 l       .rodata.str1.1	00000000 .LC50
00005b98 l     F .text	0000114c aac_decode_frame_int.isra.23
000007f7 l       .rodata.str1.1	00000000 .LC51
0000081c l       .rodata.str1.1	00000000 .LC52
0000083e l       .rodata.str1.1	00000000 .LC53
00000852 l       .rodata.str1.1	00000000 .LC54
0000089d l       .rodata.str1.1	00000000 .LC55
0000092f l       .rodata.str1.1	00000000 .LC57
000008f4 l       .rodata.str1.1	00000000 .LC56
0000094a l       .rodata.str1.1	00000000 .LC58
0000097b l       .rodata.str1.1	00000000 .LC59
000009bb l       .rodata.str1.1	00000000 .LC60
00000a02 l       .rodata.str1.1	00000000 .LC61
00000a0b l       .rodata.str1.1	00000000 .LC62
00006ce4 l     F .text	00000184 aac_decode_frame
00006e68 l     F .text	00000504 latm_decode_frame
00000a19 l       .rodata.str1.1	00000000 .LC63
00000a2b l       .rodata.str1.1	00000000 .LC64
00000a3b l       .rodata.str1.1	00000000 .LC65
00000a54 l       .rodata.str1.1	00000000 .LC66
00000a66 l       .rodata.str1.1	00000000 .LC67
00000a86 l       .rodata.str1.1	00000000 .LC68
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000020 ltp_coef
00000020 l     O .rodata	00000040 exp2_lut.11191
00000060 l     O .rodata	00000010 tags_per_config
00000070 l     O .rodata	000000f0 aac_channel_layout_map
00000160 l     O .rodata	00000010 cce_scale
00000170 l     O .rodata	00000008 __compound_literal.1
00000178 l     O .rodata	00000008 __compound_literal.0
00000180 l     O .rodata	00000080 aac_channel_layout
00000200 l     O .rodata	00000040 tns_tmp2_map_0_4
00000240 l     O .rodata	00000020 tns_tmp2_map_1_4
00000260 l     O .rodata	00000020 tns_tmp2_map_0_3
00000280 l     O .rodata	00000010 tns_tmp2_map_1_3
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l     O .data.rel.ro	0000002c aac_decoder_class
00000010 l     O .data.rel.ro.local	00000120 options
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	00000010 tns_tmp2_map
00000000 l    d  .data.rel.local	00000000 .data.rel.local
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .bss	000000b0 vlc_spectral
000000b0 l     O .bss	000004c0 table.12155
00000570 l     O .bss	00000438 table.12156
000009a8 l     O .bss	00000898 table.12157
00001240 l     O .bss	000004b0 table.12158
000016f0 l     O .bss	00000520 table.12159
00001c10 l     O .bss	00000498 table.12160
000020a8 l     O .bss	000004c8 table.12161
00002570 l     O .bss	00000430 table.12162
000029a0 l     O .bss	000007f8 table.12163
00003198 l     O .bss	000005b8 table.12164
00003750 l     O .bss	00000738 table.12165
00003e88 l     O .bss	00000010 vlc_scalefactors
00003e98 l     O .bss	00000580 table.12166
00004418 l     O .bss	00000004 aac_table_init
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_aac_kbd_long_1024
00000000         *UND*	00000000 ff_aac_kbd_short_128
00000000         *UND*	00000000 ff_sine_1024
00000000         *UND*	00000000 ff_sine_128
00000000         *UND*	00000000 ff_aac_sbr_ctx_close
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 ff_mdct_end
00000000         *UND*	00000000 ff_mdct15_uninit
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 ff_swb_offset_120
00000000         *UND*	00000000 ff_aac_num_swb_120
00000000         *UND*	00000000 ff_swb_offset_128
00000000         *UND*	00000000 ff_aac_num_swb_128
00000000         *UND*	00000000 ff_tns_max_bands_128
00000000         *UND*	00000000 ff_swb_offset_480
00000000         *UND*	00000000 ff_aac_num_swb_480
00000000         *UND*	00000000 ff_tns_max_bands_480
00000000         *UND*	00000000 ff_swb_offset_512
00000000         *UND*	00000000 ff_aac_num_swb_512
00000000         *UND*	00000000 ff_tns_max_bands_512
00000000         *UND*	00000000 ff_aac_num_swb_960
00000000         *UND*	00000000 ff_swb_offset_960
00000000         *UND*	00000000 ff_aac_num_swb_1024
00000000         *UND*	00000000 ff_swb_offset_1024
00000000         *UND*	00000000 ff_tns_max_bands_1024
00000000         *UND*	00000000 ff_aac_pred_sfb_max
00000000         *UND*	00000000 av_frame_unref
00000000         *UND*	00000000 ff_get_buffer
00000000         *UND*	00000000 ff_sine_120
00000000         *UND*	00000000 ff_aac_kbd_short_120
00000000         *UND*	00000000 ff_sine_960
00000000         *UND*	00000000 ff_aac_kbd_long_960
00000000         *UND*	00000000 ff_sine_512
00000000         *UND*	00000000 memmove
00000000         *UND*	00000000 ff_aac_eld_window_480
00000000         *UND*	00000000 ff_aac_eld_window_512
00000000         *UND*	00000000 ff_init_vlc_sparse
00000000         *UND*	00000000 ff_aac_sbr_init
00000000         *UND*	00000000 ff_kbd_window_init
00000000         *UND*	00000000 ff_sine_window_init
00000000         *UND*	00000000 ff_init_ff_sine_windows
00000000         *UND*	00000000 ff_cbrt_tableinit
00000000         *UND*	00000000 ff_aac_spectral_sizes
00000000         *UND*	00000000 ff_aac_spectral_bits
00000000         *UND*	00000000 ff_aac_spectral_codes
00000000         *UND*	00000000 ff_aac_pow2sf_tab
00000000         *UND*	00000000 ff_aac_pow34sf_tab
00000000         *UND*	00000000 ff_aac_scalefactor_bits
00000000         *UND*	00000000 ff_aac_scalefactor_code
00000000         *UND*	00000000 avpriv_request_sample
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 ff_aac_sbr_ctx_init
00000000         *UND*	00000000 ff_sbr_apply
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 avpriv_report_missing_feature
00000000         *UND*	00000000 ff_mpeg4audio_get_config_gb
00000000         *UND*	00000000 av_free
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 pthread_once
00000000         *UND*	00000000 avpriv_float_dsp_alloc
00000000         *UND*	00000000 ff_mdct_init
00000000         *UND*	00000000 ff_mdct15_init
00000000         *UND*	00000000 ff_mpeg4audio_channels
00000000         *UND*	00000000 __aeabi_idiv
00000000         *UND*	00000000 ff_aac_codebook_vector_vals
00000000         *UND*	00000000 ff_aac_codebook_vector_idx
00000000         *UND*	00000000 ff_cbrt_tab
00000000         *UND*	00000000 cbrtf
00000000         *UND*	00000000 ff_adts_header_parse
00000000         *UND*	00000000 powf
00000000         *UND*	00000000 ff_decode_sbr_extension
00000000         *UND*	00000000 sscanf
00000000         *UND*	00000000 av_packet_get_side_data
00000000 g     O .data.rel.local	00000080 ff_aac_latm_decoder
00000080 g     O .data.rel.local	00000080 ff_aac_decoder
00000000         *UND*	00000000 av_default_item_name



aacps_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aacps_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_bits
00000058 l     F .text	00000034 get_bits1
0000008c l     F .text	0000017c map_val_34_to_20
00000208 l     F .text	000000f0 map_val_20_to_34
000002f8 l     F .text	000002c0 remap34
000005b8 l     F .text	00000248 remap20
00000800 l     F .text	000004cc decorrelation
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	000000e0 make_filters_from_proto
00000ccc l     F .text	000002d4 read_ipdopd_data.isra.1
00000fa0 l     F .text	000000e4 hybrid2_re.constprop.7
00001084 l     F .text	0000004c hybrid4_8_12_cx.isra.3.constprop.8
000010d0 l     F .text	00000250 hybrid_synthesis.isra.4.constprop.9
00001320 l     F .text	00000868 stereo_processing
00000000 l       .rodata.str1.1	00000000 .LC0
0000001a l       .rodata.str1.1	00000000 .LC1
00000034 l       .rodata.str1.1	00000000 .LC2
00000053 l       .rodata.str1.1	00000000 .LC3
00000060 l       .rodata.str1.1	00000000 .LC4
0000006d l       .rodata.str1.1	00000000 .LC5
00000087 l       .rodata.str1.1	00000000 .LC6
00000098 l       .rodata.str1.1	00000000 .LC7
000000a9 l       .rodata.str1.1	00000000 .LC8
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	0000005b k_to_i_34
0000005b l     O .rodata	00000047 k_to_i_20
000000a4 l     O .rodata	00000008 NR_BANDS
000000ac l     O .rodata	00000008 NR_PAR_BANDS
000000b4 l     O .rodata	00000008 DECAY_CUTOFF
000000bc l     O .rodata	00000008 NR_ALLPASS_BANDS
000000c4 l     O .rodata	00000008 SHORT_DELAY_BAND
000000cc l     O .rodata	0000001c g1_Q2
000000e8 l     O .rodata	00000008 NR_IPDOPD_BANDS
000000f0 l     O .rodata	00000006 nr_iidicc_par_tab
000000f6 l     O .rodata	00000006 nr_iidopd_par_tab
000000fc l     O .rodata	00000008 num_env_tab
00000104 l     O .rodata	00000010 huff_iid
00000114 l     O .rodata	0000000a huff_offset
0000011e l     O .rodata	0000003d huff_iid_df1_bits
0000015c l     O .rodata	000000f4 huff_iid_df1_codes
00000250 l     O .rodata	0000003d huff_iid_dt1_bits
0000028e l     O .rodata	0000007a huff_iid_dt1_codes
00000308 l     O .rodata	0000001d huff_iid_df0_bits
00000328 l     O .rodata	00000074 huff_iid_df0_codes
0000039c l     O .rodata	0000001d huff_iid_dt0_bits
000003bc l     O .rodata	00000074 huff_iid_dt0_codes
00000430 l     O .rodata	0000000f huff_icc_df_bits
00000440 l     O .rodata	0000001e huff_icc_df_codes
0000045e l     O .rodata	0000000f huff_icc_dt_bits
0000046e l     O .rodata	0000001e huff_icc_dt_codes
0000048c l     O .rodata	00000008 huff_ipd_df_bits
00000494 l     O .rodata	00000008 huff_ipd_df_codes
0000049c l     O .rodata	00000008 huff_ipd_dt_bits
000004a4 l     O .rodata	00000008 huff_ipd_dt_codes
000004ac l     O .rodata	00000008 huff_opd_df_bits
000004b4 l     O .rodata	00000008 huff_opd_df_codes
000004bc l     O .rodata	00000008 huff_opd_dt_bits
000004c4 l     O .rodata	00000008 huff_opd_dt_codes
000004cc l     O .rodata	00000020 ipdopd_cos.9648
000004ec l     O .rodata	00000020 ipdopd_sin.9647
0000050c l     O .rodata	000000b8 iid_par_dequant.9652
000005c4 l     O .rodata	00000020 acos_icc_invq.9654
000005e4 l     O .rodata	00000020 icc_invq.9653
00000604 l     O .rodata	0000000a f_center_20.9659
00000610 l     O .rodata	0000000c fractional_delay_links.9661
0000061c l     O .rodata	00000020 f_center_34.9660
0000063c l     O .rodata	0000001c g0_Q8
00000658 l     O .rodata	0000001c g0_Q12
00000674 l     O .rodata	0000001c g1_Q8
00000690 l     O .rodata	0000001c g2_Q4
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .bss	00000320 phi_fract
00000320 l     O .bss	00000960 Q_fract_allpass
00000c80 l     O .bss	000000a0 vlc_ps
00000d20 l     O .bss	00001700 HA
00002420 l     O .bss	00001700 HB
00003b20 l     O .bss	00000800 pd_re_smooth
00004320 l     O .bss	00000800 pd_im_smooth
00004b20 l     O .bss	00000300 f34_0_12
00004e20 l     O .bss	00000200 f34_1_8
00005020 l     O .bss	00000100 f34_2_4
00005120 l     O .bss	00000200 f20_0_8
00005320 l     O .bss	00001820 table.10166
00006b40 l     O .bss	00000d00 table.10167
00007840 l     O .bss	00001000 table.10168
00008840 l     O .bss	00001030 table.10169
00009870 l     O .bss	00000880 table.10170
0000a0f0 l     O .bss	00000880 table.10171
0000a970 l     O .bss	00000800 table.10172
0000b170 l     O .bss	00000800 table.10173
0000b970 l     O .bss	00000800 table.10174
0000c170 l     O .bss	00000800 table.10175
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_idiv
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 cos
00000000         *UND*	00000000 sin
00001b88 g     F .text	00000ec4 ff_ps_read_data
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 ff_log2_tab
00002a4c g     F .text	000003a4 ff_ps_apply
000000e0 g     F .text.unlikely	00000a20 ff_ps_init
00000000         *UND*	00000000 ff_init_vlc_sparse
00000000         *UND*	00000000 hypot
00000000         *UND*	00000000 cosf
00000000         *UND*	00000000 sinf
00000000         *UND*	00000000 atan2f
00000000         *UND*	00000000 atanf
00000b00 g     F .text.unlikely	0000000c ff_ps_ctx_init
00000000         *UND*	00000000 ff_psdsp_init



aacpsdsp_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aacpsdsp_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000038 ps_add_squares_c
00000038 l     F .text	00000048 ps_mul_pair_single_c
00000080 l     F .text	000000c0 ps_hybrid_analysis_c
00000140 l     F .text	00000060 ps_hybrid_analysis_ileave_c
000001a0 l     F .text	00000060 ps_hybrid_synthesis_deint_c
00000200 l     F .text	0000012c ps_decorrelate_c
0000032c l     F .text	00000094 ps_stereo_interpolate_c
000003c0 l     F .text	000000ec ps_stereo_interpolate_ipdopd_c
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000084 ff_psdsp_init
00000000         *UND*	00000000 ff_psdsp_init_arm



aacsbr.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aacsbr.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_bits
00000058 l     F .text	00000034 get_bits1
0000008c l     F .text	00000184 sbr_hf_inverse_filter
00000210 l     F .text	00000038 sbr_turnoff
00000248 l     F .text	000001dc sbr_x_gen
00000424 l     F .text	000000d4 sbr_lf_gen
000004f8 l     F .text	00000578 sbr_hf_assemble
00000a70 l     F .text	00000400 sbr_qmf_synthesis
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000098 make_bands
00000e70 l     F .text	00000060 read_sbr_dtdf.isra.2
00000ed0 l     F .text	00000050 read_sbr_invf.isra.3
00000098 l     F .text.unlikely	00000050 check_n_master
00000000 l       .rodata.str1.1	00000000 .LC0
00000016 l       .rodata.str1.1	00000000 .LC1
00000f20 l     F .text	00000864 read_sbr_envelope.isra.7
00000057 l       .rodata.str1.1	00000000 .LC2
00001784 l     F .text	000003d0 read_sbr_noise.isra.8
00000071 l       .rodata.str1.1	00000000 .LC3
00001b54 l     F .text	00000634 read_sbr_grid.isra.11
0000008d l       .rodata.str1.1	00000000 .LC4
000000d5 l       .rodata.str1.1	00000000 .LC5
0000014b l       .rodata.str1.1	00000000 .LC8
0000011d l       .rodata.str1.1	00000000 .LC6
0000013b l       .rodata.str1.1	00000000 .LC7
00000168 l       .rodata.str1.1	00000000 .LC9
000001ca l       .rodata.str1.1	00000000 .LC10
00002188 l     F .text	00001070 sbr_make_f_master.isra.12
000001ee l       .rodata.str1.1	00000000 .LC11
00000213 l       .rodata.str1.1	00000000 .LC12
0000022d l       .rodata.str1.1	00000000 .LC13
0000022f l       .rodata.str1.1	00000000 .LC14
0000025d l       .rodata.str1.1	00000000 .LC15
00000276 l       .rodata.str1.1	00000000 .LC16
0000028c l       .rodata.str1.1	00000000 .LC17
000031f8 l     F .text	0000044c sbr_make_f_tablelim
000002a2 l       .rodata.str1.1	00000000 .LC18
000002c6 l       .rodata.str1.1	00000000 .LC19
000002eb l       .rodata.str1.1	00000000 .LC20
00000313 l       .rodata.str1.1	00000000 .LC21
0000032e l       .rodata.str1.1	00000000 .LC22
00000344 l       .rodata.str1.1	00000000 .LC23
0000037e l       .rodata.str1.1	00000000 .LC24
0000041d l       .rodata.str1.1	00000000 .LC27
00000437 l       .rodata.str1.1	00000000 .LC28
000003b7 l       .rodata.str1.1	00000000 .LC25
00000405 l       .rodata.str1.1	00000000 .LC26
00000468 l       .rodata.str1.1	00000000 .LC29
00000488 l       .rodata.str1.1	00000000 .LC30
000004b1 l       .rodata.str1.1	00000000 .LC31
000004e9 l       .rodata.str1.1	00000000 .LC33
000004db l       .rodata.str1.1	00000000 .LC32
000004fd l       .rodata.str1.1	00000000 .LC34
0000051a l       .rodata.str1.1	00000000 .LC35
00000545 l       .rodata.str1.1	00000000 .LC36
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000014 h_smooth.10963
00000014 l     O .rodata	00000006 ceil_log2
0000001a l     O .rodata	00000060 sbr_offset
0000007c l     O .rodata	0000000c bands_warped.11054
00000088 l     O .rodata	00000079 t_huffman_env_1_5dB_bits
00000104 l     O .rodata	000001e4 t_huffman_env_1_5dB_codes
000002e8 l     O .rodata	00000079 f_huffman_env_1_5dB_bits
00000364 l     O .rodata	000001e4 f_huffman_env_1_5dB_codes
00000548 l     O .rodata	00000031 t_huffman_env_bal_1_5dB_bits
0000057c l     O .rodata	000000c4 t_huffman_env_bal_1_5dB_codes
00000640 l     O .rodata	00000031 f_huffman_env_bal_1_5dB_bits
00000674 l     O .rodata	000000c4 f_huffman_env_bal_1_5dB_codes
00000738 l     O .rodata	0000003f t_huffman_env_3_0dB_bits
00000778 l     O .rodata	000000fc t_huffman_env_3_0dB_codes
00000874 l     O .rodata	0000003f f_huffman_env_3_0dB_bits
000008b4 l     O .rodata	000000fc f_huffman_env_3_0dB_codes
000009b0 l     O .rodata	00000019 t_huffman_env_bal_3_0dB_bits
000009ca l     O .rodata	00000032 t_huffman_env_bal_3_0dB_codes
000009fc l     O .rodata	00000019 f_huffman_env_bal_3_0dB_bits
00000a16 l     O .rodata	00000032 f_huffman_env_bal_3_0dB_codes
00000a48 l     O .rodata	0000003f t_huffman_noise_3_0dB_bits
00000a88 l     O .rodata	0000007e t_huffman_noise_3_0dB_codes
00000b06 l     O .rodata	00000019 t_huffman_noise_bal_3_0dB_bits
00000b1f l     O .rodata	00000019 t_huffman_noise_bal_3_0dB_codes
00000b38 l     O .rodata	00000010 exp2_tab.10849
00000b48 l     O .rodata	00000010 bw_tab.10908
00000b58 l     O .rodata	00000010 limgain.10921
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .data	00000a00 sbr_qmf_window_us
00000000 l     O .bss	00000500 sbr_qmf_window_ds
00000500 l     O .bss	000000a0 vlc_sbr
000005a0 l     O .bss	00001128 table.11016
000016c8 l     O .bss	00001110 table.11017
000027d8 l     O .bss	00000c00 table.11018
000033d8 l     O .bss	00001008 table.11019
000043e0 l     O .bss	00001088 table.11020
00005468 l     O .bss	00001070 table.11021
000064d8 l     O .bss	00000880 table.11022
00006d58 l     O .bss	00000880 table.11023
000075d8 l     O .bss	00000940 table.11024
00007f18 l     O .bss	00000800 table.11025
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 powf
00000000         *UND*	00000000 lrintf
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 __aeabi_uidiv
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 log2f
000000e8 g     F .text.unlikely	00000434 ff_aac_sbr_init
00000000         *UND*	00000000 ff_init_vlc_sparse
00000000         *UND*	00000000 ff_ps_init
0000051c g     F .text.unlikely	000000ec ff_aac_sbr_ctx_init
00000000         *UND*	00000000 ff_mdct_init
00000000         *UND*	00000000 ff_ps_ctx_init
00000000         *UND*	00000000 ff_sbrdsp_init
00000608 g     F .text.unlikely	00000028 ff_aac_sbr_ctx_close
00000000         *UND*	00000000 ff_mdct_end
00000000         *UND*	00000000 __aeabi_idiv
00003644 g     F .text	00000d24 ff_decode_sbr_extension
00000000         *UND*	00000000 memcmp
00000000         *UND*	00000000 ff_ps_read_data
00000000         *UND*	00000000 avpriv_request_sample
00004368 g     F .text	00001628 ff_sbr_apply
00000000         *UND*	00000000 ff_ps_apply
00000b70 g     O .rodata	00001040 ff_sbr_noise_table



aactab.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aactab.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00007474 l     O .rodata	00000020 swb_offset_120_8
00007494 l     O .rodata	00000020 swb_offset_120_16
000074b4 l     O .rodata	00000020 swb_offset_120_24
000074d4 l     O .rodata	0000001e swb_offset_120_48
000074f2 l     O .rodata	0000001a swb_offset_120_96
0000750c l     O .rodata	00000052 swb_offset_960_8
0000755e l     O .rodata	00000056 swb_offset_960_16
000075b4 l     O .rodata	0000005e swb_offset_960_24
00007612 l     O .rodata	00000064 swb_offset_960_32
00007676 l     O .rodata	00000064 swb_offset_960_48
000076da l     O .rodata	0000005e swb_offset_960_64
00007738 l     O .rodata	00000052 swb_offset_960_96
0000778a l     O .rodata	00000020 swb_offset_128_8
000077aa l     O .rodata	00000052 swb_offset_1024_8
000077fc l     O .rodata	00000020 swb_offset_128_16
0000781c l     O .rodata	00000058 swb_offset_1024_16
00007874 l     O .rodata	00000020 swb_offset_128_24
00007894 l     O .rodata	0000003e swb_offset_480_24
000078d2 l     O .rodata	00000040 swb_offset_512_24
00007912 l     O .rodata	00000060 swb_offset_1024_24
00007972 l     O .rodata	0000004c swb_offset_480_32
000079be l     O .rodata	0000004c swb_offset_512_32
00007a0a l     O .rodata	00000068 swb_offset_1024_32
00007a72 l     O .rodata	0000001e swb_offset_128_48
00007a90 l     O .rodata	00000048 swb_offset_480_48
00007ad8 l     O .rodata	0000004a swb_offset_512_48
00007b22 l     O .rodata	00000064 swb_offset_1024_48
00007b86 l     O .rodata	00000060 swb_offset_1024_64
00007be6 l     O .rodata	0000001a swb_offset_128_96
00007c00 l     O .rodata	00000054 swb_offset_1024_96
00007c54 l     O .rodata	00000242 codebook_vector10_idx
00007e98 l     O .rodata	00000040 codebook_vector10_vals
00007ed8 l     O .rodata	00000152 codebook_vector8_idx
0000802a l     O .rodata	00000080 codebook_vector6_idx
000080aa l     O .rodata	000000a2 codebook_vector4_idx
0000814c l     O .rodata	00000024 codebook_vector4_vals
00008170 l     O .rodata	000000a2 codebook_vector02_idx
00008214 l     O .rodata	0000000c codebook_vector0_vals
00008220 l     O .rodata	00000908 codebook_vector10
00008b30 l     O .rodata	00000548 codebook_vector8
00009080 l     O .rodata	00000200 codebook_vector6
00009280 l     O .rodata	00000288 codebook_vector4
00009510 l     O .rodata	00000510 codebook_vector2
00009a20 l     O .rodata	00000510 codebook_vector0
00009f46 l     O .rodata	00000121 bits11
0000a068 l     O .rodata	00000242 codes11
0000a2aa l     O .rodata	000000a9 bits10
0000a354 l     O .rodata	00000152 codes10
0000a4a6 l     O .rodata	000000a9 bits9
0000a550 l     O .rodata	00000152 codes9
0000a6a2 l     O .rodata	00000040 bits8
0000a6e2 l     O .rodata	00000080 codes8
0000a762 l     O .rodata	00000040 bits7
0000a7a2 l     O .rodata	00000080 codes7
0000a822 l     O .rodata	00000051 bits6
0000a874 l     O .rodata	000000a2 codes6
0000a916 l     O .rodata	00000051 bits5
0000a968 l     O .rodata	000000a2 codes5
0000aa0a l     O .rodata	00000051 bits4
0000aa5c l     O .rodata	000000a2 codes4
0000aafe l     O .rodata	00000051 bits3
0000ab50 l     O .rodata	000000a2 codes3
0000abf2 l     O .rodata	00000051 bits2
0000ac44 l     O .rodata	000000a2 codes2
0000ace6 l     O .rodata	00000051 bits1
0000ad38 l     O .rodata	000000a2 codes1
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .rodata	00001c20 ff_aac_eld_window_480_fixed
00001c20 g     O .rodata	00001c20 ff_aac_eld_window_480
00003840 g     O .rodata	00001e00 ff_aac_eld_window_512_fixed
00005640 g     O .rodata	00001e00 ff_aac_eld_window_512
00007440 g     O .rodata	0000000d ff_tns_max_bands_128
0000744d g     O .rodata	0000000d ff_tns_max_bands_480
0000745a g     O .rodata	0000000d ff_tns_max_bands_512
00007467 g     O .rodata	0000000d ff_tns_max_bands_1024
00000000 g     O .data.rel.ro.local	00000034 ff_swb_offset_120
00000034 g     O .data.rel.ro.local	00000034 ff_swb_offset_128
00000068 g     O .data.rel.ro.local	00000034 ff_swb_offset_480
0000009c g     O .data.rel.ro.local	00000034 ff_swb_offset_512
000000d0 g     O .data.rel.ro.local	00000034 ff_swb_offset_960
00000104 g     O .data.rel.ro.local	00000034 ff_swb_offset_1024
00000138 g     O .data.rel.ro.local	0000002c ff_aac_codebook_vector_idx
00000164 g     O .data.rel.ro.local	0000002c ff_aac_codebook_vector_vals
00000190 g     O .data.rel.ro.local	0000002c ff_aac_codebook_vectors
00009f30 g     O .rodata	00000016 ff_aac_spectral_sizes
000001bc g     O .data.rel.ro.local	0000002c ff_aac_spectral_bits
000001e8 g     O .data.rel.ro.local	0000002c ff_aac_spectral_codes
0000adda g     O .rodata	00000079 ff_aac_scalefactor_bits
0000ae54 g     O .rodata	000001e4 ff_aac_scalefactor_code
0000b038 g     O .rodata	0000000d ff_aac_pred_sfb_max
0000b045 g     O .rodata	0000000d ff_aac_num_swb_120
0000b052 g     O .rodata	0000000d ff_aac_num_swb_128
0000b05f g     O .rodata	0000000d ff_aac_num_swb_480
0000b06c g     O .rodata	0000000d ff_aac_num_swb_512
0000b079 g     O .rodata	0000000d ff_aac_num_swb_960
0000b086 g     O .rodata	0000000d ff_aac_num_swb_1024
00000200       O *COM*	00000020 ff_aac_kbd_short_128_fixed
00001000       O *COM*	00000020 ff_aac_kbd_long_1024_fixed
000001e0       O *COM*	00000020 ff_aac_kbd_short_120
00000f00       O *COM*	00000020 ff_aac_kbd_long_960
00000200       O *COM*	00000020 ff_aac_kbd_short_128
00001000       O *COM*	00000020 ff_aac_kbd_long_1024
000006b0       O *COM*	00000004 ff_aac_pow34sf_tab
000006b0       O *COM*	00000004 ff_aac_pow2sf_tab



ac3.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000000c4 ff_ac3_bit_alloc_calc_psd
00000000 g     O .rodata	000000fd ff_ac3_bin_to_band_tab
000000fd g     O .rodata	00000033 ff_ac3_band_start_tab
00000000         *UND*	00000000 ff_ac3_log_add_tab
000000c4 g     F .text	00000400 ff_ac3_bit_alloc_calc_mask
00000000         *UND*	00000000 ff_ac3_hearing_threshold_tab



ac3_parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3_parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_bits
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000024 ac3_parse_init
000004c0 l     F .text	00000124 ac3_sync
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000004 center_levels
00000004 l     O .rodata	00000004 surround_levels
00000008 l     O .rodata	00000004 eac3_blocks
00000000 l    d  .data.rel	00000000 .data.rel
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_ldivmod
00000058 g     F .text	00000468 ff_ac3_parse_header
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_ac3_sample_rate_tab
00000000         *UND*	00000000 ff_ac3_bitrate_tab
00000000         *UND*	00000000 ff_ac3_channels_tab
00000000         *UND*	00000000 ff_ac3_frame_size_tab
00000000         *UND*	00000000 avpriv_ac3_channel_layout_tab
000005e4 g     F .text	000000ac avpriv_ac3_parse_header
00000000         *UND*	00000000 av_mallocz
00000690 g     F .text	00000090 av_ac3_parse_header
00000000 g     O .data.rel	0000002c ff_ac3_parser
00000000         *UND*	00000000 ff_aac_ac3_parse
00000000         *UND*	00000000 ff_parse_close



ac3dec_data.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3dec_data.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .rodata	00000011 ff_eac3_default_spx_band_struct
00000011 g     O .rodata	00000040 ff_eac3_hebap_tab
00000051 g     O .rodata	00000060 ff_ac3_ungroup_3_in_5_bits_tab



ac3dec_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3dec_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_sbits
00000058 l     F .text	00000058 get_bits
000000b0 l     F .text	0000001c skip_bits
000000cc l     F .text	00000034 get_bits1
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000038 ac3_decode_end
00000100 l     F .text	00000164 do_imdct
00000038 l     F .text.unlikely	00000414 ac3_decode_init
00000264 l     F .text	00000168 decode_exponents.isra.0
00000000 l       .rodata.str1.1	00000000 .LC0
0000001b l       .rodata.str1.1	00000000 .LC1
000003cc l     F .text	00000170 decode_band_structure.constprop.2
00000085 l       .rodata.str1.1	00000000 .LC4
00000038 l       .rodata.str1.1	00000000 .LC2
00000056 l       .rodata.str1.1	00000000 .LC3
0000053c l     F .text	000002e0 ac3_decode_transform_coeffs_ch
00000099 l       .rodata.str1.1	00000000 .LC5
0000081c l     F .text	0000054c ff_eac3_decode_transform_coeffs_aht_ch
000000ba l       .rodata.str1.1	00000000 .LC6
00000d68 l     F .text	00003974 ac3_decode_frame
000000dc l       .rodata.str1.1	00000000 .LC7
000000f9 l       .rodata.str1.1	00000000 .LC8
0000010e l       .rodata.str1.1	00000000 .LC9
00000124 l       .rodata.str1.1	00000000 .LC10
0000013a l       .rodata.str1.1	00000000 .LC11
00000173 l       .rodata.str1.1	00000000 .LC14
0000014b l       .rodata.str1.1	00000000 .LC12
0000015d l       .rodata.str1.1	00000000 .LC13
00000188 l       .rodata.str1.1	00000000 .LC15
0000019c l       .rodata.str1.1	00000000 .LC16
000001c7 l       .rodata.str1.1	00000000 .LC17
000001db l       .rodata.str1.1	00000000 .LC18
000001ed l       .rodata.str1.1	00000000 .LC19
0000021f l       .rodata.str1.1	00000000 .LC21
00000201 l       .rodata.str1.1	00000000 .LC20
00000241 l       .rodata.str1.1	00000000 .LC22
0000026e l       .rodata.str1.1	00000000 .LC23
000002a4 l       .rodata.str1.1	00000000 .LC24
000002cf l       .rodata.str1.1	00000000 .LC25
000002e1 l       .rodata.str1.1	00000000 .LC26
00000304 l       .rodata.str1.1	00000000 .LC27
00000336 l       .rodata.str1.1	00000000 .LC28
0000036b l       .rodata.str1.1	00000000 .LC29
000003a5 l       .rodata.str1.1	00000000 .LC30
000003bf l       .rodata.str1.1	00000000 .LC31
000003f3 l       .rodata.str1.1	00000000 .LC32
0000041f l       .rodata.str1.1	00000000 .LC33
00000452 l       .rodata.str1.1	00000000 .LC34
0000047a l       .rodata.str1.1	00000000 .LC35
00000493 l       .rodata.str1.1	00000000 .LC36
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000010 quantization_tab
00000010 l     O .rodata	00000024 gain_levels
00000034 l     O .rodata	00000050 ac3_default_coeffs
00000084 l     O .rodata	00000080 gain_levels_lfe
00000104 l     O .rodata	00000008 __compound_literal.1
0000010c l     O .rodata	00000008 __compound_literal.0
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l     O .data.rel.ro	0000002c eac3_decoder_class
00000000 l     O .data.rel.ro.local	000001e0 options
0000002c l     O .data.rel.ro	0000002c ac3_decoder_class
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l    d  .data.rel.local	00000000 .data.rel.local
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .bss	00000180 ungroup_3_in_7_bits_tab
00000180 l     O .bss	00000180 b1_mantissas
00000300 l     O .bss	00000600 b2_mantissas
00000900 l     O .bss	00000400 b4_mantissas
00000d00 l     O .bss	00000020 b3_mantissas
00000d20 l     O .bss	00000040 b5_mantissas
00000d60 l     O .bss	00000400 dynamic_range_tab
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 ff_mdct_end
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 __aeabi_idiv
00000000         *UND*	00000000 __aeabi_idivmod
00000000         *UND*	00000000 powf
00000000         *UND*	00000000 ff_mdct_init
00000000         *UND*	00000000 ff_kbd_window_init
00000000         *UND*	00000000 ff_bswapdsp_init
00000000         *UND*	00000000 avpriv_float_dsp_alloc
00000000         *UND*	00000000 ff_fmt_convert_init
00000000         *UND*	00000000 ff_ac3dsp_init
00000000         *UND*	00000000 av_lfg_init
00000000         *UND*	00000000 ff_ac3_ungroup_3_in_5_bits_tab
00000400       O *COM*	00000004 ff_ac3_heavy_dynamic_range_tab
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_eac3_bits_vs_hebap
00000000         *UND*	00000000 ff_eac3_mantissa_vq
00000000         *UND*	00000000 ff_eac3_gaq_remap_2_4_b
00000000         *UND*	00000000 ff_eac3_gaq_remap_2_4_a
00000000         *UND*	00000000 ff_eac3_gaq_remap_1
00000000         *UND*	00000000 av_lfg_init_from_data
00000000         *UND*	00000000 ff_ac3_parse_header
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 avpriv_request_sample
00000000         *UND*	00000000 ff_ac3_slow_decay_tab
00000000         *UND*	00000000 ff_ac3_fast_decay_tab
00000000         *UND*	00000000 ff_ac3_slow_gain_tab
00000000         *UND*	00000000 ff_ac3_db_per_bit_tab
00000000         *UND*	00000000 ff_ac3_floor_tab
00000000         *UND*	00000000 ff_eac3_frm_expstr
00000000         *UND*	00000000 av_crc_get_table
00000000         *UND*	00000000 av_crc
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 ff_get_buffer
00000000         *UND*	00000000 avpriv_ac3_channel_layout_tab
00000000         *UND*	00000000 ff_ac3_dec_channel_map
00000000         *UND*	00000000 ff_eac3_default_spx_band_struct
00000000         *UND*	00000000 ff_eac3_default_cpl_band_struct
00000000         *UND*	00000000 ff_ac3_fast_gain_tab
00000000         *UND*	00000000 ff_ac3_bap_tab
00000000         *UND*	00000000 ff_eac3_hebap_tab
00000000         *UND*	00000000 ff_ac3_bit_alloc_calc_psd
00000000         *UND*	00000000 ff_ac3_bit_alloc_calc_mask
00000000         *UND*	00000000 ff_ac3_rematrix_band_tab
00000000         *UND*	00000000 ff_eac3_spx_atten_tab
00000000         *UND*	00000000 ff_ac3dsp_downmix
00000000         *UND*	00000000 ff_side_data_update_matrix_encoding
00000000         *UND*	00000000 av_downmix_info_update_side_data
00000000 g     O .data.rel.local	00000080 ff_eac3_decoder
00000080 g     O .data.rel.local	00000080 ff_ac3_decoder
00000000         *UND*	00000000 av_default_item_name



ac3dsp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3dsp.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000030 ac3_max_msb_abs_int16_c
00000030 l     F .text	00000090 ac3_lshift_int16_c
000000c0 l     F .text	00000074 ac3_rshift_int32_c
00000134 l     F .text	0000002c ac3_update_bap_counts_c
00000160 l     F .text	00000098 ac3_compute_mantissa_size_c
000001f8 l     F .text	00000078 ac3_sum_square_butterfly_int32_c
00000270 l     F .text	0000006c ac3_sum_square_butterfly_float_c
000002dc l     F .text	00000098 ac3_downmix_5_to_2_symmetric_c
00000374 l     F .text	00000084 ac3_downmix_5_to_1_symmetric_c
000003f8 l     F .text	000000b8 ac3_downmix_5_to_2_symmetric_c_fixed
000004b0 l     F .text	0000007c ac3_downmix_5_to_1_symmetric_c_fixed
0000052c l     F .text	00000064 apply_window_int16_c
00000590 l     F .text	00000040 ac3_extract_exponents_c
000005d0 l     F .text	000000d4 float_to_fixed24_c
000006a4 l     F .text	00000054 ac3_exponent_min_c
000006f8 l     F .text	000000b4 ac3_bit_alloc_calc_bap_c
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_uidiv
00000000 g     O .rodata	00000020 ff_ac3_bap_bits
00000000         *UND*	00000000 lrintf
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ff_ac3_bin_to_band_tab
00000000         *UND*	00000000 ff_ac3_band_start_tab
000007ac g     F .text	00000208 ff_ac3dsp_downmix_fixed
000009b4 g     F .text	000001e4 ff_ac3dsp_downmix
00000000 g     F .text.unlikely	000000d8 ff_ac3dsp_init
00000000         *UND*	00000000 ff_ac3dsp_init_arm



ac3tab.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3tab.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .rodata	00000010 ff_eac3_default_chmap
00000010 g     O .rodata	00000010 ff_ac3_fast_gain_tab
00000020 g     O .rodata	00000010 ff_ac3_floor_tab
00000030 g     O .rodata	00000008 ff_ac3_db_per_bit_tab
00000038 g     O .rodata	00000008 ff_ac3_slow_gain_tab
00000040 g     O .rodata	00000004 ff_ac3_fast_decay_tab
00000044 g     O .rodata	00000004 ff_ac3_slow_decay_tab
00000048 g     O .rodata	00000040 ff_ac3_bap_tab
00000088 g     O .rodata	0000012c ff_ac3_hearing_threshold_tab
000001b4 g     O .rodata	00000104 ff_ac3_log_add_tab
000002c0 g     O .rodata	00000200 ff_ac3_window
000004c0 g     O .rodata	00000012 ff_eac3_default_cpl_band_struct
000004d2 g     O .rodata	00000005 ff_ac3_rematrix_band_tab
000004d8 g     O .rodata	00000026 ff_ac3_bitrate_tab
000004fe g     O .rodata	00000006 ff_ac3_sample_rate_tab
00000504 g     O .rodata	00000060 ff_ac3_dec_channel_map
00000564 g     O .rodata	00000060 ff_ac3_enc_channel_map
000005c4 g     O .rodata	00000010 avpriv_ac3_channel_layout_tab
000005d4 g     O .rodata	00000008 ff_ac3_channels_tab
000005dc g     O .rodata	000000e4 ff_ac3_frame_size_tab



adts_header.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 adts_header.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_bits
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_uidiv
00000058 g     F .text	000001a8 ff_adts_header_parse
00000000         *UND*	00000000 avpriv_mpeg4audio_sample_rates



adts_parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 adts_parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000080 av_adts_header_parse
00000000         *UND*	00000000 ff_adts_header_parse



allcodecs.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 allcodecs.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000000d8 register_all
00000000 l     O .bss	00000004 control.11114
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 avcodec_register
00000000         *UND*	00000000 av_register_codec_parser
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_aac_decoder
00000000         *UND*	00000000 ff_ac3_decoder
00000000         *UND*	00000000 ff_eac3_decoder
00000000         *UND*	00000000 ff_mp1_decoder
00000000         *UND*	00000000 ff_mp2_decoder
00000000         *UND*	00000000 ff_aac_parser
00000000         *UND*	00000000 ff_ac3_parser
00000000         *UND*	00000000 ff_mpegaudio_parser
000000d8 g     F .text	0000001c avcodec_register_all
00000000         *UND*	00000000 pthread_once



aacpsdsp_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 aacpsdsp_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000074 ff_psdsp_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_ps_add_squares_neon
00000000         *UND*	00000000 ff_ps_mul_pair_single_neon
00000000         *UND*	00000000 ff_ps_hybrid_synthesis_deint_neon
00000000         *UND*	00000000 ff_ps_hybrid_analysis_neon
00000000         *UND*	00000000 ff_ps_stereo_interpolate_neon



aacpsdsp_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000005c ff_ps_add_squares_neon
0000005c g     F .text	000000ac ff_ps_mul_pair_single_neon
00000108 g     F .text	000000e0 ff_ps_hybrid_synthesis_deint_neon
000001e8 g     F .text	00000104 ff_ps_hybrid_analysis_neon
000002ec g     F .text	00000094 ff_ps_stereo_interpolate_neon



ac3dsp_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000028 ff_ac3_update_bap_counts_arm



ac3dsp_armv6.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000000f4 ff_ac3_bit_alloc_calc_bap_armv6
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_ac3_bin_to_band_tab
00000000         *UND*	00000000 ff_ac3_band_start_tab



ac3dsp_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 ac3dsp_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000000d8 ff_ac3dsp_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_ac3_update_bap_counts_arm
00000000         *UND*	00000000 ff_ac3_bit_alloc_calc_bap_armv6
00000000         *UND*	00000000 ff_ac3_exponent_min_neon
00000000         *UND*	00000000 ff_ac3_max_msb_abs_int16_neon
00000000         *UND*	00000000 ff_ac3_lshift_int16_neon
00000000         *UND*	00000000 ff_ac3_rshift_int32_neon
00000000         *UND*	00000000 ff_float_to_fixed24_neon
00000000         *UND*	00000000 ff_ac3_extract_exponents_neon
00000000         *UND*	00000000 ff_apply_window_int16_neon
00000000         *UND*	00000000 ff_ac3_sum_square_butterfly_int32_neon
00000000         *UND*	00000000 ff_ac3_sum_square_butterfly_float_neon



ac3dsp_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000040 ff_ac3_max_msb_abs_int16_neon
00000040 g     F .text	0000003c ff_ac3_exponent_min_neon
0000007c g     F .text	0000001c ff_ac3_lshift_int16_neon
00000098 g     F .text	00000020 ff_ac3_rshift_int32_neon
000000b8 g     F .text	0000002c ff_float_to_fixed24_neon
000000e4 g     F .text	0000002c ff_ac3_extract_exponents_neon
00000110 g     F .text	00000048 ff_apply_window_int16_neon
00000158 g     F .text	00000050 ff_ac3_sum_square_butterfly_int32_neon
000001a8 g     F .text	00000040 ff_ac3_sum_square_butterfly_float_neon



fft_fixed_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fft_fixed_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	0000006c ff_fft_fixed_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 ff_fft_fixed_calc_neon
00000000         *UND*	00000000 ff_mdct_fixed_calc_neon
00000000         *UND*	00000000 ff_mdct_fixed_calcw_neon



fft_fixed_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l     F .text	00000034 fft4_neon
00000034 l     F .text	0000008c fft8_neon
00000000 l       .rodata	00000020 coefs
000000c0 l     F .text	00000148 fft16_neon
00000208 l     F .text	000001a4 fft_pass_neon
00000000 l    d  .rodata	00000000 .rodata
000003ac l     F .text	00000044 fft32_neon
000003f0 l     F .text	00000044 fft64_neon
00000434 l     F .text	00000044 fft128_neon
00000478 l     F .text	00000044 fft256_neon
000004bc l     F .text	00000044 fft512_neon
00000500 l     F .text	00000044 fft1024_neon
00000544 l     F .text	00000044 fft2048_neon
00000588 l     F .text	00000044 fft4096_neon
000005cc l     F .text	00000044 fft8192_neon
00000610 l     F .text	00000044 fft16384_neon
00000654 l     F .text	00000044 fft32768_neon
00000698 l     F .text	00000044 fft65536_neon
00000000 l       .data.rel.ro	0000003c fft_fixed_tab_neon
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_cos_32_fixed
00000000         *UND*	00000000 ff_cos_64_fixed
00000000         *UND*	00000000 ff_cos_128_fixed
00000000         *UND*	00000000 ff_cos_256_fixed
00000000         *UND*	00000000 ff_cos_512_fixed
00000000         *UND*	00000000 ff_cos_1024_fixed
00000000         *UND*	00000000 ff_cos_2048_fixed
00000000         *UND*	00000000 ff_cos_4096_fixed
00000000         *UND*	00000000 ff_cos_8192_fixed
00000000         *UND*	00000000 ff_cos_16384_fixed
00000000         *UND*	00000000 ff_cos_32768_fixed
00000000         *UND*	00000000 ff_cos_65536_fixed
000006dc g     F .text	00000020 ff_fft_fixed_calc_neon



fft_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fft_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000000a4 ff_fft_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_fft_calc_vfp
00000000         *UND*	00000000 ff_imdct_half_vfp
00000000         *UND*	00000000 ff_fft_permute_neon
00000000         *UND*	00000000 ff_fft_calc_neon
00000000         *UND*	00000000 ff_imdct_calc_neon
00000000         *UND*	00000000 ff_imdct_half_neon
00000000         *UND*	00000000 ff_mdct_calc_neon



fft_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l     F .text	00000030 fft4_neon
00000030 l     F .text	000000bc fft8_neon
000000ec l     F .text	000001ac fft16_neon
00000010 l       .rodata	00000010 mppm
00000000 l       .rodata	00000010 pmmp
00000298 l     F .text	00000124 fft_pass_neon
000003c0 l     F .text	00000044 fft32_neon
00000440 l     F .text	00000044 fft64_neon
000004c0 l     F .text	00000044 fft128_neon
00000540 l     F .text	00000044 fft256_neon
000005c0 l     F .text	00000044 fft512_neon
00000640 l     F .text	00000044 fft1024_neon
000006c0 l     F .text	00000044 fft2048_neon
00000740 l     F .text	00000044 fft4096_neon
000007c0 l     F .text	00000044 fft8192_neon
00000840 l     F .text	00000044 fft16384_neon
000008c0 l     F .text	00000044 fft32768_neon
00000940 l     F .text	00000044 fft65536_neon
00000000 l       .data.rel.ro	0000003c fft_tab_neon
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_cos_16
00000000         *UND*	00000000 ff_cos_32
00000000         *UND*	00000000 ff_cos_64
00000000         *UND*	00000000 ff_cos_128
00000000         *UND*	00000000 ff_cos_256
00000000         *UND*	00000000 ff_cos_512
00000000         *UND*	00000000 ff_cos_1024
00000000         *UND*	00000000 ff_cos_2048
00000000         *UND*	00000000 ff_cos_4096
00000000         *UND*	00000000 ff_cos_8192
00000000         *UND*	00000000 ff_cos_16384
00000000         *UND*	00000000 ff_cos_32768
00000000         *UND*	00000000 ff_cos_65536
00000984 g     F .text	00000020 ff_fft_calc_neon
000009a4 g     F .text	0000005c ff_fft_permute_neon



fft_vfp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l       .data.rel.ro	0000003c fft_tab_vfp
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000018 l     F .text	00000064 fft4_vfp
00000174 l     F .text	00000024 fft8_vfp
0000066c l     F .text	00000024 fft32_vfp
00000850 l     F .text	00000024 fft64_vfp
00000a34 l     F .text	00000024 fft128_vfp
00000c20 l     F .text	00000024 fft256_vfp
00000e1c l     F .text	00000024 fft512_vfp
00001018 l     F .text	00000024 fft1024_vfp
00001214 l     F .text	00000024 fft2048_vfp
00001410 l     F .text	00000024 fft4096_vfp
00001610 l     F .text	00000024 fft8192_vfp
00001810 l     F .text	00000024 fft16384_vfp
00001a10 l     F .text	00000024 fft32768_vfp
00001c10 l     F .text	00000024 fft65536_vfp
00000198 l       .text	00000000 cos1pi4
0000019c l       .text	00000000 cos1pi8
000001a0 l       .text	00000000 cos3pi8
00000000 l       *ABS*	00000000 o1
00000000 l       *ABS*	00000000 o2
00000000 l       *ABS*	00000000 o3
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000018 ff_fft_calc_vfp
0000048c g     F .text	00000024 ff_fft16_vfp
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_cos_32
00000000         *UND*	00000000 ff_cos_64
00000000         *UND*	00000000 ff_cos_128
00000000         *UND*	00000000 ff_cos_256
00000000         *UND*	00000000 ff_cos_512
00000000         *UND*	00000000 ff_cos_1024
00000000         *UND*	00000000 ff_cos_2048
00000000         *UND*	00000000 ff_cos_4096
00000000         *UND*	00000000 ff_cos_8192
00000000         *UND*	00000000 ff_cos_16384
00000000         *UND*	00000000 ff_cos_32768
00000000         *UND*	00000000 ff_cos_65536



fmtconvert_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fmtconvert_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	0000006c ff_fmt_convert_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_int32_to_float_fmul_scalar_vfp
00000000         *UND*	00000000 ff_int32_to_float_fmul_array8_vfp
00000000         *UND*	00000000 ff_int32_to_float_fmul_array8_neon
00000000         *UND*	00000000 ff_int32_to_float_fmul_scalar_neon



fmtconvert_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000050 ff_int32_to_float_fmul_scalar_neon
00000050 g     F .text	00000078 ff_int32_to_float_fmul_array8_neon



fmtconvert_vfp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000210 ff_int32_to_float_fmul_array8_vfp
00000210 g     F .text	00000050 ff_int32_to_float_fmul_scalar_vfp



idctdsp_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000110 ff_add_pixels_clamped_arm



idctdsp_armv6.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000005c ff_add_pixels_clamped_armv6



idctdsp_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 idctdsp_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	0000002c simple_idct_arm_add
0000002c l     F .text	0000002c j_rev_dct_arm_add
00000058 l     F .text	0000002c simple_idct_arm_put
00000084 l     F .text	0000002c j_rev_dct_arm_put
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 ff_simple_idct_arm
00000000         *UND*	00000000 ff_add_pixels_clamped_arm
00000000         *UND*	00000000 ff_j_rev_dct_arm
00000000         *UND*	00000000 ff_put_pixels_clamped_c
00000000 g     F .text.unlikely	00000120 ff_idctdsp_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 ff_idctdsp_init_armv5te
00000000         *UND*	00000000 ff_idctdsp_init_armv6
00000000         *UND*	00000000 ff_idctdsp_init_neon



idctdsp_init_armv5te.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 idctdsp_init_armv5te.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	0000006c ff_idctdsp_init_armv5te
00000000         *UND*	00000000 ff_simple_idct_put_armv5te
00000000         *UND*	00000000 ff_simple_idct_add_armv5te
00000000         *UND*	00000000 ff_simple_idct_armv5te



idctdsp_init_armv6.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 idctdsp_init_armv6.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	0000008c ff_idctdsp_init_armv6
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_simple_idct_put_armv6
00000000         *UND*	00000000 ff_simple_idct_add_armv6
00000000         *UND*	00000000 ff_simple_idct_armv6
00000000         *UND*	00000000 ff_add_pixels_clamped_armv6



idctdsp_init_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 idctdsp_init_neon.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000000a8 ff_idctdsp_init_neon
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_simple_idct_put_neon
00000000         *UND*	00000000 ff_simple_idct_add_neon
00000000         *UND*	00000000 ff_simple_idct_neon
00000000         *UND*	00000000 ff_add_pixels_clamped_neon
00000000         *UND*	00000000 ff_put_pixels_clamped_neon
00000000         *UND*	00000000 ff_put_signed_pixels_clamped_neon



idctdsp_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000054 ff_put_pixels_clamped_neon
00000054 g     F .text	00000088 ff_put_signed_pixels_clamped_neon
000000dc g     F .text	000000a8 ff_add_pixels_clamped_neon



jrevdct_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000014 l       .text	00000000 row_loop
000001b4 l       .text	00000000 end_of_row_loop
00000194 l       .text	00000000 empty_row
000001c0 l       .text	00000000 start_column_loop
000001c8 l       .text	00000000 column_loop
00000330 l       .text	00000000 empty_odd_column
0000037c l       .text	00000000 the_end
00000000 l       .rodata	00000034 const_array
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000384 ff_j_rev_dct_arm



mdct_fixed_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000001ac ff_mdct_fixed_calc_neon
00000000         *UND*	00000000 ff_fft_fixed_calc_neon
000001ac g     F .text	000001b0 ff_mdct_fixed_calcw_neon



mdct_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000148 ff_imdct_half_neon
00000000         *UND*	00000000 ff_fft_calc_neon
00000148 g     F .text	00000070 ff_imdct_calc_neon
000001b8 g     F .text	000001fc ff_mdct_calc_neon



mdct_vfp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000040 l       *ABS*	00000000 n
00000020 l       *ABS*	00000000 n2
00000010 l       *ABS*	00000000 n4
00000008 l       *ABS*	00000000 n8
00000008 l       *ABS*	00000000 k
00000006 l       *ABS*	00000000 trig_lo
00000008 l       *ABS*	00000000 trig_hi
0000000c l       *ABS*	00000000 in_lo
00000010 l       *ABS*	00000000 in_hi
fffffffe l       *ABS*	00000000 trig_lo_head
00000010 l       *ABS*	00000000 trig_hi_head
fffffffc l       *ABS*	00000000 out_lo_head
00000020 l       *ABS*	00000000 out_hi_head
00000000 l       *ABS*	00000000 trig_lo_tail
0000000e l       *ABS*	00000000 trig_hi_tail
00000000 l       *ABS*	00000000 out_lo_tail
0000001c l       *ABS*	00000000 out_hi_tail
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000640 ff_imdct_half_vfp
00000000         *UND*	00000000 ff_fft16_vfp



mpegaudiodsp_fixed_armv6.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000003b4 ff_mpadsp_apply_window_fixed_armv6



mpegaudiodsp_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodsp_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000024 ff_mpadsp_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 ff_mpadsp_apply_window_fixed_armv6



rdft_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 rdft_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000024 ff_rdft_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 ff_rdft_calc_neon



rdft_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000001e8 ff_rdft_calc_neon
00000000         *UND*	00000000 ff_fft_permute_neon
00000000         *UND*	00000000 ff_fft_calc_neon



sbrdsp_init_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 sbrdsp_init_arm.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000104 ff_sbrdsp_init_arm
00000000         *UND*	00000000 av_get_cpu_flags
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_sbr_sum64x5_neon
00000000         *UND*	00000000 ff_sbr_sum_square_neon
00000000         *UND*	00000000 ff_sbr_neg_odd_64_neon
00000000         *UND*	00000000 ff_sbr_qmf_pre_shuffle_neon
00000000         *UND*	00000000 ff_sbr_qmf_post_shuffle_neon
00000000         *UND*	00000000 ff_sbr_qmf_deint_neg_neon
00000000         *UND*	00000000 ff_sbr_qmf_deint_bfly_neon
00000000         *UND*	00000000 ff_sbr_hf_g_filt_neon
00000000         *UND*	00000000 ff_sbr_hf_gen_neon
00000000         *UND*	00000000 ff_sbr_autocorrelate_neon
00000000         *UND*	00000000 ff_sbr_hf_apply_noise_0_neon
00000000         *UND*	00000000 ff_sbr_hf_apply_noise_1_neon
00000000         *UND*	00000000 ff_sbr_hf_apply_noise_2_neon
00000000         *UND*	00000000 ff_sbr_hf_apply_noise_3_neon



sbrdsp_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000004c ff_sbr_sum64x5_neon
0000004c g     F .text	00000024 ff_sbr_sum_square_neon
00000070 g     F .text	0000006c ff_sbr_neg_odd_64_neon
000000dc g     F .text	00000098 ff_sbr_qmf_pre_shuffle_neon
00000174 g     F .text	00000064 ff_sbr_qmf_post_shuffle_neon
000001d8 g     F .text	00000038 ff_sbr_qmf_deint_neg_neon
00000210 g     F .text	00000048 ff_sbr_qmf_deint_bfly_neon
00000258 g     F .text	00000044 ff_sbr_hf_g_filt_neon
0000029c g     F .text	0000007c ff_sbr_hf_gen_neon
00000318 g     F .text	000000d8 ff_sbr_autocorrelate_neon
000003f0 g     F .text	000000b0 ff_sbr_hf_apply_noise_0_neon
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_sbr_noise_table
000004a0 g     F .text	000000bc ff_sbr_hf_apply_noise_1_neon
0000055c g     F .text	00000008 ff_sbr_hf_apply_noise_2_neon
00000564 g     F .text	00000018 ff_sbr_hf_apply_noise_3_neon



simple_idct_arm.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000010 l       .text	00000000 __row_loop
000001bc l       .text	00000000 __end_row_loop
0000019c l       .text	00000000 __almost_empty_row
000000bc l       .text	00000000 __end_b_evaluation
00000330 l       .text	00000000 __end_bef_a_evaluation
00000130 l       .text	00000000 __end_a_evaluation
000001d0 l       .text	00000000 __col_loop
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000344 ff_simple_idct_arm



simple_idct_armv5te.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l     F .text	0000016c idct_row_armv5te
00000150 l       .text	00000000 row_dc_only
0000016c l     F .text	00000270 idct_col_armv5te
000003dc l     F .text	00000324 idct_col_put_armv5te
00000700 l     F .text	00000384 idct_col_add_armv5te
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000a84 g     F .text	00000064 ff_simple_idct_armv5te
00000ae8 g     F .text	0000006c ff_simple_idct_add_armv5te
00000b54 g     F .text	0000006c ff_simple_idct_put_armv5te



simple_idct_armv6.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l     F .text	00000168 idct_row_armv6
00000168 l     F .text	000000e0 idct_col_armv6
00000248 l     F .text	000000e4 idct_col_put_armv6
0000032c l     F .text	00000128 idct_col_add_armv6
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000454 g     F .text	000000d4 ff_simple_idct_armv6
00000528 g     F .text	000000dc ff_simple_idct_add_armv6
00000604 g     F .text	000000dc ff_simple_idct_put_armv6



simple_idct_neon.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l     F .text	00000028 idct_row4_pld_neon
00000028 l     F .text	00000120 idct_row4_neon
00000148 l     F .text	00000118 idct_col4_neon
00000280 l     F .text	00000034 idct_col4_st8_neon
00000000 l    d  .rodata	00000000 .rodata
00000000 l       .rodata	00000010 idct_coeff_neon
00000340 l     F .text	00000078 idct_col4_add8_neon
00000440 l     F .text	00000038 idct_col4_st16_neon
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
000002b4 g     F .text	00000050 ff_simple_idct_put_neon
000003b8 g     F .text	00000050 ff_simple_idct_add_neon
00000478 g     F .text	00000054 ff_simple_idct_neon



avdct.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 avdct.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	0000002c avdct_class
00000030 l     O .data.rel.ro.local	00000480 avdct_options
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000010 avcodec_dct_get_class
00000010 g     F .text	00000030 avcodec_dct_alloc
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_opt_set_defaults
00000040 g     F .text	000000ac avcodec_dct_init
00000000         *UND*	00000000 avcodec_alloc_context3
00000000         *UND*	00000000 ff_idctdsp_init
00000000         *UND*	00000000 ff_fdctdsp_init
00000000         *UND*	00000000 avcodec_close
00000000         *UND*	00000000 av_free



avfft.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 avfft.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000048 av_fft_init
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 ff_fft_init
00000000         *UND*	00000000 av_freep
00000048 g     F .text	00000008 av_fft_permute
00000050 g     F .text	00000008 av_fft_calc
00000000 g     F .text.unlikely	0000001c av_fft_end
00000000         *UND*	00000000 ff_fft_end
00000000         *UND*	00000000 av_free
00000058 g     F .text	00000058 av_mdct_init
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 ff_mdct_init
000000b0 g     F .text	00000008 av_imdct_calc
000000b8 g     F .text	00000008 av_imdct_half
000000c0 g     F .text	00000008 av_mdct_calc
0000001c g     F .text.unlikely	0000001c av_mdct_end
00000000         *UND*	00000000 ff_mdct_end
000000c8 g     F .text	00000048 av_rdft_init
00000000         *UND*	00000000 ff_rdft_init
00000110 g     F .text	00000008 av_rdft_calc
00000038 g     F .text.unlikely	0000001c av_rdft_end
00000000         *UND*	00000000 ff_rdft_end
00000118 g     F .text	00000048 av_dct_init
00000000         *UND*	00000000 ff_dct_init
00000160 g     F .text	00000008 av_dct_calc
00000054 g     F .text.unlikely	0000001c av_dct_end
00000000         *UND*	00000000 ff_dct_end



avpacket.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 avpacket.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000045 l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
00000063 l       .rodata.str1.1	00000000 .LC4
00000071 l       .rodata.str1.1	00000000 .LC5
0000007e l       .rodata.str1.1	00000000 .LC6
0000008b l       .rodata.str1.1	00000000 .LC7
00000097 l       .rodata.str1.1	00000000 .LC8
000000a6 l       .rodata.str1.1	00000000 .LC9
000000b0 l       .rodata.str1.1	00000000 .LC10
000000c3 l       .rodata.str1.1	00000000 .LC11
000000d0 l       .rodata.str1.1	00000000 .LC12
000000dd l       .rodata.str1.1	00000000 .LC13
000000ee l       .rodata.str1.1	00000000 .LC14
00000100 l       .rodata.str1.1	00000000 .LC15
00000119 l       .rodata.str1.1	00000000 .LC16
00000123 l       .rodata.str1.1	00000000 .LC17
00000133 l       .rodata.str1.1	00000000 .LC18
00000143 l       .rodata.str1.1	00000000 .LC19
00000154 l       .rodata.str1.1	00000000 .LC20
0000016f l       .rodata.str1.1	00000000 .LC21
0000018c l       .rodata.str1.1	00000000 .LC22
0000019e l       .rodata.str1.1	00000000 .LC23
0000005b l       .rodata.str1.1	00000000 .LC3
000001b2 l       .rodata.str1.1	00000000 .LC24
000001e0 l       .rodata.str1.1	00000000 .LC25
00000ce4 l     F .text	000000f4 copy_packet_data
000001e1 l       .rodata.str1.1	00000000 .LC26
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000048 av_init_packet
00000048 g     F .text	00000074 av_new_packet
00000000         *UND*	00000000 av_buffer_realloc
00000000         *UND*	00000000 memset
000000bc g     F .text	00000024 av_shrink_packet
000000e0 g     F .text	0000014c av_grow_packet
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 av_buffer_alloc
00000000         *UND*	00000000 memcpy
0000022c g     F .text	00000060 av_packet_from_data
00000000         *UND*	00000000 av_buffer_create
00000000         *UND*	00000000 av_buffer_default_free
0000028c g     F .text	00000044 av_packet_free_side_data
00000000         *UND*	00000000 av_freep
000002d0 g     F .text	00000034 av_free_packet
00000000         *UND*	00000000 av_buffer_unref
00000304 g     F .text	000000c8 av_packet_add_side_data
00000000         *UND*	00000000 av_free
00000000         *UND*	00000000 av_realloc
000003cc g     F .text	00000060 av_packet_new_side_data
00000000         *UND*	00000000 av_mallocz
0000042c g     F .text	00000064 av_packet_get_side_data
00000490 g     F .text	000001c4 av_packet_side_data_name
00000654 g     F .text	00000250 av_packet_split_side_data
00000000         *UND*	00000000 av_malloc_array
000008a4 g     F .text	000000ec av_packet_pack_dictionary
00000000         *UND*	00000000 av_dict_get
00000000         *UND*	00000000 strlen
00000990 g     F .text	000000b4 av_packet_unpack_dictionary
00000000         *UND*	00000000 av_dict_set
00000a44 g     F .text	0000005c av_packet_shrink_side_data
00000aa0 g     F .text	000000b0 av_packet_copy_props
00000b50 g     F .text	0000002c av_packet_unref
00000b7c g     F .text	00000020 av_packet_alloc
00000b9c g     F .text	0000002c av_packet_free
00000bc8 g     F .text	0000011c av_copy_packet_side_data
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 av_buffer_ref
00000dd8 g     F .text	00000060 av_dup_packet
00000e38 g     F .text	00000020 av_copy_packet
00000e58 g     F .text	00000234 av_packet_merge_side_data
0000108c g     F .text	000000cc av_packet_ref
00001158 g     F .text	00000044 av_packet_clone
0000119c g     F .text	00000028 av_packet_move_ref
000011c4 g     F .text	000000ec av_packet_rescale_ts
00000000         *UND*	00000000 av_rescale_q
000012b0 g     F .text	000000f0 ff_side_data_set_encoder_stats



avpicture.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 avpicture.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000038 avpicture_fill
00000000         *UND*	00000000 av_image_fill_arrays
00000038 g     F .text	00000030 avpicture_layout
00000000         *UND*	00000000 av_image_copy_to_buffer
00000068 g     F .text	00000008 avpicture_get_size
00000000         *UND*	00000000 av_image_get_buffer_size
00000070 g     F .text	00000044 avpicture_alloc
00000000         *UND*	00000000 av_image_alloc
00000000         *UND*	00000000 memset
000000b4 g     F .text	00000004 avpicture_free
00000000         *UND*	00000000 av_freep
000000b8 g     F .text	0000002c av_picture_copy
00000000         *UND*	00000000 av_image_copy



bitstream.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 bitstream.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000000a0 put_bits
00000000 l       .rodata.str1.1	00000000 .LC0
000000a0 l     F .text	00000354 build_table
0000002b l       .rodata.str1.1	00000000 .LC1
00000076 l       .rodata.str1.1	00000000 .LC4
0000003c l       .rodata.str1.1	00000000 .LC2
0000005a l       .rodata.str1.1	00000000 .LC3
0000008d l       .rodata.str1.1	00000000 .LC5
000000c8 l       .rodata.str1.1	00000000 .LC6
000000e6 l       .rodata.str1.1	00000000 .LC7
00000105 l       .rodata.str1.1	00000000 .LC8
00000129 l       .rodata.str1.1	00000000 .LC9
0000013b l       .rodata.str1.1	00000000 .LC10
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 av_realloc_f
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ff_reverse
000003f4 g     F .text	00000010 avpriv_align_put_bits
00000404 g     F .text	00000044 avpriv_put_string
00000448 g     F .text	000000e0 avpriv_copy_bits
00000000         *UND*	00000000 __aeabi_llsl
00000528 g     F .text	00000aac ff_init_vlc_sparse
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 av_free
00000000         *UND*	00000000 av_freep
00000fd4 g     F .text	00000008 ff_free_vlc
00000000 g     O .rodata	00000029 ff_log2_run



bitstream_filter.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 bitstream_filter.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000002 l       .rodata.str1.1	00000000 .LC1
00000000 l       .rodata.str1.1	00000000 .LC0
00000004 l       .rodata.str1.1	00000000 .LC2
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000034 av_bitstream_filter_next
00000000         *UND*	00000000 av_bsf_next
00000034 g     F .text	00000004 av_register_bitstream_filter
00000038 g     F .text	00000078 av_bitstream_filter_init
00000000         *UND*	00000000 av_bsf_get_by_name
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_freep
000000b0 g     F .text	0000002c av_bitstream_filter_close
00000000         *UND*	00000000 av_bsf_free
00000000         *UND*	00000000 av_free
000000dc g     F .text	00000280 av_bitstream_filter_filter
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 av_bsf_send_packet
00000000         *UND*	00000000 av_bsf_alloc
00000000         *UND*	00000000 avcodec_parameters_from_context
00000000         *UND*	00000000 av_opt_next
00000000         *UND*	00000000 av_opt_set_from_string
00000000         *UND*	00000000 av_bsf_init
00000000         *UND*	00000000 av_bsf_receive_packet
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 av_packet_unref
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 strstr



bitstream_filters.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 bitstream_filters.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l     O .data.rel.ro	00000008 bitstream_filters
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000028 av_bsf_next
00000028 g     F .text	0000002c av_bsf_get_by_name
00000000         *UND*	00000000 strcmp
00000000         *UND*	00000000 ff_null_bsf
00000054 g     F .text	00000078 ff_bsf_child_class_next



bsf.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 bsf.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000000d8 bsf_list_item_name
0000000c l       .rodata.str1.1	00000000 .LC3
00000004 l       .rodata.str1.1	00000000 .LC1
00000000 l       .rodata.str1.1	00000000 .LC0
00000016 l       .rodata.str1.1	00000000 .LC4
00000007 l       .rodata.str1.1	00000000 .LC2
000000d8 l     F .text	00000030 bsf_child_next
0000019c l     F .text	00000040 bsf_list_close
00000018 l       .rodata.str1.1	00000000 .LC5
00000076 l       .rodata.str1.1	00000000 .LC7
00000020 l       .rodata.str1.1	00000000 .LC6
0000007f l       .rodata.str1.1	00000000 .LC8
00000458 l     F .text	00000098 bsf_list_init
00000081 l       .rodata.str1.1	00000000 .LC9
00000648 l     F .text	00000104 bsf_list_filter
000000a9 l       .rodata.str1.1	00000000 .LC11
000000ab l       .rodata.str1.1	00000000 .LC12
000000a7 l       .rodata.str1.1	00000000 .LC10
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l     O .data.rel.ro	0000002c bsf_class
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
0000001c l     O .data.rel.ro.local	0000002c bsf_list_class
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 av_bprint_init
00000000         *UND*	00000000 av_bprintf
00000000         *UND*	00000000 av_bprint_finalize
00000108 g     F .text	00000094 av_bsf_free
00000000         *UND*	00000000 av_opt_free
00000000         *UND*	00000000 av_packet_free
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 avcodec_parameters_free
000001dc g     F .text	00000010 av_bsf_get_class
000001ec g     F .text	00000108 av_bsf_alloc
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 avcodec_parameters_alloc
00000000         *UND*	00000000 av_packet_alloc
00000000         *UND*	00000000 av_opt_set_defaults
000002f4 g     F .text	00000164 av_bsf_init
00000000         *UND*	00000000 avcodec_parameters_copy
00000000         *UND*	00000000 avcodec_descriptor_get
00000000         *UND*	00000000 av_log
000004f0 g     F .text	00000090 av_bsf_send_packet
00000000         *UND*	00000000 av_packet_move_ref
00000580 g     F .text	0000000c av_bsf_receive_packet
0000058c g     F .text	0000006c ff_bsf_get_packet
000005f8 g     F .text	00000050 ff_bsf_get_packet_ref
00000000         *UND*	00000000 av_packet_unref
0000074c g     F .text	00000008 av_bsf_list_alloc
00000754 g     F .text	00000048 av_bsf_list_free
00000000         *UND*	00000000 av_free
0000079c g     F .text	0000000c av_bsf_list_append
00000000         *UND*	00000000 av_dynarray_add_nofree
000007a8 g     F .text	00000084 av_bsf_list_append2
00000000         *UND*	00000000 av_bsf_get_by_name
00000000         *UND*	00000000 av_opt_set_dict2
0000082c g     F .text	00000088 av_bsf_list_finalize
00000000 g     O .data.rel.ro.local	0000001c ff_list_bsf
000008b4 g     F .text	00000014 av_bsf_get_null_filter
000008c8 g     F .text	0000015c av_bsf_list_parse_str
00000000         *UND*	00000000 av_strdup
00000000         *UND*	00000000 av_strtok
00000000         *UND*	00000000 av_dict_free
00000000         *UND*	00000000 av_dict_parse_string
00000000         *UND*	00000000 av_default_item_name
00000000         *UND*	00000000 ff_bsf_child_class_next



bswapdsp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 bswapdsp.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000000a8 bswap_buf
000000a8 l     F .text	00000024 bswap16_buf
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000024 ff_bswapdsp_init



cbrt_data.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 cbrt_data.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     O .bss	00010000 cbrt_tab_dbl.4614
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000001b4 ff_cbrt_tableinit
00000000         *UND*	00000000 cbrt
00008000       O *COM*	00000004 ff_cbrt_tab



codec_desc.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 codec_desc.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	00002f5c codec_descriptors
00002fec l     O .data.rel.ro.local	00000008 __compound_literal.0
00002fe4 l     O .data.rel.ro.local	00000008 __compound_literal.1
00002fdc l     O .data.rel.ro.local	00000008 __compound_literal.2
00002fd4 l     O .data.rel.ro.local	00000008 __compound_literal.3
00002fcc l     O .data.rel.ro.local	00000008 __compound_literal.4
00002fc4 l     O .data.rel.ro.local	00000008 __compound_literal.5
00002fbc l     O .data.rel.ro.local	00000008 __compound_literal.6
00002fb4 l     O .data.rel.ro.local	00000008 __compound_literal.7
00002fa8 l     O .data.rel.ro.local	0000000c __compound_literal.8
00002fa0 l     O .data.rel.ro.local	00000008 __compound_literal.9
00002f98 l     O .data.rel.ro.local	00000008 __compound_literal.10
00002f90 l     O .data.rel.ro.local	00000008 __compound_literal.11
00002f88 l     O .data.rel.ro.local	00000008 __compound_literal.12
00002f80 l     O .data.rel.ro.local	00000008 __compound_literal.13
00002f78 l     O .data.rel.ro.local	00000008 __compound_literal.14
00002f6c l     O .data.rel.ro.local	0000000c __compound_literal.15
00002f64 l     O .data.rel.ro.local	00000008 __compound_literal.16
00002f5c l     O .data.rel.ro.local	00000008 __compound_literal.17
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000004c avcodec_descriptor_get
0000004c g     F .text	00000050 avcodec_descriptor_next
0000009c g     F .text	00000038 avcodec_descriptor_get_by_name
00000000         *UND*	00000000 strcmp
000000d4 g     F .text	00000018 avcodec_get_type



d3d11va.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 d3d11va.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 av_d3d11va_alloc_context



dct.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 dct.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000000f0 dst_calc_I_c
000000f0 l     F .text	00000100 dct_calc_I_c
000001f0 l     F .text	00000120 dct_calc_III_c
00000310 l     F .text	0000010c dct_calc_II_c
0000041c l     F .text	0000000c dct32_func
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000001a4 ff_dct_init
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ff_init_ff_cos_tabs
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 ff_rdft_init
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 sin
00000000         *UND*	00000000 ff_cos_tabs
00000000         *UND*	00000000 ff_dct32_float
000001a4 g     F .text.unlikely	0000001c ff_dct_end
00000000         *UND*	00000000 ff_rdft_end



dct32_fixed.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 dct32_fixed.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000a90 ff_dct32_fixed



dct32_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 dct32_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000074c ff_dct32_float



decode.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 decode.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000044 decode_data_free
00000044 l     F .text	000000ac insert_ts
00000000 l       .rodata.str1.1	00000000 .LC3
0000000c l       .rodata.str1.1	00000000 .LC4
000000f0 l     F .text	00000238 convert_sub_to_old_ass_form
0000001f l       .rodata.str1.1	00000000 .LC5
0000002a l       .rodata.str1.1	00000000 .LC6
00000039 l       .rodata.str1.1	00000000 .LC7
0000003e l       .rodata.str1.1	00000000 .LC8
0000006d l       .rodata.str1.1	00000000 .LC9
000000ca l       .rodata.str1.1	00000000 .LC10
000000e0 l       .rodata.str1.1	00000000 .LC11
000000f4 l       .rodata.str1.1	00000000 .LC12
00000117 l       .rodata.str1.1	00000000 .LC13
00000730 l     F .text	00000c1c decode_receive_frame_internal
00000167 l       .rodata.str1.1	00000000 .LC16
0000013a l       .rodata.str1.1	00000000 .LC14
00000158 l       .rodata.str1.1	00000000 .LC15
0000027a l       .rodata.str1.1	00000000 .LC24
0000025c l       .rodata.str1.1	00000000 .LC23
000002c6 l       .rodata.str1.1	00000000 .LC25
0000017b l       .rodata.str1.1	00000000 .LC17
000001aa l       .rodata.str1.1	00000000 .LC18
000001cb l       .rodata.str1.1	00000000 .LC19
000001fd l       .rodata.str1.1	00000000 .LC20
00000211 l       .rodata.str1.1	00000000 .LC21
00000245 l       .rodata.str1.1	00000000 .LC22
000002d4 l       .rodata.str1.1	00000000 .LC26
0000034c l       .rodata.str1.1	00000000 .LC27
00000372 l       .rodata.str1.1	00000000 .LC28
00000394 l       .rodata.str1.1	00000000 .LC29
000003c0 l       .rodata.str1.1	00000000 .LC30
0000040c l       .rodata.str1.1	00000000 .LC31
00000460 l       .rodata.str1.1	00000000 .LC32
0000049c l       .rodata.str1.1	00000000 .LC33
000004d6 l       .rodata.str1.1	00000000 .LC35
00000618 l       .rodata.str1.1	00000000 .LC40
00000667 l       .rodata.str1.1	00000000 .LC42
000004a3 l       .rodata.str1.1	00000000 .LC34
000004f9 l       .rodata.str1.1	00000000 .LC36
00000535 l       .rodata.str1.1	00000000 .LC37
0000058d l       .rodata.str1.1	00000000 .LC38
000005e3 l       .rodata.str1.1	00000000 .LC39
00000644 l       .rodata.str1.1	00000000 .LC41
000006ab l       .rodata.str1.1	00000000 .LC43
000006e4 l       .rodata.str1.1	00000000 .LC44
000006e6 l       .rodata.str1.1	00000000 .LC45
00000718 l       .rodata.str1.1	00000000 .LC46
0000074d l       .rodata.str1.1	00000000 .LC47
00000772 l       .rodata.str1.1	00000000 .LC48
0000078e l       .rodata.str1.1	00000000 .LC49
000007b4 l       .rodata.str1.1	00000000 .LC50
000007d1 l       .rodata.str1.1	00000000 .LC51
000007f6 l       .rodata.str1.1	00000000 .LC52
0000080e l       .rodata.str1.1	00000000 .LC53
0000083a l       .rodata.str1.1	00000000 .LC54
00000865 l       .rodata.str1.1	00000000 .LC55
00000874 l       .rodata.str1.1	00000000 .LC56
000008b9 l       .rodata.str1.1	00000000 .LC57
000008ce l       .rodata.str1.1	00000000 .LC58
000008f6 l       .rodata.str1.1	00000000 .LC59
00000945 l       .rodata.str1.1	00000000 .LC60
00002c78 l     F .text	000001dc bsfs_init
0000095c l       .rodata.str1.1	00000000 .LC61
00000961 l       .rodata.str1.1	00000000 .LC62
00000963 l       .rodata.str1.1	00000000 .LC63
000009c0 l       .rodata.str1.1	00000000 .LC64
000031bc l     F .text	00000354 compat_decode
00000a34 l       .rodata.str1.1	00000000 .LC65
00000a56 l       .rodata.str1.1	00000000 .LC66
00000aaa l       .rodata.str1.1	00000000 .LC68
00000a77 l       .rodata.str1.1	00000000 .LC67
00000000 l    d  .rodata	00000000 .rodata
00000010 l     O .rodata	00000040 sd.10611
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 __aeabi_idiv
00000000         *UND*	00000000 av_bprintf
00000000         *UND*	00000000 av_bprint_init
00000000         *UND*	00000000 strncmp
00000000         *UND*	00000000 av_bprint_clear
00000000         *UND*	00000000 strchr
00000000         *UND*	00000000 strtol
00000000         *UND*	00000000 av_rescale_q
00000000         *UND*	00000000 av_strdup
00000000         *UND*	00000000 av_bprint_finalize
00000328 g     F .text	00000408 ff_decode_get_packet
00000000         *UND*	00000000 av_bsf_receive_packet
00000000         *UND*	00000000 av_bsf_send_packet
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 av_packet_unref
00000000         *UND*	00000000 av_packet_copy_props
00000000         *UND*	00000000 av_packet_get_side_data
00000000         *UND*	00000000 ff_set_dimensions
00000000         *UND*	00000000 ff_thread_decode_frame
00000000         *UND*	00000000 av_frame_unref
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 av_samples_copy
00000000         *UND*	00000000 av_frame_new_side_data
00000000         *UND*	00000000 av_mul_q
00000000         *UND*	00000000 av_buffer_unref
0000134c g     F .text	00000418 avcodec_decode_subtitle2
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 avsubtitle_free
00001764 g     F .text	00000100 avcodec_default_get_format
00000000         *UND*	00000000 av_pix_fmt_desc_get
00000000         *UND*	00000000 avcodec_get_hw_config
00001864 g     F .text	000000a4 avcodec_get_hw_frames_parameters
00000000         *UND*	00000000 av_hwframe_ctx_alloc
00001908 g     F .text	0000010c ff_decode_get_hw_frames_ctx
00000000         *UND*	00000000 av_hwdevice_get_type_name
00000000         *UND*	00000000 av_hwframe_ctx_init
00001a14 g     F .text	00000408 ff_get_format
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_get_pix_fmt_name
00000000         *UND*	00000000 __aeabi_idivmod
00001e1c g     F .text	00000628 avcodec_default_get_buffer2
00000000         *UND*	00000000 av_hwframe_get_buffer
00000000         *UND*	00000000 avcodec_align_dimensions2
00000000         *UND*	00000000 av_image_fill_linesizes
00000000         *UND*	00000000 av_image_fill_pointers
00000000         *UND*	00000000 av_buffer_pool_uninit
00000000         *UND*	00000000 av_buffer_pool_init
00000000         *UND*	00000000 av_sample_fmt_is_planar
00000000         *UND*	00000000 av_samples_get_buffer_size
00000000         *UND*	00000000 av_buffer_pool_get
00000000         *UND*	00000000 avpriv_set_systematic_pal2
00000000         *UND*	00000000 av_mallocz_array
00000000         *UND*	00000000 av_buffer_allocz
00002444 g     F .text	0000029c ff_init_buffer_info
00000000         *UND*	00000000 av_packet_unpack_dictionary
00000000         *UND*	00000000 av_image_check_sar
00000000         *UND*	00000000 av_get_channel_layout_nb_channels
000026e0 g     F .text	00000004 ff_decode_frame_props
000026e4 g     F .text	00000068 ff_attach_decode_data
00000000         *UND*	00000000 av_buffer_create
0000274c g     F .text	00000320 ff_get_buffer
00000000         *UND*	00000000 av_image_check_size2
00000000         *UND*	00000000 av_pix_fmt_count_planes
00002a6c g     F .text	000001cc ff_reget_buffer
00000000         *UND*	00000000 av_frame_is_writable
00000000         *UND*	00000000 av_frame_alloc
00000000         *UND*	00000000 av_frame_move_ref
00000000         *UND*	00000000 av_frame_free
00000000         *UND*	00000000 av_frame_copy
00002c38 g     F .text	00000040 ff_decode_bsfs_uninit
00000000         *UND*	00000000 av_bsf_free
00000000         *UND*	00000000 av_get_token
00000000         *UND*	00000000 av_bsf_get_by_name
00000000         *UND*	00000000 av_realloc_array
00000000         *UND*	00000000 av_bsf_alloc
00000000         *UND*	00000000 avcodec_parameters_from_context
00000000         *UND*	00000000 avcodec_parameters_copy
00000000         *UND*	00000000 av_bsf_init
00002e54 g     F .text	00000134 avcodec_send_packet
00000000         *UND*	00000000 avcodec_is_open
00000000         *UND*	00000000 av_codec_is_decoder
00000000         *UND*	00000000 av_packet_ref
00002f88 g     F .text	0000016c avcodec_receive_frame
00000000         *UND*	00000000 av_frame_apply_cropping
000030f4 g     F .text	000000c8 avcodec_flush_buffers
00000000         *UND*	00000000 ff_thread_flush
00000000         *UND*	00000000 av_frame_copy_props
00000000         *UND*	00000000 av_malloc
00003510 g     F .text	00000004 avcodec_decode_video2
00003514 g     F .text	00000004 avcodec_decode_audio4



dirac.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 dirac.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 av_dirac_parse_sequence_header



dv_profile.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 dv_profile.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 ff_dv_frame_profile
00000008 g     F .text	00000024 av_dv_frame_profile
0000002c g     F .text	00000008 av_dv_codec_profile
00000034 g     F .text	00000014 av_dv_codec_profile2



eac3_data.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 eac3_data.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000240 l     O .rodata	00001800 vq_hebap7
00001a40 l     O .rodata	00000c00 vq_hebap6
00002640 l     O .rodata	00000600 vq_hebap5
00002c40 l     O .rodata	00000180 vq_hebap4
00002dc0 l     O .rodata	000000c0 vq_hebap3
00002e80 l     O .rodata	00000060 vq_hebap2
00002ee0 l     O .rodata	00000030 vq_hebap1
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .rodata	00000180 ff_eac3_spx_atten_tab
00000180 g     O .rodata	000000c0 ff_eac3_frm_expstr
00000000 g     O .data.rel.ro.local	00000020 ff_eac3_mantissa_vq
00002f10 g     O .rodata	00000024 ff_eac3_gaq_remap_2_4_b
00002f34 g     O .rodata	00000024 ff_eac3_gaq_remap_2_4_a
00002f58 g     O .rodata	00000018 ff_eac3_gaq_remap_1
00002f70 g     O .rodata	00000014 ff_eac3_bits_vs_hebap



encode.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 encode.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l       .rodata.str1.1	00000000 .LC0
00000026 l       .rodata.str1.1	00000000 .LC1
000000bf l       .rodata.str1.1	00000000 .LC4
00000065 l       .rodata.str1.1	00000000 .LC2
00000083 l       .rodata.str1.1	00000000 .LC3
000000d3 l       .rodata.str1.1	00000000 .LC5
000000f9 l       .rodata.str1.1	00000000 .LC6
00000121 l       .rodata.str1.1	00000000 .LC7
0000015c l       .rodata.str1.1	00000000 .LC8
000001bb l       .rodata.str1.1	00000000 .LC9
00000248 l       .rodata.str1.1	00000000 .LC12
0000025e l       .rodata.str1.1	00000000 .LC13
000001d6 l       .rodata.str1.1	00000000 .LC10
0000020c l       .rodata.str1.1	00000000 .LC11
0000028c l       .rodata.str1.1	00000000 .LC14
000002cb l       .rodata.str1.1	00000000 .LC16
000002a7 l       .rodata.str1.1	00000000 .LC15
00000a98 l     F .text	00000114 do_encode
000002d4 l       .rodata.str1.1	00000000 .LC17
0000031b l       .rodata.str1.1	00000000 .LC18
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000001c8 ff_alloc_packet2
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 av_fast_padded_malloc
00000000         *UND*	00000000 av_init_packet
00000000         *UND*	00000000 av_new_packet
000001c8 g     F .text	0000002c ff_alloc_packet
000001f4 g     F .text	00000534 avcodec_encode_audio2
00000000         *UND*	00000000 av_packet_unref
00000000         *UND*	00000000 av_sample_fmt_is_planar
00000000         *UND*	00000000 av_frame_alloc
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 av_rescale_q
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 av_packet_ref
00000000         *UND*	00000000 av_buffer_realloc
00000000         *UND*	00000000 av_frame_free
00000000         *UND*	00000000 av_free
00000000         *UND*	00000000 av_frame_get_side_data
00000000         *UND*	00000000 av_frame_get_buffer
00000000         *UND*	00000000 av_frame_copy_props
00000000         *UND*	00000000 av_samples_copy
00000000         *UND*	00000000 av_samples_set_silence
00000728 g     F .text	00000370 avcodec_encode_video2
00000000         *UND*	00000000 av_image_check_size2
00000bac g     F .text	0000004c avcodec_encode_subtitle
00000bf8 g     F .text	000000bc avcodec_send_frame
00000000         *UND*	00000000 avcodec_is_open
00000000         *UND*	00000000 av_codec_is_encoder
00000cb4 g     F .text	000000e4 avcodec_receive_packet
00000000         *UND*	00000000 av_packet_move_ref



faandct.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 faandct.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000100 postscale
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000304 ff_faandct
00000000         *UND*	00000000 lrintf
00000304 g     F .text	000002d0 ff_faandct248



faanidct.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 faanidct.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	0000053c p8idct
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000100 prescale
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 lrintf
0000053c g     F .text	00000098 ff_faanidct
000005d4 g     F .text	000000a0 ff_faanidct_add
00000674 g     F .text	000000a0 ff_faanidct_put



fdctdsp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fdctdsp.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000098 ff_fdctdsp_init
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_jpeg_fdct_islow_10
00000000         *UND*	00000000 ff_fdct248_islow_10
00000000         *UND*	00000000 ff_fdct_ifast
00000000         *UND*	00000000 ff_fdct_ifast248
00000000         *UND*	00000000 ff_faandct
00000000         *UND*	00000000 ff_faandct248
00000000         *UND*	00000000 ff_jpeg_fdct_islow_8
00000000         *UND*	00000000 ff_fdct248_islow_8



fft_fixed.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fft_fixed.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000060 split_radix_permutation
00000000 l     F .text	000004ac pass
000004ac l     F .text	000000c8 fft4
00000574 l     F .text	000001f0 fft8
00000764 l     F .text	00000038 fft16
0000079c l     F .text	00000038 fft32
000007d4 l     F .text	00000038 fft64
0000080c l     F .text	00000038 fft128
00000844 l     F .text	00000038 fft256
0000087c l     F .text	00000038 fft512
000008b4 l     F .text	00000038 fft1024
000008ec l     F .text	00000038 fft2048
00000924 l     F .text	00000038 fft4096
0000095c l     F .text	00000038 fft8192
00000994 l     F .text	00000038 fft16384
000009cc l     F .text	00000038 fft32768
00000a04 l     F .text	00000038 fft65536
00000a3c l     F .text	00000038 fft131072
00000a74 l     F .text	00000020 fft_calc_c
00000060 l     F .text.unlikely	000000e4 init_ff_cos_tabs
00000144 l     F .text.unlikely	00000008 init_ff_cos_tabs_131072
0000014c l     F .text.unlikely	00000008 init_ff_cos_tabs_65536
00000154 l     F .text.unlikely	00000008 init_ff_cos_tabs_32768
0000015c l     F .text.unlikely	00000008 init_ff_cos_tabs_16384
00000164 l     F .text.unlikely	00000008 init_ff_cos_tabs_8192
0000016c l     F .text.unlikely	00000008 init_ff_cos_tabs_4096
00000174 l     F .text.unlikely	00000008 init_ff_cos_tabs_2048
0000017c l     F .text.unlikely	00000008 init_ff_cos_tabs_1024
00000184 l     F .text.unlikely	00000008 init_ff_cos_tabs_512
0000018c l     F .text.unlikely	00000008 init_ff_cos_tabs_256
00000194 l     F .text.unlikely	00000008 init_ff_cos_tabs_128
0000019c l     F .text.unlikely	00000008 init_ff_cos_tabs_64
000001a4 l     F .text.unlikely	00000008 init_ff_cos_tabs_32
000001ac l     F .text.unlikely	00000008 init_ff_cos_tabs_16
00000a94 l     F .text	00000080 fft_permute_c
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000040 avx_tab
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	00000040 fft_dispatch
00000000 l    d  .data.rel.local	00000000 .data.rel.local
00000000 l     O .data.rel.local	00000090 cos_tabs_init_once
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000010       O *COM*	00000020 ff_cos_16_fixed
00000020       O *COM*	00000020 ff_cos_32_fixed
00000040       O *COM*	00000020 ff_cos_64_fixed
00000080       O *COM*	00000020 ff_cos_128_fixed
00000100       O *COM*	00000020 ff_cos_256_fixed
00000200       O *COM*	00000020 ff_cos_512_fixed
00000400       O *COM*	00000020 ff_cos_1024_fixed
00000800       O *COM*	00000020 ff_cos_2048_fixed
00001000       O *COM*	00000020 ff_cos_4096_fixed
00002000       O *COM*	00000020 ff_cos_8192_fixed
00004000       O *COM*	00000020 ff_cos_16384_fixed
00008000       O *COM*	00000020 ff_cos_32768_fixed
00010000       O *COM*	00000020 ff_cos_65536_fixed
00020000       O *COM*	00000020 ff_cos_131072_fixed
00000000         *UND*	00000000 cos
00000000         *UND*	00000000 lrint
00000000 g     O .data.rel.ro	00000048 ff_cos_tabs_fixed
00000000         *UND*	00000000 memcpy
000001b4 g     F .text.unlikely	00000020 ff_init_ff_cos_tabs_fixed
00000000         *UND*	00000000 pthread_once
000001d4 g     F .text.unlikely	000002c8 ff_fft_init_fixed
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 ff_fft_fixed_init_arm
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_imdct_calc_c_fixed
00000000         *UND*	00000000 ff_imdct_half_c_fixed
00000000         *UND*	00000000 ff_mdct_calc_c_fixed
00000000         *UND*	00000000 ff_mdct_calcw_c
0000049c g     F .text.unlikely	00000024 ff_fft_end_fixed



fft_fixed_32.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fft_fixed_32.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000060 split_radix_permutation
00000000 l     F .text	00000608 fft_calc_c
00000060 l     F .text.unlikely	0000002c fft_lut_init
00000608 l     F .text	00000094 fft_permute_c
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000040 avx_tab
00000000 l     O .bss	00000004 control.7764
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_fft_offsets_lut
00000000         *UND*	00000000 ff_w_tab_sr
00000000         *UND*	00000000 ff_fft_lut_init
00000000         *UND*	00000000 memcpy
0000008c g     F .text.unlikely	00000004 ff_init_ff_cos_tabs_fixed_32
00000090 g     F .text.unlikely	00000298 ff_fft_init_fixed_32
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 pthread_once
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 ff_imdct_calc_c_fixed_32
00000000         *UND*	00000000 ff_imdct_half_c_fixed_32
00000000         *UND*	00000000 ff_mdct_calc_c_fixed_32
00000328 g     F .text.unlikely	00000024 ff_fft_end_fixed_32



fft_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fft_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	00000060 split_radix_permutation
00000000 l     F .text	00000324 pass
00000324 l     F .text	00000084 fft4
000003a8 l     F .text	00000140 fft8
000004e8 l     F .text	00000038 fft16
00000520 l     F .text	00000038 fft32
00000558 l     F .text	00000038 fft64
00000590 l     F .text	00000038 fft128
000005c8 l     F .text	00000038 fft256
00000600 l     F .text	00000038 fft512
00000638 l     F .text	00000038 fft1024
00000670 l     F .text	00000038 fft2048
000006a8 l     F .text	00000038 fft4096
000006e0 l     F .text	00000038 fft8192
00000718 l     F .text	00000038 fft16384
00000750 l     F .text	00000038 fft32768
00000788 l     F .text	00000038 fft65536
000007c0 l     F .text	00000038 fft131072
000007f8 l     F .text	00000020 fft_calc_c
00000060 l     F .text.unlikely	000000b4 init_ff_cos_tabs
00000114 l     F .text.unlikely	00000008 init_ff_cos_tabs_131072
0000011c l     F .text.unlikely	00000008 init_ff_cos_tabs_65536
00000124 l     F .text.unlikely	00000008 init_ff_cos_tabs_32768
0000012c l     F .text.unlikely	00000008 init_ff_cos_tabs_16384
00000134 l     F .text.unlikely	00000008 init_ff_cos_tabs_8192
0000013c l     F .text.unlikely	00000008 init_ff_cos_tabs_4096
00000144 l     F .text.unlikely	00000008 init_ff_cos_tabs_2048
0000014c l     F .text.unlikely	00000008 init_ff_cos_tabs_1024
00000154 l     F .text.unlikely	00000008 init_ff_cos_tabs_512
0000015c l     F .text.unlikely	00000008 init_ff_cos_tabs_256
00000164 l     F .text.unlikely	00000008 init_ff_cos_tabs_128
0000016c l     F .text.unlikely	00000008 init_ff_cos_tabs_64
00000174 l     F .text.unlikely	00000008 init_ff_cos_tabs_32
0000017c l     F .text.unlikely	00000008 init_ff_cos_tabs_16
00000818 l     F .text	00000094 fft_permute_c
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000040 avx_tab
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	00000040 fft_dispatch
00000000 l    d  .data.rel.local	00000000 .data.rel.local
00000000 l     O .data.rel.local	00000090 cos_tabs_init_once
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000020       O *COM*	00000020 ff_cos_16
00000040       O *COM*	00000020 ff_cos_32
00000080       O *COM*	00000020 ff_cos_64
00000100       O *COM*	00000020 ff_cos_128
00000200       O *COM*	00000020 ff_cos_256
00000400       O *COM*	00000020 ff_cos_512
00000800       O *COM*	00000020 ff_cos_1024
00001000       O *COM*	00000020 ff_cos_2048
00002000       O *COM*	00000020 ff_cos_4096
00004000       O *COM*	00000020 ff_cos_8192
00008000       O *COM*	00000020 ff_cos_16384
00010000       O *COM*	00000020 ff_cos_32768
00020000       O *COM*	00000020 ff_cos_65536
00040000       O *COM*	00000020 ff_cos_131072
00000000         *UND*	00000000 cos
00000000 g     O .data.rel.ro	00000048 ff_cos_tabs
00000000         *UND*	00000000 memcpy
00000184 g     F .text.unlikely	00000020 ff_init_ff_cos_tabs
00000000         *UND*	00000000 pthread_once
000001a4 g     F .text.unlikely	000002b4 ff_fft_init
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 ff_fft_init_arm
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 ff_imdct_calc_c
00000000         *UND*	00000000 ff_imdct_half_c
00000000         *UND*	00000000 ff_mdct_calc_c
00000458 g     F .text.unlikely	00000024 ff_fft_end



fft_init_table.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fft_init_table.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000078 ff_fft_lut_init
0000aaaa       O *COM*	00000002 ff_fft_offsets_lut
00000000 g     O .rodata	00002000 ff_w_tab_sr



fmtconvert.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 fmtconvert.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000030 int32_to_float_fmul_scalar_c
00000030 l     F .text	00000028 int32_to_float_c
00000058 l     F .text	0000004c int32_to_float_fmul_array8_c
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000034 ff_fmt_convert_init
00000000         *UND*	00000000 ff_fmt_convert_init_arm



idctdsp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 idctdsp.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
0000007c l     F .text	00000064 put_signed_pixels_clamped_c
000001a0 l     F .text	00000018 ff_jref_idct1_put
000001b8 l     F .text	0000001c ff_jref_idct1_add
000001d4 l     F .text	00000084 ff_jref_idct4_add
00000258 l     F .text	00000064 ff_jref_idct4_put
000002bc l     F .text	00000060 ff_jref_idct2_add
0000031c l     F .text	00000050 ff_jref_idct2_put
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l       .rodata.str1.1	00000000 .LC0
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000007c ff_put_pixels_clamped_c
000000e0 g     F .text	000000c0 ff_add_pixels_clamped_c
00000000         *UND*	00000000 ff_j_rev_dct4
00000000         *UND*	00000000 ff_j_rev_dct2
00000000 g     F .text.unlikely	0000004c ff_init_scantable
0000004c g     F .text.unlikely	000000d4 ff_init_scantable_permutation
00000000         *UND*	00000000 av_log
00000120 g     F .text.unlikely	00000224 ff_idctdsp_init
00000000         *UND*	00000000 ff_idctdsp_init_arm
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_j_rev_dct1
00000000         *UND*	00000000 ff_simple_idct_put_10
00000000         *UND*	00000000 ff_simple_idct_add_10
00000000         *UND*	00000000 ff_simple_idct_10
00000000         *UND*	00000000 ff_simple_idct_put_12
00000000         *UND*	00000000 ff_simple_idct_add_12
00000000         *UND*	00000000 ff_simple_idct_12
00000000         *UND*	00000000 ff_jref_idct_put
00000000         *UND*	00000000 ff_jref_idct_add
00000000         *UND*	00000000 ff_j_rev_dct
00000000         *UND*	00000000 ff_faanidct_put
00000000         *UND*	00000000 ff_faanidct_add
00000000         *UND*	00000000 ff_faanidct
00000000         *UND*	00000000 ff_simple_idct_put_8
00000000         *UND*	00000000 ff_simple_idct_add_8
00000000         *UND*	00000000 ff_simple_idct_8



imgconvert.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 imgconvert.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000000a8 is_yuv_planar
00000023 l       .rodata.str1.1	00000000 .LC2
0000001e l       .rodata.str1.1	00000000 .LC1
00000000 l       .rodata.str1.1	00000000 .LC0
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 memset
000000a8 g     F .text	00000068 avcodec_get_chroma_sub_sample
00000000         *UND*	00000000 av_pix_fmt_desc_get
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000110 g     F .text	00000004 avcodec_get_pix_fmt_loss
00000000         *UND*	00000000 av_get_pix_fmt_loss
00000114 g     F .text	00000004 avcodec_find_best_pix_fmt_of_2
00000000         *UND*	00000000 av_find_best_pix_fmt_of_2
00000118 g     F .text	00000004 avcodec_find_best_pix_fmt2
0000011c g     F .text	00000040 avcodec_find_best_pix_fmt_of_list
00000000         *UND*	00000000 __aeabi_idivmod
0000015c g     F .text	00000110 av_picture_crop
00000000         *UND*	00000000 av_image_fill_max_pixsteps
0000026c g     F .text	000003b0 av_picture_pad
00000000         *UND*	00000000 memcpy



jfdctfst.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 jfdctfst.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000268 ff_fdct_ifast
00000268 g     F .text	00000230 ff_fdct_ifast248



jfdctint.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 jfdctint.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000002ec ff_jpeg_fdct_islow_8
000002ec g     F .text	000002b0 ff_fdct248_islow_8
0000059c g     F .text	000002ec ff_jpeg_fdct_islow_10
00000888 g     F .text	000002b0 ff_fdct248_islow_10



jni.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 jni.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 av_jni_set_java_vm
00000008 g     F .text	00000008 av_jni_get_java_vm



jrevdct.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 jrevdct.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000c90 ff_j_rev_dct
00000c90 g     F .text	00000248 ff_j_rev_dct4
00000ed8 g     F .text	00000060 ff_j_rev_dct2
00000f38 g     F .text	00000014 ff_j_rev_dct1
00000f4c g     F .text	0000002c ff_jref_idct_put
00000000         *UND*	00000000 ff_put_pixels_clamped_c
00000f78 g     F .text	0000002c ff_jref_idct_add
00000000         *UND*	00000000 ff_add_pixels_clamped_c



kbdwin.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 kbdwin.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000028 l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000134 ff_kbd_window_init
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000134 g     F .text.unlikely	0000008c ff_kbd_window_init_fixed
00000000         *UND*	00000000 floor



mathtables.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mathtables.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .rodata	00000011 ff_zigzag_scan
00000011 g     O .rodata	00000040 ff_zigzag_direct
00000051 g     O .rodata	00000900 ff_crop_tab
00000951 g     O .rodata	00000100 ff_sqrt_tab
00000a54 g     O .rodata	00000404 ff_inverse



mdct15.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mdct15.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000154 fft5
00000154 l     F .text	0000019c fft15_c
000002f0 l     F .text	0000035c mdct15
0000064c l     F .text	00000154 imdct15_half
000007a0 l     F .text	000000cc postrotate_c
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000048 ff_mdct15_uninit
00000000         *UND*	00000000 ff_fft_end
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 __aeabi_idiv
00000048 g     F .text.unlikely	00000408 ff_mdct15_init
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 ff_fft_init
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 cosf
00000000         *UND*	00000000 sinf



mdct_fixed.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mdct_fixed.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000001a8 ff_imdct_half_c_fixed
000001a8 g     F .text	00000060 ff_imdct_calc_c_fixed
00000208 g     F .text	000002dc ff_mdct_calc_c_fixed
00000000 g     F .text.unlikely	0000001c ff_mdct_end_fixed
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 ff_fft_end_fixed
0000001c g     F .text.unlikely	000001bc ff_mdct_init_fixed
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ff_fft_init_fixed
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 cos
00000000         *UND*	00000000 lrint
00000000         *UND*	00000000 sin
000004e4 g     F .text	000002ec ff_mdct_calcw_c



mdct_fixed_32.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mdct_fixed_32.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000220 ff_imdct_half_c_fixed_32
00000220 g     F .text	0000005c ff_imdct_calc_c_fixed_32
0000027c g     F .text	0000034c ff_mdct_calc_c_fixed_32
00000000 g     F .text.unlikely	0000001c ff_mdct_end_fixed_32
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 ff_fft_end_fixed_32
0000001c g     F .text.unlikely	0000017c ff_mdct_init_fixed_32
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ff_fft_init_fixed_32
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 cos
00000000         *UND*	00000000 lrint
00000000         *UND*	00000000 sin



mdct_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mdct_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000158 ff_imdct_half_c
00000158 g     F .text	0000005c ff_imdct_calc_c
000001b4 g     F .text	00000268 ff_mdct_calc_c
00000000 g     F .text.unlikely	0000001c ff_mdct_end
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 ff_fft_end
0000001c g     F .text.unlikely	00000174 ff_mdct_init
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ff_fft_init
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 cos
00000000         *UND*	00000000 sin



mediacodec.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mediacodec.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 av_mediacodec_alloc_context
00000008 g     F .text	00000008 av_mediacodec_default_init
00000010 g     F .text	00000004 av_mediacodec_default_free
00000014 g     F .text	00000008 av_mediacodec_release_buffer



mjpegenc_huffman.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mjpegenc_huffman.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
0000002d l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
0000004b l       .rodata.str1.1	00000000 .LC3
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000004e4 ff_mjpegenc_huffman_compute_bits
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
000004e4 g     F .text	0000000c ff_mjpeg_encode_huffman_init
000004f0 g     F .text	000003a4 ff_mjpeg_encode_huffman_close



mpeg12framerate.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpeg12framerate.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000078 av_cmp_q
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000078 g     F .text	00000214 ff_mpeg12_find_best_frame_rate
00000000         *UND*	00000000 av_mul_q
00000000         *UND*	00000000 av_div_q
00000008 g     O .rodata	00000080 ff_mpeg12_frame_rate_tab



mpeg4audio.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpeg4audio.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_bits
00000058 l     F .text	00000040 show_bits
00000098 l     F .text	00000034 get_bits1
000000cc l     F .text	0000002c get_object_type
000000f8 l     F .text	00000044 get_sample_rate
00000000 l       .rodata.str1.1	00000000 .LC0
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000008 g     O .rodata	00000040 avpriv_mpeg4audio_sample_rates
0000013c g     F .text	00000354 ff_mpeg4audio_get_config_gb
00000000         *UND*	00000000 av_log
00000000 g     O .rodata	00000008 ff_mpeg4audio_channels
00000490 g     F .text	0000007c avpriv_mpeg4audio_get_config



mpegaudio.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudio.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_idiv
00000000 g     F .text	00000098 ff_mpa_l2_select_table



mpegaudio_parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudio_parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	000002b8 mpegaudio_parse
00000000 l       .rodata.str1.1	00000000 .LC0
00000013 l       .rodata.str1.1	00000000 .LC1
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .data.rel	00000000 .data.rel
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_ldivmod
00000000         *UND*	00000000 ff_mpa_decode_header
00000000         *UND*	00000000 avpriv_report_missing_feature
00000000         *UND*	00000000 ff_combine_frame
00000000         *UND*	00000000 memcmp
00000000 g     O .data.rel	0000002c ff_mpegaudio_parser
00000000         *UND*	00000000 ff_parse_close



mpegaudiodata.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodata.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	000000c4 alloc_table_4
000000c4 l     O .rodata	00000070 alloc_table_3
00000134 l     O .rodata	0000012c alloc_table_1
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .data.rel.ro.local	00000014 ff_mpa_alloc_tables
00000260 g     O .rodata	00000044 ff_mpa_quant_bits
000002a4 g     O .rodata	00000044 ff_mpa_quant_steps
000002e8 g     O .rodata	00000014 ff_mpa_sblimit_table
000002fc g     O .rodata	00000006 avpriv_mpa_freq_tab
00000302 g     O .rodata	000000b4 avpriv_mpa_bitrate_tab



mpegaudiodec_fixed.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodec_fixed.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000030 skip_bits_long
00000030 l     F .text	00000058 get_bits
00000088 l     F .text	00000034 get_bits1
000000bc l     F .text	00000060 l1_unscale
0000011c l     F .text	00000054 l2_unscale_group
00000170 l     F .text	00000054 l3_unscale
000001c4 l     F .text	000000e8 imdct12
000002ac l     F .text	00000040 flush
000002ec l     F .text	000008d8 mp_decode_layer2
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     F .text.unlikely	000008c8 decode_init_static
00000061 l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
000008c8 l     F .text.unlikely	00000090 decode_init
00000bc4 l     F .text	0000009c switch_buffer
00000c60 l     F .text	000002b0 compute_imdct.isra.6
00000f10 l     F .text	00001d90 mp_decode_layer3
000000ac l       .rodata.str1.1	00000000 .LC5
00000084 l       .rodata.str1.1	00000000 .LC3
00000098 l       .rodata.str1.1	00000000 .LC4
000000c1 l       .rodata.str1.1	00000000 .LC6
000000e4 l       .rodata.str1.1	00000000 .LC7
00002ca0 l     F .text	00000898 decode_frame
000001be l       .rodata.str1.1	00000000 .LC8
000000f2 l       .rodata.str1.1	00000000 .LC9
00000106 l       .rodata.str1.1	00000000 .LC10
00000116 l       .rodata.str1.1	00000000 .LC11
00000128 l       .rodata.str1.1	00000000 .LC12
0000015b l       .rodata.str1.1	00000000 .LC13
00000174 l       .rodata.str1.1	00000000 .LC14
0000018d l       .rodata.str1.1	00000000 .LC15
00000196 l       .rodata.str1.1	00000000 .LC16
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000024 scale_factor_mult2
00000024 l     O .rodata	00000040 huff_vlc_tables_sizes
00000064 l     O .rodata	00000020 mpa_quad_bits
00000084 l     O .rodata	00000020 mpa_quad_codes
000000a4 l     O .rodata	000000c6 band_size_long
00000170 l     O .rodata	00000020 exp2_lut.9975
00000190 l     O .rodata	00000020 ci_table
000001b0 l     O .rodata	00000020 slen_table
000001d0 l     O .rodata	00000048 lsf_nsf_table
00000218 l     O .rodata	0000002c mpa_pretab
00000244 l     O .rodata	00000075 band_size_short
000002b9 l     O .rodata	00000040 mpa_huff_data
000002fc l     O .rodata	00000040 idxtab.10356
0000033c l     O .rodata	0000000c __compound_literal.1
00000348 l     O .rodata	0000000c __compound_literal.0
00000354 l     O .rodata	00000100 mpa_huffbits_24
00000454 l     O .rodata	00000200 mpa_huffcodes_24
00000654 l     O .rodata	00000100 mpa_huffbits_16
00000754 l     O .rodata	00000200 mpa_huffcodes_16
00000954 l     O .rodata	00000100 mpa_huffbits_15
00000a54 l     O .rodata	00000200 mpa_huffcodes_15
00000c54 l     O .rodata	00000100 mpa_huffbits_13
00000d54 l     O .rodata	00000200 mpa_huffcodes_13
00000f54 l     O .rodata	00000040 mpa_huffbits_12
00000f94 l     O .rodata	00000080 mpa_huffcodes_12
00001014 l     O .rodata	00000040 mpa_huffbits_11
00001054 l     O .rodata	00000080 mpa_huffcodes_11
000010d4 l     O .rodata	00000040 mpa_huffbits_10
00001114 l     O .rodata	00000080 mpa_huffcodes_10
00001194 l     O .rodata	00000024 mpa_huffbits_9
000011b8 l     O .rodata	00000048 mpa_huffcodes_9
00001200 l     O .rodata	00000024 mpa_huffbits_8
00001224 l     O .rodata	00000048 mpa_huffcodes_8
0000126c l     O .rodata	00000024 mpa_huffbits_7
00001290 l     O .rodata	00000048 mpa_huffcodes_7
000012d8 l     O .rodata	00000010 mpa_huffbits_6
000012e8 l     O .rodata	00000020 mpa_huffcodes_6
00001308 l     O .rodata	00000010 mpa_huffbits_5
00001318 l     O .rodata	00000020 mpa_huffcodes_5
00001338 l     O .rodata	00000009 mpa_huffbits_3
00001342 l     O .rodata	00000012 mpa_huffcodes_3
00001354 l     O .rodata	00000009 mpa_huffbits_2
0000135e l     O .rodata	00000012 mpa_huffcodes_2
00001370 l     O .rodata	00000004 mpa_huffbits_1
00001374 l     O .rodata	00000008 mpa_huffcodes_1
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	00000010 division_tabs
0003e66c l     O .bss	00000080 division_tab3
0003e46c l     O .bss	00000200 division_tab5
0003d46c l     O .bss	00001000 division_tab9
00000010 l     O .data.rel.ro.local	000000c0 mpa_huff_tables
00000000 l    d  .data.rel.local	00000000 .data.rel.local
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .bss	00000080 scale_factor_modshift
00000080 l     O .bss	000000b4 scale_factor_mult
00000134 l     O .bss	0000803c table_4_3_exp
00008170 l     O .bss	000200f0 table_4_3_value
00028260 l     O .bss	00000100 huff_vlc
00028360 l     O .bss	00003a88 huff_vlc_tables
0002bde8 l     O .bss	00000020 huff_quad_vlc
0002be08 l     O .bss	00000240 huff_quad_vlc_tables
0002c048 l     O .bss	0000019e band_index_long
0002c1e8 l     O .bss	00000080 pow43_lut.9976
0002c268 l     O .bss	00008000 expval_table_float
00034268 l     O .bss	00000800 exp_table_float
00034a68 l     O .bss	00008000 expval_table_fixed
0003ca68 l     O .bss	00000800 exp_table_fixed
0003d268 l     O .bss	00000080 is_table
0003d2e8 l     O .bss	00000100 is_table_lsf
0003d3e8 l     O .bss	00000080 csa_table
0003d468 l     O .bss	00000004 initialized_tables.10132
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_llsl
00000000         *UND*	00000000 __aeabi_lasr
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 __aeabi_idiv
00000000         *UND*	00000000 __aeabi_idivmod
00000000         *UND*	00000000 ff_mpa_l2_select_table
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_mpa_sblimit_table
00000000         *UND*	00000000 ff_mpa_alloc_tables
00000000         *UND*	00000000 ff_mpa_quant_bits
00000000         *UND*	00000000 ff_mpa_quant_steps
00000000         *UND*	00000000 __aeabi_ldivmod
00000000         *UND*	00000000 ff_mpa_synth_init_fixed
00000000         *UND*	00000000 ff_init_vlc_sparse
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 cbrt
00000000         *UND*	00000000 frexp
00000000         *UND*	00000000 llrint
00000000         *UND*	00000000 tan
00000000         *UND*	00000000 exp2
00000000         *UND*	00000000 ff_mpa_synth_window_fixed
00000000         *UND*	00000000 ff_mpadsp_init
00000000         *UND*	00000000 ff_mdct_win_fixed
00000000         *UND*	00000000 avpriv_request_sample
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 avpriv_mpegaudio_decode_header
00000000         *UND*	00000000 memmove
00000000         *UND*	00000000 ff_get_buffer
00000000         *UND*	00000000 ff_mpa_synth_filter_fixed
00000000 g     O .data.rel.local	00000080 ff_mp2_decoder
00000080 g     O .data.rel.local	00000080 ff_mp1_decoder



mpegaudiodecheader.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodecheader.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_idiv
00000000 g     F .text	000001a0 avpriv_mpegaudio_decode_header
00000000         *UND*	00000000 avpriv_mpa_freq_tab
00000000         *UND*	00000000 avpriv_mpa_bitrate_tab
000001a0 g     F .text	000000c0 ff_mpa_decode_header



mpegaudiodsp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodsp.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l     O .bss	00000004 mpadsp_float_table_init
00000004 l     O .bss	00000004 mpadsp_fixed_table_init
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000000d4 ff_mpadsp_init
00000000         *UND*	00000000 ff_dct_init
00000000         *UND*	00000000 pthread_once
00000000         *UND*	00000000 ff_mpadsp_init_arm
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 ff_init_mpadsp_tabs_float
00000000         *UND*	00000000 ff_init_mpadsp_tabs_fixed
00000000         *UND*	00000000 ff_mpadsp_apply_window_float
00000000         *UND*	00000000 ff_mpadsp_apply_window_fixed
00000000         *UND*	00000000 ff_dct32_fixed
00000000         *UND*	00000000 ff_imdct36_blocks_float
00000000         *UND*	00000000 ff_imdct36_blocks_fixed



mpegaudiodsp_data.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodsp_data.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     O .rodata	00000404 ff_mpa_enwindow



mpegaudiodsp_fixed.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodsp_fixed.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000024 icos36h
00000024 l     O .rodata	00000024 icos36
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	0000042c ff_mpadsp_apply_window_fixed
00000000         *UND*	00000000 memcpy
0000042c g     F .text	00000060 ff_mpa_synth_filter_fixed
00000000 g     F .text.unlikely	000000cc ff_mpa_synth_init_fixed
00000000         *UND*	00000000 ff_mpa_enwindow
00000000         *UND*	00000000 __aeabi_idivmod
00000000         *UND*	00000000 __aeabi_idiv
000000cc g     F .text.unlikely	00000254 ff_init_mpadsp_tabs_fixed
00000000         *UND*	00000000 sin
00000000         *UND*	00000000 cos
00000500       O *COM*	00000010 ff_mdct_win_fixed
0000048c g     F .text	00000410 ff_imdct36_blocks_fixed
00000c00       O *COM*	00000010 ff_mpa_synth_window_fixed



mpegaudiodsp_float.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 mpegaudiodsp_float.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000024 icos36h
00000024 l     O .rodata	00000024 icos36
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000368 ff_mpadsp_apply_window_float
00000000         *UND*	00000000 memcpy
00000368 g     F .text	00000060 ff_mpa_synth_filter_float
00000000 g     F .text.unlikely	000000c8 ff_mpa_synth_init_float
00000000         *UND*	00000000 ff_mpa_enwindow
00000000         *UND*	00000000 __aeabi_idivmod
00000000         *UND*	00000000 __aeabi_idiv
000000c8 g     F .text.unlikely	00000220 ff_init_mpadsp_tabs_float
00000000         *UND*	00000000 sin
00000000         *UND*	00000000 cos
00000500       O *COM*	00000010 ff_mdct_win_float
000003c8 g     F .text	000003a8 ff_imdct36_blocks_float
00000c00       O *COM*	00000010 ff_mpa_synth_window_float



null_bsf.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 null_bsf.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	0000003c null_filter
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 ff_bsf_get_packet
00000000         *UND*	00000000 av_packet_move_ref
00000000         *UND*	00000000 av_packet_free
00000000 g     O .data.rel.ro.local	0000001c ff_null_bsf



options.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 options.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000050 context_to_name
00000000 l       .rodata.str1.1	00000000 .LC0
00000050 l     F .text	00000028 get_category
00000078 l     F .text	00000054 codec_child_class_next
000000cc l     F .text	00000090 copy_context_reset
0000015c l     F .text	00000038 codec_child_next
00000194 l     F .text	000001ec init_context_defaults
0000002c l       .rodata.str1.1	00000000 .LC3
00000005 l       .rodata.str1.1	00000000 .LC1
00000023 l       .rodata.str1.1	00000000 .LC2
00000041 l       .rodata.str1.1	00000000 .LC4
0000007e l       .rodata.str1.1	00000000 .LC5
00000000 l    d  .data.rel.ro.local	00000000 .data.rel.ro.local
00000000 l     O .data.rel.ro.local	0000002c av_codec_context_class
000003e8 l     O .data.rel.ro.local	00004740 avcodec_options
0000002c l     O .data.rel.ro.local	0000002c av_frame_class
00000208 l     O .data.rel.ro.local	000001e0 frame_options
00000058 l     O .data.rel.ro.local	0000002c av_subtitle_rect_class
00000088 l     O .data.rel.ro.local	00000180 subtitle_rect_options
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 av_codec_next
00000000         *UND*	00000000 av_opt_free
00000000         *UND*	00000000 av_frame_free
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 av_buffer_unref
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 av_opt_set_defaults2
00000000         *UND*	00000000 av_opt_set_defaults
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_opt_set
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
00000000         *UND*	00000000 avcodec_default_get_buffer2
00000000         *UND*	00000000 avcodec_default_get_format
00000000         *UND*	00000000 avcodec_default_execute
00000000         *UND*	00000000 avcodec_default_execute2
00000380 g     F .text	00000004 avcodec_get_context_defaults3
00000384 g     F .text	0000003c avcodec_alloc_context3
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 av_free
000003c0 g     F .text	00000050 avcodec_free_context
00000000         *UND*	00000000 avcodec_close
00000410 g     F .text	000002a4 avcodec_copy_context
00000000         *UND*	00000000 avcodec_is_open
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 av_opt_copy
00000000         *UND*	00000000 av_buffer_ref
000006b4 g     F .text	00000010 avcodec_get_class
000006c4 g     F .text	00000014 avcodec_get_frame_class
000006d8 g     F .text	00000014 avcodec_get_subtitle_rect_class



parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000032 l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
00000046 l       .rodata.str1.1	00000000 .LC3
00000070 l       .rodata.str1.1	00000000 .LC4
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .bss	00000004 av_first_parser
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000020 av_parser_next
00000020 g     F .text	00000040 av_register_codec_parser
00000060 g     F .text	00000104 av_parser_init
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 av_free
00000164 g     F .text	000000fc ff_fetch_timestamp
00000260 g     F .text	000001cc av_parser_parse2
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
0000042c g     F .text	000000e4 av_parser_change
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 memcpy
00000510 g     F .text	00000034 av_parser_close
00000544 g     F .text	00000238 ff_combine_frame
00000000         *UND*	00000000 av_fast_realloc
0000077c g     F .text	00000008 ff_parse_close
00000784 g     F .text	00000068 ff_mpeg4video_split
00000000         *UND*	00000000 avpriv_find_start_code



profiles.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 profiles.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes



pthread.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 pthread.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l       .rodata.str1.1	00000000 .LC0
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	000000f4 ff_thread_init
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 ff_slice_thread_init
00000000         *UND*	00000000 ff_frame_thread_init
000000f4 g     F .text	00000018 ff_thread_free
00000000         *UND*	00000000 ff_frame_thread_free
00000000         *UND*	00000000 ff_slice_thread_free



pthread_frame.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 pthread_frame.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000048 async_lock
00000048 l     F .text	000001ec update_context_from_thread
00000234 l     F .text	0000007c async_unlock
0000002f l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
000002b0 l     F .text	000000a4 park_frame_worker_threads
00000354 l     F .text	000000ac release_delayed_buffers
0000004a l       .rodata.str1.1	00000000 .LC3
000000a3 l       .rodata.str1.1	00000000 .LC4
000000ce l       .rodata.str1.1	00000000 .LC5
000000e7 l       .rodata.str1.1	00000000 .LC6
0000010c l       .rodata.str1.1	00000000 .LC7
00000b18 l     F .text	00000218 frame_worker_thread
0000014d l       .rodata.str1.1	00000000 .LC9
00000135 l       .rodata.str1.1	00000000 .LC8
000001ab l       .rodata.str1.1	00000000 .LC10
000001c7 l       .rodata.str1.1	00000000 .LC11
00000205 l       .rodata.str1.1	00000000 .LC12
00000243 l       .rodata.str1.1	00000000 .LC13
0000025f l       .rodata.str1.1	00000000 .LC14
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 pthread_mutex_lock
00000000         *UND*	00000000 pthread_cond_wait
00000000         *UND*	00000000 pthread_mutex_unlock
00000000         *UND*	00000000 av_buffer_unref
00000000         *UND*	00000000 av_buffer_ref
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 pthread_cond_broadcast
00000000         *UND*	00000000 av_frame_unref
00000400 g     F .text	000004f8 ff_thread_decode_frame
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 av_reallocp_array
00000000         *UND*	00000000 av_packet_unref
00000000         *UND*	00000000 av_packet_ref
00000000         *UND*	00000000 pthread_cond_signal
00000000         *UND*	00000000 ff_get_format
00000000         *UND*	00000000 ff_get_buffer
00000000         *UND*	00000000 av_frame_move_ref
00000000         *UND*	00000000 avcodec_default_get_format
00000000         *UND*	00000000 avcodec_default_get_buffer2
000008f8 g     F .text	000000a0 ff_thread_report_progress
00000998 g     F .text	000000b4 ff_thread_await_progress
00000a4c g     F .text	000000cc ff_thread_finish_setup
00000d30 g     F .text	00000250 ff_frame_thread_free
00000000         *UND*	00000000 pthread_join
00000000         *UND*	00000000 av_frame_free
00000000         *UND*	00000000 pthread_mutex_destroy
00000000         *UND*	00000000 pthread_cond_destroy
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 av_opt_free
00000f80 g     F .text	000002f8 ff_frame_thread_init
00000000         *UND*	00000000 av_cpu_count
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_mallocz_array
00000000         *UND*	00000000 pthread_mutex_init
00000000         *UND*	00000000 pthread_cond_init
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 av_frame_alloc
00000000         *UND*	00000000 pthread_create
00001278 g     F .text	000000bc ff_thread_flush
00001334 g     F .text	00000074 ff_thread_can_start_frame
000013a8 g     F .text	000000e8 ff_thread_get_format
00001490 g     F .text	00000224 ff_thread_get_buffer
00000000         *UND*	00000000 ff_init_buffer_info
00000000         *UND*	00000000 av_buffer_alloc
00000000         *UND*	00000000 _GLOBAL_OFFSET_TABLE_
000016b4 g     F .text	00000120 ff_thread_release_buffer
00000000         *UND*	00000000 av_fast_realloc



pthread_slice.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 pthread_slice.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000010 main_function
00000010 l     F .text	00000054 worker_func
00000064 l     F .text	00000074 thread_execute
000000d8 l     F .text	0000002c thread_execute2
00000045 l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l     O .bss	00000004 mainfunc.9942
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 avcodec_default_execute
00000000         *UND*	00000000 avpriv_slicethread_execute
00000104 g     F .text	00000074 ff_slice_thread_free
00000000         *UND*	00000000 avpriv_slicethread_free
00000000         *UND*	00000000 pthread_mutex_destroy
00000000         *UND*	00000000 pthread_cond_destroy
00000000         *UND*	00000000 av_freep
00000178 g     F .text	00000038 ff_slice_thread_execute_with_mainfunc
000001b0 g     F .text	00000144 ff_slice_thread_init
00000000         *UND*	00000000 av_codec_is_encoder
00000000         *UND*	00000000 av_cpu_count
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 avpriv_slicethread_create
000002f4 g     F .text	00000050 ff_thread_report_progress2
00000000         *UND*	00000000 pthread_mutex_lock
00000000         *UND*	00000000 pthread_cond_signal
00000000         *UND*	00000000 pthread_mutex_unlock
00000344 g     F .text	00000090 ff_thread_await_progress2
00000000         *UND*	00000000 pthread_cond_wait
000003d4 g     F .text	0000015c ff_alloc_entries
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 av_mallocz_array
00000000         *UND*	00000000 av_malloc_array
00000000         *UND*	00000000 pthread_mutex_init
00000000         *UND*	00000000 pthread_cond_init
00000530 g     F .text	0000001c ff_reset_entries
00000000         *UND*	00000000 memset



qsv_api.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 qsv_api.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000008 av_qsv_alloc_context



raw.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 raw.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000010 avpriv_get_raw_pix_fmt_tags
00000098 g     O .rodata	00000790 ff_raw_pix_fmt_tags
00000010 g     F .text	00000038 avcodec_pix_fmt_to_codec_tag
00000000 g     O .rodata	00000048 avpriv_pix_fmt_bps_mov
00000048 g     O .rodata	00000050 avpriv_pix_fmt_bps_avi



rdft.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 rdft.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	0000023c rdft_calc_c
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000000d4 ff_rdft_init
00000000         *UND*	00000000 ff_fft_init
00000000         *UND*	00000000 ff_init_ff_cos_tabs
00000000         *UND*	00000000 ff_rdft_init_arm
00000000         *UND*	00000000 ff_cos_tabs
000000d4 g     F .text.unlikely	00000008 ff_rdft_end
00000000         *UND*	00000000 ff_fft_end



sbrdsp.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 sbrdsp.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000050 sbr_sum_square_c
00000050 l     F .text	0000002c sbr_neg_odd_64_c
0000007c l     F .text	00000078 sbr_qmf_pre_shuffle_c
000000f4 l     F .text	00000050 sbr_qmf_post_shuffle_c
00000144 l     F .text	00000030 sbr_qmf_deint_neg_c
00000174 l     F .text	00000154 sbr_autocorrelate_c
000002c8 l     F .text	000000b8 sbr_hf_gen_c
00000380 l     F .text	00000050 sbr_hf_g_filt_c
000003d0 l     F .text	0000004c sbr_sum64x5_c
0000041c l     F .text	0000003c sbr_qmf_deint_bfly_c
00000458 l     F .text	0000008c sbr_hf_apply_noise_0
000004e4 l     F .text	000000ac sbr_hf_apply_noise_1
00000590 l     F .text	0000008c sbr_hf_apply_noise_2
0000061c l     F .text	000000b0 sbr_hf_apply_noise_3
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 ff_sbr_noise_table
00000000 g     F .text.unlikely	000000e4 ff_sbrdsp_init
00000000         *UND*	00000000 ff_sbrdsp_init_arm



simple_idct.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 simple_idct.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000284 idctSparseColAdd_8
00000284 l     F .text	0000009c idct4col_put
00000320 l     F .text	000000b0 idct4col_add
000003d0 l     F .text	00000090 idct4row
00000460 l     F .text	00000240 idctRowCondDC_12.constprop.3
000006a0 l     F .text	00000214 idctRowCondDC_10.constprop.5
000008b4 l     F .text	00000208 idctRowCondDC_8.constprop.6
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000abc g     F .text	000002b0 ff_simple_idct_put_8
00000d6c g     F .text	0000004c ff_simple_idct_add_8
00000db8 g     F .text	00000210 ff_simple_idct_8
00000fc8 g     F .text	0000029c ff_simple_idct_put_10
00001264 g     F .text	000002e0 ff_simple_idct_add_10
00001544 g     F .text	000001e8 ff_simple_idct_10
0000172c g     F .text	000002d8 ff_simple_idct_put_12
00001a04 g     F .text	00000320 ff_simple_idct_add_12
00001d24 g     F .text	00000230 ff_simple_idct_12
00001f54 g     F .text	0000013c ff_simple_idct248_put
00002090 g     F .text	0000004c ff_simple_idct84_add
000020dc g     F .text	0000004c ff_simple_idct48_add
00002128 g     F .text	0000004c ff_simple_idct44_add
00002174 g     F .text	00000414 ff_prores_idct



sinewin.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 sinewin.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	00000080 ff_sine_window_init
00000000         *UND*	00000000 sinf
00000080 g     F .text.unlikely	00000020 ff_init_ff_sine_windows
00000000 g     O .data.rel.ro	00000040 ff_sine_windows
00008000       O *COM*	00000020 ff_sine_8192
00004000       O *COM*	00000020 ff_sine_4096
00002000       O *COM*	00000020 ff_sine_2048
00001000       O *COM*	00000020 ff_sine_1024
00000800       O *COM*	00000020 ff_sine_512
00000400       O *COM*	00000020 ff_sine_256
00000200       O *COM*	00000020 ff_sine_128
00000100       O *COM*	00000020 ff_sine_64
00000080       O *COM*	00000020 ff_sine_32
00000f00       O *COM*	00000020 ff_sine_960
000001e0       O *COM*	00000020 ff_sine_120



sinewin_fixed.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 sinewin_fixed.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .text.unlikely	00000000 .text.unlikely
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text.unlikely	000000a4 ff_sine_window_init_fixed
00000000         *UND*	00000000 sinf
00000000         *UND*	00000000 floor
000000a4 g     F .text.unlikely	00000020 ff_init_ff_sine_windows_fixed
00000000 g     O .data.rel.ro	00000040 ff_sine_windows_fixed
00008000       O *COM*	00000020 ff_sine_8192_fixed
00004000       O *COM*	00000020 ff_sine_4096_fixed
00002000       O *COM*	00000020 ff_sine_2048_fixed
00001000       O *COM*	00000020 ff_sine_1024_fixed
00000800       O *COM*	00000020 ff_sine_512_fixed
00000400       O *COM*	00000020 ff_sine_256_fixed
00000200       O *COM*	00000020 ff_sine_128_fixed
00000100       O *COM*	00000020 ff_sine_64_fixed
00000080       O *COM*	00000020 ff_sine_32_fixed



utils.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 utils.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000054 codec_parameters_reset
00000054 l     F .text	000000c4 ff_fast_malloc.constprop.7
0000002f l       .rodata.str1.1	00000000 .LC2
00000000 l       .rodata.str1.1	00000000 .LC0
0000001e l       .rodata.str1.1	00000000 .LC1
00000118 l     F .text	0000011c default_lockmgr_cb
00000000 l    d  .text.unlikely	00000000 .text.unlikely
0000004a l       .rodata.str1.1	00000000 .LC3
0000007e l       .rodata.str1.1	00000000 .LC5
00000067 l       .rodata.str1.1	00000000 .LC4
000000a4 l       .rodata.str1.1	00000000 .LC8
00000091 l       .rodata.str1.1	00000000 .LC6
00000096 l       .rodata.str1.1	00000000 .LC7
000000c9 l       .rodata.str1.1	00000000 .LC9
000000cc l       .rodata.str1.1	00000000 .LC10
000000d1 l       .rodata.str1.1	00000000 .LC11
00000606 l       .rodata.str1.1	00000000 .LC12
00001194 l     F .text	00000820 get_audio_frame_duration
00001a38 l     F .text	00000080 get_bit_rate
00000631 l       .rodata.str1.1	00000000 .LC13
00000634 l       .rodata.str1.1	00000000 .LC14
00000699 l       .rodata.str1.1	00000000 .LC22
000006a0 l       .rodata.str1.1	00000000 .LC23
0000063c l       .rodata.str1.1	00000000 .LC15
0000063e l       .rodata.str1.1	00000000 .LC16
000006a6 l       .rodata.str1.1	00000000 .LC24
000006bd l       .rodata.str1.1	00000000 .LC25
000006cc l       .rodata.str1.1	00000000 .LC26
000006cf l       .rodata.str1.1	00000000 .LC27
000006d8 l       .rodata.str1.1	00000000 .LC28
000006dd l       .rodata.str1.1	00000000 .LC29
0000067c l       .rodata.str1.1	00000000 .LC21
0000063f l       .rodata.str1.1	00000000 .LC17
00000649 l       .rodata.str1.1	00000000 .LC18
00000656 l       .rodata.str1.1	00000000 .LC19
00000670 l       .rodata.str1.1	00000000 .LC20
000006e8 l       .rodata.str1.1	00000000 .LC30
000006ec l       .rodata.str1.1	00000000 .LC31
000006f2 l       .rodata.str1.1	00000000 .LC32
000006fb l       .rodata.str1.1	00000000 .LC33
00000712 l       .rodata.str1.1	00000000 .LC34
0000071a l       .rodata.str1.1	00000000 .LC35
00000724 l       .rodata.str1.1	00000000 .LC36
00000736 l       .rodata.str1.1	00000000 .LC37
00000741 l       .rodata.str1.1	00000000 .LC38
00000749 l       .rodata.str1.1	00000000 .LC39
0000074e l       .rodata.str1.1	00000000 .LC40
00000758 l       .rodata.str1.1	00000000 .LC41
00000763 l       .rodata.str1.1	00000000 .LC42
00000770 l       .rodata.str1.1	00000000 .LC43
00000778 l       .rodata.str1.1	00000000 .LC44
00000781 l       .rodata.str1.1	00000000 .LC45
0000078a l       .rodata.str1.1	00000000 .LC46
00000796 l       .rodata.str1.1	00000000 .LC47
000007a7 l       .rodata.str1.1	00000000 .LC48
000007b9 l       .rodata.str1.1	00000000 .LC49
00000823 l       .rodata.str1.1	00000000 .LC50
0000085d l       .rodata.str1.1	00000000 .LC51
00000870 l       .rodata.str1.1	00000000 .LC52
0000088f l       .rodata.str1.1	00000000 .LC55
000008b5 l       .rodata.str1.1	00000000 .LC56
00000901 l       .rodata.str1.1	00000000 .LC57
00000923 l       .rodata.str1.1	00000000 .LC58
00000949 l       .rodata.str1.1	00000000 .LC59
00000966 l       .rodata.str1.1	00000000 .LC60
00000887 l       .rodata.str1.1	00000000 .LC54
0000087f l       .rodata.str1.1	00000000 .LC53
00000983 l       .rodata.str1.1	00000000 .LC61
000009f1 l       .rodata.str1.1	00000000 .LC62
00000a22 l       .rodata.str1.1	00000000 .LC63
00000a5f l       .rodata.str1.1	00000000 .LC64
00000a81 l       .rodata.str1.1	00000000 .LC65
00000a84 l       .rodata.str1.1	00000000 .LC66
00000af3 l       .rodata.str1.1	00000000 .LC68
00000b1e l       .rodata.str1.1	00000000 .LC69
00000b3c l       .rodata.str1.1	00000000 .LC70
00000b91 l       .rodata.str1.1	00000000 .LC71
00000bc3 l       .rodata.str1.1	00000000 .LC72
00000c12 l       .rodata.str1.1	00000000 .LC73
00000c26 l       .rodata.str1.1	00000000 .LC74
00000c5b l       .rodata.str1.1	00000000 .LC75
00000c90 l       .rodata.str1.1	00000000 .LC76
00000cd1 l       .rodata.str1.1	00000000 .LC77
00000d22 l       .rodata.str1.1	00000000 .LC78
00000d61 l       .rodata.str1.1	00000000 .LC79
00000dd9 l       .rodata.str1.1	00000000 .LC80
00000e15 l       .rodata.str1.1	00000000 .LC81
00000e5f l       .rodata.str1.1	00000000 .LC82
00000ec3 l       .rodata.str1.1	00000000 .LC83
00000abc l       .rodata.str1.1	00000000 .LC67
00000efc l       .rodata.str1.1	00000000 .LC84
00000f2c l       .rodata.str1.1	00000000 .LC85
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	00000060 map.13680
00000000 l    d  .data.rel.local	00000000 .data.rel.local
00000000 l     O .data.rel.local	00000004 last_avcodec
00000000 l     O .bss	00000004 first_avcodec
00000004 l     O .data.rel.local	00000004 lockmgr_cb
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000004 l     O .bss	00000004 initialized.13170
00000008 l     O .bss	00000004 codec_mutex
0000000c l     O .bss	00000004 avformat_mutex
00000010 l     O .bss	00000004 entangled_thread_counter
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 av_freep
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 abort
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 pthread_mutex_init
00000000         *UND*	00000000 av_free
00000000         *UND*	00000000 pthread_mutex_destroy
00000000         *UND*	00000000 pthread_mutex_lock
00000000         *UND*	00000000 pthread_mutex_unlock
00000234 g     F .text	0000004c av_fast_padded_malloc
00000280 g     F .text	00000048 av_fast_padded_mallocz
000002c8 g     F .text	00000020 av_codec_next
000002e8 g     F .text	00000038 av_codec_is_encoder
00000320 g     F .text	0000002c av_codec_is_decoder
00000000 g     F .text.unlikely	00000098 avcodec_register
0000034c g     F .text	00000078 ff_set_dimensions
00000000         *UND*	00000000 av_image_check_size2
000003c4 g     F .text	0000007c ff_set_sar
00000000         *UND*	00000000 av_image_check_sar
00000440 g     F .text	0000004c ff_side_data_update_matrix_encoding
00000000         *UND*	00000000 av_frame_get_side_data
00000000         *UND*	00000000 av_frame_new_side_data
0000048c g     F .text	00000254 avcodec_align_dimensions2
00000000         *UND*	00000000 av_pix_fmt_desc_get
000006e0 g     F .text	00000084 avcodec_align_dimensions
00000764 g     F .text	00000040 avcodec_enum_to_chroma_pos
000007a4 g     F .text	00000068 avcodec_chroma_pos_to_enum
0000080c g     F .text	000000e4 avcodec_fill_audio_frame
00000000         *UND*	00000000 av_samples_get_buffer_size
00000000         *UND*	00000000 av_sample_fmt_is_planar
00000000         *UND*	00000000 av_mallocz_array
00000000         *UND*	00000000 av_samples_fill_arrays
000008f0 g     F .text	00000128 ff_color_frame
00000a18 g     F .text	00000050 avcodec_default_execute
00000a68 g     F .text	00000050 avcodec_default_execute2
00000ab8 g     F .text	0000002c avpriv_find_pix_fmt
00000ae4 g     F .text	00000018 av_codec_get_pkt_timebase
00000afc g     F .text	0000001c av_codec_set_pkt_timebase
00000b18 g     F .text	00000008 av_codec_get_codec_descriptor
00000b20 g     F .text	00000008 av_codec_set_codec_descriptor
00000b28 g     F .text	00000008 av_codec_get_lowres
00000b30 g     F .text	00000008 av_codec_set_lowres
00000b38 g     F .text	00000008 av_codec_get_seek_preroll
00000b40 g     F .text	00000008 av_codec_set_seek_preroll
00000b48 g     F .text	00000008 av_codec_get_chroma_intra_matrix
00000b50 g     F .text	00000008 av_codec_set_chroma_intra_matrix
00000b58 g     F .text	00000008 av_codec_get_codec_properties
00000b60 g     F .text	00000008 av_codec_get_max_lowres
00000b68 g     F .text	00000010 avpriv_codec_get_cap_skip_frame_fill_param
00000b78 g     F .text	000000ac avsubtitle_free
00000c24 g     F .text	00000070 avcodec_find_encoder
00000c94 g     F .text	00000068 avcodec_find_encoder_by_name
00000000         *UND*	00000000 strcmp
00000cfc g     F .text	00000070 avcodec_find_decoder
00000d6c g     F .text	00000068 avcodec_find_decoder_by_name
00000dd4 g     F .text	00000080 avcodec_get_name
00000000         *UND*	00000000 avcodec_descriptor_get
00000e54 g     F .text	000000c8 av_get_codec_tag_string
00000000         *UND*	00000000 snprintf
00000f1c g     F .text	00000040 av_get_profile_name
00000f5c g     F .text	00000050 avcodec_profile_name
00000fac g     F .text	0000000c avcodec_version
00000fb8 g     F .text	00000010 avcodec_configuration
00000fc8 g     F .text	00000014 avcodec_license
00000fdc g     F .text	000001b8 av_get_exact_bits_per_sample
00000000         *UND*	00000000 __aeabi_idiv
00000000         *UND*	00000000 __aeabi_ldivmod
000019b4 g     F .text	00000030 av_get_pcm_codec
000019e4 g     F .text	00000054 av_get_bits_per_sample
00001ab8 g     F .text	00000a2c avcodec_string
00000000         *UND*	00000000 av_get_media_type_string
00000000         *UND*	00000000 strlen
00000000         *UND*	00000000 av_log_get_level
00000000         *UND*	00000000 av_fourcc_make_string
00000000         *UND*	00000000 av_strlcat
00000000         *UND*	00000000 av_get_pix_fmt_name
00000000         *UND*	00000000 av_strlcatf
00000000         *UND*	00000000 av_color_range_name
00000000         *UND*	00000000 av_color_space_name
00000000         *UND*	00000000 av_color_primaries_name
00000000         *UND*	00000000 av_color_transfer_name
00000000         *UND*	00000000 av_get_colorspace_name
00000000         *UND*	00000000 av_chroma_location_name
00000000         *UND*	00000000 av_reduce
00000000         *UND*	00000000 av_gcd
00000000         *UND*	00000000 av_get_channel_layout_string
00000000         *UND*	00000000 av_get_sample_fmt_name
00000000         *UND*	00000000 av_get_bytes_per_sample
000024e4 g     F .text	00000054 av_get_audio_frame_duration
00002538 g     F .text	00000054 av_get_audio_frame_duration2
0000258c g     F .text	00000028 av_xiphlacing
000025b4 g     F .text	0000003c ff_match_2uint16
000025f0 g     F .text	0000003c avcodec_get_hw_config
0000262c g     F .text	00000008 av_hwaccel_next
00002634 g     F .text	00000004 av_register_hwaccel
00002638 g     F .text	000000fc av_lockmgr_register
00002734 g     F .text	000000e0 ff_unlock_avcodec
00000004       O *COM*	00000004 ff_avcodec_locked
00002814 g     F .text	00000178 ff_lock_avcodec
0000298c g     F .text	00000048 avpriv_lock_avformat
000029d4 g     F .text	00000048 avpriv_unlock_avformat
00002a1c g     F .text	00000058 avpriv_toupper4
00002a74 g     F .text	000000bc ff_thread_ref_frame
00000000         *UND*	00000000 av_frame_ref
00000000         *UND*	00000000 av_buffer_ref
00000000         *UND*	00000000 ff_thread_release_buffer
00002b30 g     F .text	00000010 avcodec_is_open
00002b40 g     F .text	00000f78 avcodec_open2
00000000         *UND*	00000000 av_dict_copy
00000000         *UND*	00000000 av_frame_alloc
00000000         *UND*	00000000 av_packet_alloc
00000000         *UND*	00000000 av_opt_set_defaults
00000000         *UND*	00000000 av_opt_set_dict
00000000         *UND*	00000000 av_match_list
00000000         *UND*	00000000 ff_thread_init
00000000         *UND*	00000000 av_get_planar_sample_fmt
00000000         *UND*	00000000 av_get_channel_layout_nb_channels
00000000         *UND*	00000000 av_mul_q
00000000         *UND*	00000000 av_dict_free
00000000         *UND*	00000000 av_opt_free
00000000         *UND*	00000000 av_frame_free
00000000         *UND*	00000000 av_packet_free
00003ab8 g     F .text	00000040 ff_codec_open2_recursive
00000098 g     F .text.unlikely	000001d0 avcodec_close
00000000         *UND*	00000000 ff_thread_free
00000000         *UND*	00000000 av_buffer_pool_uninit
00000000         *UND*	00000000 ff_decode_bsfs_uninit
00000000         *UND*	00000000 av_buffer_unref
00003af8 g     F .text	00000058 avpriv_bprint_to_extradata
00000000         *UND*	00000000 av_bprint_finalize
00003b50 g     F .text	00000100 avpriv_find_start_code
00003c50 g     F .text	00000034 av_cpb_properties_alloc
00003c84 g     F .text	0000009c ff_add_cpb_side_data
00000000         *UND*	00000000 av_realloc_array
00003d20 g     F .text	00000020 avcodec_parameters_alloc
00003d40 g     F .text	00000028 avcodec_parameters_free
00003d68 g     F .text	00000074 avcodec_parameters_copy
00000000         *UND*	00000000 memcpy
00003ddc g     F .text	00000174 avcodec_parameters_from_context
00003f50 g     F .text	00000188 avcodec_parameters_to_context
000040d8 g     F .text	000000dc ff_alloc_a53_sei
000041b4 g     F .text	000000a0 ff_guess_coded_bitrate
00000000         *UND*	00000000 av_get_bits_per_pixel
00000060 g     O .rodata	00000020 av_codec_ffversion



v4l2_buffers.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 v4l2_buffers.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000048 buf_to_m2mctx.isra.0
00000048 l     F .text	000000a4 v4l2_set_pts
000000ec l     F .text	0000006c v4l2_get_pts
00000158 l     F .text	000000a8 v4l2_buf_to_bufref
00000744 l     F .text	000000a0 v4l2_free_buffer
00000000 l       .rodata.str1.1	00000000 .LC1
00000019 l       .rodata.str1.1	00000000 .LC2
00000000 l    d  .rodata	00000000 .rodata
00000008 l     O .rodata	0000000a CSWTCH.18
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 __aeabi_ldivmod
00000000         *UND*	00000000 av_rescale_q
00000000         *UND*	00000000 av_buffer_create
00000200 g     F .text	00000090 ff_v4l2_buffer_avframe_to_buf
00000000         *UND*	00000000 memcpy
00000290 g     F .text	00000310 ff_v4l2_buffer_buf_to_avframe
00000000         *UND*	00000000 av_frame_unref
00000000         *UND*	00000000 av_log
000005a0 g     F .text	000000c0 ff_v4l2_buffer_buf_to_avpkt
00000000         *UND*	00000000 av_packet_unref
00000660 g     F .text	00000088 ff_v4l2_buffer_avpkt_to_buf
000006e8 g     F .text	0000005c ff_v4l2_buffer_enqueue
00000000         *UND*	00000000 ioctl
00000000         *UND*	00000000 __errno
00000000         *UND*	00000000 sem_post
00000000         *UND*	00000000 ff_v4l2_m2m_codec_end
000007e4 g     F .text	00000228 ff_v4l2_buffer_initialize
00000000         *UND*	00000000 mmap64



v4l2_context.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 v4l2_context.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000048 ctx_to_m2mctx
00000048 l     F .text	00000014 logger
0000005c l     F .text	0000005c v4l2_try_raw_format
000000b8 l     F .text	00000084 v4l2_get_framesize_compressed
0000013c l     F .text	00000058 v4l2_save_to_context
00000194 l     F .text	0000007c v4l2_resolution_changed
00000000 l       .rodata.str1.1	00000000 .LC0
00000210 l     F .text	00000508 v4l2_dequeue_v4l2buf
0000001f l       .rodata.str1.1	00000000 .LC1
0000002b l       .rodata.str1.1	00000000 .LC2
0000003e l       .rodata.str1.1	00000000 .LC3
0000004f l       .rodata.str1.1	00000000 .LC4
0000006b l       .rodata.str1.1	00000000 .LC5
0000008c l       .rodata.str1.1	00000000 .LC6
000000a3 l       .rodata.str1.1	00000000 .LC7
00000718 l     F .text	00000088 v4l2_getfree_v4l2buf
000000c0 l       .rodata.str1.1	00000000 .LC8
000000d0 l       .rodata.str1.1	00000000 .LC9
000000e0 l       .rodata.str1.1	00000000 .LC10
000000f6 l       .rodata.str1.1	00000000 .LC11
0000011b l       .rodata.str1.1	00000000 .LC12
00000133 l       .rodata.str1.1	00000000 .LC13
00000145 l       .rodata.str1.1	00000000 .LC14
00000164 l       .rodata.str1.1	00000000 .LC15
000001b3 l       .rodata.str1.1	00000000 .LC16
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 ff_v4l2_format_avfmt_to_v4l2
00000000         *UND*	00000000 ioctl
00000000         *UND*	00000000 av_codec_is_decoder
00000000         *UND*	00000000 av_log
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 poll
00000000         *UND*	00000000 __errno
00000000         *UND*	00000000 memcpy
00000000         *UND*	00000000 ff_v4l2_m2m_codec_full_reinit
00000000         *UND*	00000000 ff_set_dimensions
00000000         *UND*	00000000 ff_v4l2_m2m_codec_reinit
00000000         *UND*	00000000 av_strerror
000007a0 g     F .text	0000006c ff_v4l2_context_set_status
0000080c g     F .text	000000f4 ff_v4l2_context_enqueue_frame
00000000         *UND*	00000000 ff_v4l2_buffer_avframe_to_buf
00000000         *UND*	00000000 ff_v4l2_buffer_enqueue
00000900 g     F .text	000000fc ff_v4l2_context_enqueue_packet
00000000         *UND*	00000000 ff_v4l2_buffer_avpkt_to_buf
000009fc g     F .text	00000058 ff_v4l2_context_dequeue_frame
00000000         *UND*	00000000 ff_v4l2_buffer_buf_to_avframe
00000a54 g     F .text	00000058 ff_v4l2_context_dequeue_packet
00000000         *UND*	00000000 ff_v4l2_buffer_buf_to_avpkt
00000aac g     F .text	0000019c ff_v4l2_context_get_format
00000000         *UND*	00000000 ff_v4l2_format_v4l2_to_avfmt
00000000         *UND*	00000000 ff_v4l2_format_avcodec_to_v4l2
00000c48 g     F .text	00000028 ff_v4l2_context_set_format
00000c70 g     F .text	0000016c ff_v4l2_context_release
00000000         *UND*	00000000 munmap
00000000         *UND*	00000000 av_free
00000ddc g     F .text	0000026c ff_v4l2_context_init
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 ff_v4l2_buffer_initialize
00000000         *UND*	00000000 av_fourcc_make_string



v4l2_fmt.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 v4l2_fmt.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .rodata	00000000 .rodata
00000000 l     O .rodata	000001c8 fmt_map
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000044 ff_v4l2_format_avcodec_to_v4l2
00000044 g     F .text	00000048 ff_v4l2_format_avfmt_to_v4l2
0000008c g     F .text	0000005c ff_v4l2_format_v4l2_to_avfmt



v4l2_m2m.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 v4l2_m2m.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000108 v4l2_prepare_contexts
00000000 l       .rodata.str1.1	00000000 .LC0
00000008 l       .rodata.str1.1	00000000 .LC1
00000010 l       .rodata.str1.1	00000000 .LC2
0000002a l       .rodata.str1.1	00000000 .LC3
0000003a l       .rodata.str1.1	00000000 .LC4
00000054 l       .rodata.str1.1	00000000 .LC5
0000007e l       .rodata.str1.1	00000000 .LC6
0000009c l       .rodata.str1.1	00000000 .LC7
000000b4 l       .rodata.str1.1	00000000 .LC8
000000c9 l       .rodata.str1.1	00000000 .LC9
000000f7 l       .rodata.str1.1	00000000 .LC10
00000110 l       .rodata.str1.1	00000000 .LC11
00000120 l       .rodata.str1.1	00000000 .LC12
00000142 l       .rodata.str1.1	00000000 .LC13
00000165 l       .rodata.str1.1	00000000 .LC14
00000183 l       .rodata.str1.1	00000000 .LC15
000001a5 l       .rodata.str1.1	00000000 .LC16
000001c7 l       .rodata.str1.1	00000000 .LC17
000001ea l       .rodata.str1.1	00000000 .LC18
00000201 l       .rodata.str1.1	00000000 .LC19
0000020c l       .rodata.str1.1	00000000 .LC21
00000214 l       .rodata.str1.1	00000000 .LC22
00000206 l       .rodata.str1.1	00000000 .LC20
00000227 l       .rodata.str1.1	00000000 .LC23
00000246 l       .rodata.str1.1	00000000 .LC24
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000         *UND*	00000000 sem_init
00000000         *UND*	00000000 memset
00000000         *UND*	00000000 ioctl
00000000         *UND*	00000000 av_log
00000108 g     F .text	0000013c ff_v4l2_m2m_codec_reinit
00000000         *UND*	00000000 ff_v4l2_context_set_status
00000000         *UND*	00000000 ff_v4l2_context_release
00000000         *UND*	00000000 ff_v4l2_context_get_format
00000000         *UND*	00000000 sem_wait
00000000         *UND*	00000000 __errno
00000000         *UND*	00000000 ff_v4l2_context_set_format
00000000         *UND*	00000000 sem_destroy
00000244 g     F .text	0000013c ff_v4l2_m2m_codec_end
00000000         *UND*	00000000 close
00000000         *UND*	00000000 av_strerror
00000380 g     F .text	0000023c ff_v4l2_m2m_codec_full_reinit
00000000         *UND*	00000000 open
00000000         *UND*	00000000 ff_v4l2_context_init
00000000         *UND*	00000000 av_codec_is_decoder
000005bc g     F .text	000003b0 ff_v4l2_m2m_codec_init
00000000         *UND*	00000000 opendir
00000000         *UND*	00000000 readdir
00000000         *UND*	00000000 strncmp
00000000         *UND*	00000000 snprintf
00000000         *UND*	00000000 strlen
00000000         *UND*	00000000 strncpy
00000000         *UND*	00000000 closedir



vorbis_parser.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 vorbis_parser.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l     F .text	00000058 get_bits
00000000 l       .rodata.str1.1	00000000 .LC0
00000010 l       .rodata.str1.1	00000000 .LC1
00000028 l       .rodata.str1.1	00000000 .LC2
0000003c l       .rodata.str1.1	00000000 .LC3
00000054 l       .rodata.str1.1	00000000 .LC4
00000074 l       .rodata.str1.1	00000000 .LC5
0000007b l       .rodata.str1.1	00000000 .LC6
000000a2 l       .rodata.str1.1	00000000 .LC7
000000c4 l       .rodata.str1.1	00000000 .LC8
000000df l       .rodata.str1.1	00000000 .LC9
00000102 l       .rodata.str1.1	00000000 .LC10
0000012c l       .rodata.str1.1	00000000 .LC11
0000013b l       .rodata.str1.1	00000000 .LC12
00000151 l       .rodata.str1.1	00000000 .LC13
00000198 l       .rodata.str1.1	00000000 .LC14
00000000 l    d  .data.rel.ro	00000000 .data.rel.ro
00000000 l     O .data.rel.ro	0000002c vorbis_parser_class
00000000 l    d  .rodata.str1.1	00000000 .rodata.str1.1
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_ranges	00000000 .debug_ranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000058 g     F .text	0000010c av_vorbis_parse_frame_flags
00000000         *UND*	00000000 av_log
00000164 g     F .text	00000008 av_vorbis_parse_frame
0000016c g     F .text	00000014 av_vorbis_parse_reset
00000180 g     F .text	00000004 av_vorbis_parse_free
00000000         *UND*	00000000 av_freep
00000184 g     F .text	000004d8 av_vorbis_parse_init
00000000         *UND*	00000000 av_mallocz
00000000         *UND*	00000000 avpriv_split_xiph_headers
00000000         *UND*	00000000 memcmp
00000000         *UND*	00000000 av_malloc
00000000         *UND*	00000000 avpriv_request_sample
00000000         *UND*	00000000 av_free
00000000         *UND*	00000000 av_default_item_name



xiph.o:     file format elf32-littlearm

SYMBOL TABLE:
00000000 l    df *ABS*	00000000 xiph.c
00000000 l    d  .text	00000000 .text
00000000 l    d  .data	00000000 .data
00000000 l    d  .bss	00000000 .bss
00000000 l    d  .debug_info	00000000 .debug_info
00000000 l    d  .debug_abbrev	00000000 .debug_abbrev
00000000 l    d  .debug_loc	00000000 .debug_loc
00000000 l    d  .debug_aranges	00000000 .debug_aranges
00000000 l    d  .debug_line	00000000 .debug_line
00000000 l    d  .debug_str	00000000 .debug_str
00000000 l    d  .note.GNU-stack	00000000 .note.GNU-stack
00000000 l    d  .debug_frame	00000000 .debug_frame
00000000 l    d  .comment	00000000 .comment
00000000 l    d  .ARM.attributes	00000000 .ARM.attributes
00000000 g     F .text	00000144 avpriv_split_xiph_headers


