/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OPUS_AUDIO_DECODER_BF_H
#define INCLUDED_DRM_OPUS_AUDIO_DECODER_BF_H

#include <drm/api.h>
#include <gnuradio/block.h>
#include "drm_transm_params.h"

namespace gr { namespace drm {

/*! Decode an experimental DReaM-style Opus MSC superframe to mono float PCM.
 *
 * Input is one vector containing L_MUX unpacked bits. This extension is not
 * part of ETSI Digital Radio Mondiale.
 */
class DRM_API opus_audio_decoder_bf : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<opus_audio_decoder_bf>;
    static sptr make(transm_params* tp);
};

} }
#endif
