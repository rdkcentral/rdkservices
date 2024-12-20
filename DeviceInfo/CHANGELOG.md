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
## [1.0.15] - 2024-09-16
### Changed
- DELIA-65827:  Return Empty SerialNumber in curl for "method": "DeviceInfo.1.systeminfo"

## [1.0.14] - 2024-08-02
### Changed
- RDKTV-31447: Sky CP added in manufacturer list

## [1.0.13] - 2024-05-31
### Changed
- RDK-45345: Upgrade Sky Glass devices to use Thunder R4.4.1

## [1.0.12] - 2023-01-26
### Changed
- RDK-44991: Upgrade Flex-2.0 devices to use Thunder R4.4.1

## [1.0.11] - 2023-10-20
### Fixed
- Verify that deviceType property is functional

## [1.0.10] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.0.9] - 2023-07-13
### Fixed
- Updated correct header file inclusion of DeviceInfo to support R4 on RDK

## [1.0.8] - 2023-08-31
### Fixed 
- Added valid values (hybrid/mediaclient) for devicetype property of DeviceInfo plugin for RPI

## [1.0.7] - 2023-08-28
### Fixed 
- Added valid values for modelid and make property of DeviceInfo plugin for RPI

## [1.0.6] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.0.5]
### Fixed
- Move IARMMgrs patches to generic layer

## [1.0.4]
### Fixed
- Duplicates in SupportedVideoDisplays

## [1.0.3]
### Fixed
- Split interfaces
- Query implementations via IShell::Root
- Revisit source of data: prefer files over RFC, prefer MFR over RFC

## [1.0.2]
### Fixed
- Missing std:: before vector, min, and numeric_limits
- Do not require RFC and DS
- Enable exception handling

## [1.0.1]
### Changed
- Get default Audio port name from DS

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
