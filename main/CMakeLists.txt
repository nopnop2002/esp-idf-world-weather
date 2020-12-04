set(srcs "main.c"
	"ili9340"
	"fontx.c"
	"m5stack.c"
	)

idf_component_register(SRCS ${srcs}
	INCLUDE_DIRS "."
	EMBED_TXTFILES metaweather_com_root_cert.pem)


# Create a SPIFFS image from the contents of the 'font' directory
# that fits the partition named 'storage'. FLASH_IN_PROJECT indicates that
# the generated image should be flashed when the entire project is flashed to
# the target with 'idf.py -p PORT flash
spiffs_create_partition_image(storage0 ../fonts FLASH_IN_PROJECT)
spiffs_create_partition_image(storage1 ../images1 FLASH_IN_PROJECT)
spiffs_create_partition_image(storage2 ../images2 FLASH_IN_PROJECT)

