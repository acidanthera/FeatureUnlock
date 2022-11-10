//
//  kern_model_info.cpp
//  FeatureUnlock.kext
//
//  Copyright © 2022 Mykola Grymalyuk. All rights reserved.
//


#pragma mark - MacBooks

static char *macbook_legacy_models[] = {
    (char *)"MacBook4,1",
    (char *)"MacBook5,1", (char *)"MacBook5,2",
    (char *)"MacBook6,1",
    (char *)"MacBook7,1"
};

static char *macbook_modern_models[] = {
    (char *)"MacBook8,1"
};

#pragma mark - MacBook Airs

static char *macbookair_legacy_models[] = {
    (char *)"MacBookAir2,1",
    (char *)"MacBookAir3,1", (char *)"MacBookAir3,2",
    (char *)"MacBookAir4,1", (char *)"MacBookAir4,2"
};

static char *macbookair_2012_models[] = {
    (char *)"MacBookAir5,1", (char *)"MacBookAir5,2"
};

static char *macbookair_2013_models[] = {
    (char *)"MacBookAir6,1", (char *)"MacBookAir6,2"
};

static char *macbookair_2015_models[] = {
    (char *)"MacBookAir7,1", (char *)"MacBookAir7,2"
};

#pragma mark - MacBook Pros

static char *macbookpro_legacy_models[] = {
    (char *)"MacBookPro4,1",
    (char *)"MacBookPro5,1", (char *)"MacBookPro5,2", (char *)"MacBookPro5,3", (char *)"MacBookPro5,4", (char *)"MacBookPro5,5",
    (char *)"MacBookPro6,1", (char *)"MacBookPro6,2",
    (char *)"MacBookPro7,1",
    (char *)"MacBookPro8,1", (char *)"MacBookPro8,2", (char *)"MacBookPro8,3"
};

static char *macbookpro_2012_models[] = {
    (char *)"MacBookPro9,1", (char *)"MacBookPro9,2",
    (char *)"MacBookPro10,1", (char *)"MacBookPro10,2"
};

static char *macbookpro_2013_models[] = {
    (char *)"MacBookPro11,1", (char *)"MacBookPro11,2", (char *)"MacBookPro11,3"
};

static char *macbookpro_2015_models[] = {
    (char *)"MacBookPro11,4", (char *)"MacBookPro11,5",
    (char *)"MacBookPro12,1"
};

static char *macbookpro_2016_models[] = {
    (char *)"MacBookPro13,1", (char *)"MacBookPro13,2", (char *)"MacBookPro13,3"
};

static char *macbookpro_2017_models[] = {
    (char *)"MacBookPro14,1", (char *)"MacBookPro14,2", (char *)"MacBookPro14,3"
};

#pragma mark - iMacs

static char *imac_legacy_models[] = {
    (char *)"iMac7,1",
    (char *)"iMac8,1",
    (char *)"iMac9,1",
    (char *)"iMac10,1",
    (char *)"iMac11,1", (char *)"iMac11,2", (char *)"iMac11,3",
    (char *)"iMac12,1", (char *)"iMac12,2",
};

static char *imac_2012_models[] = {
    (char *)"iMac13,1", (char *)"iMac13,2", (char *)"iMac13,3"
};

static char *imac_2013_models[] = {
    (char *)"iMac14,1", (char *)"iMac14,2", (char *)"iMac14,3", (char *)"iMac14,4"
};

static char *imac_2014_models[] = {
    (char *)"iMac15,1"
};

static char *imac_2015_broadwell_models[] = {
    (char *)"iMac16,1", (char *)"iMac16,2"
};

static char *imac_2015_2017_models[] = {
    (char *)"iMac17,1",
    (char *)"iMac18,1", (char *)"iMac18,2", (char *)"iMac18,3"
};

#pragma mark - Mac Pros

static char *macpro_legacy_models[] = {
    (char *)"MacPro3,1",
    (char *)"MacPro4,1",
    (char *)"MacPro5,1"
};

static char *macpro_2010_2012_models[] = {
    (char *)"MacPro5,1",
};

static char *macpro_2013_models[] = {
    (char *)"MacPro6,1"
};

#pragma mark - Mac minis

static char *macmini_legacy_models[] = {
    (char *)"Macmini3,1",
    (char *)"Macmini4,1",
    (char *)"Macmini5,1", (char *)"Macmini5,2", (char *)"Macmini5,3",
};

static char *macmini_2012_models[] = {
    (char *)"Macmini6,1", (char *)"Macmini6,2",
};

static char *macmini_2014_models[] = {
    (char *)"Macmini7,1"
};

static char *macmini_2018_models[] = {
    (char *)"Macmini8,1"
};
