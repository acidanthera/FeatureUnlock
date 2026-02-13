//
// kern_uc_model.hpp
// FeatureUnlock.kext
//
// Universal Control Model Spoofing
// Fixes Universal Control on blacklisted Macs (MacBookPro12,1, etc.)
//

#ifndef kern_uc_model_hpp
#define kern_uc_model_hpp

#include <Headers/kern_api.hpp>

class UCModelSpoof {
public:
    /**
     * Initialize Universal Control model spoofing
     */
    void init();
    
    /**
     * Check if current Mac model is blacklisted for UC
     */
    static bool isBlacklistedModel();
    
    /**
     * Process kernel patches for Universal Control
     */
    void processKernelPatches(KernelPatcher &patcher);
    
private:
    /**
     * Hook sysctlbyname to spoof model for UC processes
     */
    bool hookSysctlByName(KernelPatcher &patcher);
    
    /**
     * Original sysctlbyname function pointer
     */
    static int (*orgSysctlbyname)(const char *name, void *oldp, size_t *oldlenp,
                                   void *newp, size_t newlen);
    
    /**
     * Hooked sysctlbyname implementation
     */
    static int hookedSysctlbyname(const char *name, void *oldp, size_t *oldlenp,
                                   void *newp, size_t newlen);
};

#endif /* kern_uc_model_hpp */
