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


##[1.4.2] - 2023-12-15
###Changed
- Added support for handling the Audio Mixing, Video plane Handling, Zorder handling for Cherry Use Case

## [1.4.1] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.4.0] - 2023-09-29
### Added
- Added Support for Enabling/Disabling ALLM and get the current ALLM Status

## [1.3.2] - 2023-09-06
### Fixed
- portID should accept both number and string as a valid portID

## [1.3.1] - 2023-08-18
### Added
- Added missing Return Response in AVInput thunder plugin

## [1.3.0] - 2023-08-04
### Added
- Added Event for AV Content Type update

## [1.2.2] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.2.1] - 2023-04-12
### Fixed
- Fixed IARM_Bus_UnRegisterEventHandler  call to IARM_Bus_RemoveEventHandler

## [1.2.0] - 2023-04-03
### Added
- Typo in RPC response to AVInput call org.rdk.AVInput.getSupportedGameFeatures

## [1.1.0] - 2022-10-10
### Added
- Combine HdmiInput and CompositeInput Plugins into AVInput

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
