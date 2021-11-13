FeatureUnlock
==============

[![Build Status](https://github.com/acidanthera/FeatureUnlock/workflows/CI/badge.svg?branch=master)](https://github.com/acidanthera/FeatureUnlock/actions) [![Scan Status](https://scan.coverity.com/projects/23354/badge.svg?flat=1)](https://scan.coverity.com/projects/23354)

[Lilu](https://github.com/acidanthera/Lilu) Kernel extension for enabling:
* [Sidecar](https://support.apple.com/en-ca/HT210380)
* [NightShift](https://support.apple.com/guide/mac-help/use-night-shift-mchl97bc676d/mac)
* [AirPlay to Mac](https://www.apple.com/macos/monterey-preview/)
* [Universal Control](https://www.apple.com/macos/monterey-preview/)
* [Content Caching](https://support.apple.com/en-ca/guide/mac-help/mchl9388ba1b/mac)

```sh
# Sidecar and Universal Control Unlock
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

# NightShift Unlock
MacBook1,1    - MacBook7,1
MacBookAir1,1 - MacBookAir4,x
MacBookPro1,1 - MacBookPro8,x
Macmini1,1    - Macmini5,x
MacPro1,1     - MacPro5,1
iMac4,1       - iMac12,x

# Content Caching Unlock
For systems returning 1 from 'sysctl kern.hv_vmm_present'
```

**Note**: Sidecar requires a machine with an Intel iGPU active for reliable usage, most dGPU-only machines will experience difficulties. An H.265 capable iGPU is recommended for best streaming quality.

* AMD dGPU-only machines may work if using an iMac19,x or iMac20,x SMBIOS with the following applied:

```sh
defaults write com.apple.AppleGVA gvaForceAMDKE -boolean yes
```

AirPlay to Mac does not have such limitation and can work on H.264 dGPUs. However requires macOS 12, Monterey or newer to use

#### Boot arguments

- `-caroff` (or `-liluoff`) to disable
- `-cardbg` (or `-liludbgall`) to enable verbose logging (in DEBUG builds)
- `-carbeta` (or `-lilubetaall`) to enable on macOS newer than 12
- `-allow_assetcache` enables Content Cache support for systems exposing `kern.hv_vmm_supported`
- `-allow_sidecar_ipad` enables Sidecar support for unsupported iPads (only functional with iOS 13, iOS 14+ implements an iOS-side check)
- `-disable_sidecar_mac` disables Sidecar/AirPlay/Universal Control patches
- `-disable_nightshift` disables NightShift patches

#### Credits

- [Apple](https://www.apple.com) for macOS  
- [vit9696](https://github.com/vit9696) for [Lilu.kext](https://github.com/vit9696/Lilu)
- [khronokernel](https://github.com/khronokernel) for developing AirPlay to Mac and Universal Control patch sets and maintaining Sidecar patch set
- [Osy](https://github.com/Osy/Polaris22Fixup/) and [DhinakG](https://github.com/dhinakg/Polaris22Fixup/) for Polaris22Fixup base
- [Ben-z](https://github.com/ben-z/free-sidecar) for original SidecarCore patch set
- [Pike R. Alpha](https://pikeralpha.wordpress.com/2017/01/30/4398/) and [cdf](https://github.com/cdf/NightShiftEnabler) for NightShift patch set
