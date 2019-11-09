#pragma once

#include <gst/base/gstbaseparse.h>

G_BEGIN_DECLS

#define CR_TYPE_SBC_PARSE            (gst_sbc_parse_get_type())
#define GST_SBC_PARSE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj),CR_TYPE_SBC_PARSE,GstSbcParse))
#define GST_SBC_PARSE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),CR_TYPE_SBC_PARSE,GstSbcParseClass))
#define GST_SBC_PARSE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),CR_TYPE_SBC_PARSE,GstSbcParseClass))
#define GST_IS_SBC_PARSE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj),CR_TYPE_SBC_PARSE))
#define GST_IS_SBC_PARSE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),CR_TYPE_SBC_PARSE))
#define GST_SBC_PARSE_CAST(obj)       ((GstSbcParse *)(obj))

typedef enum {
  GST_SBC_CHANNEL_MODE_INVALID = -1,
  GST_SBC_CHANNEL_MODE_MONO = 0,
  GST_SBC_CHANNEL_MODE_DUAL = 1,
  GST_SBC_CHANNEL_MODE_STEREO = 2,
  GST_SBC_CHANNEL_MODE_JOINT_STEREO = 3
} GstSbcChannelMode;

typedef enum {
  GST_SBC_ALLOCATION_METHOD_INVALID = -1,
  GST_SBC_ALLOCATION_METHOD_LOUDNESS = 0,
  GST_SBC_ALLOCATION_METHOD_SNR = 1
} GstSbcAllocationMethod;

typedef struct _GstSbcParse GstSbcParse;
typedef struct _GstSbcParseClass GstSbcParseClass;

struct _GstSbcParse {
  GstBaseParse baseparse;

  /* current output format */
  GstSbcAllocationMethod  alloc_method;
  GstSbcChannelMode       ch_mode;
  uint                    rate;
  uint                    n_blocks;
  uint                    n_subbands;
  uint                    bitpool;

  gboolean                sent_codec_tag;

  uint   m_currenBufferSize = 0;
};

struct _GstSbcParseClass {
  GstBaseParseClass baseparse_class;
};

GType gst_sbc_parse_get_type (void);

G_END_DECLS
