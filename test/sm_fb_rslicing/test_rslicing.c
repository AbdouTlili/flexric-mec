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

#include <assert.h>
#include <stdio.h>

#include "rslicing_msg_dec.h"
#include "rslicing_msg_enc.h"

#include "rslicing_reader.h"
#include "rslicing_verifier.h"
#include "rslicing_builder.h"
#include "rslicing_json_printer.h"
#include "rslicing_json_parser.h"

void test_event_trigger()
{
  const mac_rslicing_ReportOccasion_enum_t occ = mac_rslicing_ReportOccasion_onChange;
  byte_array_t ba = mac_rslicing_encode_event_trigger(occ);
  assert(ba.buf && ba.len > 0);
  const mac_rslicing_ReportOccasion_enum_t ret = mac_rslicing_decode_event_trigger(ba);
  assert(ret == occ);
  free(ba.buf);
}

void test_indication_header()
{
  byte_array_t ba = mac_rslicing_encode_empty_indication_header();
  assert(ba.buf && ba.len > 0);
  free(ba.buf);
}

void create_dummy_slice_indication_message(struct flatcc_builder* B)
{
  mac_rslicing_IndicationMessage_start(B);

  mac_rslicing_IndicationMessage_sliceConfig_start(B);
    mac_rslicing_SliceConfig_dl_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_NVS);
      mac_rslicing_UlDlSliceConfig_slices_start(B);

      mac_rslicing_NvsRate_ref_t nvsrate = mac_rslicing_NvsRate_create(B, 5.0, 10.0);
      mac_rslicing_NvsSliceConfig_union_ref_t unvsrate = mac_rslicing_NvsSliceConfig_as_rate(nvsrate);
      mac_rslicing_NvsSlice_ref_t nvs1 = mac_rslicing_NvsSlice_create(B, unvsrate);
      mac_rslicing_SliceParams_union_ref_t p1 = mac_rslicing_SliceParams_as_nvs(nvs1);
      flatbuffers_string_ref_t label = flatbuffers_string_create_str(B, "label");
      flatbuffers_string_ref_t scheduler = flatbuffers_string_create_str(B, "scheduler");
      //mac_rslicing_UlDlSliceConfig_slices_push_create(B, 1, label, scheduler, p1);
      mac_rslicing_Slice_vec_push_start(B);
      // in order of mac_rslicing_Slice_create()
      mac_rslicing_Slice_id_force_add(B, 1);
      mac_rslicing_Slice_label_add(B, label);
      mac_rslicing_Slice_scheduler_add(B, scheduler);
      mac_rslicing_Slice_params_add(B, p1);
      mac_rslicing_Slice_vec_push_end(B);

      mac_rslicing_NvsCapacity_ref_t nvscap = mac_rslicing_NvsCapacity_create(B, 0.5);
      mac_rslicing_NvsSliceConfig_union_ref_t unvscap = mac_rslicing_NvsSliceConfig_as_capacity(nvscap);
      mac_rslicing_NvsSlice_ref_t nvs2 = mac_rslicing_NvsSlice_create(B, unvscap);
      mac_rslicing_SliceParams_union_ref_t p2 = mac_rslicing_SliceParams_as_nvs(nvs2);
      mac_rslicing_UlDlSliceConfig_slices_push_create(B, 2, label, scheduler, p2);

      mac_rslicing_StaticSlice_ref_t stat = mac_rslicing_StaticSlice_create(B, 5, 8);
      mac_rslicing_SliceParams_union_ref_t p3 = mac_rslicing_SliceParams_as_static(stat);
      mac_rslicing_UlDlSliceConfig_slices_push_create(B, 3, label, scheduler, p3);

      mac_rslicing_UlDlSliceConfig_slices_end(B);
    mac_rslicing_SliceConfig_dl_end(B);

    mac_rslicing_SliceConfig_ul_start(B);
      mac_rslicing_UlDlSliceConfig_algorithm_force_add(B, mac_rslicing_SliceAlgorithm_None);
      flatbuffers_string_ref_t sched = flatbuffers_string_create_str(B, "pf");
      mac_rslicing_UlDlSliceConfig_scheduler_add(B, sched);
    mac_rslicing_SliceConfig_ul_end(B);
  mac_rslicing_IndicationMessage_sliceConfig_end(B);

  mac_rslicing_IndicationMessage_ueSliceConfig_start(B);
  mac_rslicing_UeSliceConfig_ues_start(B);
  for (size_t i = 1; i < 3; ++i) {
    const uint32_t rnti = 0x1234 * i;
    const uint32_t dlslice = i;
    const uint32_t ulslice = i;
    mac_rslicing_UeSliceConfig_ues_push_create(B, rnti, dlslice, ulslice);
    // above or for individual fields as below:
    //mac_rslicing_UeSliceAssoc_start(B);
    //mac_rslicing_UeSliceAssoc_rnti_force_add(B, rnti);
    //mac_rslicing_UeSliceConfig_ues_push(B, mac_rslicing_UeSliceAssoc_end(B));
  }
  mac_rslicing_UeSliceConfig_ues_end(B);
  mac_rslicing_IndicationMessage_ueSliceConfig_end(B);
  mac_rslicing_IndicationMessage_end_as_root(B);
}

