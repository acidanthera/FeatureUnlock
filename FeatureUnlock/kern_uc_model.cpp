//
// kern_uc_model.cpp
// FeatureUnlock.kext
//
// Universal Control Model Spoofing
// Intercepts sysctlbyname for UC processes to spoof blacklisted models
//

#include "kern_uc_model.hpp"
#include <sys/sysctl.h>
#include <sys/proc.h>

// Static member initialization
int (*UCModelSpoof::orgSysctlbyname)(const char *, void *, size_t *, void *, size_t) = nullptr;

// Shared blacklist of models that don't support Universal Control
static const char *UC_BLACKLISTED_MODELS[] = {
    "MacBookPro12,1",
    "MacBookPro11,4",
    "MacBookPro11,5",
    "MacBookAir7,1",
    "MacBookAir7,2",
    "iMac16,1",
    "iMac16,2",
    "Macmini7,1",
    "MacPro6,1"
};

void UCModelSpoof::init() {
    DBGLOG("featureunlock", "UC: Initializing Universal Control model spoofing");
    // TODO: Add UCModelSpoof-specific initialization here if needed in future.
}

bool UCModelSpoof::isBlacklistedModel() {
    char model[64] = {};
    size_t len = sizeof(model);
    
    if (sysctlbyname("hw.model", model, &len, nullptr, 0) != 0) {
        return false;
    }
    
    for (size_t i = 0; i < arrsize(UC_BLACKLISTED_MODELS); i++) {
        if (strcmp(model, UC_BLACKLISTED_MODELS[i]) == 0) {
            DBGLOG("featureunlock", "UC: Detected blacklisted model: %s", model);
            return true;
        }
    }
    
    return false;
}

void UCModelSpoof::processKernelPatches(KernelPatcher &patcher) {
    if (!isBlacklistedModel()) {
        DBGLOG("featureunlock", "UC: Model not blacklisted, skipping patch");
        return;
    }
    
    if (!hookSysctlByName(patcher)) {
        SYSLOG("featureunlock", "UC: Failed to hook sysctlbyname");
    }
}

bool UCModelSpoof::hookSysctlByName(KernelPatcher &patcher) {
    DBGLOG("featureunlock", "UC: Attempting to hook sysctlbyname");
    
    // Find sysctlbyname in the kernel
    mach_vm_address_t kern_sysctlbyname = patcher.solveSymbol(
        KernelPatcher::KernelID,
        "_sysctlbyname"
    );
    
    if (!kern_sysctlbyname) {
        SYSLOG("featureunlock", "UC: Failed to find sysctlbyname symbol");
        return false;
    }
    
    // Route the function to our hook
    if (patcher.routeFunction(
        kern_sysctlbyname,
        reinterpret_cast<mach_vm_address_t>(hookedSysctlbyname),
        reinterpret_cast<mach_vm_address_t *>(&orgSysctlbyname)
    )) {
        DBGLOG("featureunlock", "UC: Successfully hooked sysctlbyname");
        return true;
    }
    
    SYSLOG("featureunlock", "UC: Failed to route sysctlbyname");
    return false;
}

int UCModelSpoof::hookedSysctlbyname(const char *name, void *oldp, size_t *oldlenp,
                                      void *newp, size_t newlen) {
    // Call original function first
    int result = orgSysctlbyname(name, oldp, oldlenp, newp, newlen);
    
    // Only process successful hw.model queries
    if (result != 0 || name == nullptr || oldp == nullptr || oldlenp == nullptr ||
        strcmp(name, "hw.model") != 0) {
        return result;
    }
    
    // Get calling process name
    proc_t proc = current_proc();
    if (proc == nullptr) {
        return result;
    }
    
    char procname[MAXCOMLEN + 1] = {};
    proc_name(proc_pid(proc), procname, sizeof(procname));
    
    // Check if caller is a Universal Control-related process
    const char *uc_processes[] = {
        "UniversalControl",
        "sharingd",
        "rapportd",
        "ControlCenter"
    };
    
    bool isUCProcess = false;
    for (size_t i = 0; i < arrsize(uc_processes); i++) {
        if (strcmp(procname, uc_processes[i]) == 0) {
            isUCProcess = true;
            break;
        }
    }
    
    if (!isUCProcess) {
        return result;
    }
    
    // Check if the returned model is blacklisted
    char *model = static_cast<char *>(oldp);
    
    // Ensure null termination
    if (*oldlenp == 0 || model[*oldlenp - 1] != '\0') {
        DBGLOG("featureunlock", "UC: hw.model buffer not null-terminated, skipping spoof");
        return result;
    }
    
    bool isBlacklisted = false;
    for (size_t i = 0; i < arrsize(UC_BLACKLISTED_MODELS); i++) {
        if (strcmp(model, UC_BLACKLISTED_MODELS[i]) == 0) {
            isBlacklisted = true;
            break;
        }
    }
    
    if (!isBlacklisted) {
        return result;
    }
    
    // Spoof to MacBookPro16,4 (2019 16" MBP - more future-proof)
    const char *spoofed_model = "MacBookPro16,4";
    size_t spoofed_len = strlen(spoofed_model) + 1;
    if (*oldlenp < spoofed_len) {
        DBGLOG("featureunlock", "UC: Buffer too small (%zu < %zu)", *oldlenp, spoofed_len);
        return result;
    }
    strlcpy(model, spoofed_model, *oldlenp);
    *oldlenp = spoofed_len;
    
    DBGLOG("featureunlock", "UC: Spoofed model to %s for process %s",
           spoofed_model, procname);
    
    return result;
}
