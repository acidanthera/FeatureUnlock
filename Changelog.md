FeatureUnlock Changelog
======================
#### v1.0.5
- Fixed AirPlay to Mac UDM patching regression

#### v1.0.4
- Fixed AirPlay to Mac support with macOS 12.1
- Refactored patch sets to only patch model families
- Resolved rare OS-side crashing from Sidecar patching
- Added AssetCache patch for `kern.hv_vmm_present` usage
- Disabled iPad/Sidecar exemption by default
  - Patch set does not work on devices running iOS 14 or newer
- Added additional boot-args for specific patch disabling

#### v1.0.3
- Rename project from SidecarFixup to FeatureUnlock

#### v1.0.2
- Added constants for macOS 12 support
- Fixed macOS 12 shared cache compatibility
- Unlock AirPlay to Mac, Universal Control and NightShift Functionality

#### v1.0.1
- Fixed excessive memory comparison
- Fixed boot argument handling on Secure Boot enabled macOS

#### v1.0.0
- Initial release
