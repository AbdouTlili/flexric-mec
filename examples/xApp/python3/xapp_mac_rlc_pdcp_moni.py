import xapp_sdk as ric
import time
import os
import pdb

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
            t_now = time.time_ns() / 1000.0
            t_mac = ind.tstamp / 1.0
            t_diff = t_now - t_mac
            print('MAC Indication tstamp = ' + str(t_mac) + ' diff = ' + str(t_diff))
            # print('MAC rnti = ' + str(ind.ue_stats[0].rnti))

####################
#### RLC INDICATION CALLBACK
####################

class RLCCallback(ric.rlc_cb):
    # Define Python class 'constructor'
    def __init__(self):
        # Call C++ base class constructor
        ric.rlc_cb.__init__(self)
    # Override C++ method: virtual void handle(swig_rlc_ind_msg_t a) = 0;
    def handle(self, ind):
        # Print swig_rlc_ind_msg_t
        if len(ind.rb_stats) > 0:
            t_now = time.time_ns() / 1000.0
            t_rlc = ind.tstamp / 1.0
            t_diff = t_now - t_rlc
            print('RLC Indication tstamp = ' + str(ind.tstamp) + ' diff = ' + str(t_diff))
            # print('RLC rnti = '+ str(ind.rb_stats[0].rnti))

####################
#### PDCP INDICATION CALLBACK
####################

class PDCPCallback(ric.pdcp_cb):
    # Define Python class 'constructor'
    def __init__(self):
        # Call C++ base class constructor
        ric.pdcp_cb.__init__(self)
   # Override C++ method: virtual void handle(swig_pdcp_ind_msg_t a) = 0;
    def handle(self, ind):
        # Print swig_pdcp_ind_msg_t
        if len(ind.rb_stats) > 0:
            t_now = time.time_ns() / 1000.0
            t_pdcp = ind.tstamp / 1.0
            t_diff = t_now - t_pdcp
            print('PDCP Indication tstamp = ' + str(ind.tstamp) + ' diff = ' + str(t_diff))
            # print('PDCP rnti = '+ str(ind.rb_stats[0].rnti))


####################
####  GENERAL 
####################

ric.init()

conn = ric.conn_e2_nodes()
assert(len(conn) > 0)
for i in range(0, len(conn)):
    print("Global E2 Node [" + str(i) + "]: PLMN MCC = " + str(conn[i].id.plmn.mcc))
    print("Global E2 Node [" + str(i) + "]: PLMN MNC = " + str(conn[i].id.plmn.mnc))

####################
#### MAC INDICATION
####################

for i in range(0, len(conn)):
    mac_cb = MACCallback()
    ric.report_mac_sm(conn[i].id, ric.Interval_ms_1, mac_cb)
    time.sleep(1)

####################
#### RLC INDICATION
####################

for i in range(0, len(conn)):
    rlc_cb = RLCCallback()
    ric.report_rlc_sm(conn[i].id, ric.Interval_ms_1, rlc_cb)
    time.sleep(1)

####################
#### PDCP INDICATION
####################

for i in range(0, len(conn)):
    pdcp_cb = PDCPCallback()
    ric.report_pdcp_sm(conn[i].id, ric.Interval_ms_1, pdcp_cb)
    time.sleep(1)

time.sleep(10)

### End

ric.rm_report_mac_sm()

ric.rm_report_rlc_sm()

ric.rm_report_pdcp_sm()

# Avoid deadlock. ToDo revise architecture 
while ric.try_stop == 0:
    time.sleep(1)

print("Test finished")
