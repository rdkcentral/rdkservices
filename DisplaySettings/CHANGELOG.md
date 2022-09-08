# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.8] - 2022-09-08
### Fixed
- Use Controller via COM-RPC instead of JSON-RPC

##[1.0.7] - 2022-09-08
### Fixed
- Fixed No audio from AVR issue in case of delayed HPD scenarios

##[1.0.6] - 2022-09-01
### Changed
- Fixed DisplaySettings dtor crash in LinkType

##[1.0.5] - 2022-09-01
### Changed
- Fixed DisplaySettings::sendHdmiCecSinkAudioDevicePowerOn fprintf.c crash

##[1.0.4] - 2022-08-31
### Changed
- Changed functionality to route audio to AVR on HDMI CEC off incase of eArc devices as well.


## [1.0.3] - 2022-08-25
### Fixed
- Fixed std:: Displaysettings side JSONRPC::LinkType crash fix

## [1.0.1] - 2022-08-18
### Fixed
- RDKTV-18555 Device locked up for 2 minutes after turning BUILT-IN SPEAKER
  setting OFF while using input

## [1.0.0] - 2022-05-11
### Added
- Add CHANGELOG

### Change
- Reset API version to 1.0.0
- Change README to inform how to update changelog and API version
