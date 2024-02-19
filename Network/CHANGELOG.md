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

## [1.3.5] - 2024-02-19
### Fixed
- Fixed Network connnectivity monitor crash during reboot

## [1.3.4] - 2024-01-05
### Added
- Post Internet Status Event upon interface connect and disconnect event

## [1.3.3] - 2024-01-02
### Security
- resolved security vulnerabilities

## [1.3.2] - 2023-12-21
### Changed
- Changed eventHandler function to public

## [1.3.1] - 2023-11-30
### Fixed
- Addressed the IPv4 & IPv6 cache issue where it was not suppose to be cached when DNS or other parameter is not received yet

## [1.3.0] - 2023-11-25
### Added 
- added connectivity monitor class remove Connectivity related IARM Calls

## [1.2.3] - 2023-10-23
### Fixed
- Fixed the isConnectedToInternet method to not to call getPublicIP; as they are independent RPCs.

## [1.2.2] - 2023-09-29
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.2.1] - 2023-09-20
### Fixed
- Fixed multiple concurrent isConnectedToInternet() requests by using the cache

## [1.2.0] - 2023-08-16
### Added
- Added new params in isConnectedToInternet for Query Internet Connectivity specific to Ipversion

## [1.1.1] - 2023-08-16
### Fixed
- Remove the Cache Logic for getInterface Network Plugin api

## [1.1.0] - 2023-08-16
### Added
- Added new method to configure PNI settings

## [1.0.10] - 2023-06-21
### Added
- Added setting of Internet SubSystem to isConnectedToInternet call

## [1.0.9] - 2023-06-13
### Added
- Added Captive Portal Detection and its access methods.

## [1.0.8] - 2023-04-12
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.0.7] - 2023-04-04
### Fixed
- Fixed traceroute command error.

## [1.0.6] - 2022-12-13
### Fixed
- Fixed empty IP Address value being returned.

## [1.0.5] - 2022-12-13
### Added
- Added Network plugin unitTest cases.

## [1.0.4] - 2022-10-21
### Fixed
- pingoutput text files that are not deleted in failure cases are now deleted.

## [1.0.3] - 2022-10-18
### Changed
- API access on all versions of the handler


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
