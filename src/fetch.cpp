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


std::vector<std::string> fetch(std::string tag, uint64_t timestamp, const Configurator& cfg)
{
  curl_version_info_data *curlver_data = curl_version_info(CURLVERSION_NOW);
  std::string useragent{"curl/" + std::string(curlver_data->version)};

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  if (curl)
  {
    std::ostringstream url;
    url << cfg.db.url() << "/payloadiovs/?gtName=" << tag << "&majorIOV=0&minorIOV=" << timestamp;

    std::cout << "url: " << url.str() << '\n';

    long http_code(0);
    std::string http_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_data);

    CURLcode result = curl_easy_perform(curl);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (result != CURLE_OK)
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(result) << '\n';

    curl_easy_cleanup(curl);

    if (http_code != CURLE_HTTP_RETURNED_ERROR)
    {
      try
      {
        nlohmann::json json = nlohmann::json::parse(http_data);

        std::vector<std::string> paths;

        for (const auto& j : json)
          paths.push_back(cfg.db.path + '/' + j["payload_iov"][0]["payload_url"].get<std::string>());

        return paths;
      }
      catch (nlohmann::json::exception& e)
      {
        std::cerr << "Error: " << e.what() << '\n';
        return {};
      }

    } else {
      return {};
    }
  }

  curl_global_cleanup();

  return {};
}


std::vector<std::string> fetch(std::string tag, uint64_t timestamp)
{
  Configurator cfg;
  return fetch(tag, timestamp, cfg);
}

}
