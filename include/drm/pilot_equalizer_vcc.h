/* -*- c++ -*- */
#ifndef INCLUDED_DRM_PILOT_EQUALIZER_VCC_H
#define INCLUDED_DRM_PILOT_EQUALIZER_VCC_H
#include <drm/api.h>
#include <gnuradio/sync_block.h>
#include "drm_transm_params.h"
namespace gr { namespace drm {
/*! Pilot-aided flat-channel equalizer for synchronized DRM OFDM symbols. */
class DRM_API pilot_equalizer_vcc : virtual public gr::sync_block
{
public:
    using sptr = std::shared_ptr<pilot_equalizer_vcc>;
    static sptr make(transm_params* tp);
};
} }
#endif
