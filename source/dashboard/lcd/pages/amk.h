#ifndef AMK_H
#define AMK_H

/**
 * @file amk.h
 * @brief AMK page implementation
 *
 * @author Amruth Nadimpally (nadimpaa@purdue.edu)
 * @author Aditya Saini (sain91@purdue.edu)
 */

#define AMK_STRING "amk"

// Nextion object names
#define INVA_STATUS            "A"
#define INVB_STATUS            "B"
#define INVC_STATUS            "C"
#define INVD_STATUS            "D"
#define INVA_BERROR            "aberror"
#define INVB_BERROR            "bberror"
#define INVC_BERROR            "cberror"
#define INVD_BERROR            "dberror"
#define INVA_DIAGNOSTIC        "da"
#define INVB_DIAGNOSTIC        "db"
#define INVC_DIAGNOSTIC        "dc"
#define INVD_DIAGNOSTIC        "dd"
#define INVA_ON                "ia"
#define INVB_ON                "ib"
#define INVC_ON                "ic"
#define INVD_ON                "id"

void amk_telemetry_update();

#endif // AMK_H