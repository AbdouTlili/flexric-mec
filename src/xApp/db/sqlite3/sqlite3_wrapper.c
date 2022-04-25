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

#include "sqlite3_wrapper.h"
#include "../../../util/time_now_us.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

static
void create_table(sqlite3* db, char* sql)
{
  char* err_msg = NULL;
  int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  assert(rc == SQLITE_OK && "Error while creating the DB. Check the err_msg string for further info" );
}

static
void create_mac_ue_table(sqlite3* db)
{
  assert(db != NULL);

  // ToDo: PRIMARY KEY UNIQUE
  char* sql_mac = "DROP TABLE IF EXISTS MAC_UE;"
  "CREATE TABLE MAC_UE(tstamp INT CHECK(tstamp > 0)," 
                       "ngran_node INT CHECK(ngran_node >= 0 AND ngran_node < 9),"
                       "mcc INT,"
                       "mnc INT,"
                       "mnc_digit_len INT,"
                       "nb_id INT,"
                       "dl_aggr_tbs INT CHECK(dl_aggr_tbs  >= 0 AND dl_aggr_tbs < 4294967296),"
                       "ul_aggr_tbs INT CHECK(ul_aggr_tbs  >= 0 AND ul_aggr_tbs < 4294967296 ),"
                       "dl_aggr_bytes_sdus  INT CHECK(dl_aggr_bytes_sdus  >= 0 AND dl_aggr_bytes_sdus    < 4294967296 ),"
                       "ul_aggr_bytes_sdus INT CHECK(ul_aggr_bytes_sdus >= 0 AND ul_aggr_bytes_sdus < 4294967296 ),"
                       "pusch_snr REAL  CHECK(pusch_snr  >= 0 AND pusch_snr < 4294967296 ),"
                       "pucch_snr REAL CHECK(pucch_snr  >= 0 AND pucch_snr < 4294967296 ) ,"
                       "rnti INT  CHECK(rnti  >= 0 AND rnti < 4294967296 ),"
                       "dl_aggr_prb INT  CHECK(dl_aggr_prb >= 0 AND dl_aggr_prb < 4294967296 ),"
                       "ul_aggr_prb INT  CHECK(ul_aggr_prb >= 0 AND ul_aggr_prb < 4294967296 ),"
                       "dl_aggr_sdus INT  CHECK(dl_aggr_sdus >= 0 AND dl_aggr_sdus < 4294967296 ),"
                       "ul_aggr_sdus INT  CHECK(ul_aggr_sdus >= 0 AND ul_aggr_sdus < 4294967296 ),"
                       "dl_aggr_retx_prb  INT  CHECK(dl_aggr_retx_prb >= 0 AND dl_aggr_retx_prb < 4294967296 ),"
                       "wb_cqi INT  CHECK(wb_cqi >= 0 AND wb_cqi < 256 ),"
                       "dl_mcs1 INT  CHECK(dl_mcs1>= 0 AND dl_mcs1 < 256),"
                       "ul_mcs1  INT CHECK(ul_mcs1 >= 0 AND ul_mcs1 < 256),"
                       "dl_mcs2  INT CHECK(dl_mcs2 >= 0 AND dl_mcs2 < 256),"
                       "ul_mcs2 INT CHECK(ul_mcs2 >= 0 AND ul_mcs2 < 256),"
                       "phr INT CHECK(phr > -24 AND  phr < 41)" // −23 dB to +40 dB 
                       ");";

  create_table(db, sql_mac);
}

