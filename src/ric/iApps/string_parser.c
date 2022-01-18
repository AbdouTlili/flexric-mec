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



#include "string_parser.h"
#include <assert.h>                                      // for assert
#include <stdio.h>                                       // for snprintf
#include "ric/iApps/../../sm/mac_sm/ie/mac_data_ie.h"    // for mac_ue_stats...
#include "ric/iApps/../../sm/pdcp_sm/ie/pdcp_data_ie.h"  // for pdcp_radio_b...
#include "ric/iApps/../../sm/rlc_sm/ie/rlc_data_ie.h"    // for rlc_radio_be...


void to_string_mac_ue_stats(mac_ue_stats_impl_t* stats, int64_t tstamp, char* out, size_t out_len)
{
  assert(stats != NULL);
  assert(out != NULL);
  const size_t max = 512;
  assert(out_len >= max);
  int rc = snprintf(out, max, "mac_stats: "
        "tstamp=%ld,"
        "dl_aggr_tbs=%lu,"
        "ul_aggr_tbs=%lu,"
        "dl_aggr_bytes_sdus=%lu,"
        "ul_aggr_bytes_sdus=%lu,"
        "pusch_snr=%g,"
        "pucch_snr=%g,"
        "rnti=%u,"
        "dl_aggr_prb=%u,"
        "ul_aggr_prb=%u,"
        "dl_aggr_sdus=%u,"
        "ul_aggr_sdus=%u,"
        "dl_aggr_retx_prb=%u,"
        "wb_cqi=%u,"
        "dl_mcs1=%u,"
        "ul_mcs1=%u,"
        "dl_mcs2=%u,"
        "ul_mcs2=%u,"
        "phr=%d"
        "\n"
        ,tstamp
        ,stats->dl_aggr_tbs
        ,stats->ul_aggr_tbs 
        ,stats->dl_aggr_bytes_sdus 
        ,stats->ul_aggr_bytes_sdus  
        ,stats->pusch_snr 
        ,stats->pucch_snr 
        ,stats->rnti 
        ,stats->dl_aggr_prb  
        ,stats->ul_aggr_prb 
        ,stats->dl_aggr_sdus 
        ,stats->ul_aggr_sdus 
        ,stats->dl_aggr_retx_prb  
        ,stats->wb_cqi
        ,stats->dl_mcs1
        ,stats->ul_mcs1
        ,stats->dl_mcs2
        ,stats->ul_mcs2
        ,stats->phr 
        );
  assert(rc < (int)max && "Not enough space in the char array to write all the data");
}


