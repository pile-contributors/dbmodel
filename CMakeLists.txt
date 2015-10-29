if (NOT DBMODEL_BUILD_MODE)
    set (DBMODEL_BUILD_MODE STATIC)
endif ()

include(pile_support)
pileInclude (DbModel)
dbmodelInit(${DBMODEL_BUILD_MODE})
