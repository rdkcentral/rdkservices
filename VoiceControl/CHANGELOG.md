# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

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