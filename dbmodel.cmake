
# enable/disable cmake debug messages related to this pile
set (DBMODEL_DEBUG_MSG ON)

# make sure support code is present; no harm
# in including it twice; the user, however, should have used
# pileInclude() from pile_support.cmake module.
include(pile_support)

# initialize this module
macro    (dbmodelInit
          ref_cnt_use_mode)

    # default name
    if (NOT DBMODEL_INIT_NAME)
        set(DBMODEL_INIT_NAME "DbModel")
    endif ()

    # compose the list of headers and sources
    set(DBMODEL_HEADERS
        "dbmodel.h"
        "dbmodeltbl.h"
        "dbmodelcol.h")
    set(DBMODEL_SOURCES
        "dbmodel.cc"
        "dbmodeltbl.cc"
        "dbmodelprivate.cc"
        "dbmodelcol.cc")
    set(DBDELEGATE_QT_MODS
        Core Sql)

    pileSetSources(
        "${DBMODEL_INIT_NAME}"
        "${DBMODEL_HEADERS}"
        "${DBMODEL_SOURCES}")

    pileSetCommon(
        "${DBMODEL_INIT_NAME}"
        "0;0;1;d"
        "ON"
        "${ref_cnt_use_mode}"
        ""
        "category1"
        "tag1;tag2")

endmacro ()
