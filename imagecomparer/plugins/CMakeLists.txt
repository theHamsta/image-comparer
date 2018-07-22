# GLOB ( *.py PLUGINS )
configure_file( histogram.py ${CMAKE_CURRENT_BINARY_DIR}/histogram.py COPYONLY )
configure_file( diff_image.py ${CMAKE_CURRENT_BINARY_DIR}/diff_image.py COPYONLY )
configure_file( dummy_plugin.py ${CMAKE_CURRENT_BINARY_DIR}/dummy_plugin.py COPYONLY )

install(FILES histogram.py DESTINATION bin/plugins )
install(FILES diff_image.py DESTINATION bin/plugins )
