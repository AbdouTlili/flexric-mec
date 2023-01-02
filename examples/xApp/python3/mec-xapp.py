import sqlite3
import json
import pika
import xapp_sdk as ric
import time
import os
import pdb

kpi_list = ['cqi','rsrp','mcs_ul','mcs_dl','phr','bler_ul','bler_dl','errors_dl','errors_ul','data_ul','data_dl','throughput','snr','amf_ue_ngap_id']


################### northbound part of the xApp ###################### 

# function to parse raw kpis to json object
def json_payload(kpi,timestamp,value,ue_id,ue_tag=0,slice_id=None,source="RAN"):

    a = { "kpi": kpi,
         "slice_id": slice_id,
         "source": source,
         "timestamp": timestamp,
         "unit": "None",
         "value": value,
         "labels": [
             {"ue_id": ue_id},
             {"ue_tag":ue_tag}
             ] 
        }
    return json.dumps(a)


def northbound():
    #establising the connection with the broker
    connection = pika.BlockingConnection(pika.ConnectionParameters('localhost'))
    channel = connection.channel()
    channel.queue_declare(queue='xapp-queue')



    # creating sqlite3 connection 
    con = sqlite3.connect('/home/netsoft/oai/flexric/db/xapp_db_met')

    # creating sql cursor 
    cur = con.cursor()

    # fetching for sql tables in the ric db
    res = cur.execute("SELECT tstamp, wb_cqi, ul_mcs1,dl_mcs1,dl_mcs2,phr,ul_bler,dl_bler,ul_aggr_bytes_sdus,dl_aggr_bytes_sdus, pucch_snr FROM MAC_UE ORDER BY tstamp DESC LIMIT 10")

    for tstamp, wb_cqi, ul_mcs1,dl_mcs1,dl_mcs2,phr,ul_bler,dl_bler,ul_aggr_bytes_sdus,dl_aggr_bytes_sdus, pucch_snr in res.fetchall():
        print(dl_bler,'--',tstamp)
        amf_ue_ngap_id = dl_mcs2;
        tmp_payload_list = []
        
        # creating the json object for the KPI 
        tmp_payload_list.append(json_payload(kpi='cqi',timestamp=tstamp,value=wb_cqi,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='rsrp',timestamp=tstamp,value=0,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='mcs_ul',timestamp=tstamp,value=ul_mcs1,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='mcs_dl',timestamp=tstamp,value=dl_mcs1,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='phr',timestamp=tstamp,value=phr,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='bler_ul',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='bler_dl',timestamp=tstamp,value=dl_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='errors_dl',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='errors_ul',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='data_ul',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='data_dl',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='throughput',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        tmp_payload_list.append(json_payload(kpi='snr',timestamp=tstamp,value=ul_bler,ue_id=amf_ue_ngap_id,ue_tag=0,slice_id=None,source="RAN"))

        # sending the json payloads to the broker 
        print("sending json payloads to the broker")
        for payload in tmp_payload_list:
            channel.basic_publish(exchange='',routing_key='xapp-queue',body=payload)

        
        
    #closing the db
    print("closing db")
    con.close()

    # closing the connection with the broker
    print("closing connection with the broker")
    connection.close()


################### southbound part of the xApp ###################### 

####################
#### MAC INDICATION CALLBACK
####################

#  MACCallback class is defined and derived from C++ class mac_cb
class MACCallback(ric.mac_cb):
    # Define Python class 'constructor'
    def __init__(self):
        # Call C++ base class constructor
        ric.mac_cb.__init__(self)
    # Override C++ method: virtual void handle(swig_mac_ind_msg_t a) = 0;
    def handle(self, ind):
        # Print swig_mac_ind_msg_t
        if len(ind.ue_stats) > 0:
            # t_now = time.time_ns() / 1000.0
            # t_mac = ind.tstamp / 1.0
            # t_diff = t_now - t_mac
            print('MAC Indication tstamp = ' )
            # print('MAC rnti = ' + str(ind.ue_stats[0].rnti))
            
def southbound():

    ric.init()

    conn = ric.conn_e2_nodes()
    assert(len(conn) > 0)
    for i in range(0, len(conn)):
        print("Global E2 Node [" + str(i) + "]: PLMN MCC = " + str(conn[i].id.plmn.mcc))
        print("Global E2 Node [" + str(i) + "]: PLMN MNC = " + str(conn[i].id.plmn.mnc))

    ####################
    #### MAC INDICATION
    ####################

    mac_hndlr = []
    for i in range(0, len(conn)):
        mac_cb = MACCallback()
        hndlr = ric.report_mac_sm(conn[i].id, ric.Interval_ms_10, mac_cb)
        mac_hndlr.append(hndlr)     
        time.sleep(1)
        
    time.sleep(10)
    
    for i in range(0, len(mac_hndlr)):
        ric.rm_report_mac_sm(mac_hndlr[i])
        
    while ric.try_stop == 0:
        time.sleep(1)
        
        
def main():
    try:
        southbound()
    except:
        print('Error while executing the southbound')
        
    try: 
        northbound()
    except:
        print('Error while executing the southbound')
        
if __name__ == "__main__":
    main()

    
    