static
void create_rlc_bearer_table(sqlite3* db)
{
  assert(db != NULL);

  // ToDo: PRIMARY KEY UNIQUE
  char* sql_rlc = "DROP TABLE IF EXISTS RLC_bearer;"
  "CREATE TABLE RLC_bearer(tstamp INT CHECK(tstamp > 0)," 
                            "ngran_node INT CHECK(ngran_node >= 0 AND ngran_node < 9),"
                            "mcc INT,"
                            "mnc INT,"
                            "mnc_digit_len INT,"
                            "nb_id INT,"
                            "txpdu_pkts INT CHECK(txpdu_pkts >= 0 AND txpdu_pkts < 4294967296)," // 1 << 32 = 4294967296 
                            "txpdu_bytes INT CHECK(txpdu_bytes >= 0 AND  txpdu_bytes < 4294967296 ),"\
                            "txpdu_wt_ms  INT CHECK(txpdu_wt_ms  >= 0 AND  txpdu_wt_ms  < 4294967296 ),"\
                            "txpdu_dd_pkts INT CHECK(txpdu_dd_pkts >= 0 AND  txpdu_dd_pkts < 4294967296 ) ,"\
                            "txpdu_dd_bytes INT  CHECK(txpdu_dd_bytes >= 0 AND  txpdu_dd_bytes < 4294967296 ),"\
                            "txpdu_retx_pkts INT CHECK(txpdu_retx_pkts >= 0 AND  txpdu_retx_pkts < 4294967296 ) ,"\
                            "txpdu_retx_bytes INT  CHECK(txpdu_retx_bytes >= 0 AND  txpdu_retx_bytes < 4294967296 ),"\
                            "txpdu_segmented INT  CHECK(txpdu_segmented >= 0 AND  txpdu_segmented < 4294967296 ),"\
                            "txpdu_status_pkts INT  CHECK(txpdu_status_pkts >= 0 AND  txpdu_status_pkts < 4294967296 ),"\
                            "txpdu_status_bytes INT  CHECK(txpdu_status_bytes >= 0 AND  txpdu_status_bytes < 4294967296 ),"\
                            "txbuf_occ_bytes INT  CHECK(txbuf_occ_bytes >= 0 AND  txbuf_occ_bytes < 4294967296 ),"\
                            "txbuf_occ_pkts INT  CHECK(txbuf_occ_pkts >= 0 AND  txbuf_occ_pkts < 4294967296 ),"\
                            "rxpdu_pkts INT  CHECK(rxpdu_pkts >= 0 AND  rxpdu_pkts < 4294967296 ),"\
                            "rxpdu_bytes INT  CHECK(rxpdu_bytes >= 0 AND  rxpdu_bytes < 4294967296 ),"\
                            "rxpdu_dup_pkts INT CHECK(rxpdu_dup_pkts >= 0 AND  rxpdu_dup_pkts < 4294967296 ),"\
                            "rxpdu_dup_bytes INT CHECK(rxpdu_dup_bytes >= 0 AND  rxpdu_dup_bytes < 4294967296 ),"\
                            "rxpdu_dd_pkts INT CHECK(rxpdu_dd_pkts >= 0 AND  rxpdu_dd_pkts < 4294967296 ),"\
                            "rxpdu_dd_bytes INT CHECK(rxpdu_dd_bytes >= 0 AND  rxpdu_dd_bytes < 4294967296 ),"\
                            "rxpdu_ow_pkts INT CHECK(rxpdu_ow_pkts >= 0 AND  rxpdu_ow_pkts < 4294967296 ),"\
                            "rxpdu_ow_bytes INT CHECK(rxpdu_ow_bytes >= 0 AND  rxpdu_ow_bytes < 4294967296 ),"\
                            "rxpdu_status_pkts INT CHECK(rxpdu_status_pkts >= 0 AND  rxpdu_status_pkts < 4294967296 ),"\
                            "rxpdu_status_bytes INT CHECK(rxpdu_status_bytes >= 0 AND  rxpdu_status_bytes < 4294967296 ),"\
                            "rxbuf_occ_bytes INT CHECK(rxbuf_occ_bytes >= 0 AND  rxbuf_occ_bytes < 4294967296 ),"\
                            "rxbuf_occ_pkts INT CHECK(rxbuf_occ_pkts >= 0 AND  rxbuf_occ_pkts < 4294967296 ),"\
                            "txsdu_pkts INT CHECK(txsdu_pkts >= 0 AND  txsdu_pkts < 4294967296 ),"\
                            "txsdu_bytes INT CHECK(txsdu_bytes >= 0 AND  txsdu_bytes < 4294967296 ),"\
                            "rxsdu_pkts INT CHECK(rxsdu_pkts >= 0 AND  rxsdu_pkts < 4294967296 ),"\
                            "rxsdu_bytes INT CHECK(rxsdu_bytes >= 0 AND  rxsdu_bytes < 4294967296 ),"\
                            "rxsdu_dd_pkts INT CHECK(rxsdu_dd_pkts >= 0 AND  rxsdu_dd_pkts < 4294967296 ),"\
                            "rxsdu_dd_bytes INT CHECK(rxsdu_dd_bytes >= 0 AND  rxsdu_dd_bytes < 4294967296 ),"\
                            "rnti INT CHECK(rnti >= 0 AND  rnti < 4294967296 ),"\
                            "mode INT CHECK(mode >= 0 AND  mode < 3),"\
                            "rbid INT CHECK(rbid >= 0 AND  rbid < 16)"
                            ");";

  create_table(db, sql_rlc);
}

