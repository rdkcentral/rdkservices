# Changelog

All notable changes to this RDK Service will be documented in this file.

* Each RDK Service has a CHANGELOG file that contains all changes done so far. When version is updated, add a entry in the CHANGELOG.md at the top with user friendly information on what was changed with the new version. Please don't mention JIRA tickets in CHANGELOG. 

* Please Add entry in the CHANGELOG for each version change and indicate the type of change with these labels:
    * **Added** for new features.
    * **Changed** for changes in existing functionality.
    * **Deprecated** for soon-to-be removed features.
    * **Removed** for now removed features.
    * **Fixed** for any bug fixes.
    * **Security** in case of vulnerabilities.

* Changes in CHANGELOG should be updated when commits are added to the main or release branches. There should be one CHANGELOG entry per JIRA Ticket. This is not enforced on sprint branches since there could be multiple changes for the same JIRA ticket during development. 

* For more details, refer to [versioning](https://github.com/rdkcentral/rdkservices#versioning) section under Main README.

## [1.0.7] - 2024-03-21
### Added
- Account scope implementation
- L0, L2 unit tests
- Out-of-process

## [1.0.6] - 2024-03-11
### Added
- RPC COM interface IStore2, IStoreInspector, IStoreLimit
- JSON RPC API getStorageSizes, getNamespaceStorageLimit, setNamespaceStorageLimit
- "scope" parameter (defaults to "device")
- "ttl" parameter
- Configuration "limit" parameter
- L0, L1 unit tests
- Thunder master branch support
- Out-of-process support

## [1.0.5] - 2024-01-02
### Security
- resolved security vulnerabilities

## [1.0.4] - 2023-12-13
### Fixed
- Increased persistent key storage value to 3k in config.in for Thunder R4

## [1.0.3] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.0.2] - 2022-12-08
### Fixed
- Remove the old workaround for an empty config

## [1.0.1] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
