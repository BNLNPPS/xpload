#pragma once

#include <nlohmann/json.hpp>

#include "xpload/configurator.h"
#include "xpload/fetch.h"
#include "xpload/push.h"


namespace xpload {

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DbConfig, host, port, apiroot, apiver, path, use_cache, dry_run, verbosity, retry_times, retry_max_delay);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Configurator, name, db);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(RequestParams, tag, domain, timestamp);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Result, payload, paths, response_code, byte_count, cache_size, total_retries);
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Payload, file, domain, tag, tag_type, tag_status, ts_min, ts_max);

}
