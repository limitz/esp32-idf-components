
if (CONFIG_LMTZ_UNIQUE_ID_SRC_MANUAL)
	idf_component_register(INCLUDE_DIRS . ..)
else (CONFIG_LMTZ_UNIQUE_ID_SRC_MANUAL)
	idf_component_register(SRCS "unique_id.c"
			INCLUDE_DIRS . ..)
endif (CONFIG_LMTZ_UNIQUE_ID_SRC_MANUAL)


