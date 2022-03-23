#include <chrono>
#include <cstdint>
#include <optional>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
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


void parse_response(const std::string& http_data, Result& result)
{
  const RequestParams &reqpars = result.reqpars;
  try
  {
    nlohmann::json json = nlohmann::json::parse(http_data);

    for (const auto& obj : json) {
      if (!reqpars.domain.empty() && obj["payload_type"] != reqpars.domain)
         continue;

      for (const auto& prefix : reqpars.cfg.db.path)
      {
        std::filesystem::path fullpath = prefix/obj["payload_iov"][0]["payload_url"].get<std::string>();
        if (std::filesystem::exists(fullpath))
        {
          result.paths.push_back(fullpath);
          break;
        }
        else
        {
          if (reqpars.cfg.db.verbosity >= 1)
            std::cerr << "Warning: File " << fullpath << " does not exist\n";
        }
      }
    }
    // Save a copy of the first path if found
    result.payload = result.paths.size() > 0 ? result.paths[0] : "";
  }
  catch (nlohmann::json::exception& e)
  {
    std::cerr << "Error: " << e.what() << '\n';
  }
}


// Maps key(tag, timestamp as string) : http_data
using CachedResponses = std::unordered_map<std::string, std::string>;


bool from_cache(Result& result, const CachedResponses& cache)
{
  const RequestParams &reqpars = result.reqpars;
  std::string key{reqpars.tag + std::to_string(reqpars.timestamp)};

  auto cached_response = cache.find(key);

  if (cached_response == cache.end()) {
    return false;
  } else {
    const std::string &http_data = cached_response->second;
    parse_response(http_data, result);
    result.response_code = 0xC; // = 12 status code for results from cache
    return true;
  }
}


void to_cache(const std::string& http_data, Result& result, CachedResponses& cache)
{
  std::string key{result.reqpars.tag + std::to_string(result.reqpars.timestamp)};

  cache[key] = http_data;

  auto calc_total = [](size_t total, const CachedResponses::value_type& entry) { return total + entry.second.size(); };
  result.cache_size = std::accumulate(cache.begin(), cache.end(), 0, calc_total);
}


Result fetch(std::string tag, std::string domain, uint64_t timestamp, const Configurator& cfg)
{
  // Initialize response_code with 0
  Result result{ {tag, domain, timestamp, cfg}, "", {}, 0};

  static CachedResponses cached_responses;

  // Use cached server response for same parameters when requested
  if (cfg.db.use_cache && from_cache(result, cached_responses)) {
    return result;
  }

  curl_version_info_data *curlver_data = curl_version_info(CURLVERSION_NOW);
  std::string useragent{"curl/" + std::string(curlver_data->version)};

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();

  if (curl) {
    std::ostringstream url;
    url << cfg.db.url() << "/payloadiovs/?gtName=" << tag << "&majorIOV=0&minorIOV=" << timestamp;

    if (cfg.db.verbosity >= 2)
      std::cerr << "Info: " << url.str() << '\n';

    std::string http_data;

    curl_easy_setopt(curl, CURLOPT_URL, url.str().c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &http_data);

    int retries = cfg.db.retry_times + 1; // retry_times can be 0, so run it at least once
    result.total_retries = -1;
    int delay = 0;

    while (result.response_code != 200 && retries > 0)
    {
      http_data.clear();
      std::this_thread::sleep_for(std::chrono::seconds(delay));

      CURLcode res = curl_easy_perform(curl);
      result.total_retries++;

      if (res != CURLE_OK) {
        std::cerr << "Error: curl_easy_perform() failed: " << curl_easy_strerror(res) << '\n';
      }

      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &result.response_code);

      delay *= 2;
      retries--;

      if (delay < 1) delay = 1;
      if (delay > cfg.db.retry_max_delay) delay = cfg.db.retry_max_delay;

      if (cfg.db.verbosity >= 1 && result.response_code != 200 && retries > 0)
        std::cerr << "Warning: HTTP response code " << result.response_code << ", retry in " << delay << " seconds, " << retries << " more attempts" << '\n';
    }

    curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &result.byte_count);
    curl_easy_cleanup(curl);

    if (result.response_code == 200) {
      parse_response(http_data, result);

      if (cfg.db.use_cache) {
        to_cache(http_data, result, cached_responses);
      }
    }
  }

  curl_global_cleanup();

  return result;
}

}
