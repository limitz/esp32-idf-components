idf_component_register(
	SRCS 
		"i2cbus.c" 
		"ht16k33.c" 
		"bma.c" 
		"bma423/bma4.c"
		"bma423/bma423.c"
		"mpr121.c"
		"ft5206.c"
		"axp202.c"
	
	INCLUDE_DIRS 
		. 
		"./bma423"
)

