//
//  kern_start.cpp
//  FeatureUnlock.kext
//
//  Copyright © 2020 osy86, DhinakG, 2021-2022 Mykola Grymalyuk. All rights reserved.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>
#include <Headers/kern_user.hpp>
#include <Headers/kern_devinfo.hpp>
#include <sys/sysctl.h>
#include "kern_dyld_patch.hpp"
#include "kern_usr_patch.hpp"
#include "kern_model_info.hpp"

#define MODULE_SHORT "fu_fix"

// Original function pointers
static mach_vm_address_t orig_cs_validate {};

// Boot-arg configurations
bool allow_sidecar_ipad;
bool disable_sidecar_mac;
bool disable_nightshift;
bool force_universal_control;

// OS Feature Set
bool os_supports_nightshift_old;
bool os_supports_nightshift_new;
bool os_supports_sidecar;
bool os_supports_airplay_to_mac;
bool os_supports_airplay_to_mac_vmm_checks;
bool os_supports_universal_control;

// Host needs patches
bool host_needs_nightshift_patch;
bool host_needs_sidecar_patch;
bool host_needs_airplay_to_mac_patch;
bool host_needs_airplay_to_mac_vmm_patch;
bool host_needs_universal_control_patch;
bool host_needs_continuity_patch;
int  host_needs_vmm_patch;

// Model family detected
bool model_is_iMac_pre_2012;        // iMac7,1 - iMac12,x
bool model_is_iMac_2012;            // iMac13,x
bool model_is_iMac_2013;            // iMac14,x
bool model_is_iMac_2014;            // iMac15,1
bool model_is_iMac_2015_broadwell;  // iMac16,x
bool model_is_iMac_2015_2017;       // iMac17,x - iMac18,x

bool model_is_MacBook_pre_2015;     // MacBook4,1 - MacBook7,1
bool model_is_MacBook_2015;         // MacBook8,1

bool model_is_MacBookAir_pre_2012;  // MacBookAir2,1 - MacBookAir4,x
bool model_is_MacBookAir_2012;      // MacBookAir5,x
bool model_is_MacBookAir_2013;      // MacBookAir6,x
bool model_is_MacBookAir_2015;      // MacBookAir7,x

bool model_is_MacBookPro_pre_2012;  // MacBookPro4,1 - MacBookPro8,x
bool model_is_MacBookPro_2012;      // MacBookPro9,x - MacBookPro10,x
bool model_is_MacBookPro_2013;      // MacBookPro11,1 - 3
bool model_is_MacBookPro_2015;      // MacBookPro11,4 - 5 - MacBookPro12,1
bool model_is_MacBookPro_2016;      // MacBookPro13,x
bool model_is_MacBookPro_2017;      // MacBookPro14,x

bool model_is_Macmini_pre_2012;     // Macmini3,1 - Macmini5,x
bool model_is_Macmini_2012;         // Macmini6,x
bool model_is_Macmini_2014;         // Macmini7,x
bool model_is_Macmini_2018;         // Macmini8,x

bool model_is_MacPro_pre_2013;      // MacPro3,1 - MacPro5,x
bool model_is_MacPro_2010_2012;     // MacPro5,x
bool model_is_MacPro_2013;          // MacPro6,x

bool has_applied_nightshift_patch;
bool has_applied_airplay_to_mac_vmm_patch;
bool has_applied_iPad_sidecar_patch;
bool has_applied_continuity_patch;


// Misc variables
int number_of_loops = 0;
int total_allowed_loops = 0;

uint64_t start_time;
uint64_t current_time;

bool time_to_exit = false;

#pragma mark - Kernel patching code

