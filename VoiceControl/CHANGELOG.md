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

## [1.4.1] - 2023-06-01
### Fixed
- Added optional audio file name parameter to voice session request API

## [1.4.0] - 2022-06-01
### Added
- Updated API version to 1.4.0
- Added voiceSessionAudioStart API

## [1.3.1] - 2023-04-10
### Fixed
- Fixed warnings treated as errors for unit tests workflow

## [1.3.0] - 2022-10-06
### Added
- Updated API version to 1.3.0
- Updated to call voiceStatus to get mask pii on initialization instead of calling getMaskPii

## [1.2.0] - 2022-10-04
### Added
- Updated API version to 1.2.0
- Added call to get mask pii on initialization

## [1.1.0] - 2022-08-18
### Added
- Updated API version to 1.1.0
- Added voiceSessionTypes, voiceSessionRequest, and voiceSessionTerminate APIs
- Added aows:// and sdt:// protocols to the URL schemes
- Added urlMicTap device to indicate a virtual microphone tap

### Deprecated
- Deprecate voiceSessionByText API. Instead, use voiceSessionRequest with type "ptt_transcription".

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