static
void create_pdcp_bearer_table(sqlite3* db)
{
  assert(db != NULL);

  // ToDo: PRIMARY KEY UNIQUE
  char* sql_pdcp = "DROP TABLE IF EXISTS PDCP_bearer;"
  "CREATE TABLE PDCP_bearer(tstamp INT CHECK(tstamp > 0)," 
                       "ngran_node INT CHECK(ngran_node >= 0 AND ngran_node < 9),"
                       "mcc INT,"
                       "mnc INT,"
                       "mnc_digit_len INT,"
                       "nb_id INT,"
                       "txpdu_pkts INT CHECK(txpdu_pkts  >= 0 AND txpdu_pkts  < 4294967296),"
                       "txpdu_bytes INT CHECK(txpdu_bytes >=0 AND txpdu_bytes < 4294967296 ),"
                       "txpdu_sn INT CHECK(txpdu_sn >=0 AND txpdu_sn < 4294967296 ),"
                       "rxpdu_pkts INT CHECK(rxpdu_pkts >=0 AND rxpdu_pkts < 4294967296 ),"
                       "rxpdu_bytes INT CHECK(rxpdu_bytes >=0 AND rxpdu_bytes < 4294967296 ),"
                       "rxpdu_sn  INT CHECK(rxpdu_sn  >= 0 AND rxpdu_sn  < 4294967296 ) ,"
                       "rxpdu_oo_pkts INT  CHECK(rxpdu_oo_pkts >= 0 AND rxpdu_oo_pkts < 4294967296 ),"
                       "rxpdu_oo_bytes INT  CHECK(rxpdu_oo_bytes >= 0 AND rxpdu_oo_bytes < 4294967296 ),"
                       "rxpdu_dd_pkts INT  CHECK(rxpdu_dd_pkts >= 0 AND rxpdu_dd_pkts < 4294967296 ),"
                       "rxpdu_dd_bytes INT  CHECK(rxpdu_dd_bytes >= 0 AND rxpdu_dd_bytes < 4294967296 ),"
                       "rxpdu_ro_count INT  CHECK(rxpdu_ro_count >= 0 AND rxpdu_ro_count < 4294967296 ),"
                       "txsdu_pkts INT  CHECK(txsdu_pkts >= 0 AND txsdu_pkts < 4294967296 ),"
                       "txsdu_bytes INT  CHECK(txsdu_bytes >= 0 AND txsdu_bytes <4294967296 ),"
                       "rxsdu_pkts INT  CHECK(rxsdu_pkts >= 0 AND rxsdu_pkts <4294967296),"
                       "rxsdu_bytes INT CHECK(rxsdu_bytes >= 0 AND rxsdu_bytes <4294967296),"
                       "rnti INT CHECK(rnti >= 0 AND rnti <4294967296),"
                       "mode INT CHECK(mode >= 0 AND mode < 4294967296),"
                       "rbid INT CHECK(rbid >= 0 AND rbid < 4294967296)" // −23 dB to +40 dB 
                       ");";
  create_table(db, sql_pdcp);
}


static
void insert_db(sqlite3* db, char* sql)
{
  assert(db != NULL);
  assert(sql != NULL);

  char* err_msg = NULL;
  int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  assert(rc == SQLITE_OK && "Error while inserting into the DB. Check the err_msg string for further info");
}


