All notable changes to this RDK Service will be documented in this file.

    Each RDK Service has a CHANGELOG file that contains all changes done so far. When version is updated, add a entry in the CHANGELOG.md at the top with user friendly information on what was changed with the new version. Please don't mention JIRA tickets in CHANGELOG.

    Please Add entry in the CHANGELOG for each version change and indicate the type of change with these labels:
        Added for new features.
        Changed for changes in existing functionality.
        Deprecated for soon-to-be removed features.
        Removed for now removed features.
        Fixed for any bug fixes.
        Security in case of vulnerabilities.

    Changes in CHANGELOG should be updated when commits are added to the main or release branches. There should be one CHANGELOG entry per JIRA Ticket. This is not enforced on sprint branches since there could be multiple changes for the same JIRA ticket during development.

    For more details, refer to versioning section under Main README.
## [1.0.5] - 2024-04-04
### Fixed
- Added the synchronization for enable/disable the MiracastService discovery.
- Added the check to avoid the P2P events when MiracastService discovery not enabled.

## [1.0.4] - 2024-03-06
### Fixed
- Added the timer to get and update the friendlyName for Miracast if it fails get it from SystemServices.

## [1.0.3] - 2023-12-22
### Added
- Added the opt flags to suppress P2P events and change config methods and dynamically handled the authType.

## [1.0.2] - 2023-12-16
### Added
- Fix for RDKTV-26356.
- Resolve racing condition for wlan and p2p active sacn. Change P2P scan from active to passive scanning

## [1.0.1] - 2023-11-22
### Added
- Enabled the necessary logs in both MiracastService and MiracastPlayer plugins
- Fixed the Device goes Offline issue
- Fixed the ENT-32203 Error issue
- Implemented the first-frame-video-callback to fix the couple of secs Blank observed while streaming

## [1.0.0] - 2023-11-01
### Added
- New RDK Service MiracastService and MiracastPlayer to manage screen mirrorning with the device via WiFi-Direct
