//
//  kern_start.cpp
//  FeatureUnlock.kext
//
//  Copyright © 2020 osy86, DhinakG, 2021 Mykola Grymalyuk. All rights reserved.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include <Headers/kern_user.hpp>
#include <Headers/kern_devinfo.hpp>
#include "kern_patch.hpp"

#define MODULE_SHORT "sidecar"

static mach_vm_address_t orig_cs_validate {};

#pragma mark - Kernel patching code

template <size_t findSize, size_t replaceSize>
static inline void searchAndPatch(const void *haystack, size_t haystackSize, const char *path, const uint8_t (&needle)[findSize], const uint8_t (&patch)[replaceSize], const char *name) {
   if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(haystack), haystackSize, needle, findSize, patch, replaceSize)))
	   DBGLOG(MODULE_SHORT, "found function %s to patch at %s!", name, path);
}

#pragma mark - Patched functions

// pre Big Sur
static boolean_t patched_cs_validate_range(vnode_t vp, memory_object_t pager, memory_object_offset_t offset, const void *data, vm_size_t size, unsigned *result) {
    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    boolean_t res = FunctionCast(patched_cs_validate_range, orig_cs_validate)(vp, pager, offset, data, size, result);
    if (res && vn_getpath(vp, path, &pathlen) == 0 && UserPatcher::matchSharedCachePath(path)) {
        auto deviceInfo = BaseDeviceInfo::get();
        if (strstr(deviceInfo.modelIdentifier, "Book", strlen("Book"))) {
            if (strstr(deviceInfo.modelIdentifier, "Pro", strlen("Pro"))) {
                searchAndPatch(data, size, path, kSideCarAirPlayMacBookProOriginal, kSideCarAirPlayMacBookProPatched, "Sidecar/AirPlay/UniversalControl (MacBookPro)");
            } else {
                searchAndPatch(data, size, path, kSideCarAirPlayMacBookOriginal, kSideCarAirPlayMacBookPatched, "Sidecar/AirPlay/UniversalControl (MacBook/MacBookAir)");
            }
        } else if (strstr(deviceInfo.modelIdentifier, "iMac", strlen("iMac"))){
            searchAndPatch(data, size, path, kSideCarAirPlayiMacOriginal, kSideCarAirPlayiMacPatched, "Sidecar/AirPlay/UniversalControl (iMac)");
        } else {
            searchAndPatch(data, size, path, kSideCarAirPlayStandaloneDesktopOriginal, kSideCarAirPlayStandaloneDesktopPatched, "Sidecar/AirPlay/UniversalControl (Macmini/MacPro)");
        }
        searchAndPatch(data, size, path, kSidecariPadModelOriginal, kSidecariPadModelPatched, "Sidecar/UniversalControl (iPad)");
        if ((getKernelVersion() == KernelVersion::HighSierra && getKernelMinorVersion() >= 2) || getKernelVersion() >= KernelVersion::Mojave) {
            searchAndPatch(data, size, path, kNightShiftOriginal, kNightShiftPatched, "NightShift");
        } else {
            searchAndPatch(data, size, path, kNightShiftLegacyOriginal, kNightShiftLegacyPatched, "NightShift");
        }
    }
    return res;
}

// For Big Sur and newer
static void patched_cs_validate_page(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void *data, int *validated_p, int *tainted_p, int *nx_p) {
    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    FunctionCast(patched_cs_validate_page, orig_cs_validate)(vp, pager, page_offset, data, validated_p, tainted_p, nx_p);
	if (vn_getpath(vp, path, &pathlen) == 0 && UserPatcher::matchSharedCachePath(path)) {
        auto deviceInfo = BaseDeviceInfo::get();
        if (strstr(deviceInfo.modelIdentifier, "Book", strlen("Book"))) {
            if (strstr(deviceInfo.modelIdentifier, "Pro", strlen("Pro"))) {
                searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookProOriginal, kSideCarAirPlayMacBookProPatched, "Sidecar/AirPlay/UniversalControl (MacBookPro)");
            } else {
                searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookOriginal, kSideCarAirPlayMacBookPatched, "Sidecar/AirPlay/UniversalControl (MacBook/MacBookAir)");
            }
        } else if (strstr(deviceInfo.modelIdentifier, "iMac", strlen("iMac"))){
            searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacOriginal, kSideCarAirPlayiMacPatched, "Sidecar/AirPlay/UniversalControl (iMac)");
        } else {
            searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayStandaloneDesktopOriginal, kSideCarAirPlayStandaloneDesktopPatched, "Sidecar/AirPlay/UniversalControl (Macmini/MacPro)");
        }
        searchAndPatch(data, PAGE_SIZE, path, kSidecariPadModelOriginal, kSidecariPadModelPatched, "Sidecar/UniversalControl (iPad)");
        searchAndPatch(data, PAGE_SIZE, path, kNightShiftOriginal, kNightShiftPatched, "NightShift");
        if (getKernelVersion() >= KernelVersion::Monterey) {
            searchAndPatch(data, PAGE_SIZE, path, kMacModelAirplayExtendedOriginal, kMacModelAirplayExtendedOriginal, "AirPlay to Mac (Extended)");
        }
    }
}

#pragma mark - Patches on start/stop

static void pluginStart() {
	DBGLOG(MODULE_SHORT, "start");
	lilu.onPatcherLoadForce([](void *user, KernelPatcher &patcher) {
		KernelPatcher::RouteRequest csRoute =
			getKernelVersion() >= KernelVersion::BigSur ?
			KernelPatcher::RouteRequest("_cs_validate_page", patched_cs_validate_page, orig_cs_validate) :
			KernelPatcher::RouteRequest("_cs_validate_range", patched_cs_validate_range, orig_cs_validate);
		if (!patcher.routeMultipleLong(KernelPatcher::KernelID, &csRoute, 1))
			SYSLOG(MODULE_SHORT, "failed to route cs validation pages");
	});
}

// Boot args.
static const char *bootargOff[] {
    "-caroff"
};
static const char *bootargDebug[] {
    "-cardbg"
};
static const char *bootargBeta[] {
    "-carbeta"
};

// Plugin configuration.
PluginConfiguration ADDPR(config) {
    xStringify(PRODUCT_NAME),
    parseModuleVersion(xStringify(MODULE_VERSION)),
    LiluAPI::AllowNormal,
    bootargOff,
    arrsize(bootargOff),
    bootargDebug,
    arrsize(bootargDebug),
    bootargBeta,
    arrsize(bootargBeta),
    KernelVersion::Catalina,
    KernelVersion::Monterey,
    pluginStart
};
