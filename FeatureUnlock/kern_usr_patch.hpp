//
//  kern_usr_patch.hpp
//  FeatureUnlock
//
//  Created by Mykola Grymalyuk on 2021-11-13.
//  Copyright © 2021 Khronokernel. All rights reserved.
//

// Patch sets used by userspace patcher, see respective sections for patch set explanation
// For dyld-based patches, see kern_dyld_patch.hpp

#include <stdint.h>

#pragma mark - Binary Paths on disk

static const char *assetCachePath = "/usr/libexec/AssetCache/AssetCache";

#pragma mark - Binary Patches

// With macOS 11.3 and newer, Apple added a new systcl entry called kern.hv_vmm_present
// If a system has the VMM flag set, IOGetVMMPresent will return true and set hv_vmm_present to 1
// https://opensource.apple.com/source/xnu/xnu-7195.141.2/bsd/kern/kern_sysctl.c.auto.html

// AssetCache has an explicit check for both the VMM flag and hv_vmm_present to block usage of AssetCache on Virtual Machines.
// Since macOS High Sierra, Apple has outright blocked usage of Content Caching on Virtual Machines:
// https://support.apple.com/en-gb/HT207828

// However projects such as OpenCore Legacy Patcher may use hv_vmm_present to work around OS checks, allowing for older SMBIOS
// models to be used in newer OSes.
// To resolve, simply patch with non-existent sysctl entry.
static const uint8_t kAssetCacheHypervisorOriginal[] = "kern.hv_vmm_present";
static const uint8_t kAssetCacheHypervisorPatched[] = "virtual_acidanthera";
