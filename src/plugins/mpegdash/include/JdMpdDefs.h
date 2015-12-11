#define ELEMENT_MPD              "MPD"
#define ELEMENT_ProgramInformation "ProgramInformation"
#define ELEMENT_Location         "Location"
#define ELEMENT_Metrics          "Metrics"
#define ELEMENT_Period           "Period"
#define ELEMENT_AdaptationSet    "AdaptationSet"
#define ELEMENT_ContentProtection "ContentProtection"
#define ELEMENT_Representation    "Representation"
#define ELEMENT_BaseURL           "BaseURL"
#define ELEMENT_Role              "Role"

#define ELEMENT_SegmentBase       "SegmentBase"
#define ELEMENT_SegmentTemplate   "SegmentTemplate"
#define ELEMENT_SegmentList       "SegmentList"

#define ELEMENT_SegmentTimeline   "SegmentTimeline"
#define ELEMENT_S                 "S"
#define ELEMENT_ContentComponent  "ContentComponent"
#define ELEMENT_SegmentURL        "SegmentURL"
#define ELEMENT_SegmentInitialization  "Initialization"

#define ELEMENT_AssetIdentifier    "AssetIdentifier"

#define ATTRIB_NAME_MPD_XMLNS_XSI        "xmlns:xsi"
#define ATTRIB_VAL_MPD_XMLNS_XSI         "http://www.w3.org/2001/XMLSchema-instance"
#define ATTRIB_NAME_MPD_XMLNS            "xmlns"
#define ATTRIB_VAL_MPD_XMLNS             "urn:mpeg:DASH:schema:MPD:2011" 
#define ATTRIB_NAME_MPD_XSI_SCHEMA_LOCN  "xsi:schemaLocation"
#define ATTRIB_VAL_MPD_XSI_SCHEMA_LOCN   "urn:mpeg:DASH:schema:MPD:2011 DASH-MPD.xsd" 

#define ATTRIB_VAL_MPD_MP4PROTECTION     "urn:mpeg:dash:mp4protection:2011"

#define ATTRIB_VAL_MPD_CA_DESCRIPTOR     "urn:mpeg:dash:13818:1:CA_descriptor:2011"
#define ATTRIB_VAL_MPD_FRAME_PACKING     "urn:mpeg:dash:14496:10:frame_packing_arrangement_type:2011"
#define ATTRIB_VAL_MPD_STEREO_VID_FORMAT "urn:mpeg:dash:13818:1:stereo_video_format_type:2011"
#define ATTRIB_VAL_MPD_AUD_CHAN_CONFIG   "urn:mpeg:dash:23003:3:audio_channel_configuration:2011"
#define ATTRIB_VAL_MPD_ROLE              "urn:mpeg:dash:role:2011"
#define ATTRIB_VAL_MPD_STEREOID          "urn:mpeg:dash:stereoid:2011"

#define ATTRIB_NAME_MPD_TYPE             "type"
#define ATTRIB_VAL_MPD_TYPE_STATIC       "static" 
#define ATTRIB_VAL_MPD_TYPE_DYNAMIC      "dynamic" 

#define ATTRIB_NAME_MPD_availabilityStartTime       "availabilityStartTime"
#define ATTRIB_NAME_MPD_availabilityEndTime         "availabilityEndTime"
#define ATTRIB_NAME_MPD_mediaPresentationDuration   "mediaPresentationDuration"
#define ATTRIB_NAME_MPD_minimumUpdatePeriod         "minimumUpdatePeriod"
#define ATTRIB_NAME_MPD_minBufferTime               "minBufferTime"
#define ATTRIB_NAME_MPD_timeShiftBufferDepth        "timeShiftBufferDepth"
#define ATTRIB_NAME_MPD_suggestedPresentationDelay  "suggestedPresentationDelay"
#define ATTRIB_MPD_maxSegmentDuration               "maxSegmentDuration"
#define ATTRIB_MPD_maxSubsegmentDuration            "maxSubsegmentDuration"
#define ATTRIB_NAME_MPD_profiles                    "profiles"
#define ATTRIB_NAME_MPD_customAvailabilityDelay     "customAvailabilityDelay"

#define ATTRIB_VAL_MPD_PROFILES_full            "urn:mpeg:dash:profile:full:2011"


#define ATTRIB_VAL_MPD_PROFILE_isoff_on_demand  "urn:mpeg:dash:profile:isoff-on-demand:2011"
#define ATTRIB_VAL_MPD_PROFILE_isoff_live       "urn:mpeg:dash:profile:isoff-live:2011"
#define ATTRIB_VAL_MPD_PROFILE_isoff_main       "urn:mpeg:dash:profile:isoff-main:2011"
#define ATTRIB_VAL_MPD_PROFILES_mp2t_main       "urn:mpeg:dash:profile:mp2t-main:2011"
#define ATTRIB_VAL_MPD_PROFILES_mp2t_simple     "urn:mpeg:dash:profile:mp2t-simple:2011"

