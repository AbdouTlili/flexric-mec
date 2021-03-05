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

#include "rrc_node_conf.h"
#include "rrc_conf_reader.h"
#include "rrc_conf_json_printer.h"

namespace flexric {
namespace flexran {
namespace rib {

std::string rrc_node_conf::to_json_inner() const
{
  std::string json = "\"conf\":";
  std::lock_guard<std::mutex> guard(_lock);
  if (_msg.size() == 0)
    return json + "{}";

  flatcc_json_printer_t printer_ctx;
  flatcc_json_printer_init_dynamic_buffer(&printer_ctx, 0);

  rrc_conf_IndicationMessage_print_json_as_root(&printer_ctx, _msg.data(), _msg.size(), NULL);
  size_t buf_size;
  char *buf = (char*)flatcc_json_printer_get_buffer(&printer_ctx, &buf_size);
  json += buf;
  flatcc_json_printer_clear(&printer_ctx);

  return json;
}

}
}
}
