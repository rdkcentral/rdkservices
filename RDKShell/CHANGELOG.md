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

## [1.6.7] - 2025-02-27
### Fixed
- Increase Hibernation delay to 30sec and get from RFC

## [1.6.6] - 2025-02-06
### Fixed
- Fix RDKShell.kill for hibernated app

## [1.6.4] - 2024-12-16
### Added
- Added proper synchronization in in RDKShell::subscribeForSystemEvent()

## [1.6.3] - 2024-09-09
### Added
- Added for response is getting empty on launching DAC application

## [1.6.2] - 2024-08-26
### Added
- Added Delay of Auto Hibernation for Native Apps: 15sec for L2S, 5sec for others

## [1.6.1] - 2024-07-17
### Fixed
- Fix for Retry to get wake up keycode when Unknown

## [1.6.0] - 2024-06-12
### Added
- Added Upstream onApplicationFocusChanged event change handling

## [1.5.3] - 2024-06-10
### Changed
- ResidentApp plugin unable to activate due to unable to get the Initialize/activating Notification from Thunder(R4.4.1)

## [1.5.2] - 2024-06-04
### Changed
- Added support for dobby amazon container

## [1.5.1] - 2024-05-31
### Changed
- RDK-45345: Upgrade Sky Glass devices to use Thunder R4.4.1

## [1.5.0] - 2024-05-29
### Added
- Added support for setKeyIntercepts

## [1.4.20] - 2024-05-29
### Fixed
- Activation of RA in RDKShell and setting the autostart to false

## [1.4.19] - 2024-05-22
### Added
- RDKShell must claim its ownership of Graphics subsystem to Thunder

## [1.4.18] - 2024-05-13
### Fixed
- Disable Auto Hibernation for Launch to Suspended

## [1.4.17] - 2024-04-10
### Added
- Fix for rdkshell missing events

## [1.4.16] - 2024-03-13
### Addded
- Added long key press support

## [1.4.15] - 2024-03-05
### Fixed
- Fix RDKShell Initialize and Deinitialize

## [1.4.15] - 2024-02-28
### Fixed
- Fix for delayed init in RDKShell

## [1.4.14] - 2024-02-22
### Fixed
- Fix RDKShell deadlock on gSuspendedOrHibernatedApplicationMutex

## [1.4.13] - 2024-02-06
### Fixed
- Fixed coverity reported issues

## [1.4.12] - 2023-12-11
### Added
- RDKShell changes for dual ODM FTA

## [1.4.11] - 2023-12-04
### Added
- Added Hibernate and Restore api and Auto Hibernation for suspended native apps

## [1.4.11] - 2023-11-17
### Added
- onApplicationLaunched event and proper error messages are not triggering for DAC apps

## [1.4.10] - 2023-10-23
### Added
- RDKShell changes for ignoring factoryMode

## [1.4.9] - 2023-10-19
### Added
- RDKShell changes to use DeviceModeSelector Plugin

## [1.4.8] - 2023-10-18
### Added
- RDKShell changes to use DeviceModeSelector Plugin to launch factoryapp and easteregg sequence parsing

## [1.4.7] - 2023-10-04
### Added
- Implement Thunder Plugin Configuration for Kirkstone builds(CMake-3.20 & above)

## [1.4.6] - 2023-09-22
### Fixed
- Updating getEthernetMac usage as per factory-hal code review recommendation

## [1.4.5] - 2023-08-29
### Fixed
- Handled Deactivation Event that is specific to Thunder 4.2

## [1.4.4] - 2023-08-14
### Fixed
- Reverted the RDKSEC-289 WebProcess crashed while launching WebKitBrowser with "type": "HtmlApp" changes.

## [1.4.3] - 2023-07-12
### Fixed
- Gracefull exit on failures

## [1.4.2] - 2023-06-23
### Fixed
- subscribed to systemservices event earlier

## [1.4.1] - 2023-06-21
### Fixed
- Fixed the ResidentApp failed to activate & crash in Thunder R4

## [1.4.0] - 2023-06-07
### Modified
-Rialto support for RDKShell

## [1.3.14] - 2023-06-13
### Fixed
- Fixed the crash while launching WebKitBrowser with "type": "HtmlApp" in container mode.

## [1.3.13] - 2023-06-05
### Added 
- Added Support to build the plugin to both R4 & R2

## [1.3.12] - 2023-04-11
### Added
-added unitest for rdkshell plugin

## [1.3.11] - 2023-01-25
### Fixed
- Flush cache after setting value to Persistent storage

## [1.3.10] - 2022-12-27
### Fixed
- Prioritize graphics subsystem event early

## [1.3.9] - 2022-11-30
### Fixed
- Fix RA launch in factory mode

## [1.3.8] - 2022-11-22
### Fixed
- Focus was setting true to RA while launching FTA

## [1.3.7] - 2022-11-08
### Fixed
- Use interfaces instead of JSON-RPC

## [1.3.6] - 2022-10-27
### Added
- Added support for LISA to launch Application

## [1.3.5] - 2022-10-27
### Fixed
- Fixed bug to trigger events

## [1.3.4] - 2022-10-26
### Fixed
- Fixed bug to use rdkshell_display when surface mode is enabled.

## [1.3.3] - 2022-10-04
### Fixed
- Fixed warnings that are treated as errors with "-Wall -Werror"

## [1.3.2] - 2022-09-16
### Fixed
- Leak

## [1.3.1] - 2022-09-07
### Fixed
- Use Controller via COM-RPC instead of JSON-RPC

## [1.3.0] - 2022-09-08
### Added
- Added setAVBlocked api.
- Added getBlockedAVApplications api.

## [1.2.2]
### Changed
- launch webkitbrowser plugin in container mode

## [1.2.1]
### Fixed
- Optimize include

## [1.2.0] - 2022-08-12
### Added
- Added setGraphicsFrameRate api.
- Added getGraphicsFrameRate api.

## [1.1.0] - 2022-08-12
### Added
- Added keyRepeatConfig method.

## [1.0.1] - 2022-08-11
### Fixed
- Fixed crash during Plugin deactivation.

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
