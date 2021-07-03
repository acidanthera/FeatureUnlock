SidecarFixup
==============

[![Build Status](https://github.com/acidanthera/SidecarFixup/workflows/CI/badge.svg?branch=master)](https://github.com/acidanthera/SidecarFixup/actions) [![Scan Status](https://scan.coverity.com/projects/23155/badge.svg?flat=1)](https://scan.coverity.com/projects/23155)

[Lilu](https://github.com/acidanthera/Lilu) Kernel extension for enabling [Sidecar](https://support.apple.com/en-ca/HT210380), [NightShift](https://support.apple.com/guide/mac-help/use-night-shift-mchl97bc676d/mac) , [AirPlay to Mac](https://www.apple.com/macos/monterey-preview/) and [Universal Control](https://www.apple.com/macos/monterey-preview/) support on the following SMBIOS:

```sh
# Sidecar and Universal Control Unlock
MacBook8,1
MacBookAir5,x - MacBookAir7,x
MacBookPro9,x - MacBookPro12,x
Macmini6,x    - Macmini7,1
MacPro5,1     - MacPro6,1
iMac13,x      - iMac16,x
iPad4,x       - iPad6,x

# AirPlay to Mac Unlock
MacBook8,1
MacBookAir5,x - MacBookAir7,x
MacBookPro9,x - MacBookPro14,x
Macmini6,x    - Macmini8,1
MacPro5,1     - MacPro6,1
iMac13,x      - iMac18,x

# NightShift Unlock
MacBook7,1 and older
MacBookAir4,x and older
MacBookPro8,x and older
Macmini5,x and older
MacPro5,x and older
iMac12,x and older
```

**Note**: Sidecar requires a machine with an Intel iGPU enabled and active, dGPU-only machines will not work. H.265 capable iGPU recommended for best streaming quality

* AirPlay to Mac does not have such limitation and can work on H.264 dGPUs. However requires macOS 12, Monterey or newer to use

#### Boot arguments

- `-caroff` (or `-liluoff`) to disable
- `-cardbg` (or `-liludbgall`) to enable verbose logging (in DEBUG builds)
- `-carbeta` (or `-lilubetaall`) to enable on macOS newer than 12

#### Credits

- [Apple](https://www.apple.com) for macOS  
- [vit9696](https://github.com/vit9696) for [Lilu.kext](https://github.com/vit9696/Lilu)
- [khronokernel](https://github.com/khronokernel) for developing AirPlay to Mac and Universal Control patch sets and maintaining Sidecar patch set
- [Osy](https://github.com/Osy/Polaris22Fixup/) and [DhinakG](https://github.com/dhinakg/Polaris22Fixup/) for Polaris22Fixup base
- [Ben-z](https://github.com/ben-z/free-sidecar) for original patch SidecarCore patch set
- [Pike R. Alpha](https://pikeralpha.wordpress.com/2017/01/30/4398/) and [cdf](https://github.com/cdf/NightShiftEnabler) for NightShift patch set
