# Changelog

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
