//
//  kern_args.hpp
//  FeatureUnlock
//
//  Created by Mykola Grymalyuk on 2021-11-13.
//  Copyright © 2021 Khronokernel. All rights reserved.
//

// Arguments used to determine whether to enable/disable specific patches

#pragma mark - Boot Arguments

static const char allow_assetcache = checkKernelArgument("-allow_assetcache");
static const char allow_sidecar_ipad = checkKernelArgument("-allow_sidecar_ipad");
static const char disable_sidecar_mac = checkKernelArgument("-disable_sidecar_mac");
static const char disable_nightshift = checkKernelArgument("-disable_nightshift");