#define ATTRIB_NAME_PERIOD_start                "start"

#define ATTRIB_NAME_ADAPTSET_mimeType         "mimeType"
#define ATTRIB_VAL_ADAPTSET_MIMETYPE_MP2T     "video/mp2t"
#define ATTRIB_VAL_ADAPTSET_MIMETYPE_MP4      "video/mp4"

#define ATTRIB_NAME_ADAPTSET_codecs              "codecs"
#define ATTRIB_VAL_ADAPTSET_codecs_AVC1_42E01E  "avc1.42E01E"
#define ATTRIB_NAME_ADAPTSET_frameRate           "frameRate"
#define ATTRIB_NAME_ADAPTSET_segmentAlignment    "segmentAlignment" 
#define ATTRIB_NAME_ADAPTSET_subsegmentAlignment "subsegmentAlignment"
#define ATTRIB_NAME_ADAPTSET_bitstreamSwitching   "bitstreamSwitching"
#define ATTRIB_NAME_ADAPTSET_startWithSAP         "startWithSAP"
#define ATTRIB_NAME_ADAPTSET_subsegmentStartsWithSAP  "subsegmentStartsWithSAP"

#define ATTRIB_NAME_SEGTMPLT_media                "media"
#define ATTRIB_VAL_SEGTMPLT_MEDIA_DEFAULT         "$RepresentationID$_$Number%05$.ts"

#define ATTRIB_NAME_SEGTMPLT_index                "index"
#define ATTRIB_NAME_SEGTMPLT_bitstreamSwitching   "bitstreamSwitching"
#define ATTRIB_NAME_SEGTMPLT_timescale            "timescale"
#define ATTRIB_NAME_SEGTMPLT_initialization       "initialization"
#define ATTRIB_NAME_SEGTMPLT_startNumber          "startNumber"
#define ATTRIB_NAME_SEGTMPLT_duration             "duration"

#define ATTRIB_NAME_REPRESENTATION_id              "id"
#define ATTRIB_NAME_REPRESENTATION_mimetype        "mimeType"
#define ATTRIB_VAL_REPRESENTATION_MIMETYPE_MP2T    "video/mp2t"
#define ATTRIB_VAL_REPRESENTATION_MIMETYPE_video_mp4 "video/mp4"
#define ATTRIB_VAL_REPRESENTATION_MIMETYPE_audio_mp4 "audio/mp4"

#define ATTRIB_NAME_REPRESENTATION_width           "width" 
#define ATTRIB_NAME_REPRESENTATION_height          "height" 
#define ATTRIB_NAME_REPRESENTATION_rameRate        "frameRate" 
#define ATTRIB_NAME_REPRESENTATION_bandwidth       "bandwidth"

#define ATTRIB_NAME_S_t                      "t" 
#define ATTRIB_NAME_S_d                      "d"
#define ATTRIB_NAME_S_r                      "r"

#define ELEMENT_ATTRIB_NAME_CONTCOMP_contentType    "contentType"
#define ELEMENT_ATTRIB_NAME_CONTCOMP_id             "id"
#define ELEMENT_ATTRIB_NAME_CONTCOMP_lang           "lang"

#define ATTRIB_NAME_PERIOD_id                        "id"
#define ATTRIB_NAME_PERIOD_duration                  "duration"

#define ATTRIB_NAME_SEGMENTLIST_presentationTimeOffset "presentationTimeOffset"
#define ATTRIB_NAME_SEGMENTLIST_timescale              "timescale"
#define ATTRIB_NAME_SEGMENTLIST_bitstreamSwitching   "bitstreamSwitching"
#define ATTRIB_NAME_SEGMENTLIST_duration             "duration"
#define ATTRIB_NAME_SEGMENTLIST_startNumber          "startNumber"
#define ATTRIB_NAME_SEGMENTURL_media                 "media"

#define ATTRIB_NAME_SEGMENT_INIT_sourceURL          "sourceURL"

#define ATTRIB_NAME_SCHEME_ID_URI                     "urn:mpeg:dash:event:2012"
#define ATTRIB_VAL_SCHEME_ID_URI_1                    "1"

#define ATTRIB_NAME_XLINK_HREF                       "xlink:href"
#define ATTRIB_NAME_XLINK_ACTUATE                    "xlink:actuate"
#define ATTRIB_VAL_XLINK_ACTUATE_onRequest           "onRequest"

// USER DEFINED (not part of MPEF_DASH ISO 23999
#define ATTRIB_NAME_MCN_REPRESENTATION_source       "customSource"
#define ATTRIB_NAME_MCN_MPD_Folder                  "customFolder"
#define ATTRIB_NAME_MCN_REPRESENTATION_moofLength   "customMoofLength"