void test_indication_message()
{
  byte_array_t ba = mac_rslicing_encode_indication_message(create_dummy_slice_indication_message);
  assert(ba.buf && ba.len > 0);

  int ret;
  if ((ret = mac_rslicing_IndicationMessage_verify_as_root(ba.buf, ba.len))) {
    printf("IndicationMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  /* we are ok if JSON prints for the moment. TODO: make full test */
  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_IndicationMessage_print_json_as_root(&printer_ctx, ba.buf, ba.len, NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s", buf);
  flatcc_json_printer_clear(&printer_ctx);

  free(ba.buf);
}

void test_add_mod_slice_command_manual()
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_SliceConfig_start(B);
  mac_rslicing_SliceConfig_ref_t sc = mac_rslicing_SliceConfig_end(B);
  mac_rslicing_AddModSliceCommand_ref_t amsc = mac_rslicing_AddModSliceCommand_create(B, sc);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_addModSlice(amsc);

  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_ControlMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *pbuf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s", pbuf);
  flatcc_json_printer_clear(&printer_ctx);

  free(buf);
}

rslicing_rc_t read_add_mod_slice_dummy_test(mac_rslicing_AddModSliceCommand_table_t t)
{
  assert(t);
  mac_rslicing_SliceConfig_table_t sc = mac_rslicing_AddModSliceCommand_sliceConfig(t);
  mac_rslicing_UlDlSliceConfig_table_t dl = mac_rslicing_SliceConfig_dl(sc);
  assert(mac_rslicing_UlDlSliceConfig_algorithm(dl) == mac_rslicing_SliceAlgorithm_NVS);
  mac_rslicing_Slice_vec_t slices = mac_rslicing_UlDlSliceConfig_slices(dl);
  size_t slices_len = mac_rslicing_Slice_vec_len(slices);
  for (size_t i = 0; i < slices_len; ++i) {
    mac_rslicing_Slice_table_t slice = mac_rslicing_Slice_vec_at(slices, i);
    assert(mac_rslicing_Slice_id(slice) == i + 1);

    if (i == 0)
      assert(strcmp(mac_rslicing_Slice_label(slice), "label") == 0);
    mac_rslicing_SliceParams_union_t p = mac_rslicing_Slice_params_union(slice);
    assert(p.type == mac_rslicing_SliceParams_nvs);
    assert(p.type != mac_rslicing_SliceParams_static);
    assert(p.type != mac_rslicing_SliceParams_edf);
    //printf("test slice ID %d\n", mac_rslicing_Slice_id(slice));
    mac_rslicing_NvsSlice_table_t nvs = p.value;

    flatcc_json_printer_t printer_ctx;
    flatcc_json_printer_init(&printer_ctx, stdout);
    printer_ctx.indent = 2;
    flatcc_json_printer_table_descriptor_t td;
    td.count = 0;
    td.ttl = FLATCC_JSON_PRINT_MAX_LEVELS;
    td.table = nvs;
    td.vtable = (uint8_t *) nvs - __flatbuffers_soffset_read_from_pe(nvs);
    td.vsize = __flatbuffers_voffset_read_from_pe(td.vtable);
    //printf("{");
    //mac_rslicing_NvsSlice_print_json_table(&printer_ctx, &td);
    flatcc_json_printer_clear(&printer_ctx);
    //printf("}\n");

    mac_rslicing_NvsSliceConfig_union_t unvs = mac_rslicing_NvsSlice_config_union(nvs);
    if (unvs.type == mac_rslicing_NvsSliceConfig_rate) {
      mac_rslicing_NvsRate_table_t rate = unvs.value;
      //printf("rate mbpsRequired %f mbpsReference %f\n",
      //       mac_rslicing_NvsRate_mbpsRequired(rate),
      //       mac_rslicing_NvsRate_mbpsReference(rate));
    } else {
      assert(unvs.type == mac_rslicing_NvsSliceConfig_capacity);
      mac_rslicing_NvsCapacity_table_t cap = unvs.value;
      //printf("capacity pctReserved %f\n",
      //       mac_rslicing_NvsCapacity_pctReserved(cap));
    }
  }
  return (rslicing_rc_t) { .success = true, .error_msg = NULL };
}

void test_add_mod_slice_command()
{

  const char* cmd =
  //"{"
    //"\"sliceConfig\":"
      "{"
        "\"dl\": {"
          "\"algorithm\": \"NVS\","
          "\"slices\": [{"
              "\"id\": 1,"
              "\"label\": \"label\","
              "\"scheduler\": \"scheduler\","
              "\"params_type\": \"nvs\","
              "\"params\": {"
                "\"config_type\": \"rate\","
                "\"config\": {"
                  "\"mbpsRequired\": 5,"
                  "\"mbpsReference\": 10"
                "}"
              "}"
            "},{"
              "\"id\": 2,"
              "\"label\": \"lab\","
              "\"scheduler\": \"sched\","
              "\"params_type\": \"nvs\","
              "\"params\": {"
                "\"config_type\": \"rate\","
                "\"config\": {"
                  "\"mbpsRequired\": 3,"
                  "\"mbpsReference\": 7"
                "}"
              "}"
            "}"
          "]"
        "},"
        "\"ul\": {"
          "\"algorithm\": \"None\","
          "\"scheduler\": \"pf\""
        "}"
      "}"
    //"}"
    ;


  //printf("%s\n", cmd);

  // cf. monster_test_parse_json()

  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  flatcc_json_parser_t ctx;
  flatcc_builder_ref_t root;
  int flags = 0;

  //int rc = flatcc_builder_start_buffer(B, NULL, 0, 0);
  //assert(rc == 0);
  //mac_rslicing_ControlMessage_start(B);
  //mac_rslicing_ControlMessage_command_start(B);
  //mac_rslicing_Command_addModSlice_start(B);

  flatcc_json_parser_init(&ctx, B, cmd, cmd + strlen(cmd), flags);
  const char* end = mac_rslicing_SliceConfig_parse_json_table(&ctx, cmd, cmd + strlen(cmd), &root);
  if(end < cmd + strlen(cmd)) {
    printf("error: superfluous symbols at end: '%s'\n", end);
    flatcc_builder_clear(B);
    return;
  }
  if (ctx.error) {
    printf("error at line %d column %d: %s\n", ctx.line, ctx.pos, flatcc_json_parser_error_string(ctx.error));
    flatcc_builder_clear(B);
    return;
  }
  mac_rslicing_AddModSliceCommand_ref_t amsc = mac_rslicing_AddModSliceCommand_create(B, root);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_addModSlice(amsc);

  mac_rslicing_ControlMessage_create_as_root(B, u);

  //mac_rslicing_IndicationMessage_sliceConfig_end(B);
  //mac_rslicing_Command_addModSlice_end(B);
  //mac_rslicing_ControlMessage_command_end(B);
  //mac_rslicing_ControlMessage_end_as_root(B);

  //rc = flatcc_builder_end_buffer(B, root);
  //assert(rc != 0); // yes, it really seems to be the opposite of flatcc_builder_start_buffer()
  //ctx.end_loc = cmd;

  size_t size;
  uint8_t* buf = flatcc_builder_finalize_buffer(B, &size);
  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_ControlMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *pbuf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s", pbuf);
  flatcc_json_printer_clear(&printer_ctx);

  mac_rslicing_ControlMessage_table_t cm = mac_rslicing_ControlMessage_as_root(buf);
  mac_rslicing_ControlCommand_union_t cc = mac_rslicing_ControlMessage_command_union(cm);
  assert(cc.type == mac_rslicing_ControlCommand_addModSlice);

  read_add_mod_slice_dummy_test(cc.value);

  free(buf);
}

void test_del_slice_command_manual()
{
  flatcc_builder_t builder;
  flatcc_builder_t* B = &builder;
  flatcc_builder_init(B);

  mac_rslicing_DelSliceCommand_start(B);
  mac_rslicing_DelSliceCommand_delSliceConfig_start(B);
  mac_rslicing_DelSliceConfig_dl_start(B);
  mac_rslicing_DelSliceConfig_dl_push_create(B, 1);
  mac_rslicing_DelSliceConfig_dl_push_create(B, 2);
  mac_rslicing_DelSliceConfig_dl_end(B);
  mac_rslicing_DelSliceCommand_delSliceConfig_end(B);
  mac_rslicing_DelSliceCommand_ref_t dsc = mac_rslicing_DelSliceCommand_end(B);
  mac_rslicing_ControlCommand_union_ref_t u = mac_rslicing_ControlCommand_as_delSlice(dsc);

  mac_rslicing_ControlMessage_create_as_root(B, u);

  size_t size;
  uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
  byte_array_t ba = { .buf = buf, .len = size };

  flatcc_builder_clear(B);

  int ret;
  if ((ret = mac_rslicing_ControlMessage_verify_as_root(buf, size))) {
    printf("ControlMessage is invalid: %s\n", flatcc_verify_error_string(ret));
    abort();
  }

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);
  printer_ctx.indent = 2;
  mac_rslicing_ControlMessage_print_json_as_root(&printer_ctx, buf, size, NULL);
  size_t buf_size;
  char *pbuf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  //printf("%s", pbuf);
  flatcc_json_printer_clear(&printer_ctx);

  free(buf);
}

void test_control_header()
{
  byte_array_t ba = mac_rslicing_encode_empty_control_header();
  assert(ba.buf && ba.len > 0);
  free(ba.buf);
}

void test_control_outcome()
{
  const char* outcome = "OUTCOME";
  byte_array_t ba = mac_rslicing_encode_control_outcome(outcome);
  assert(ba.buf && ba.len > 0);
  char* ret = mac_rslicing_decode_control_outcome(ba);
  assert(strcmp(outcome, ret) == 0);
  free(ba.buf);
  free(ret);
}

void test_call_process_id()
{
  const uint32_t id = 1337;
  byte_array_t ba = mac_rslicing_encode_call_process_id(id);
  assert(ba.buf && ba.len > 0);
  uint32_t ret = mac_rslicing_decode_call_process_id(ba);
  assert(id == ret);
  free(ba.buf);
}

void test_ran_function()
{
  const size_t num = 2;
  mac_rslicing_SliceAlgorithm_enum_t sa[num];
  sa[0] = mac_rslicing_SliceAlgorithm_NVS;
  sa[1] = mac_rslicing_SliceAlgorithm_EDF;
  byte_array_t ba = mac_rslicing_encode_ran_function(sa, num);
  assert(ba.buf && ba.len > 0);
  mac_rslicing_SliceAlgorithm_enum_t* ret;
  const size_t ret_num = mac_rslicing_decode_ran_function(ba, &ret);
  assert(ret_num == num);
  for (size_t i = 0; i < num; ++i)
    assert(sa[i] == ret[i]);
  free(ba.buf);
  free(ret);
}

int main()
{
  test_event_trigger();
  test_indication_header();
  test_indication_message();
  test_control_header();
  test_add_mod_slice_command();
  test_add_mod_slice_command_manual();
  test_del_slice_command_manual();
  test_control_outcome();
  test_call_process_id();
  test_ran_function();
}