template <size_t findSize, size_t findMaskSize, size_t replaceSize, size_t replaceMaskSize>
static inline bool searchAndPatchWithMask(const void *haystack, size_t haystackSize, const char *path, const uint8_t (&needle)[findSize], const uint8_t (&findMask)[findMaskSize], const uint8_t (&patch)[replaceSize], const uint8_t (&patchMask)[replaceMaskSize], const char *name, bool is_dyld) {
    if (KernelPatcher::findAndReplaceWithMask(const_cast<void *>(haystack), haystackSize, needle, findSize, findMask, findMaskSize, patch, replaceSize, patchMask, replaceMaskSize, 0, 0)) {
        DBGLOG(MODULE_SHORT, "found function %s to patch at %s!", name, path);
        if (is_dyld) {
            number_of_loops++;
            DBGLOG(MODULE_SHORT, "number of loops: %d", number_of_loops);
            if (number_of_loops == total_allowed_loops) {
                DBGLOG(MODULE_SHORT, "Reached maximum loops (%d), no more dyld patching", total_allowed_loops);
            }
        }
        return true;
    }
    return false;
}

template <size_t findSize, size_t replaceSize>
static inline bool searchAndPatch(const void *haystack, size_t haystackSize, const char *path, const uint8_t (&needle)[findSize], const uint8_t (&patch)[replaceSize], const char *name, bool is_dyld) {
    if (UNLIKELY(KernelPatcher::findAndReplace(const_cast<void *>(haystack), haystackSize, needle, findSize, patch, replaceSize))) {
        DBGLOG(MODULE_SHORT, "found function %s to patch at %s!", name, path);
        if (is_dyld) {
            number_of_loops++;
            DBGLOG(MODULE_SHORT, "number of loops: %d", number_of_loops);
            if (number_of_loops == total_allowed_loops) {
                DBGLOG(MODULE_SHORT, "Reached maximum loops (%d), no more dyld patching", total_allowed_loops);
            }
        }
        return true;
    }
    return false;
}

static inline bool check_time_elapsed() {
    current_time = mach_absolute_time();
    uint64_t elapsed_time = current_time - start_time;
    uint64_t elapsed_time_ms = elapsed_time / 1000000;
    // Check if 5 minutes have elapsed
    if (elapsed_time_ms > 300000) {
        DBGLOG(MODULE_SHORT, "Time elapsed since start: %llu ms, exiting", elapsed_time_ms);
        time_to_exit = true;
        return true;
    }
    return false;
}

#pragma mark - Patched functions

// pre Big Sur
static boolean_t patched_cs_validate_range(vnode_t vp, memory_object_t pager, memory_object_offset_t offset, const void *data, vm_size_t size, unsigned *result) {
    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    boolean_t res = FunctionCast(patched_cs_validate_range, orig_cs_validate)(vp, pager, offset, data, size, result);
    // If patch was successful, exit early
    bool patch_result = false;

    if (res && vn_getpath(vp, path, &pathlen) == 0 && UserPatcher::matchSharedCachePath(path)) {
        if (number_of_loops >= total_allowed_loops) {
            return res;
        }
        if (!disable_nightshift && host_needs_nightshift_patch && !has_applied_nightshift_patch) {
            if (os_supports_nightshift_new) {
                patch_result = searchAndPatch(data, size, path, kNightShiftOriginal, kNightShiftPatched, "NightShift", true);
                if (patch_result) {
                    has_applied_nightshift_patch = true;
                    return res;
                }
            } else if (os_supports_nightshift_old) {
                patch_result = searchAndPatch(data, size, path, kNightShiftLegacyOriginal, kNightShiftLegacyPatched, "NightShift Legacy", true);
                if (patch_result) {
                    has_applied_nightshift_patch = true;
                    return res;
                }
            }
        }

        if (!disable_sidecar_mac && os_supports_sidecar) {
            if (host_needs_sidecar_patch) {
                if (model_is_MacBookPro_2012 || model_is_MacBookPro_2013 || model_is_MacBookPro_2015) {
                    patch_result = searchAndPatch(data, size, path, kSideCarAirPlayMacBookProOriginal, kSideCarAirPlayMacBookProPatched, "Sidecar (MacBookPro)", true);
                    if (patch_result) {
                        return res;
                    }
                } else if (model_is_MacBookAir_2012 || model_is_MacBookAir_2013 || model_is_MacBookAir_2015 || model_is_MacBook_2015) {
                    patch_result = searchAndPatch(data, size, path, kSideCarAirPlayMacBookOriginal, kSideCarAirPlayMacBookPatched, "Sidecar (MacBook/MacBookAir)", true);
                    if (patch_result) {
                        return res;
                    }
                } else if (model_is_iMac_2012 || model_is_iMac_2013 || model_is_iMac_2014 || model_is_iMac_2015_broadwell) {
                    patch_result = searchAndPatch(data, size, path, kSideCarAirPlayiMacOriginal, kSideCarAirPlayiMacPatched, "Sidecar (iMac)", true);
                    if (patch_result) {
                        return res;
                    }
                } else if (model_is_Macmini_2012 || model_is_Macmini_2014 || model_is_MacPro_2013 || model_is_MacPro_2010_2012) {
                    patch_result = searchAndPatch(data, size, path, kSideCarAirPlayStandaloneDesktopOriginal, kSideCarAirPlayStandaloneDesktopPatched, "Sidecar (Macmini/MacPro)", true);
                    if (patch_result) {
                        return res;
                    }
                }
            }
        }
        if (allow_sidecar_ipad && os_supports_sidecar && !has_applied_iPad_sidecar_patch) {
            patch_result = searchAndPatch(data, size, path, kSidecariPadModelOriginal, kSidecariPadModelPatched, "Sidecar (iPad)", true);
            if (patch_result) {
                has_applied_iPad_sidecar_patch = true;
                return res;
            }
        }
    }
    return res;
}

