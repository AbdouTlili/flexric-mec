#!/bin/python3

import os
import atexit
from colorama import Fore, Style

def run_test(program):
    ret = os.system(program)
    if ret == 0:
        print(Fore.GREEN + f"test {program} run successfully" + Style.RESET_ALL)
        return (1,1)
    else:
        exitcode = ret >> 8
        signal = ret & 0xff
        print(Fore.RED + f"test {program} FAILED with exit code {exitcode} signal {signal}" + Style.RESET_ALL)
        return (0,1)

def run_test_if_exists(program):
    if os.path.isfile(program):
        print(f"found test {program}")
        return run_test(program)
    print(Fore.YELLOW + f"did not find test {program}" + Style.RESET_ALL)
    return (0,0)

results = []

results += [run_test_if_exists("./test_e2ap_msg")]
results += [run_test_if_exists("./e2ap_encode_asn")]
results += [run_test_if_exists("./e2ap_enc_dec_asn")]
results += [run_test_if_exists("./e2ap_enc_dec_fb")]
results += [run_test("./sm_asn1_hw/test_e2sm_hw")]
# we don't run ./e2sm_hw_agent as this requires the RIC to work
results += [run_test("./test_ran_mgmt")]
results += [run_test("./test_cb_list")]
results += [run_test("./test_sub_mgmt")]
results += [run_test("./test_hash_table")]
results += [run_test_if_exists("./sm_fb_hw/test_e2sm_hw_fb")]
results += [run_test_if_exists("./sm_fb_mac_stats/test_mac_stats")]
results += [run_test_if_exists("./sm_fb_pdcp_stats/test_pdcp_stats")]
results += [run_test_if_exists("./sm_fb_rlc_stats/test_rlc_stats")]
results += [run_test_if_exists("./sm_fb_rrc_stats/test_rrc_stats")]
results += [run_test_if_exists("./sm_fb_rrc_conf/test_rrc_conf")]
results += [run_test_if_exists("./sm_fb_rrc_event/test_rrc_event")]
results += [run_test_if_exists("./sm_fb_rrc_event/e2e_rrc_event")]
results += [run_test_if_exists("./sm_fb_rslicing/test_rslicing")]
results += [run_test("./e2ap_test")]

# sums elementwise (x,y) in results
result = tuple(map(sum, zip(*results)))
if result[0] == result[1]:
    print(Fore.GREEN, end="")
else:
    print(Fore.RED, end="")
print(f"result: {result[0]} out of {result[1]} tests ran successfully" + Style.RESET_ALL)
