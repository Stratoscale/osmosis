TARGET_osmosis = cpp/Osmosis/main.cpp
osmosis_LIBRARIES = -lboost_regex -lboost_filesystem -lboost_system -lboost_program_options -lcrypto

TARGET_testtaskqueue = cpp/Common/WhiteboxTests/testtaskqueue.cpp
testtaskqueue_LIBRARIES = -pthread
