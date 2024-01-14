#ifndef _EMBEDDEDFILES_H_
#define _EMBEDDEDFILES_H_

#include <stdint.h>

typedef enum
{
    EF_EFLAGS_None = 0,
    EF_EFLAGS_GZip = 1,
} EF_EFLAGS;

typedef struct
{
    const char* strFilename;
    uint32_t u32Length;
    EF_EFLAGS eFlags;
    const uint8_t* pu8StartAddr;
} EF_SFile;

typedef enum
{
    EF_EFILE_FAVICON_ICO = 0,    /*!< @brief File: favicon.ico (size: 7 KB) */
    EF_EFILE_INDEX_HTML = 1,    /*!< @brief File: index.html (size: 606  B) */
    EF_EFILE_CSS_CONTENT_CSS = 2,    /*!< @brief File: css/content.css (size: 3 KB) */
    EF_EFILE_IMG_HOME_SVG = 3,    /*!< @brief File: img/home.svg (size: 1 KB) */
    EF_EFILE_JS_APP_JS = 4,    /*!< @brief File: js/app.js (size: 1 KB) */
    EF_EFILE_JS_VUE_MIN_JS = 5,    /*!< @brief File: js/vue.min.js (size: 92 KB) */
    EF_EFILE_COUNT = 6
} EF_EFILE;

/*! @brief Check if compressed flag is active */
#define EF_ISFILECOMPRESSED(x) ((x & EF_EFLAGS_GZip) == EF_EFLAGS_GZip)

extern const EF_SFile EF_g_sFiles[EF_EFILE_COUNT];
extern const uint8_t EF_g_u8Blobs[];

#endif
