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
#define INVA_STATE      "A"
#define INVB_STATE      "B"
#define INVC_STATE      "C"
#define INVD_STATE      "D"
#define INVA_ERROR      "aberror"
#define INVB_ERROR      "bberror"
#define INVC_ERROR      "cberror"
#define INVD_ERROR      "dberror"
#define INVA_DIAGNOSTIC "da"
#define INVB_DIAGNOSTIC "db"
#define INVC_DIAGNOSTIC "dc"
#define INVD_DIAGNOSTIC "dd"
#define INVA_ON         "ia"
#define INVB_ON         "ib"
#define INVC_ON         "ic"
#define INVD_ON         "id"

void amk_telemetry_update();

#endif // AMK_H