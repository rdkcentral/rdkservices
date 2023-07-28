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

## [1.1.16] - 2023-07-25
### Added
- Initial support for WPEWebKit 2.38

## [1.1.15] - 2023-04-11
### Fixed
- Change clock type to monotonic based

## [1.1.14] - 2023-03-27
### Fixed
- Check load percentage before triggering LoadFinished notification

## [1.1.13] - 2023-03-22
### Fixed
- Fixed Worker thread callback crash after browser plugin deactivation

## [1.1.12] - 2023-03-08
### Fixed
- Undefined symbols on qemu x86 platform

## [1.1.11] - 2023-03-05
### Fixed
- LoadFailed event is not triggerring with the URL(failed to load by the browser)

## [1.1.10] - 2023-03-03
### Added
- Config entry to enable WebAudio

## [1.1.9] - 2023-03-01
### Fixed
- Verify source frame of response before reading http status code

## [1.1.8] - 2023-02-27
### Added
- Option to rename WPEWebProcess

## [1.1.7] - 2023-02-17
###Fixed
- Apply config localstorageenabled value for GLIB API

## [1.1.6] - 2023-02-13
### Changed
- Added an option to specify different Cairo GL compositor for
  ResidentApp and SearchAndDiscoveryApp callsigns

## [1.1.5] - 2023-02-10
### Changed
- Cloud cookie jar support
- Option to select different Cairo GL Compositor a build time
- Update TZ from file (/opt/persistent/timeZoneDST)
- Option to redirect logs to a specific journald unit

## [1.1.4] - 2023-02-09
### Removed
- Log enhancement, avoid flooding of BridgeQuery logs

## [1.1.3] - 2022-11-01
### Changed
- Added log enhancement

## [1.1.2] - 2022-10-14
### Changed
- Added JSPP callsign

## [1.1.1] - 2022-09-14
### Changed
- Disabled Page Cache

## [1.1.0] - 2022-09-01
### Added
- Support for GLib based APIs (wpe-2.28)
- Added options for
    * Local Storage Quota
    * WebInspector Server
    * IndexedDB (location, quota)
    * Spatial Navigation
    * Window closure through scripts
    * Universal access
- Configuration to build with libsoup3
- Provision to pass user scripts
- Log enhancement

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
