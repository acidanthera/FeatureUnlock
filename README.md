FeatureUnlock
==============

[![Build Status](https://github.com/acidanthera/FeatureUnlock/workflows/CI/badge.svg?branch=master)](https://github.com/acidanthera/FeatureUnlock/actions) [![Scan Status](https://scan.coverity.com/projects/23354/badge.svg?flat=1)](https://scan.coverity.com/projects/23354)

[Lilu](https://github.com/acidanthera/Lilu) Kernel extension for enabling:
* [Sidecar](https://support.apple.com/en-ca/HT210380)
* [NightShift](https://support.apple.com/guide/mac-help/use-night-shift-mchl97bc676d/mac)
* [AirPlay to Mac](https://www.apple.com/macos/monterey-preview/)
* [Universal Control](https://www.apple.com/macos/monterey-preview/)

```sh
# Sidecar Unlock
MacBook8,1
MacBookAir5,x - MacBookAir7,x
MacBookPro9,x - MacBookPro12,x
Macmini6,x    - Macmini7,1
MacPro5,1     - MacPro6,1
iMac13,x      - iMac16,x

# Sidecar (running iOS 13)
iPad4,x       - iPad5,x
iPad6,11      - iPad6,12

# AirPlay to Mac Unlock
MacBook8,1
MacBookAir5,x - MacBookAir7,x
MacBookPro9,x - MacBookPro14,x
Macmini6,x    - Macmini8,1
MacPro5,1     - MacPro6,1
iMac13,x      - iMac18,x
Systems with 'kern.hv_vmm_present' set to as 1 (Ventura and newer)

# NightShift Unlock
MacBook1,1    - MacBook7,1
MacBookAir1,1 - MacBookAir4,x
MacBookPro1,1 - MacBookPro8,x
Macmini1,1    - Macmini5,x
MacPro1,1     - MacPro5,1
iMac4,1       - iMac12,x

# Universal Control Unlock
MacBookAir7,x
MacBookPro11,4 - MacBookPro12,1
Macmini7,1
MacPro6,1
iMac16,x
```

#### Additional Notes

* **NightShift** (macOS 10.12.4+)

* **AirPlay to Mac** (macOS 12.0+)

* **Universal Control** (macOS 12.3+)
  * Requires minimum of Wifi N and Bluetooth 4.0 for wireless, wired is supported between iPad and Mac as an alternative
  * Note all parties check each other for compatibility, thus requiring FeatureUnlock on all models in the chain
    * Seen as `Ineligible Device Found Md MacBookPro11,4, SV 340.17.2` in Console under `UniversalControl: com.apple.universalcontrol`
    * Recommend using a different SMBIOS if possible, otherwise use `-force_uc_unlock` for machines that are not blacklisted but are connecting with a blacklisted model.
    * Due to the nature of requiring FeatureUnlock for both models, Apple Silicon and iPads will not work

* **Sidecar** (macOS 10.15.0+)
  * Requires minimum of Wifi N and Bluetooth 4.0 for wireless, wired is supported between iPad and Mac as an alternative
  * Requires a machine with an Intel iGPU active for reliable usage, most dGPU-only machines will experience difficulties. An H.265 capable iGPU is recommended for best streaming quality.
  * AMD dGPU-only machines may work if using an iMac19,x or iMac20,x SMBIOS with `gvaForceAMDKE` setting applied:
 
```sh
defaults write com.apple.AppleGVA gvaForceAMDKE -boolean yes
```

#### Boot arguments

- `-caroff` (or `-liluoff`) to disable
- `-cardbg` (or `-liludbgall`) to enable verbose logging (in DEBUG builds)
- `-carbeta` (or `-lilubetaall`) to enable on macOS newer than 12
- `-allow_sidecar_ipad` enables Sidecar support for unsupported iPads (only functional with iOS 13, iOS 14+ implements an iOS-side check)
- `-disable_sidecar_mac` disables Sidecar/AirPlay patches
- `-disable_nightshift` disables NightShift patches
- `-disable_uni_control` disables Universal Control patches
- `-force_uni_control` forces Universal Control patching even when model doesn't require

#### Credits

- [Apple](https://www.apple.com) for macOS  
- [vit9696](https://github.com/vit9696) for [Lilu.kext](https://github.com/vit9696/Lilu)
- [khronokernel](https://github.com/khronokernel) for developing AirPlay to Mac and Universal Control patch sets and maintaining Sidecar patch set
- [Osy](https://github.com/Osy/Polaris22Fixup/) and [DhinakG](https://github.com/dhinakg/Polaris22Fixup/) for Polaris22Fixup base
- [Ben-z](https://github.com/ben-z/free-sidecar) for original SidecarCore patch set
- [Pike R. Alpha](https://pikeralpha.wordpress.com/2017/01/30/4398/) and [cdf](https://github.com/cdf/NightShiftEnabler) for NightShift patch set
