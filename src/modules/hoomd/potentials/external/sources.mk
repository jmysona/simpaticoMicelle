modules_hoomd_potentials_external_SRCS=$(SRC_DIR)/modules/hoomd/potentials/external/HoomdExternalFactory.cpp \
    $(SRC_DIR)/modules/hoomd/potentials/external/HoomdLamellarOrderingExternal.cpp \
    $(SRC_DIR)/modules/hoomd/potentials/external/HoomdLocalLamellarOrderingExternal.cpp 


modules_hoomd_potentials_external_OBJS=$(modules_hoomd_potentials_external_SRCS:.cpp=.o)

