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



## [2.1.0] - 2025-02-18
### Updated
- Added accessibility settings to user settings plugin.

## [2.0.0] - 2025-01-08
### Updated
- Updated input param names for pinControl, liveWatershed, playbackWatershed, blockNotRatedContent, pinOnPurchase properties.

## [1.2.0] - 2024-09-17
### Added
- Added ParentalControl new properties in Usersetttings.

## [1.1.2] - 2024-08-26
### Fixed
- Updated api documentation according to interface.

## [1.1.1] - 2024-07-08
### Fixed
- UserSettings Plugin missing default values updated.

## [1.1.0] - 2024-06-02
### Added
- Added GetPrivacyMode/GetPrivacyMode calls.

## [1.0.0] - 2024-05-08
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
