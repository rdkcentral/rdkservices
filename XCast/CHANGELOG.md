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
## [1.0.14] - 2023-07-19
### Fixed
- XCast limit payload to avoid overflow

## [1.0.13] - 2023-07-05
### Fixed
- Fixed code to use Sssystem Plugin API for  get and set friendly name of the device.

## [1.0.12] - 2023-04-11
### Fixed
- Removed CJson dependency from XCast plugin.
- Updated Logging, and initialize function to be more verbose for testing purposes

## [1.0.11] - 2023-03-16
### Fixed
- Missed state added to documentation

## [1.0.10] - 2023-03-05
### Changed
- control initial state/setup of the XCast plugin via compilation flags

## [1.0.9] - 2023-03-02
### Fixed
- XCast: updated documentation

## [1.0.8] - 2023-02-09
### Fixed
- XCast: retry reconnection till success

## [1.0.7] - 2023-01-20
### Fixed
- Fixed Default return values

## [1.0.6] - 2023-01-20
### Fixed
- Fixed Linking with cjson

## [1.0.5] - 2022-12-18
### Fixed
- Fixed power change event processing to thread 

## [1.0.4] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.3] - 2022-09-29
### Fixed
- unregisterApplications API Implementation

## [1.0.2]
### Fixed
- Optimize include

## [1.0.1] - 2022-08-16
### Fixed
- Added fix to stop retry timer when distruct xcast plugin.

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
