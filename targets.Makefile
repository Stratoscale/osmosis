TARGET_osmosis = cpp/Osmosis/main.cpp
osmosis_LIBRARIES = -pthread -lboost_regex$(BOOST_MT) -lboost_filesystem$(BOOST_MT) -lboost_system$(BOOST_MT) -lboost_program_options$(BOOST_MT) -lcrypto -lboost_thread$(BOOST_MT)

TARGET_testtaskqueue = cpp/Common/WhiteboxTests/testtaskqueue.cpp
testtaskqueue_LIBRARIES = -pthread -lboost_system$(BOOST_MT)
