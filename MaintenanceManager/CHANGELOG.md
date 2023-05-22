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

## [1.0.19] - 2023-05-23
### Added
- Implementation to get the partner provisioning context from whoAmI

## [1.0.18] - 2023-05-16
### Fixed
- Updated unsolicited_maintenance completed status as true in stopMaintenance()

## [1.0.17] - 2023-05-11
### Fixed
- Updated maintenance tasks names

## [1.0.16] - 2023-04-11
### Fixed
- Software update start and stop works through single access point, swupdate_utility.sh

## [1.0.15] - 2023-02-15
### Fixed
- Fixed race condition in startMaintenance()

## [1.0.14] - 2023-02-13
### Fixed
- rdkvfwupgrader support during stop maintenance

## [1.0.13] - 2023-01-30
### Fixed
- Allow to set OptOut value when maintenance is in progress

## [1.0.12] - 2023-01-18
### Fixed
- Fixed deadlock in Maintenance Manager code

## [1.0.11] - 2023-01-09
### Fixed
- Fixed locking issues in iarmEventHandler

## [1.0.10] - 2022-12-08
### Fixed
- Send MAINTENANCE_ERROR event from stopMaintenance()

## [1.0.9] - 2022-11-11
### Fixed
- Fixed race condition in MM while checking for network connectivity"

## [1.0.8] - 2022-11-08
### Fixed
- Use interfaces instead of JSON-RPC

## [1.0.7] - 2022-10-29
### Changed
- Send maintenance mode using IARM event to download manager and added new readRFC api for reading rfc value

## [1.0.6] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.0.5] - 2022-09-16
### Fixed
- Missing security token

## [1.0.4] - 2022-09-07
### Fixed
- Use Controller via COM-RPC instead of JSON-RPC

## [1.0.3]
### Fixed
- Optimize include

## [1.0.2] - 2022-08-12
### Fixed
- Fixed std::terminate crash in Maintenance Manager with startMaintenance API.

## [1.0.1] - 2022-08-12
### Fixed
- Fixed std::terminate crash in Maintenance Manager with stopMaintenance API.

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
