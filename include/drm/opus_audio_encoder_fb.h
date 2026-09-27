/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OPUS_AUDIO_ENCODER_FB_H
#define INCLUDED_DRM_OPUS_AUDIO_ENCODER_FB_H

#include <drm/api.h>
#include <gnuradio/block.h>
#include <string>
#include "drm_transm_params.h"

namespace gr { namespace drm {

/*! Encode mono float PCM into an experimental DReaM-style Opus superframe.
 *
 * This extension is not part of ETSI Digital Radio Mondiale. It uses twenty
 * 20 ms Opus packets, 12-bit cumulative packet borders, and one CRC-8 byte per
 * packet. Output items are unpacked bits for the existing MSC chain.
 */
class DRM_API opus_audio_encoder_fb : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<opus_audio_encoder_fb>;
    static sptr make(transm_params* tp,
                     int bitrate = 0,
                     bool vbr = false,
                     const std::string& application = "audio",
                     const std::string& signal = "music",
                     const std::string& bandwidth = "auto",
                     int complexity = 10,
                     bool dtx = false);
};

} }
#endif