static
int to_sql_string_mac_ue(global_e2_node_id_t const* id, mac_ue_stats_impl_t* stats, int64_t tstamp, char* out, size_t out_len)
{
  assert(stats != NULL);       
  assert(out != NULL);
  const size_t max = 512;      
  assert(out_len >= max);      
  int rc = snprintf(out, max, 
      "INSERT INTO MAC_UE VALUES("
      "%ld,"//tstamp
      "%d," //ngran_node  
      "%d," //mcc
      "%d," //mnc
      "%d," //mnc_digit_len   
      "%d," //nb_id 
      "%lu,"//dl_aggr_tbs     
      "%lu,"//ul_aggr_tbs     
      "%lu," //dl_aggr_bytes_sdus   
      "%lu,"//ul_aggr_bytes_sdus       
      "%g,"// pusch_snr        
      "%g,"//  pucch_snr      
      "%u,"// rnti            
      "%u,"//dl_aggr_prb      
      "%u,"//  ul_aggr_prb    
      "%u,"//  dl_aggr_sdus   
      "%u,"//  ul_aggr_sdus   
      "%u,"// dl_aggr_retx_prb
      "%u,"// wb_cqi
      "%u,"// dl_mcs1
      "%u,"// ul_mcs1
      "%u,"// dl_mcs2         
      "%u,"// ul_mcs2         
      "%d"// phr              
      ");"
      ,tstamp                
      ,id->type
      ,id->plmn.mcc
      ,id->plmn.mnc
      ,id->plmn.mnc_digit_len
      ,id->nb_id 
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
  return rc;
}


static
int to_sql_string_rlc_rb(global_e2_node_id_t const* id,rlc_radio_bearer_stats_t* rlc, int64_t tstamp, char* out, size_t out_len)
{
  assert(rlc != NULL);
  assert(out != NULL);
  const size_t max = 1024;
  assert(out_len >= max);

  int const rc = snprintf(out, max,
        "INSERT INTO RLC_bearer VALUES("
        "%ld,"// tstamp
        "%d," //ngran_node  
        "%d," //mcc
        "%d," //mnc
        "%d," //mnc_digit_len   
        "%d," //nb_id 
        "%u," //rlc->txpdu_pkts
        "%u," //rlc->txpdu_bytes
        "%u," //rlc->txpdu_wt_ms 
        "%u," //rlc->txpdu_dd_pkts
        "%u," //rlc->txpdu_dd_bytes 
        "%u," //rlc->txpdu_retx_pkts
        "%u," //rlc->txpdu_retx_bytes
        "%u," //rlc->txpdu_segmented
        "%u," //rlc->txpdu_status_pkts
        "%u," //rlc->txpdu_status_bytes
        "%u," //rlc->txbuf_occ_bytes
        "%u," //rlc->txbuf_occ_pkts
        "%u," //rlc->rxpdu_pkts
        "%u," //rlc->rxpdu_bytes
        "%u," //rlc->rxpdu_dup_pkts
        "%u," //rlc->rxpdu_dup_bytes
        "%u," //rlc->rxpdu_dd_pkts
        "%u," //rlc->rxpdu_dd_bytes
        "%u," //rlc->rxpdu_ow_pkts
        "%u," //rlc->rxpdu_ow_bytes
        "%u," //rlc->rxpdu_status_pkts
        "%u," //rlc->rxpdu_status_bytes
        "%u," //rlc->rxbuf_occ_bytes
        "%u," //rlc->rxbuf_occ_pkts
        "%u," //rlc->txsdu_pkts
        "%u," //rlc->txsdu_bytes
        "%u," //rlc->rxsdu_pkts
        "%u," //rlc->rxsdu_bytes
        "%u," //rlc->rxsdu_dd_pkts
        "%u," //rlc->rxsdu_dd_bytes
        "%u," //rlc->rnti
        "%u,"  //rlc->mode
        "%u"  //rlc->rbid 
        ");"
        , tstamp
        , id->type
        , id->plmn.mcc
        , id->plmn.mnc
        , id->plmn.mnc_digit_len
        , id->nb_id 
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
  return rc;
}


static
int to_sql_string_pdcp_rb(global_e2_node_id_t const* id, pdcp_radio_bearer_stats_t* pdcp, int64_t tstamp, char* out, size_t out_len)
{                              
  assert(pdcp != NULL);        
  assert(out != NULL);
  const size_t max = 512;      
  assert(out_len >= max);      
  
  int rc = snprintf(out, out_len, 
      "INSERT INTO PDCP_bearer VALUES("
        "%ld," //tstamp         
        "%d," //ngran_node  
        "%d," //mcc
        "%d," //mnc
        "%d," //mnc_digit_len   
        "%d," //nb_id 
        "%u," //txpdu_pkts       
        "%u," //txpdu_bytes     
        "%u," // txpdu_sn   
        "%u," //rxpdu_pkts      
        "%u," //rxpdu_bytes     
        "%u,"     //rxpdu_sn    
        "%u," //rxpdu_oo_pkts   
        "%u," //rxpdu_oo_bytes  
        "%u,"   //rxpdu_dd_pkts 
        "%u,"  //rxpdu_dd_bytes 
        "%u," //rxpdu_ro_count  
        "%u,"//txsdu_pkts
        "%u," //txsdu_bytes     
        "%u,"//rxsdu_pkts
        "%u," //rxsdu_bytes     
        "%u," //rnti            
        "%u," //mode            
        "%u" //rbid
        ");"
        , tstamp               
        , id->type
        , id->plmn.mcc
        , id->plmn.mnc
        , id->plmn.mnc_digit_len
        , id->nb_id 
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
  return rc;
}

static
void write_mac_stats(sqlite3* db, global_e2_node_id_t const* id, mac_ind_data_t const* ind )
{
  assert(db != NULL);
  assert(ind != NULL);

  mac_ind_msg_t const* ind_msg_mac = &ind->msg; 

  char buffer[2048] = {0};
  int pos = 0;

  for(size_t i = 0; i < ind_msg_mac->len_ue_stats; ++i){
    pos += to_sql_string_mac_ue(id, &ind_msg_mac->ue_stats[i], ind_msg_mac->tstamp, buffer + pos, 2048 - pos);
  }

  insert_db(db, buffer);
}

static
void write_rlc_stats(sqlite3* db, global_e2_node_id_t const* id, rlc_ind_data_t const* ind)
{
  assert(db != NULL);
  assert(ind != NULL);

  rlc_ind_msg_t const* ind_msg_rlc = &ind->msg; 

  char buffer[2048] = {0};
  int pos = 0;

  for(size_t i = 0; i < ind_msg_rlc->len; ++i){
    pos += to_sql_string_rlc_rb(id, &ind_msg_rlc->rb[i], ind_msg_rlc->tstamp, buffer + pos, 2048 - pos);
  }

  insert_db(db, buffer);

}

static
void write_pdcp_stats(sqlite3* db, global_e2_node_id_t const* id, pdcp_ind_data_t const* ind)
{
  assert(db != NULL);
  assert(ind != NULL);

  pdcp_ind_msg_t const* ind_msg_pdcp = &ind->msg; 

  char buffer[2048] = {0};
  int pos = 0;

  for(size_t i = 0; i < ind_msg_pdcp->len; ++i){
    pos += to_sql_string_pdcp_rb(id, &ind_msg_pdcp->rb[i], ind_msg_pdcp->tstamp, buffer + pos, 2048 - pos);
  }

  insert_db(db, buffer);
}

void init_db_sqlite3(sqlite3** db, char const* db_filename)
{
  assert(db != NULL);
  assert(db_filename != NULL);

  int const rc = sqlite3_open(db_filename, db);
  assert(rc != SQLITE_CANTOPEN && "SQLITE3 cannot open the directory. Does it already exist?");
  assert(rc == SQLITE_OK && "Error while creating the DB at /tmp/db_xapp");


  // Optimizations. Write Ahead Logging
  char* err_msg = NULL;
  int const rc_2 = sqlite3_exec(*db, "pragma journal_mode=wal" , 0, 0, &err_msg);
  assert(rc_2 == SQLITE_OK && "Error while setting the wal mode in sqlite3");

  int const rc_3 = sqlite3_exec(*db, "pragma synchronous=normal" , 0, 0, &err_msg);
  assert(rc_3 == SQLITE_OK && "Error while setting the syncronous mode to normal");


  //////
  // MAC
  //////
  create_mac_ue_table(*db);

  //////
  // RLC
  //////
  create_rlc_bearer_table(*db);

  //////
  // PDCP
  //////
  create_pdcp_bearer_table(*db);
}

void close_db_sqlite3(sqlite3* db)
{
  assert(db != NULL);
  int const rc = sqlite3_close(db);
  assert(rc == SQLITE_OK && "Error while closing the DB");
}

void write_db_sqlite3(sqlite3* db, global_e2_node_id_t const* id, sm_ag_if_rd_t const* rd)
{
  assert(db != NULL);
  assert(rd != NULL);
  assert(rd->type == MAC_STATS_V0 || rd->type == RLC_STATS_V0|| rd->type == PDCP_STATS_V0);

  if(rd->type == MAC_STATS_V0){
    write_mac_stats(db, id, &rd->mac_stats);
  } else if(rd->type == RLC_STATS_V0 ){
    write_rlc_stats(db, id, &rd->rlc_stats);
  } else if( rd->type == PDCP_STATS_V0) {
    write_pdcp_stats(db, id, &rd->pdcp_stats);
  } else {
    assert(0!=0 && "Unknown statistics type received ");
  }

}

