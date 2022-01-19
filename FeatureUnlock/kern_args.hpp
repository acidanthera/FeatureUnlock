//
//  kern_args.hpp
//  FeatureUnlock
//
//  Created by Mykola Grymalyuk on 2021-11-13.
//  Copyright © 2021-2022 Khronokernel. All rights reserved.
//

// Arguments used to determine whether to enable/disable specific patches

#ifndef kern_args_h
#define kern_args_h

extern bool allow_sidecar_ipad;
extern bool disable_sidecar_mac;
extern bool disable_nightshift;

extern bool os_supports_nightshift_old;
extern bool os_supports_nightshift_new;
extern bool os_supports_sidecar;
extern bool os_supports_airplay_to_mac;

extern bool model_is_iMac;
extern bool model_is_iMac_2012;
extern bool model_is_iMac_2013;
extern bool model_is_iMac_2014;
extern bool model_is_iMac_2015_17;
extern bool model_is_MacBook;
extern bool model_is_MacBookAir;
extern bool model_is_MacBookPro;
extern bool model_is_MacBookPro_2016_2017;
extern bool model_is_Macmini;
extern bool model_is_MacPro;

#endif /* kern_args_h */