void to_string_rlc_rb(rlc_radio_bearer_stats_t* rlc, int64_t tstamp , char* out, size_t out_len)
{
  assert(rlc != NULL);
  assert(out != NULL);
  const size_t max = 1024;
  assert(out_len >= max);
 
  int const rc = snprintf(out, max,
        "rlc_stats: tstamp=%ld,"
        "txpdu_pkts=%u,"
        "txpdu_bytes=%u,"
        "txpdu_wt_ms=%u,"
        "txpdu_dd_pkts=%u,"
        "txpdu_dd_bytes=%u,"
        "txpdu_retx_pkts=%u,"
        "txpdu_retx_bytes=%u,"
        "txpdu_segmented=%u,"
        "txpdu_status_pkts=%u,"
        "txpdu_status_bytes=%u,"
        "txbuf_occ_bytes=%u,"
        "txbuf_occ_pkts=%u,"
        "rxpdu_pkts=%u,"
        "rxpdu_bytes=%u,"
        "rxpdu_dup_pkts=%u,"
        "rxpdu_dup_bytes=%u,"
        "rxpdu_dd_pkts=%u,"
        "rxpdu_dd_bytes=%u,"
        "rxpdu_ow_pkts=%u,"
        "rxpdu_ow_bytes=%u,"
        "rxpdu_status_pkts=%u,"
        "rxpdu_status_bytes=%u,"
        "rxbuf_occ_bytes=%u,"
        "rxbuf_occ_pkts=%u,"
        "txsdu_pkts=%u,"
        "txsdu_bytes=%u,"
        "rxsdu_pkts=%u,"
        "rxsdu_bytes=%u,"
        "rxsdu_dd_pkts=%u,"
        "rxsdu_dd_bytes=%u,"
        "rnti=%u,"
        "mode=%u,"
        "rbid=%u"
        "\n"
        , tstamp 
        , rlc->txpdu_pkts
        , rlc->txpdu_bytes
        , rlc->txpdu_wt_ms
        , rlc->txpdu_dd_pkts
        , rlc->txpdu_dd_bytes
        , rlc->txpdu_retx_pkts
        , rlc->txpdu_retx_bytes
        , rlc->txpdu_segmented
        , rlc->txpdu_status_pkts
        , rlc->txpdu_status_bytes
        , rlc->txbuf_occ_bytes
        , rlc->txbuf_occ_pkts
        , rlc->rxpdu_pkts
        , rlc->rxpdu_bytes
        , rlc->rxpdu_dup_pkts
        , rlc->rxpdu_dup_bytes
        , rlc->rxpdu_dd_pkts
        , rlc->rxpdu_dd_bytes
        , rlc->rxpdu_ow_pkts
        , rlc->rxpdu_ow_bytes
        , rlc->rxpdu_status_pkts
        , rlc->rxpdu_status_bytes
        , rlc->rxbuf_occ_bytes
        , rlc->rxbuf_occ_pkts
        , rlc->txsdu_pkts
        , rlc->txsdu_bytes
        , rlc->rxsdu_pkts
        , rlc->rxsdu_bytes
        , rlc->rxsdu_dd_pkts
        , rlc->rxsdu_dd_bytes
        , rlc->rnti
        , rlc->mode
        , rlc->rbid
        );
  assert(rc < (int)max && "Not enough space in the char array to write all the data");
}


void to_string_pdcp_rb(pdcp_radio_bearer_stats_t* pdcp, int64_t tstamp, char* out, size_t out_len)
{
  assert(pdcp != NULL);
  assert(out != NULL);
  const size_t max = 512;
  assert(out_len >= max);
 
  int rc = snprintf(out, out_len,  "pdcp_stats: " 
        "tstamp=%ld,"
        "txpdu_pkts=%u,"
        "txpdu_bytes=%u,"
        "txpdu_sn=%u,"
        "rxpdu_pkts=%u,"
        "rxpdu_bytes=%u,"
        "rxpdu_sn=%u,"
        "rxpdu_oo_pkts=%u,"
        "rxpdu_oo_bytes=%u,"
        "rxpdu_dd_pkts=%u,"
        "rxpdu_dd_bytes=%u,"
        "rxpdu_ro_count=%u,"
        "txsdu_pkts=%u,"
        "txsdu_bytes=%u,"
        "rxsdu_pkts=%u,"
        "rxsdu_bytes=%u,"
        "rnti=%u,"
        "mode=%u,"
        "rbid=%u"
        "\n"
        , tstamp
        , pdcp->txpdu_pkts
        , pdcp->txpdu_bytes 
        , pdcp->txpdu_sn
        , pdcp->rxpdu_pkts
        , pdcp->rxpdu_bytes
        , pdcp->rxpdu_sn
        , pdcp->rxpdu_oo_pkts
        , pdcp->rxpdu_oo_bytes
        , pdcp->rxpdu_dd_pkts
        , pdcp->rxpdu_dd_bytes
        , pdcp->rxpdu_ro_count
        , pdcp->txsdu_pkts
        , pdcp->txsdu_bytes
        , pdcp->rxsdu_pkts
        , pdcp->rxsdu_bytes
        , pdcp->rnti
        , pdcp->mode
        , pdcp->rbid
        );
  assert(rc < (int)max && "Not enough space in the char array to write all the data");
}

