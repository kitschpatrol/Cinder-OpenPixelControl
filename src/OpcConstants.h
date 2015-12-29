//
//  Opc.h
//  CinderOpenPixelControlSandbox
//
//  Created by Eric Mika on 12/24/15.
//
//

#pragma once

namespace kp {
namespace opc {

static const int32_t DEFAULT_PORT = 7890;
static const std::string DEFAULT_IP = "localhost";
static const int HEADER_LENGTH = 4;
static const int BYTES_PER_LED = 3;

enum ColorOrder {
	RED_GREEN_BLUE = 0,
	BLUE_RED_GREEN = 1,
};

static const int COLOR_ORDER_LOOKUP[2][3] = {{0, 1, 2}, {2, 0, 1}};
static const ColorOrder DEFAULT_COLOR_ORDER = RED_GREEN_BLUE;

} // namespace opc
} // namespace kp
