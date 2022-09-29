#include <iostream>
#include <sstream>
#include <string>

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "xpload/configurator.h"


namespace xpload {

std::size_t save_data(const char* input, std::size_t chunk_size, std::size_t n_chunks, std::string* out)
{
  const std::size_t total_bytes(chunk_size * n_chunks);
  out->append(input, total_bytes);
  return total_bytes;
}


nlohmann::json send(std::string request, std::string endpoint, std::string jsonpars, const Configurator& cfg)
{
  nlohmann::json result;

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  if (!curl) {
    std::cerr << "Error: curl initialization failed\n";
    curl_global_cleanup();
    return result;
  }

  std::ostringstream url;
  url << cfg.db.url() << "/" << endpoint;

  if (cfg.db.verbosity >= 2)
    std::cerr << "Info: " << url.str() << " - " << jsonpars << '\n';

  curl_version_info_data *curlver_data = curl_version_info(CURLVERSION_NOW);
  std::string useragent{"curl/" + std::string(curlver_data->version)};

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers, "Content-Type: application/json");

  std::string http_data;

  curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonpars.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_data);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.c_str());

  CURLcode res = curl_easy_perform(curl);

  if (res != CURLE_OK) {
    std::cerr << "Error: curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
  }

  curl_easy_cleanup(curl);
  curl_slist_free_all(headers);
  curl_global_cleanup();

  try
  {
    result = nlohmann::json::parse(http_data);
  }
  catch (nlohmann::json::exception& e)
  {
    std::cerr << "Error: " << e.what() << '\n';
  }

  return result;
}


nlohmann::json _get_data(std::string endpoint, std::string jsonpars, const Configurator& cfg)
{
  return send("GET", endpoint, jsonpars, cfg);
}


nlohmann::json _post_data(std::string endpoint, std::string jsonpars, const Configurator& cfg)
{
  return send("POST", endpoint, jsonpars, cfg);
}


nlohmann::json _put_data(std::string endpoint, std::string jsonpars, const Configurator& cfg)
{
  return send("PUT", endpoint, jsonpars, cfg);
}

}
