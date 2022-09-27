#include <string>

#include <nlohmann/json.hpp>

#include "xpload/configurator.h"
#include "xpload/push.h"
#include "xpload/requests.h"


namespace xpload {

void copy_payload(std::filesystem::path payload_file, std::vector<std::filesystem::path> prefixes, std::string domain)
{
  namespace fs = std::filesystem;

  // XXX Verify prefixes
  fs::path prefix = prefixes[0];
  fs::path output_file = prefix/domain/payload_file.filename();

  fs::create_directories(output_file.parent_path());
  fs::copy_file(payload_file, output_file);
}


void push(Payload payload, const Configurator& cfg)
{
  _post_data("gttype", "{\"name\": \"" + payload.tag_type + "\"}", cfg);
  _post_data("gtstatus", "{\"name\": \"" + payload.tag_status + "\"}", cfg);
  _post_data("gt", "{\"name\": \"" + payload.tag + "\", \"status\": \"" + payload.tag_status + "\", \"type\": \"" + payload.tag_type + "\"}", cfg);
  _post_data("pt", "{\"name\": \"" + payload.domain + "\"}", cfg);

  nlohmann::json result = _get_data("gtPayloadLists/" + payload.tag, "{}", cfg);

  std::string pill{""};

  if (result[payload.domain].is_null())
  {
    pill = _post_data("pl", "{\"payload_type\": \"" + payload.domain + "\"}", cfg)["name"];
  }
  else
  {
    pill = result[payload.domain];
  }

  _put_data("pl_attach", "{\"global_tag\": \"" + payload.tag + "\", \"payload_list\": \"" + pill + "\"}", cfg);

  copy_payload(payload.file, cfg.db.path, payload.domain);

  result = _post_data("piov", "{\"payload_url\": \"" + payload.file.filename().string() + "\", \"minor_iov\": " + std::to_string(payload.ts_min) + ", \"minor_iov_end\": " + std::to_string(payload.ts_max) + ", \"major_iov\": 0}", cfg);

  _put_data("piov_attach", "{\"piov_id\": " + std::to_string(result["id"].get<long>()) + ", \"payload_list\": \"" + pill + "\"}", cfg);
}

}