// For Big Sur and newer
static void patched_cs_validate_page(vnode_t vp, memory_object_t pager, memory_object_offset_t page_offset, const void *data, int *validated_p, int *tainted_p, int *nx_p) {
    char path[PATH_MAX];
    int pathlen = PATH_MAX;
    FunctionCast(patched_cs_validate_page, orig_cs_validate)(vp, pager, page_offset, data, validated_p, tainted_p, nx_p);

    // If patch was successful, exit early
    bool patch_result = false;
    bool patch_result_tmp = false;

    if (vn_getpath(vp, path, &pathlen) == 0) {
        // dyld_shared_cache patching
        if (UserPatcher::matchSharedCachePath(path)) {
            // If we've already patched everything we can, exit early
            if (number_of_loops >= total_allowed_loops) {
                return;
            }

            /*
            Check if too much time has passed since start
            We know the dyld patching should finish within 5 minutes, otherwise our patches
            are likely between pages and will never apply (ie. wasted loops)
            */
            if (time_to_exit) {
                return;
            } else if (check_time_elapsed()) {
                return;
            }
            
            // Continuity Camera patch
            if (!has_applied_continuity_patch && host_needs_continuity_patch) {
                patch_result = searchAndPatchWithMask(data, PAGE_SIZE, path, kContinuityCameraOriginal, kContinuityCameraOriginalMask, kContinuityCameraPatched, kContinuityCameraPatchedMask, "Continuity Camera", true);
                if (patch_result) {
                    has_applied_continuity_patch = true;
                    return;
                }
            }

            // Night Shift patch
            if (!disable_nightshift && os_supports_nightshift_new) {
                if (host_needs_nightshift_patch && !has_applied_nightshift_patch) {
                    patch_result = searchAndPatch(data, PAGE_SIZE, path, kNightShiftOriginal, kNightShiftPatched, "NightShift", true);
                    if (patch_result) {
                        has_applied_nightshift_patch = true;
                        return;
                    }
                }
            }

            // Sidecar, AirPlay and Universal Control patches
            if (!disable_sidecar_mac && (os_supports_sidecar || os_supports_airplay_to_mac || os_supports_universal_control || os_supports_airplay_to_mac_vmm_checks)) {
                if (host_needs_airplay_to_mac_patch || host_needs_sidecar_patch || host_needs_universal_control_patch || host_needs_vmm_patch) {
                    /* Note: VMM check may be inside the same page as the model check, thus don't return early
                             when patch has been applied.
                    */

                    // Sidecar and AirPlay model checks
                    if (model_is_MacBookPro_2012) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookPro2012Original, kSideCarAirPlayMacBookPro2012Patched, "Sidecar/AirPlay (MacBook Pro 2012)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    } else if (model_is_MacBookPro_2013 || model_is_MacBookPro_2015) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookPro2013_2015Original, kSideCarAirPlayMacBookPro2013_2015Patched, "Sidecar/AirPlay (MacBook Pro 2013 - 2015)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    } else if (model_is_MacBook_2015 || model_is_MacBookAir_2012) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookMacBookAir2012Original, kSideCarAirPlayMacBooMacBookAir2012Patched, "Sidecar/AirPlay (MacBook 2015/MacBook Air 2012)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    } else if (model_is_MacBookAir_2013 || model_is_MacBookAir_2015) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacBookAir2013_2015Original, kSideCarAirPlayMacBookAir2013_2015Patched, "Sidecar/AirPlay (MacBook Air 2013 - 2015)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    } else if (model_is_iMac_2012 || model_is_iMac_2013 || model_is_iMac_2014 || model_is_iMac_2015_broadwell) {
                        if (model_is_iMac_2012) {
                            patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacAlternative2012Original, kSideCarAirPlayiMacAlternative2012Patched, "Sidecar/AirPlay (iMac 2012)", true);
                            if (patch_result_tmp) {
                                patch_result = true;
                            }
                        } else if (model_is_iMac_2013) {
                            patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacAlternative2013Original, kSideCarAirPlayiMacAlternative2013Patched, "Sidecar/AirPlay (iMac 2013)", true);
                            if (patch_result_tmp) {
                                patch_result = true;
                            }
                        } else if (model_is_iMac_2014 || model_is_iMac_2015_broadwell) {
                            patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayiMacAlternative2014Original, kSideCarAirPlayiMacAlternative2014Patched, "Sidecar/AirPlay (iMac 2014)", true);
                            if (patch_result_tmp) {
                                patch_result = true;
                            }
                        }
                    } else if (model_is_Macmini_2012 || model_is_Macmini_2014) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacminiOriginal, kSideCarAirPlayMacminiPatched, "Sidecar/AirPlay (Mac mini 2012 - 2014)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    } else if (model_is_MacPro_2013 || model_is_MacPro_2010_2012) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kSideCarAirPlayMacProOriginal, kSideCarAirPlayMacProPatched, "Sidecar/AirPlay (Mac Pro 2010 - 2013)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    }
                    else if (os_supports_airplay_to_mac && (model_is_MacBookPro_2016 || model_is_MacBookPro_2017 || model_is_iMac_2015_2017 || model_is_Macmini_2018)) {
                        patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kMacModelAirplayExtendedOriginal, kMacModelAirplayExtendedPatched, "AirPlay to Mac (Extended)", true);
                        if (patch_result_tmp) {
                            patch_result = true;
                        }
                    }

                    // AirPlay to Mac VMM check
                    if (os_supports_airplay_to_mac_vmm_checks && !has_applied_airplay_to_mac_vmm_patch) {
                        if (host_needs_vmm_patch) {
                            patch_result_tmp = searchAndPatch(data, PAGE_SIZE, path, kAirPlayVmmOriginal, kAirPlayVmmPatched, "AirPlay to Mac (VMM)", true);
                            if (patch_result_tmp) {
                                has_applied_airplay_to_mac_vmm_patch = true;
                                patch_result = true;
                            }
                        }
                    }
                }
                // Sidecar iPad check
                if (allow_sidecar_ipad && !has_applied_iPad_sidecar_patch) {
                    patch_result = searchAndPatch(data, PAGE_SIZE, path, kSidecariPadModelOriginal, kSidecariPadModelPatched, "Sidecar (iPad)", true);
                    if (patch_result) {
                        has_applied_iPad_sidecar_patch = true;
                    }
                }

                if (patch_result) {
                    return;
                }
            }
        }
        // Individual binary patching
        else {
            // Universal Control.app patch
            if (!disable_sidecar_mac && os_supports_universal_control && host_needs_universal_control_patch) {
                if (UNLIKELY(strcmp(path, universalControlPath) == 0)) {
                    patch_result = searchAndPatch(data, PAGE_SIZE, path, kUniversalControlFind, kUniversalControlReplace, "Universal Control (app)", false);
                    if (patch_result) {
                        return;
                    }
                }
            }
            if (!disable_sidecar_mac && host_needs_airplay_to_mac_vmm_patch) {
                if (UNLIKELY(strcmp(path, controlCenterPath) == 0)) {
                    patch_result = searchAndPatch(data, PAGE_SIZE, path, kGenericVmmOriginal, kGenericVmmPatched, "Control Center (app)", false);
                    if (patch_result) {
                        return;
                    }
                }
            }
        }
    }
}

