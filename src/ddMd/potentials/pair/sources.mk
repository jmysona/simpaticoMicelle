ddMd_potentials_pair_SRCS= \
    $(SRC_DIR)/ddMd/potentials/pair/PairPotential.cpp \
    $(SRC_DIR)/ddMd/potentials/pair/PairFactory.cpp \
    $(SRC_DIR)/ddMd/potentials/pair/DpdSoftPair.cpp \
    $(SRC_DIR)/ddMd/potentials/pair/LJPair.cpp

ddMd_potentials_pair_OBJS=$(ddMd_potentials_pair_SRCS:.cpp=.o)

