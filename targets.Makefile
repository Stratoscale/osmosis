TARGET_osmosis = cpp/Osmosis/main.cpp
osmosis_LIBRARIES = -lboost_regex-mt -lboost_filesystem-mt -lboost_system-mt -lboost_program_options-mt -lcrypto -lboost_thread-mt

TARGET_testtaskqueue = cpp/Common/WhiteboxTests/testtaskqueue.cpp
testtaskqueue_LIBRARIES = -pthread -lboost_system-mt
