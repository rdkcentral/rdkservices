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

## [1.0.14] - 2025-07-2025
### Fixed
- Updated to use pthread_cond_wait insteadof pthread_cond_timedwait

## [1.0.13] - 2024-09-03
### Fixed
- Updated to handle unhandled exceptions

## [1.0.12] - 2023-09-12
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

##[1.0.11] - 2023-07-19
### Added
-Adding UtilsSynchro.hpp to provide tools for automatically synchronizing API calls for given plugin
-Adding UtilsSynchroIarm.hpp to additionally synchronize iarm event handlers.

## [1.0.10] - 2023-04-20
### Fixed
- Fixed CEC crash by initializing status variables inside the ctor.

## [1.0.9] - 2023-04-12
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.0.8] - 2023-04-11
### Fixed
- Updated Logging, and initialize function to be more verbose for testing purposes

## [1.0.7] - 2023-03-20
### Fixed
- Fixed thread handling issue in CECDisable.

* For more details, refer to [versioning](https://github.com/rdkcentral/rdkservices#versioning) section under Main README.

## [1.0.6] - 2023-02-09
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.0.5] - 2022-12-15
### Fixed
- Treat warnings as errors for unit tests workflow

## [1.0.4] - 2022-12-09
### Fixed
- Self ping removed

## [1.0.3] - 2022-11-22
### Fixed
- RAII thread midifications

## [1.0.2] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.1]
### Fixed
- Optimize include

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
