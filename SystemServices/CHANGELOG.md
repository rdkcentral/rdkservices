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

## [1.7.4] - 2023-01-04
### Fixed
- Fix for getPlatformConfiguration api returning empty deviceType

## [2.0.1] - 2024-01-02
### Security
- resolved security vulnerabilities

## [2.0.0] - 2023-11-17
### Removed
- Removed the hardcoded implementation of hasRebootBeenRequested

## [1.7.2] - 2023-10-18
### Fixed
- Fix for onFirmwareUpdateStateChange events with the same data.

## [1.7.1] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.7.0] - 2023-08-31
### Changed
- Changed references to deviceInitiatedFWDnld.sh to swupdate_utility.sh

## [1.6.0] - 2023-07-05
### Added
- Added API to get and set friendly name of the device

## [1.5.2] - 2023-07-03
### Added
- Added In getSystemVersions response the version value taken from the BRANCH value instead of using the existing value from VERSION

## [1.5.1] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.5.0] - 2023-05-17
### Added 
- Set Linux local time based on Timezone support

## [1.4.3] - 2023-05-11
### Fixed
- Move IARMMgrs patches to generic layer

## [1.4.2] - 2023-04-25
### Fixed
- Added the condition check while reading territory from file

## [1.4.1] - 2023-04-12
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.4.0] - 2023-04-13
### Added
- Added new API's uploadLogsAsync and abortLogUpload.
- Added listener for power state changes to call uploadLogsAsync and abortLogUpload.

## [1.3.0] - 2023-04-03
### Added
- Added new API getWakeupSrcConfiguration and updated setWakeupSrcConfiuration API to use across all platforms.

## [1.2.1] - 2023-03-28
### Fixed
- Fix for onTimeZoneDSTChanged events with the same data.

## [1.2.0] - 2023-03-3
### Added
- Added "accuracy" property to getTimeZoneDST/setTimeZoneDST calls
- Added "oldAccuracy" and "newAccuracy" properties to onTimeZoneDSTChanged event

## [1.1.11] - 2023-01-24
### Fixed
- Added setting of "TZ" env variable to setTimeZoneDST call.

## [1.1.10] - 2023-01-13
### Fixed
- Fix for setTimeZoneDST API to avoid invalid input 

## [1.1.9] - 2023-01-10
### Fixed
- Fix one global definition

## [1.1.8] - 2022-11-28
### Fixed
- Treat warnings as errors for unit tests workflow

## [1.1.7] - 2022-10-21
### Fixed
- Fix uploadLogs API "(HTTP) 405 Method Not Allowed"

## [1.1.6] - 2022-10-18
### Changed
- API access on all versions of the handler
- Remove initialization steps from constructor

## [1.1.5] - 2022-10-14
### Changed
- Updated changes for new power manager.

## [1.1.4] - 2022-10-11
### Deprecated
- Deprecated below API's
- cacheContains, getCachedValue, removeCacheKey, setCachedValue, getMilestones, isGzEnabled & setGzEnabled

## [1.0.4] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.3] - 2022-09-07
### Fixed
- Warnings fix

## [1.0.2] - 2022-08-24
### Fixed
- Update SystemServices to call container_setup.sh script after deleting persistent data

## [1.0.1]
### Fixed
- Optimize include

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
