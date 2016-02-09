#pragma once

#ifdef _WIN32
#include <SDKDDKVer.h>
#include <tchar.h>
#endif

// C Lib
#include <csignal>
#include <cstdint>
#include <cstdio>

// STL
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <type_traits>
#include <vector>

// Boost
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>
