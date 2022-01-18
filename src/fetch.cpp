#include <cstdint>
#include <optional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "xpload/configurator.h"
#include "xpload/fetch.h"


namespace xpload {


std::size_t save_data(const char* input, std::size_t chunk_size, std::size_t n_chunks, std::string* out)
{
  const std::size_t total_bytes(chunk_size * n_chunks);
  out->append(input, total_bytes);
  return total_bytes;
}


Result fetch(std::string tag, std::string domain, uint64_t timestamp, const Configurator& cfg)
{
  Result result;

  curl_version_info_data *curlver_data = curl_version_info(CURLVERSION_NOW);
  std::string useragent{"curl/" + std::string(curlver_data->version)};

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  if (curl)
  {
    std::ostringstream url;
    url << cfg.db.url() << "/payloadiovs/?gtName=" << tag << "&majorIOV=0&minorIOV=" << timestamp;

    if (cfg.db.verbosity > 0)
      std::cout << "url: " << url.str() << '\n';

    std::string http_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_data);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';

    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &result.byte_count);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.response_code);

    curl_easy_cleanup(curl);

    if (result.response_code != CURLE_HTTP_RETURNED_ERROR)
    {
      try
      {
        nlohmann::json json = nlohmann::json::parse(http_data);

        for (const auto& obj : json) {
          if (!domain.empty() && obj["domain"] != domain)
             continue;
          result.paths.push_back(cfg.db.path + '/' + obj["payloads"][0]["name"].get<std::string>());
        }
      }
      catch (nlohmann::json::exception& e)
      {
        std::cerr << "Error: " << e.what() << '\n';
      }
    }
  }

  curl_global_cleanup();

  return result;
}

}
