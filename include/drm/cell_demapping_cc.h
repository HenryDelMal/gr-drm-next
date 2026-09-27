/* -*- c++ -*- */
#ifndef INCLUDED_DRM_CELL_DEMAPPING_CC_H
#define INCLUDED_DRM_CELL_DEMAPPING_CC_H

#include <drm/api.h>
#include <gnuradio/block.h>
#include "drm_transm_params.h"

namespace gr { namespace drm {

/*! Extract MSC, SDC and FAC QAM cells from one DRM superframe.
 *
 * Input items are FFT vectors of length ``tp->ofdm().nfft()``. One output
 * item is produced after ``N_S * M_TF`` input symbols. Outputs are vector
 * items in the same ordering expected by the transmitter's channel coders:
 * MSC, SDC, FAC.
 */
class DRM_API cell_demapping_cc : virtual public gr::block
{
public:
    using sptr = std::shared_ptr<cell_demapping_cc>;
    static sptr make(transm_params* tp);
};

} }
#endif
