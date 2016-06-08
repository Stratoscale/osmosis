osmosis_SOURCES = $(shell find cpp/Osmosis/ -type f -iname "*.cpp" -not -iname "Test*.cpp" -not -iname "*main.cpp")
TARGET_osmosis = ${osmosis_SOURCES} cpp/Osmosis/main.cpp
osmosis_LIBRARIES = -pthread \
	$(STATIC_BOOST_LIBS_DIR)/libboost_regex$(BOOST_MT).a \
	$(STATIC_BOOST_LIBS_DIR)/libboost_filesystem$(BOOST_MT).a \
	$(STATIC_BOOST_LIBS_DIR)/libboost_system$(BOOST_MT).a \
	$(STATIC_BOOST_LIBS_DIR)/libboost_program_options$(BOOST_MT).a \
	-lcrypto \
	$(STATIC_BOOST_LIBS_DIR)/libboost_thread$(BOOST_MT).a

TARGET_testtaskqueue = cpp/Common/WhiteboxTests/testtaskqueue.cpp
testtaskqueue_LIBRARIES = -pthread $(STATIC_BOOST_LIBS_DIR)/libboost_system$(BOOST_MT).a

TARGET_runner_TestProtocolVersionNegotiator = \
				build/cpp/Unittests/runner_TestProtocolVersionNegotiator.cpp \
				cpp/Unittests/MockTCPConnection.cpp
runner_TestProtocolVersionNegotiator_LIBRARIES = ${osmosis_LIBRARIES}
