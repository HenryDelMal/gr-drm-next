/* -*- c++ -*- */
#ifndef INCLUDED_DRM_OFDM_SYNCHRONIZER_CC_H
#define INCLUDED_DRM_OFDM_SYNCHRONIZER_CC_H
#include <drm/api.h>
#include <gnuradio/block.h>
namespace gr { namespace drm {
/*! Acquire a DRM cyclic prefix boundary and correct coarse carrier offset. */
class DRM_API ofdm_synchronizer_cc : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<ofdm_synchronizer_cc>;
    static sptr make(unsigned int nfft, unsigned int ncp);
};
} }
#endif