#pragma mark - Detect Model

static void detectMachineProperties() {
    // Detect Hypervisor
    size_t vmm_present_size = sizeof(host_needs_vmm_patch);
    if (sysctlbyname("kern.hv_vmm_present", &host_needs_vmm_patch, &vmm_present_size, NULL, 0) == 0) {
        if (host_needs_vmm_patch == 1) {
            DBGLOG(MODULE_SHORT, "Detected VMM system");
        }
    }

    // Detect model
    // Function will return once a match is found
    auto deviceInfo = BaseDeviceInfo::get();
    SYSLOG(MODULE_SHORT, "Host model detected: %s", deviceInfo.modelIdentifier);

    // Sometimes the model can have garbage on the end, so partial matches are used
    // ex. MacBookPro13,1DvcPtsupre
    if (strstr(deviceInfo.modelIdentifier, "Book", sizeof("Book")-1)) {
        if (strstr(deviceInfo.modelIdentifier, "Pro", sizeof("Pro")-1)) {
            // MacBook Pro
            for (size_t i = 0; i < arrsize(macbookpro_legacy_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookpro_legacy_models[i], strlen(macbookpro_legacy_models[i])) == 0) {
                    model_is_MacBookPro_pre_2012 = true;
                    DBGLOG(MODULE_SHORT, "Detected legacy MacBookPro model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookpro_2012_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookpro_2012_models[i], strlen(macbookpro_2012_models[i])) == 0) {
                    model_is_MacBookPro_2012 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookPro 2012 model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookpro_2013_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookpro_2013_models[i], strlen(macbookpro_2013_models[i])) == 0) {
                    model_is_MacBookPro_2013 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookPro 2013 model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookpro_2015_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookpro_2015_models[i], strlen(macbookpro_2015_models[i])) == 0) {
                    model_is_MacBookPro_2015 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookPro 2015 model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookpro_2016_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookpro_2016_models[i], strlen(macbookpro_2016_models[i])) == 0) {
                    model_is_MacBookPro_2016 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookPro 2016 model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookpro_2017_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookpro_2017_models[i], strlen(macbookpro_2017_models[i])) == 0) {
                    model_is_MacBookPro_2017 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookPro 2017 model");
                    return;
                }
            }
        // MacBook Air
        } else if (strstr(deviceInfo.modelIdentifier, "Air", sizeof("Air")-1)) {
            for (size_t i = 0; i < arrsize(macbookair_legacy_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookair_legacy_models[i], strlen(macbookair_legacy_models[i])) == 0) {
                    model_is_MacBookAir_pre_2012 = true;
                    DBGLOG(MODULE_SHORT, "Detected legacy MacBookAir model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookair_2012_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookair_2012_models[i], strlen(macbookair_2012_models[i])) == 0) {
                    model_is_MacBookAir_2012 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookAir 2012 model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookair_2013_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookair_2013_models[i], strlen(macbookair_2013_models[i])) == 0) {
                    model_is_MacBookAir_2013 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookAir 2013 model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbookair_2015_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbookair_2015_models[i], strlen(macbookair_2015_models[i])) == 0) {
                    model_is_MacBookAir_2015 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBookAir 2015 model");
                    return;
                }
            }
        } else {
            // MacBook
            for (size_t i = 0; i < arrsize(macbook_legacy_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbook_legacy_models[i], strlen(macbook_legacy_models[i])) == 0) {
                    model_is_MacBook_pre_2015 = true;
                    DBGLOG(MODULE_SHORT, "Detected legacy MacBook model");
                    return;
                }
            }
            for (size_t i = 0; i < arrsize(macbook_modern_models); i++) {
                if (strncmp(deviceInfo.modelIdentifier, macbook_modern_models[i], strlen(macbook_modern_models[i])) == 0) {
                    model_is_MacBook_2015 = true;
                    DBGLOG(MODULE_SHORT, "Detected MacBook 2015 model");
                    return;
                }
            }
        }
    } else if (strstr(deviceInfo.modelIdentifier, "mini", sizeof("mini")-1)) {
        // Mac mini
        for (size_t i = 0; i < arrsize(macmini_legacy_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, macmini_legacy_models[i], strlen(macmini_legacy_models[i])) == 0) {
                model_is_Macmini_pre_2012 = true;
                DBGLOG(MODULE_SHORT, "Detected legacy Mac mini model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(macmini_2012_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, macmini_2012_models[i], strlen(macmini_2012_models[i])) == 0) {
                model_is_Macmini_2012 = true;
                DBGLOG(MODULE_SHORT, "Detected Mac mini 2012 model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(macmini_2014_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, macmini_2014_models[i], strlen(macmini_2014_models[i])) == 0) {
                model_is_Macmini_2014 = true;
                DBGLOG(MODULE_SHORT, "Detected Mac mini 2014 model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(macmini_2018_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, macmini_2018_models[i], strlen(macmini_2018_models[i])) == 0) {
                model_is_Macmini_2018 = true;
                DBGLOG(MODULE_SHORT, "Detected Mac mini 2018 model");
                return;
            }
        }

    } else if (strstr(deviceInfo.modelIdentifier, "Pro", sizeof("Pro")-1)) {
        // Mac Pro
        for (size_t i = 0; i < arrsize(macpro_legacy_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, macpro_legacy_models[i], strlen(macpro_legacy_models[i])) == 0) {
                model_is_MacPro_pre_2013 = true;
                DBGLOG(MODULE_SHORT, "Detected legacy Mac Pro model");
                for (size_t i = 0; i < arrsize(macpro_2010_2012_models); i++) {
                    if (strncmp(deviceInfo.modelIdentifier, macpro_2010_2012_models[i], strlen(macpro_2010_2012_models[i])) == 0) {
                        model_is_MacPro_2010_2012 = true;
                        DBGLOG(MODULE_SHORT, "Detected Mac Pro 2010-2012 model");
                        return;
                    }
                }
                return;
            }
        }
        for (size_t i = 0; i < arrsize(macpro_2013_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, macpro_2013_models[i], strlen(macpro_2013_models[i])) == 0) {
                model_is_MacPro_2013 = true;
                DBGLOG(MODULE_SHORT, "Detected Mac Pro 2013 model");
                return;
            }
        }
    } else if (strstr(deviceInfo.modelIdentifier, "iMac", sizeof("iMac")-1)) {
        // iMac
        for (size_t i = 0; i < arrsize(imac_legacy_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, imac_legacy_models[i], strlen(imac_legacy_models[i])) == 0) {
                model_is_iMac_pre_2012 = true;
                DBGLOG(MODULE_SHORT, "Detected legacy iMac model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(imac_2012_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, imac_2012_models[i], strlen(imac_2012_models[i])) == 0) {
                model_is_iMac_2012 = true;
                DBGLOG(MODULE_SHORT, "Detected iMac 2012 model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(imac_2013_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, imac_2013_models[i], strlen(imac_2013_models[i])) == 0) {
                model_is_iMac_2013 = true;
                DBGLOG(MODULE_SHORT, "Detected iMac 2013 model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(imac_2014_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, imac_2014_models[i], strlen(imac_2014_models[i])) == 0) {
                model_is_iMac_2014 = true;
                DBGLOG(MODULE_SHORT, "Detected iMac 2014 model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(imac_2015_broadwell_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, imac_2015_broadwell_models[i], strlen(imac_2015_broadwell_models[i])) == 0) {
                model_is_iMac_2015_broadwell = true;
                DBGLOG(MODULE_SHORT, "Detected iMac 2015 Broadwell model");
                return;
            }
        }
        for (size_t i = 0; i < arrsize(imac_2015_2017_models); i++) {
            if (strncmp(deviceInfo.modelIdentifier, imac_2015_2017_models[i], strlen(imac_2015_2017_models[i])) == 0) {
                model_is_iMac_2015_2017 = true;
                DBGLOG(MODULE_SHORT, "Detected iMac 2015-2017 model");
                return;
            }
        }
    } else {
        // Unsupported model
        DBGLOG(MODULE_SHORT, "Model is non-standard, assuming no SMBIOS patching is required");
        return;
    }
    DBGLOG(MODULE_SHORT, "Model appears to not require SMBIOS patching, assuming native");
}

#pragma mark - Detect Supported Patch sets

static void detectSupportedPatchSets() {
    // Find all supported patch sets

    // NightShift
    if ((getKernelVersion() == KernelVersion::HighSierra && getKernelMinorVersion() >= 2) || getKernelVersion() >= KernelVersion::Mojave) {
        DBGLOG(MODULE_SHORT, "OS implements Modern NightShift blacklist");
        os_supports_nightshift_new = true;
    } else if (getKernelVersion() == KernelVersion::Sierra && getKernelMinorVersion() >= 5) {
        DBGLOG(MODULE_SHORT, "OS implements Legacy NightShift blacklist");
        os_supports_nightshift_old = true;
    }
    // Sidecar
    if (getKernelVersion() >= KernelVersion::Catalina) {
        DBGLOG(MODULE_SHORT, "OS implements Sidecar blacklist");
        os_supports_sidecar = true;
    }
    // AirPlay and Universal Control
    if (getKernelVersion() >= KernelVersion::Monterey) {
        DBGLOG(MODULE_SHORT, "OS implements AirPlay to Mac blacklist");
        os_supports_airplay_to_mac = true;
        if (getKernelVersion() >= KernelVersion::Ventura) {
            // Apple added kern.hv_vmm_present checks in Ventura, in addition to their normal model checks...
            DBGLOG(MODULE_SHORT, "OS implements AirPlay to Mac VMM blacklist");
            os_supports_airplay_to_mac_vmm_checks = true;
        }
        if ((getKernelVersion() == KernelVersion::Monterey && getKernelMinorVersion() >= 4) || getKernelVersion() >= KernelVersion::Ventura) {
            DBGLOG(MODULE_SHORT, "OS implements Universal Control blacklist");
            os_supports_universal_control = true;
        }
    }

    // Determine if we need to patch the SMBIOS
    if (!disable_nightshift && (os_supports_nightshift_old || os_supports_nightshift_new)) {
        if (model_is_iMac_pre_2012 || model_is_MacPro_pre_2013 || model_is_Macmini_pre_2012 || model_is_MacBook_pre_2015 || model_is_MacBookAir_pre_2012 || model_is_MacBookPro_pre_2012) {
            DBGLOG(MODULE_SHORT, "Model requires NightShift patch");
            host_needs_nightshift_patch = true;
        }
    }
    if (!disable_sidecar_mac && (os_supports_sidecar || os_supports_airplay_to_mac || os_supports_universal_control)) {
        // Sidecar (iPad specific)
        if (allow_sidecar_ipad) {
            DBGLOG(MODULE_SHORT, "Model requested Sidecar iPad patch");
        }

        // Sidecar and AirPlay to Mac
        if (// Ivy Bridge to Broadwell (including MacPro5,1)
            model_is_iMac_2012 || model_is_iMac_2013 || model_is_iMac_2014 || model_is_iMac_2015_broadwell || \
            model_is_MacPro_2010_2012 || model_is_MacPro_2013 || model_is_Macmini_2012 || model_is_Macmini_2014 || \
            model_is_MacBook_2015 || model_is_MacBookAir_2012 || model_is_MacBookAir_2013 || model_is_MacBookAir_2015 || \
            model_is_MacBookPro_2012 || model_is_MacBookPro_2013 || model_is_MacBookPro_2015
        ) {
            DBGLOG(MODULE_SHORT, "Model requires Sidecar patch");
            host_needs_sidecar_patch = true;
            if (os_supports_airplay_to_mac) {
                DBGLOG(MODULE_SHORT, "Model requires AirPlay patch");
                host_needs_airplay_to_mac_patch = true;
            }
        }
        // AirPlay to Mac extended
        else if (model_is_MacBookPro_2016 || model_is_MacBookPro_2017 || model_is_iMac_2015_2017 || model_is_Macmini_2018) {
            DBGLOG(MODULE_SHORT, "Model requires AirPlay extended patch");
            host_needs_airplay_to_mac_patch = true;
        }
        if (host_needs_vmm_patch && os_supports_airplay_to_mac_vmm_checks) {
            DBGLOG(MODULE_SHORT, "Model requires AirPlay VMM patch");
            host_needs_airplay_to_mac_vmm_patch = true;
        }

        // Universal Control
        if (
            // Pre-Skylake models native to Monterey
            model_is_iMac_2015_broadwell || model_is_MacPro_2013 || model_is_Macmini_2014 || model_is_MacBook_2015 || model_is_MacBookAir_2015 || model_is_MacBookPro_2015
        ) {
            DBGLOG(MODULE_SHORT, "Model requires Universal Control patch");
            host_needs_universal_control_patch = true;
        }
        else if (force_universal_control) {
            DBGLOG(MODULE_SHORT, "Model requested Universal Control patch");
            host_needs_universal_control_patch = true;
        }
        
        // Continuity Camera
        if (getKernelVersion() >= KernelVersion::Ventura && BaseDeviceInfo::get().cpuGeneration < CPUInfo::CpuGeneration::KabyLake) {
            DBGLOG(MODULE_SHORT, "Model requires Continuity Camera patch");
            host_needs_continuity_patch = true;
        }
    }

}

static void detectNumberOfPatches() {
    // Detects Number of patches applied in dyld
    if ((os_supports_nightshift_new || os_supports_nightshift_old) && !disable_nightshift) {
        if (host_needs_nightshift_patch) {
            total_allowed_loops++;
        }
    }

    if (os_supports_sidecar) {
        if (!disable_sidecar_mac) {
            // Sidecar Models
            if (host_needs_sidecar_patch) {
                total_allowed_loops++;
            }
            if (os_supports_airplay_to_mac) {
                // AirPlay to Mac Models
                if (host_needs_airplay_to_mac_patch) {
                    total_allowed_loops++;
                }
                if (os_supports_universal_control && host_needs_universal_control_patch) {
                    total_allowed_loops++;
                }
                if (os_supports_airplay_to_mac_vmm_checks && host_needs_airplay_to_mac_vmm_patch) {
                    total_allowed_loops++;
                }
            }
        }
        if (allow_sidecar_ipad) {
            total_allowed_loops++;
        }
    }
    if (host_needs_continuity_patch) {
        total_allowed_loops++;
    }
    DBGLOG(MODULE_SHORT, "Total allowed loops: %d", total_allowed_loops);
}

#pragma mark - Boot Arguments

static void detectBootArgs() {
    allow_sidecar_ipad      = checkKernelArgument("-allow_sidecar_ipad");
    disable_sidecar_mac     = checkKernelArgument("-disable_sidecar_mac");
    disable_nightshift      = checkKernelArgument("-disable_nightshift");
    force_universal_control = checkKernelArgument("-force_uni_control");
}

#pragma mark - Patches on start/stop

static void pluginStart() {
    DBGLOG(MODULE_SHORT, "start");
    start_time = mach_absolute_time();
    detectBootArgs();
    detectMachineProperties();
    detectSupportedPatchSets();
    detectNumberOfPatches();
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
    KernelVersion::Sierra,
    KernelVersion::Sonoma,
    pluginStart
};
