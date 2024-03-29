# Changelog

## [0.8.2] - 2022-08-30

* Use cache in docker build push action by @plexoos in https://github.com/BNLNPPS/xpload/pull/33
* Enhance docker build and GitHub action workflow by @plexoos in https://github.com/BNLNPPS/xpload/pull/31
* Fixes and enhancements to the build process by @plexoos in https://github.com/BNLNPPS/xpload/pull/34
* Fix compiler warnings and redefine as errors by @plexoos in https://github.com/BNLNPPS/xpload/pull/35
* Link to std::filesystem if necessary, based on compiler by @plexoos in https://github.com/BNLNPPS/xpload/pull/37
* README: Fix build status badge by @plexoos in https://github.com/BNLNPPS/xpload/pull/30
* Refactor CLI xpl by @plexoos in https://github.com/BNLNPPS/xpload/pull/40
* xpl: Switch API end point to get list of all tags by @plexoos in https://github.com/BNLNPPS/xpload/pull/39
* test: Update condor parameters + fix CSV header by @plexoos in https://github.com/BNLNPPS/xpload/pull/44
* README: Fix option name in example by @plexoos in https://github.com/BNLNPPS/xpload/pull/43
* config: Introduce dry_run option by @plexoos in https://github.com/BNLNPPS/xpload/pull/45
* fetch: Do not limit the number of returned payload paths by @plexoos in https://github.com/BNLNPPS/xpload/pull/46


## [0.8.1] - 2022-04-25

* Explicitly link standard c++ library (filesystem) by @plexoos in https://github.com/BNLNPPS/xpload/pull/26
* Make python code compatible with 3.8 version by @plexoos in https://github.com/BNLNPPS/xpload/pull/27
* Introduce container build arguments for compiler and python version by @plexoos in
  https://github.com/BNLNPPS/xpload/pull/28


## [0.8.0] - 2022-04-20

- Introduced add and push actions into xpl CLI tool
  - Removed deprecated insert action
- Improved existing and added new tests for CLI and the library
- Cleaned and restructured available configurations
  - More strict search rules for configuration files
  - Switched to production settings by default
- Registered project with zenodo https://zenodo.org/badge/latestdoi/419745626

* config: Update domain to reflect the change introduced by openshift admins by @plexoos in https://github.com/BNLNPPS/xpload/pull/17
* fetch: Add the first payload path to returned results by @plexoos in https://github.com/BNLNPPS/xpload/pull/18
* Constrain search for configuration file by environment variable by @plexoos in https://github.com/BNLNPPS/xpload/pull/19
* config: Throw runtime exceptions in Configurator constructor by @plexoos in https://github.com/BNLNPPS/xpload/pull/21
* Support search for payload files in multiple prefixes by @plexoos in https://github.com/BNLNPPS/xpload/pull/23


## [0.7.0] - 2022-02-14

* docker: Switch from CentOS 8 to RockyLinux 8.5 by @plexoos in https://github.com/BNLNPPS/xpload/pull/8
* Introduce caching capabilities when fetching results by @plexoos in https://github.com/BNLNPPS/xpload/pull/6
* Implement retries for unsuccessful fetching attempts by @plexoos in https://github.com/BNLNPPS/xpload/pull/10

- config: Rename environment variable `s/XPLOAD_CONFIG_NAME/XPLOAD_CONFIG/`
- config: Improve error handling while parsing json file
- Various improvements and enhancements of the tests


## [0.6.0] - 2022-01-12

- Added a test to simulate real jobs with multiple calls fetching payload
  information from the database
- The primary API function xpload::fetch() now accepts another parameter
  `domain` in order to select payloads belonging to that specific type
  - xpload::fetch() now returns a struct which can be further extended to return
    other useful information
  - Example and tests were updated accordingly
- New configuration options added for verbosity and cached calls


## [0.5.0] - 2021-12-01

- CLI tool xpl is now installed by CMake
- Example updated to accept command line options
- Documentation updated


## [0.4.0] - 2021-11-24

- Update xpload.py to match changed API in plexoos/npdb:develop repository
- Add basic tests for xpload.py to CI workflow
- Configuration settings are redefined for prod, dev, and test environments
- Documentation updated


## [0.3.0] - 2021-11-02

- Add xpload.py tool to push new and fetch existing entries to and from database


## [0.2.1] - 2021-10-26

- Update documentation and the project installation tree


## [0.2.0] - 2021-10-25

- Integrate nlohmann/json library v3.10.4 for JSON parsing
  - Get rid of jsoncpp as external dependency
- Update CI to build and run the Django web app from plexoos/npdb fork
- Introduce a basic test with CTest and include into CI


## [0.1.0] - 2021-10-21

Initial implementation with an example illustrating the basic usage

- Provides library target exports for seamless integration with other CMake
  projects
