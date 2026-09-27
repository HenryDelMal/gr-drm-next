/* -*- c++ -*- */
#ifndef INCLUDED_DRM_MLC_DECODER_BB_H
#define INCLUDED_DRM_MLC_DECODER_BB_H
#include <drm/api.h>
#include <gnuradio/block.h>
#include "drm_transm_params.h"
#include <string>
namespace gr { namespace drm {
/*! Hard-decision inverse of the transmitter MLC encoder for FAC, SDC or MSC. */
class DRM_API mlc_decoder_bb : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<mlc_decoder_bb>;
    static sptr make(transm_params* tp, const std::string& channel_type);
};
} }
#endif
