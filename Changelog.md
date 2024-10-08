FeatureUnlock Changelog
======================
### v1.1.7
- Fixed loading on macOS 10.10 and older due to a MacKernelSDK regression

### v1.1.6
- Added constants for macOS 15 support

### v1.1.5
- Added constants for macOS 14 support

### v1.1.4
- Added Continuity Camera unlocking for pre-Kaby Lake CPUIDs

### v1.1.3
- Allow binaries to be patched upon re-paging
  - Resolves patches disappearing for small binaries after prolonged period (ex. Control Center.app)

### v1.1.2
- Added AirPlay to Mac unlocking inside Control Center.app
  - Applicable for systems with `kern.hv_vmm_present` set to `1` in macOS Ventura

### v1.1.1
- Resolved Macmini8,1 patch regression from 1.1.0
- Resolved iPad Sidecar patch regression from 1.1.0
  - Applicable if host model did not require Sidecar patch

### v1.1.0
- Refactored model patch set detection
  - Implemented proper VMM detection for AirPlay to Mac on Ventura
  - Avoids unnessary patching on supported models (ex. NightShift on 2012+)
- Removed unused `-disable_uni_control` boot argument
  - Was non-functional previously, use `-disable_sidecar_mac` instead

### v1.0.9
- Added constants for macOS 13 support
- Added AirPlay to Mac unlocking for systems with `kern.hv_vmm_present` set to `1` in Ventura

### v1.0.8
- Add AirPlay to Mac patching for Macmini8,1

### v1.0.7
- Add force patching for Universal Control via `-force_uni_control`
  - Required for when a device in the chain is unsupported, as all parties perform support checks before pairing
- Update AirPlay to Mac patchset for 12.3 Beta 2

### v1.0.6
- Removed AssetCache Patch
  - Superseded by [RestrictEvents's `-revasset`](https://github.com/acidanthera/RestrictEvents)
- Renamed MODULE_SHORT to "fu_fix"
- Lowered OS requirement to macOS Sierra
  - NightShift introduced in 10.12.4
- Improved OS performance during patching
- Added Universal Control unlocking for unsupported Macs
  - Re-introduced by Apple with macOS 12.3
  - iPadOS has iPad specific blacklists, patching is ineffective

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
