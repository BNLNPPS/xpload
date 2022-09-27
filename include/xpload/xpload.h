#pragma once

#include "xpload/configurator.h"
#include "xpload/fetch.h"


namespace xpload {

/**
 * Appends `chunk_size`*`n_chunks` characters from the `input` buffer to the
 * output string `out`
 *
 * Complies with the signature of a "write function" as defined by libcurl:
 *
 *     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, save_data);
 */
std::size_t save_data(const char* input, std::size_t chunk_size, std::size_t n_chunks, std::string* out);

}
