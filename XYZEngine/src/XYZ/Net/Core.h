#pragma once


#ifdef _WIN64
#define _WIN32_WINNT 0x0A00
#endif

#include <memory>
#include <cstdint>
#include <iostream>

#define ASIO_STANDALONE
#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>
