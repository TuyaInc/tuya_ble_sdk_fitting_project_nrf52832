
# Change Log

## [1.2.2] - 2020-04-23
### Added
- Added on-demand connection interface.
- Added the interface to get the total size, space and number of events of the scheduling queue under no OS environment.
- Added configuration item to configure whether to use the MAC address of tuya authorization information.
- Added tuya app log interface.

### Changed
- Changed the production test code structure, and remove the application layer header file to obtain the application version number, fingerprint and other information methods.
- Changed the encryption method of ble channel used in production test to support unencrypted transmission.

### Fixed
- Fix a problem that caused a compilation error in the gcc environment.


