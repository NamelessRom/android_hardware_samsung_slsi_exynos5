/*
**
** Copyright 2013, Samsung Electronics Co. LTD
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef EXYNOS_CAMERA_LUT_IMX240_H
#define EXYNOS_CAMERA_LUT_IMX240_H

/* -------------------------
    SIZE_RATIO_16_9 = 0,
    SIZE_RATIO_4_3,
    SIZE_RATIO_1_1,
    SIZE_RATIO_3_2,
    SIZE_RATIO_5_4,
    SIZE_RATIO_5_3,
    SIZE_RATIO_11_9,
    SIZE_RATIO_END
----------------------------
    RATIO_ID,
    SENSOR_W   = 1,
    SENSOR_H,
    BNS_W,
    BNS_H,
    BCROP_W,
    BCROP_H,
    BDS_W,
    BDS_H,
    TARGET_W,
    TARGET_H,
-----------------------------*/

static int PREVIEW_SIZE_LUT_IMX240[][SIZE_OF_LUT] = //Ok
{
    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
      5328, 3000,       /* [sensor ] *//* w/o margin */
      2664, 1500,       /* [bns    ] *//* if sensor do not support bns, set this value equals sensor */
      2648, 1490,       /* [bcrop  ] */
      1920, 1080,       /* [bds    ] */
      1920, 1080        /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
      4000, 3000,       /* [sensor ] */
      2000, 1500,       /* [bns    ] */
      1986, 1490,       /* [bcrop  ] */
      1440, 1080,       /* [bds    ] */
      1440, 1080        /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
      3008, 3000,       /* [sensor ] */
      1504, 1500,       /* [bns    ] */
      1488, 1488,       /* [bcrop  ] */
      1088, 1088,       /* [bds    ] */
      1088, 1088        /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
      5328, 3000,       /* [sensor ] */
      2664, 1500,       /* [bns    ] */
      2236, 1490,       /* [bcrop  ] */
      1616, 1080,       /* [bds    ] *//* w=1620, Reduced for 8 pixel align */
      1616, 1080        /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
      4000, 3000,       /* [sensor ] */
      2000, 1500,       /* [bns    ] */
      1862, 1490,       /* [bcrop  ] */
      1344, 1080,       /* [bds    ] *//* w=1350, Increased for 8 pixel align */
      1344, 1080        /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
      5328, 3000,       /* [sensor ] */
      2664, 1500,       /* [bns    ] */
      2484, 1490,       /* [bcrop  ] */
      1792, 1080,       /* [bds    ] */
      1792, 1080        /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
      4000, 3000,       /* [sensor ] */
      2000, 1500,       /* [bns    ] */
      1822, 1490,       /* [bcrop  ] */
      1312, 1080,       /* [bds    ] */
      1312, 1080        /* [target ] */
    }
};

static int PICTURE_SIZE_LUT_IMX240[][SIZE_OF_LUT] = //Ok
{
    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
      5328, 3000,       /* [sensor ] */
      5328, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      5312, 2988,       /* [bds    ] */
      5312, 2988        /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
      4000, 3000,       /* [sensor ] */
      4000, 3000,       /* [bns    ] */
      3984, 2988,       /* [bcrop  ] */
      3984, 2988,       /* [bds    ] */
      3984, 2988        /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
      3008, 3000,       /* [sensor ] */
      3008, 3000,       /* [bns    ] */
      2976, 2976,       /* [bcrop  ] */
      2976, 2976,       /* [bds    ] */
      2976, 2976        /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_IMX240[][SIZE_OF_LUT] = //Ok
{
    /*  16:9 (Single) */
    { SIZE_RATIO_16_9,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    },
    /*  4:3 (Single) */
    { SIZE_RATIO_4_3,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
      5238, 3000,       /* [sensor ] */
      5238, 3000,       /* [bns    ] */
      5312, 2988,       /* [bcrop  ] */
      3840, 2160,       /* [bds    ] */
      3840, 2160        /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_BNS_IMX240[][SIZE_OF_LUT] = //Ok
{
    /*  16:9 (Single) */
    { SIZE_RATIO_16_9,
      5328, 3000,       /* [sensor ] */
      3552, 2000,       /* [bns    ] */
      3536, 1988,       /* [bcrop  ] */
      1920, 1080,       /* [bds    ] */
      1920, 1080        /* [target ] */
    },
    /*  4:3 (Single) */
    { SIZE_RATIO_4_3,
      4000, 3000,       /* [sensor ] */
      2664, 2000,       /* [bns    ] */
      2650, 1988,       /* [bcrop  ] */
      1440, 1080,       /* [bds    ] */
      1440, 1080        /* [target ] */
    },
    /*  1:1 (Single, Dual) */
    { SIZE_RATIO_1_1,
      3008, 3000,       /* [sensor ] */
      2004, 2000,       /* [bns    ] */
      1988, 1988,       /* [bcrop  ] */
      1088, 1088,       /* [bds    ] */
      1088, 1088        /* [target ] */
    },
    /*  3:2 (Single, Dual) */
    { SIZE_RATIO_3_2,
      5328, 3000,       /* [sensor ] */
      3552, 2000,       /* [bns    ] */
      2982, 1988,       /* [bcrop  ] */
      1616, 1080,       /* [bds    ] */
      1616, 1080        /* [target ] */
    },
    /*  5:4 (Single, Dual) */
    { SIZE_RATIO_5_4,
      4000, 3000,       /* [sensor ] */
      2664, 2000,       /* [bns    ] */
      2486, 1988,       /* [bcrop  ] */
      1344, 1080,       /* [bds    ] */
      1344, 1080        /* [target ] */
    },
    /*  5:3 (Single, Dual) */
    { SIZE_RATIO_5_3,
      5328, 3000,       /* [sensor ] */
      3552, 2000,       /* [bns    ] */
      3314, 1988,       /* [bcrop  ] */
      1792, 1080,       /* [bds    ] */
      1792, 1080        /* [target ] */
    },
    /*  11:9 (Single, Dual) */
    { SIZE_RATIO_11_9,
      4000, 3000,       /* [sensor ] */
      2664, 2000,       /* [bns    ] */
      2430, 1988,       /* [bcrop  ] */
      1312, 1080,       /* [bds    ] */
      1312, 1080        /* [target ] */
    }
};

static int VIDEO_SIZE_LUT_HIGH_SPEED_IMX240[][SIZE_OF_LUT] = //Ok
{
    /*   HD_60  16:9 (Single) */
    { SIZE_RATIO_16_9,
      2664,  1500,       /* [sensor ] */
      2664,  1500,       /* [bns    ] */
      2648,  1490,       /* [bcrop  ] */
      1920,  1080,       /* [bds    ] */
      1920,  1080        /* [target ] */
    }
};

static int FASTEN_AE_SIZE_IMX240[][SIZE_OF_LUT] = //Ok
{
    /* 16:9 (Single, Dual) */
    { SIZE_RATIO_16_9,
      1328,  748,       /* [sensor ] */
      1328,  748,       /* [bns    ] */
      1312,  738,       /* [bcrop  ] */
      1280,  720,       /* [bds    ] */
      1280,  720        /* [target ] */
    },
    /*  4:3 (Single, Dual) */
    { SIZE_RATIO_4_3,
      1328,  748,       /* [sensor ] */                
      1328,  748,       /* [bns    ] */ 
       960,  720,       /* [bcrop  ] */
       960,  720,       /* [bds    ] */
       960,  720        /* [target ] */
    }
};
#endif
