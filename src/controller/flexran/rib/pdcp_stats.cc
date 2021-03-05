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

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "pdcp_stats.h"
#include "pdcp_stats_reader.h"
#include "pdcp_stats_json_printer.h"

namespace flexric {
namespace flexran {
namespace rib {

std::string pdcp_stats::to_json_inner() const
{
  std::lock_guard<std::mutex> guard(_lock);
  if (_msg.size() == 0 && _hdr.size() == 0)
    return "";

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);

  std::string json = "\"ueStatsHeader\":";
  pdcp_stats_IndicationHeader_print_json_as_root(&printer_ctx, _hdr.data(), _hdr.size(), NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  json += buf;
  flatcc_json_printer_reset(&printer_ctx);

  pdcp_stats_IndicationMessage_print_json_as_root(&printer_ctx, _msg.data(), _msg.size(), NULL);
  buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  json += ",";
  json.append(&buf[1], buf_size - 2);
  flatcc_json_printer_clear(&printer_ctx);

  return json;
}

/*
void pdcp_stats::update(const ric_indication_t* ind)
{
  ind_info::update(ind);
  std::cout << to_json() << "\n";
  std::cout << "frame " << get_frame_slot().first << " slot " << get_frame_slot().second << "\n";
  const size_t n = get_num_ues();
  std::cout << n << " UEs\n";
  for (uint16_t rnti : get_rntis())
    std::cout << "RNTI " << rnti << "\n";
  const size_t last = n - 1;
  std::cout << "last RNTI " << get_rnti(last) << "\n";
  std::cout << "num RB " << get_num_rb(last) << "\n";
  std::cout << "RB ID \"" << (int)get_rbid(last, 0) << "\"\n";
  std::cout << "txPduPkts " << get_tx_pdu_pkts(last, 0) << "\n";
  std::cout << "rxPduPkts " << get_rx_pdu_pkts(last, 0) << "\n";
  std::cout << "txPduBytes " << get_tx_pdu_bytes(last, 0) << "\n";
  std::cout << "rxPduBytes " << get_rx_pdu_bytes(last, 0) << "\n";
  std::cout << "txPduSn " << get_tx_pdu_sn(last, 0) << "\n";
  std::cout << "rxPduSn " << get_rx_pdu_sn(last, 0) << "\n";
  std::cout << "rxPduOoPkts " << get_rx_pdu_oo_pkts(last, 0) << "\n";
  std::cout << "rxPduOoBytes " << get_rx_pdu_oo_bytes(last, 0) << "\n";
  std::cout << "rxPduDdPkts " << get_rx_pdu_dd_pkts(last, 0) << "\n";
  std::cout << "rxPduDdBytes " << get_rx_pdu_dd_bytes(last, 0) << "\n";
  std::cout << "rxPduRoCount " << get_rx_pdu_ro_count(last, 0) << "\n";
}
*/

std::pair<uint16_t, uint16_t> pdcp_stats::get_frame_slot() const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_IndicationHeader_table_t ih = pdcp_stats_IndicationHeader_as_root(_hdr.data());
  const uint16_t frame = pdcp_stats_IndicationHeader_frame(ih);
  const uint16_t slot = pdcp_stats_IndicationHeader_slot(ih);
  return std::make_pair(frame, slot);
}

size_t pdcp_stats::get_num_ues() const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_IndicationMessage_table_t ih = pdcp_stats_IndicationMessage_as_root(_msg.data());
  pdcp_stats_UEStats_vec_t uestats = pdcp_stats_IndicationMessage_ueStats(ih);
  return pdcp_stats_UEStats_vec_len(uestats);
}

std::vector<uint16_t> pdcp_stats::get_rntis() const
{
  std::vector<uint16_t> rntis;
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_IndicationMessage_table_t ih = pdcp_stats_IndicationMessage_as_root(_msg.data());
  pdcp_stats_UEStats_vec_t uestats = pdcp_stats_IndicationMessage_ueStats(ih);
  for (size_t i = 0; i < pdcp_stats_UEStats_vec_len(uestats); ++i) {
    pdcp_stats_UEStats_table_t ue = pdcp_stats_UEStats_vec_at(uestats, i);
    rntis.push_back(pdcp_stats_UEStats_rnti(ue));
  }
  return rntis;
}


uint16_t pdcp_stats::get_rnti(size_t ue_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  return pdcp_stats_UEStats_rnti(ue);
}

size_t pdcp_stats::get_num_rb(size_t ue_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_vec_t rbstats = pdcp_stats_UEStats_rb(ue);
  return pdcp_stats_RBStats_vec_len(rbstats);
}


uint8_t pdcp_stats::get_rbid(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rbid(rb);
}

uint32_t pdcp_stats::get_tx_pdu_pkts(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_txPduPkts(rb);
}

uint32_t pdcp_stats::get_rx_pdu_pkts(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduPkts(rb);
}

uint64_t pdcp_stats::get_tx_pdu_bytes(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_txPduBytes(rb);
}

uint64_t pdcp_stats::get_rx_pdu_bytes(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduBytes(rb);
}

uint32_t pdcp_stats::get_tx_pdu_sn(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_txPduSn(rb);
}

uint32_t pdcp_stats::get_rx_pdu_sn(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduSn(rb);
}

uint32_t pdcp_stats::get_rx_pdu_oo_pkts(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduOoPkts(rb);
}

uint64_t pdcp_stats::get_rx_pdu_oo_bytes(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduOoBytes(rb);
}

uint32_t pdcp_stats::get_rx_pdu_dd_pkts(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduDdPkts(rb);
}

uint64_t pdcp_stats::get_rx_pdu_dd_bytes(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduDdBytes(rb);
}

uint32_t pdcp_stats::get_rx_pdu_ro_count(size_t ue_idx, size_t rb_idx) const
{
  std::lock_guard<std::mutex> guard(_lock);
  pdcp_stats_UEStats_table_t ue = get_fb_ue(_msg.data(), ue_idx);
  pdcp_stats_RBStats_table_t rb = get_fb_rb(ue, rb_idx);
  return pdcp_stats_RBStats_rxPduRoCount(rb);
}

pdcp_stats_UEStats_table_t pdcp_stats::get_fb_ue(const uint8_t* buf, size_t ue_idx) const
{
  pdcp_stats_IndicationMessage_table_t ih = pdcp_stats_IndicationMessage_as_root(_msg.data());
  pdcp_stats_UEStats_vec_t uestats = pdcp_stats_IndicationMessage_ueStats(ih);
  if (ue_idx >= pdcp_stats_UEStats_vec_len(uestats))
    throw std::out_of_range("UEStats index is out of range");
  return pdcp_stats_UEStats_vec_at(uestats, ue_idx);
}

pdcp_stats_RBStats_table_t pdcp_stats::get_fb_rb(pdcp_stats_UEStats_table_t ue, size_t rb_idx) const
{
  pdcp_stats_RBStats_vec_t rbstats = pdcp_stats_UEStats_rb(ue);
  if (rb_idx >= pdcp_stats_RBStats_vec_len(rbstats))
    throw std::out_of_range("RBStats index is out of range");
  return pdcp_stats_RBStats_vec_at(rbstats, rb_idx);
}

}
}
}
