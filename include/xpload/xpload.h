#pragma once

#include <cstdint>
#include <optional>

#include <curl/curl.h>
#include <json/json.h>


namespace xpload {

std::optional<std::string> fetch(uint64_t timestamp)
{
  CURL *curl;
  CURLcode result;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();

  if (curl) {
    // First set the URL that is about to receive our POST. This URL can just as
    // well be a https:// URL if that is what should receive the data.
    curl_easy_setopt(curl, CURLOPT_URL, "http://postit.example.com/moo.cgi");

    // Now specify the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");

    // Perform the request, result will get the return code
    result = curl_easy_perform(curl);

    if (result != CURLE_OK)
      std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(result) << '\n';

    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();

  return {};
}

}
