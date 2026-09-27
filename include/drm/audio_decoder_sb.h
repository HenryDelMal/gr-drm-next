/* -*- c++ -*- */
#ifndef INCLUDED_DRM_AUDIO_DECODER_SB_H
#define INCLUDED_DRM_AUDIO_DECODER_SB_H

#include <drm/api.h>
#include <gnuradio/block.h>
#include "drm_transm_params.h"

namespace gr { namespace drm {

/*! Decode one DRM MSC audio multiplex frame to mono float PCM with FAAD2.
 *
 * Input is one vector of decoded MSC bits (one byte per bit) per work item.
 * The block reverses the AAC superframe packing performed by audio_encoder_sb.
 */
class DRM_API audio_decoder_sb : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<audio_decoder_sb>;
    static sptr make(transm_params* tp);
};

} }
#endif
