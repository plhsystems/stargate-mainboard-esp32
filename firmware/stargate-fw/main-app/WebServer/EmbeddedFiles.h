#ifndef _EMBEDDEDFILES_H_
#define _EMBEDDEDFILES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
    EF_EFLAGS_None = 0,
    EF_EFLAGS_GZip = 1,
} EF_EFLAGS;

typedef struct
{
    const char* filename;
    uint32_t length;
    EF_EFLAGS flags;
    const uint8_t* start_addr;
} EF_SFile;

typedef enum
{
    EF_EFILE_FAVICON_ICO = 0,    /*!< @brief File: favicon.ico (size: 7 KB) */
    EF_EFILE_SETUP_INDEX_HTML = 1,    /*!< @brief File: setup/index.html (size: 6 KB) */
    EF_EFILE_SETUP_SETTINGS_HTML = 2,    /*!< @brief File: setup/settings.html (size: 2 KB) */
    EF_EFILE_SETUP_JS_SETTINGS_JS = 3,    /*!< @brief File: setup/js/settings.js (size: 415  B) */
    EF_EFILE_SETUP_JS_APP_JS = 4,    /*!< @brief File: setup/js/app.js (size: 3 KB) */
    EF_EFILE_COMMON_CSS_CONTENT_CSS = 5,    /*!< @brief File: common/css/content.css (size: 151  B) */
    EF_EFILE_COMMON_JS_APIURL_JS = 6,    /*!< @brief File: common/js/apiurl.js (size: 2 KB) */
    EF_EFILE_COMMON_JS_VUE_MIN_JS = 7,    /*!< @brief File: common/js/vue.min.js (size: 92 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_STARGATE_CSS = 8,    /*!< @brief File: galaxies/milkyway/stargate.css (size: 20 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_README_MD = 9,    /*!< @brief File: galaxies/milkyway/README.md (size: 3 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_INDEX_HTML = 10,    /*!< @brief File: galaxies/milkyway/index.html (size: 0  B) */
    EF_EFILE_GALAXIES_MILKYWAY_STARGATE_JS = 11,    /*!< @brief File: galaxies/milkyway/stargate.js (size: 7 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_STARGATE_HTML = 12,    /*!< @brief File: galaxies/milkyway/stargate.html (size: 16 KB) */
    EF_EFILE_GALAXIES_PEGASUS_INDEX_HTML = 13,    /*!< @brief File: galaxies/pegasus/index.html (size: 0  B) */
    EF_EFILE_GALAXIES_UNIVERSE_DIAL_CSS = 14,    /*!< @brief File: galaxies/universe/dial.css (size: 15 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_DIAL_HTML = 15,    /*!< @brief File: galaxies/universe/dial.html (size: 5 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_INDEX_HTML = 16,    /*!< @brief File: galaxies/universe/index.html (size: 625  B) */
    EF_EFILE_GALAXIES_UNIVERSE_DIAL_JS = 17,    /*!< @brief File: galaxies/universe/dial.js (size: 14 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_JS_APP_JS = 18,    /*!< @brief File: galaxies/universe/js/app.js (size: 20 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_028_SVG = 19,    /*!< @brief File: galaxies/universe/glyphs/028.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_012_SVG = 20,    /*!< @brief File: galaxies/universe/glyphs/012.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_002_SVG = 21,    /*!< @brief File: galaxies/universe/glyphs/002.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_021_SVG = 22,    /*!< @brief File: galaxies/universe/glyphs/021.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_034_SVG = 23,    /*!< @brief File: galaxies/universe/glyphs/034.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_001_SVG = 24,    /*!< @brief File: galaxies/universe/glyphs/001.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_026_SVG = 25,    /*!< @brief File: galaxies/universe/glyphs/026.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_014_SVG = 26,    /*!< @brief File: galaxies/universe/glyphs/014.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_036_SVG = 27,    /*!< @brief File: galaxies/universe/glyphs/036.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_031_SVG = 28,    /*!< @brief File: galaxies/universe/glyphs/031.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_007_SVG = 29,    /*!< @brief File: galaxies/universe/glyphs/007.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_009_SVG = 30,    /*!< @brief File: galaxies/universe/glyphs/009.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_006_SVG = 31,    /*!< @brief File: galaxies/universe/glyphs/006.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_005_SVG = 32,    /*!< @brief File: galaxies/universe/glyphs/005.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_020_SVG = 33,    /*!< @brief File: galaxies/universe/glyphs/020.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_027_SVG = 34,    /*!< @brief File: galaxies/universe/glyphs/027.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_010_SVG = 35,    /*!< @brief File: galaxies/universe/glyphs/010.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_019_SVG = 36,    /*!< @brief File: galaxies/universe/glyphs/019.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_004_SVG = 37,    /*!< @brief File: galaxies/universe/glyphs/004.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_033_SVG = 38,    /*!< @brief File: galaxies/universe/glyphs/033.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_029_SVG = 39,    /*!< @brief File: galaxies/universe/glyphs/029.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_030_SVG = 40,    /*!< @brief File: galaxies/universe/glyphs/030.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_003_SVG = 41,    /*!< @brief File: galaxies/universe/glyphs/003.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_025_SVG = 42,    /*!< @brief File: galaxies/universe/glyphs/025.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_035_SVG = 43,    /*!< @brief File: galaxies/universe/glyphs/035.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_022_SVG = 44,    /*!< @brief File: galaxies/universe/glyphs/022.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_011_SVG = 45,    /*!< @brief File: galaxies/universe/glyphs/011.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_018_SVG = 46,    /*!< @brief File: galaxies/universe/glyphs/018.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_023_SVG = 47,    /*!< @brief File: galaxies/universe/glyphs/023.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_013_SVG = 48,    /*!< @brief File: galaxies/universe/glyphs/013.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_032_SVG = 49,    /*!< @brief File: galaxies/universe/glyphs/032.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_024_SVG = 50,    /*!< @brief File: galaxies/universe/glyphs/024.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_008_SVG = 51,    /*!< @brief File: galaxies/universe/glyphs/008.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_015_SVG = 52,    /*!< @brief File: galaxies/universe/glyphs/015.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_017_SVG = 53,    /*!< @brief File: galaxies/universe/glyphs/017.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_UNIVERSE_GLYPHS_016_SVG = 54,    /*!< @brief File: galaxies/universe/glyphs/016.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A36_SVG = 55,    /*!< @brief File: galaxies/pegasus/glyphs/A36.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A35_SVG = 56,    /*!< @brief File: galaxies/pegasus/glyphs/A35.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A05_SVG = 57,    /*!< @brief File: galaxies/pegasus/glyphs/A05.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A03_SVG = 58,    /*!< @brief File: galaxies/pegasus/glyphs/A03.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A30_SVG = 59,    /*!< @brief File: galaxies/pegasus/glyphs/A30.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A13_SVG = 60,    /*!< @brief File: galaxies/pegasus/glyphs/A13.svg (size: 11 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A16_SVG = 61,    /*!< @brief File: galaxies/pegasus/glyphs/A16.svg (size: 5 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A27_SVG = 62,    /*!< @brief File: galaxies/pegasus/glyphs/A27.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A21_SVG = 63,    /*!< @brief File: galaxies/pegasus/glyphs/A21.svg (size: 12 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A28_SVG = 64,    /*!< @brief File: galaxies/pegasus/glyphs/A28.svg (size: 6 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A26_SVG = 65,    /*!< @brief File: galaxies/pegasus/glyphs/A26.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A32_SVG = 66,    /*!< @brief File: galaxies/pegasus/glyphs/A32.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A25_SVG = 67,    /*!< @brief File: galaxies/pegasus/glyphs/A25.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A02_SVG = 68,    /*!< @brief File: galaxies/pegasus/glyphs/A02.svg (size: 7 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A06_SVG = 69,    /*!< @brief File: galaxies/pegasus/glyphs/A06.svg (size: 10 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A29_SVG = 70,    /*!< @brief File: galaxies/pegasus/glyphs/A29.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A08_SVG = 71,    /*!< @brief File: galaxies/pegasus/glyphs/A08.svg (size: 10 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A24_SVG = 72,    /*!< @brief File: galaxies/pegasus/glyphs/A24.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A31_SVG = 73,    /*!< @brief File: galaxies/pegasus/glyphs/A31.svg (size: 7 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A18_SVG = 74,    /*!< @brief File: galaxies/pegasus/glyphs/A18.svg (size: 7 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A15_SVG = 75,    /*!< @brief File: galaxies/pegasus/glyphs/A15.svg (size: 7 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A34_SVG = 76,    /*!< @brief File: galaxies/pegasus/glyphs/A34.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A20_SVG = 77,    /*!< @brief File: galaxies/pegasus/glyphs/A20.svg (size: 11 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A14_SVG = 78,    /*!< @brief File: galaxies/pegasus/glyphs/A14.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A10_SVG = 79,    /*!< @brief File: galaxies/pegasus/glyphs/A10.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A19_SVG = 80,    /*!< @brief File: galaxies/pegasus/glyphs/A19.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A04_SVG = 81,    /*!< @brief File: galaxies/pegasus/glyphs/A04.svg (size: 5 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A12_SVG = 82,    /*!< @brief File: galaxies/pegasus/glyphs/A12.svg (size: 7 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A17_SVG = 83,    /*!< @brief File: galaxies/pegasus/glyphs/A17.svg (size: 11 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A07_SVG = 84,    /*!< @brief File: galaxies/pegasus/glyphs/A07.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A09_SVG = 85,    /*!< @brief File: galaxies/pegasus/glyphs/A09.svg (size: 5 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A11_SVG = 86,    /*!< @brief File: galaxies/pegasus/glyphs/A11.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A22_SVG = 87,    /*!< @brief File: galaxies/pegasus/glyphs/A22.svg (size: 6 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A23_SVG = 88,    /*!< @brief File: galaxies/pegasus/glyphs/A23.svg (size: 10 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A01_SVG = 89,    /*!< @brief File: galaxies/pegasus/glyphs/A01.svg (size: 8 KB) */
    EF_EFILE_GALAXIES_PEGASUS_GLYPHS_A33_SVG = 90,    /*!< @brief File: galaxies/pegasus/glyphs/A33.svg (size: 9 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_028_SVG = 91,    /*!< @brief File: galaxies/milkyway/glyphs/028.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_012_SVG = 92,    /*!< @brief File: galaxies/milkyway/glyphs/012.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_002_SVG = 93,    /*!< @brief File: galaxies/milkyway/glyphs/002.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_021_SVG = 94,    /*!< @brief File: galaxies/milkyway/glyphs/021.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_034_SVG = 95,    /*!< @brief File: galaxies/milkyway/glyphs/034.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_001_SVG = 96,    /*!< @brief File: galaxies/milkyway/glyphs/001.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_026_SVG = 97,    /*!< @brief File: galaxies/milkyway/glyphs/026.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_039_SVG = 98,    /*!< @brief File: galaxies/milkyway/glyphs/039.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_014_SVG = 99,    /*!< @brief File: galaxies/milkyway/glyphs/014.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_036_SVG = 100,    /*!< @brief File: galaxies/milkyway/glyphs/036.svg (size: 873  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_031_SVG = 101,    /*!< @brief File: galaxies/milkyway/glyphs/031.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_038_SVG = 102,    /*!< @brief File: galaxies/milkyway/glyphs/038.svg (size: 820  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_007_SVG = 103,    /*!< @brief File: galaxies/milkyway/glyphs/007.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_009_SVG = 104,    /*!< @brief File: galaxies/milkyway/glyphs/009.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_006_SVG = 105,    /*!< @brief File: galaxies/milkyway/glyphs/006.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_005_SVG = 106,    /*!< @brief File: galaxies/milkyway/glyphs/005.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_037_SVG = 107,    /*!< @brief File: galaxies/milkyway/glyphs/037.svg (size: 995  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_020_SVG = 108,    /*!< @brief File: galaxies/milkyway/glyphs/020.svg (size: 925  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_027_SVG = 109,    /*!< @brief File: galaxies/milkyway/glyphs/027.svg (size: 985  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_010_SVG = 110,    /*!< @brief File: galaxies/milkyway/glyphs/010.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_019_SVG = 111,    /*!< @brief File: galaxies/milkyway/glyphs/019.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_004_SVG = 112,    /*!< @brief File: galaxies/milkyway/glyphs/004.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_033_SVG = 113,    /*!< @brief File: galaxies/milkyway/glyphs/033.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_029_SVG = 114,    /*!< @brief File: galaxies/milkyway/glyphs/029.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_030_SVG = 115,    /*!< @brief File: galaxies/milkyway/glyphs/030.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_003_SVG = 116,    /*!< @brief File: galaxies/milkyway/glyphs/003.svg (size: 2 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_025_SVG = 117,    /*!< @brief File: galaxies/milkyway/glyphs/025.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_035_SVG = 118,    /*!< @brief File: galaxies/milkyway/glyphs/035.svg (size: 849  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_022_SVG = 119,    /*!< @brief File: galaxies/milkyway/glyphs/022.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_011_SVG = 120,    /*!< @brief File: galaxies/milkyway/glyphs/011.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_018_SVG = 121,    /*!< @brief File: galaxies/milkyway/glyphs/018.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_023_SVG = 122,    /*!< @brief File: galaxies/milkyway/glyphs/023.svg (size: 888  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_013_SVG = 123,    /*!< @brief File: galaxies/milkyway/glyphs/013.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_032_SVG = 124,    /*!< @brief File: galaxies/milkyway/glyphs/032.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_024_SVG = 125,    /*!< @brief File: galaxies/milkyway/glyphs/024.svg (size: 814  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_008_SVG = 126,    /*!< @brief File: galaxies/milkyway/glyphs/008.svg (size: 1 KB) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_015_SVG = 127,    /*!< @brief File: galaxies/milkyway/glyphs/015.svg (size: 1010  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_017_SVG = 128,    /*!< @brief File: galaxies/milkyway/glyphs/017.svg (size: 1008  B) */
    EF_EFILE_GALAXIES_MILKYWAY_GLYPHS_016_SVG = 129,    /*!< @brief File: galaxies/milkyway/glyphs/016.svg (size: 1 KB) */
    EF_EFILE_COUNT = 130
} EF_EFILE;

/*! @brief Check if compressed flag is active */
#define EF_ISFILECOMPRESSED(x) ((x & EF_EFLAGS_GZip) == EF_EFLAGS_GZip)

extern const EF_SFile EF_g_files[EF_EFILE_COUNT];
extern const uint8_t EF_g_blobs[];

#ifdef __cplusplus
}
#endif

#endif
