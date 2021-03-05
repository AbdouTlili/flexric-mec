/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#ifndef RLC_STATS_H_
#define RLC_STATS_H_

#include <cstdint>
#include <string>
#include <vector>

#include "ind_info.h"

// for private accessors, defined by Flatbuffers
typedef const struct rlc_stats_UEStats_table *rlc_stats_UEStats_table_t;
typedef const struct rlc_stats_RBStats_table *rlc_stats_RBStats_table_t;

namespace flexric {
namespace flexran {
namespace rib {

class rlc_stats : public ind_info {
public:
  std::string to_json_inner() const;
  //void update(const ric_indication_t* ind);

  std::pair<uint16_t, uint16_t> get_frame_slot() const;

  size_t get_num_ues() const;
  std::vector<uint16_t> get_rntis() const;

  uint16_t get_rnti(size_t ue_idx) const;
  size_t get_num_rb(size_t ue_idx) const;

  uint8_t get_rbid(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_tx_pdu_pkts(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_rx_pdu_pkts(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_tx_pdu_bytes(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_rx_pdu_bytes(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_tx_pdu_dd_pkts(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_rx_pdu_dd_pkts(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_tx_pdu_dd_bytes(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_rx_pdu_dd_bytes(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_tx_buf_occ_bytes(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_rx_buf_occ_bytes(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_tx_buf_occ_pkts(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_rx_buf_occ_pkts(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_tx_pdu_retx_pkts(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_tx_pdu_retx_bytes(size_t ue_idx, size_t rb_idx) const;
  uint16_t get_tx_pdu_wt_ms(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_tx_buf_wd_ms(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_rx_pdu_ow_pkts(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_rx_pdu_ow_bytes(size_t ue_idx, size_t rb_idx) const;
  uint16_t get_rx_pdu_rotout_ms(size_t ue_idx, size_t rb_idx) const;
  uint16_t get_rx_pdu_potout_ms(size_t ue_idx, size_t rb_idx) const;
  uint16_t get_rx_pdu_sptout_ms(size_t ue_idx, size_t rb_idx) const;
  uint32_t get_rx_pdu_dup_pkts(size_t ue_idx, size_t rb_idx) const;
  uint64_t get_rx_pdu_dup_bytes(size_t ue_idx, size_t rb_idx) const;

private:
  rlc_stats_UEStats_table_t get_fb_ue(const uint8_t* buf, size_t ue_idx) const;
  rlc_stats_RBStats_table_t get_fb_rb(rlc_stats_UEStats_table_t ue, size_t rb_idx) const;
};

}
}
}

#endif /* RLC_STATS_H_ */
