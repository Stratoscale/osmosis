all: all-targets

CONFIGURATION ?= RELEASE
SHELL ?= /bin/bash
COMMON_CXXFLAGS = -Ic -Icpp -I/usr/include/python2.7 -Ibuild/cpp-netlib-0.11.1-final -Icxxtest -std=gnu++0x -Werror -Wall
COMMON_CFLAGS = -Ic -Werror -Wall

DEBUG_CXXFLAGS = -ggdb -DDEBUG
DEBUG_CFLAGS = $(DEBUG_CXXFLAGS)

RELEASE_CXXFLAGS = -O3
RELEASE_CFLAGS = $(RELEASE_CXXFLAGS)
RELEASE_LDFLAGS = -O3

CXXFLAGS = $(COMMON_CXXFLAGS) $($(CONFIGURATION)_CXXFLAGS)
CFLAGS = $(COMMON_CFLAGS) $($(CONFIGURATION)_CFLAGS)
LDFLAGS = -L/usr/lib/x86_64-linux-gnu $($(CONFIGURATION)_LDFLAGS)

ifneq ($(PROFILE),)
CFLAGS += -pg
LDFLAGS += -pg
endif

STATIC_BOOST_LIBS_DIR = $(shell \
if [ -e /usr/lib/x86_64-linux-gnu/libboost_regex-mt.a ] || [ -e /usr/lib/x86_64-linux-gnu/libboost_regex.a ]; then \
	echo /usr/lib/x86_64-linux-gnu; \
else \
	if [ -e /usr/lib64/libboost_regex-mt.a ] || [ -e /usr/lib64/libboost_regex.a ]; then \
		echo /usr/lib64; \
	else \
		echo BOOST_STATIC_NOT_INSTALLED; \
	fi; \
fi)
ifeq "$(STATIC_BOOST_LIBS_DIR)" "BOOST_STATIC_NOT_INSTALLED"
$(error BOOST static libraries were not found)
endif
BOOST_MT = $(shell if [ -e $(STATIC_BOOST_LIBS_DIR)/libboost_thread-mt.a ]; then echo '-mt'; fi)


include targets.Makefile

define template_per_OBJECT
-include $(1).deps
endef
#endef template_per_OBJECT

define template_per_TARGET
template_per_TARGET_target = $$(if $$(filter %.cpp,$$($(1))), \
									$$(patsubst TARGET_%,build/cpp/%.bin,$(1)), \
									$$(if $$(filter %.c,$$($(1))), \
										$$(patsubst TARGET_%,build/c/%.bin,$(1)), \
										$$(patsubst TARGET_%,build/%.bin,$(1))))
template_per_TARGET_objects = $$(patsubst build/%.cp,build/%.o, $$(patsubst build/%.cxx,build/%.o, $$(patsubst c/%.c,build/c/%.o, $$(patsubst cpp/%.cpp,build/cpp/%.o,$$($(1))))))
all-targets: $$(template_per_TARGET_target)
$$(template_per_TARGET_target): $$(template_per_TARGET_objects) $$($$(patsubst TARGET_%,%,$(1))_TP_OBJECTS)
$$(foreach object,$$(template_per_TARGET_objects), \
	$$(eval $$(call template_per_OBJECT,$$(object))) \
)
endef
#endef template_per_TARGET

$(foreach target,$(filter TARGET_%, $(.VARIABLES)), \
	$(eval $(call template_per_TARGET,$(target))) \
)

define template_per_SHAREDLIBRARY
template_per_SHAREDLIBRARY_target = $$(if $$(filter %.cpp,$$($(1))), \
											$$(patsubst SHAREDLIBRARY_%,build/cpp/%.so,$(1)), \
											$$(patsubst SHAREDLIBRARY_%,build/c/%.so,$(1)))
template_per_SHAREDLIBRARY_objects = $$(patsubst build/%.cp,build/%.o, $$(patsubst build/%.cxx,build/%.o, $$(patsubst c/%.c,build/c/%.o, $$(patsubst cpp/%.cpp,build/cpp/%.o,$$($(1))))))
all-targets: $$(template_per_SHAREDLIBRARY_target)
$$(template_per_SHAREDLIBRARY_target): $$(template_per_SHAREDLIBRARY_objects)
$$(foreach object,$$(template_per_SHAREDLIBRARY_objects), \
	$$(eval $$(call template_per_OBJECT,$$(object))) \
)
endef
#endef template_per_SHAREDLIBRARY

$(foreach target,$(filter SHAREDLIBRARY_%, $(.VARIABLES)), \
	$(eval $(call template_per_SHAREDLIBRARY,$(target))) \
)

ifeq ($(V),1)
  Q =
  SWALLOW_STDOUT =
else
  Q = @
  SWALLOW_STDOUT = > /dev/null
endif

build/cpp/%.o: cpp/%.cpp
	@mkdir -p $(@D)
	@echo 'C++        ' $@
	$(Q)g++ $(CXXFLAGS) $($(subst /,_,$(subst .,_,$*))_o_CFLAGS) -MMD -MF $@.deps -c $< -o $@

build/c/%.o: c/%.c
	@mkdir -p $(@D)
	@echo 'C          ' $@
	$(Q)gcc $(CFLAGS) $($(subst /,_,$(subst .,_,$*))_o_CFLAGS) -MMD -MF $@.deps -c $< -o $@

build/cpp/%.bin:
	@mkdir -p $(@D)
	@echo 'LINK++     ' $@
	$(Q)g++ $(LDFLAGS) -o $@ $^ $($*_LDFLAGS) $($*_LIBRARIES)

build/c/%.bin:
	@mkdir -p $(@D)
	@echo 'LINK       ' $@
	$(Q)gcc $(LDFLAGS) -o $@ $^ $($*_LDFLAGS) $($*_LIBRARIES)

build/cpp/%.so:
	@mkdir -p $(@D)
	@echo 'LINK++     ' $@
	$(Q)g++ -shared $(LDFLAGS) -o $@ $^ $($*_LDFLAGS) $($*_LIBRARIES)

build/c/%.so:
	@mkdir -p $(@D)
	@echo 'LINK       ' $@
	$(Q)gcc -shared $(LDFLAGS) -o $@ $^ $($*_LDFLAGS) $($*_LIBRARIES)

build/cpp/Unittests/runner_Test%.o: build/cpp/Unittests/runner_%.cpp
	@mkdir -p $(@D)
	@echo 'C++        ' $@
	$(Q)g++ $(CXXFLAGS) $($(subst /,_,$(subst .,_,$*))_o_CFLAGS) -MMD -MF $@.deps -c $< -o $@

build/cpp/Unittests/runner_Test%.cpp: cpp/Unittests/Test%.cpp
	@mkdir -p $(@D)
	@echo 'CXXTESTGEN ' $@
	$(Q)cxxtest/bin/cxxtestgen --error-printer -o $@ $^

build/cpp/runner_Test%.bin:
	$(MAKE) -f build.Makefile cxxtest/cxxtest
	@mkdir -p $(@D)
	@echo 'LINKTEST++ ' $@
	$(Q)g++ $(LDFLAGS) -o $@ $^ $($*_LDFLAGS) $(runner_Test$*_LIBRARIES) $(CXXFLAGS)

cxxtest/cxxtest:
	git submodule init
	git submodule update -f
