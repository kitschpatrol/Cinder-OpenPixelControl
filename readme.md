<!-- title({ prefix: "Cinder-" }) -->

# Cinder-OpenPixelControl

<!-- /title -->

<!-- badges -->

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/license/mit)

<!-- /badges -->

<!-- short-description -->

**CinderBlock for OpenPixelControl, with a basic client and server implementing (most of) the protocol for controlling large numbers of LEDs in real time over TCP/IP.**

<!-- /short-description -->

## Overview

A [CinderBlock](https://libcinder.org/docs/guides/cinder-blocks/index.html) for [Cinder](https://libcinder.org/) providing a basic implementation of the [OpenPixelControl](https://web.archive.org/web/20260704010252/http://openpixelcontrol.org/) protocol for controlling large numbers of LEDs in real time over TCP/IP. Networking is provided by the [Cinder-Asio block](https://github.com/BanTheRewind/Cinder-Asio).

The utility of a server implementation is somewhat dubious, since that role is usually taken care of by the microcontroller (or similar) that's communicating directly with the LEDs — Cinder's much more likely to come in handy creating OPC data rather than consuming it. Nevertheless, the server can be useful for creating simulators and testing layouts. (Similar conceptually to the gl_server example included in the official [OpenPixelControl library](https://github.com/zestyping/openpixelcontrol).)

This library forgoes direct support for JSON layout files, but the OpcLayoutClient and OpcLayoutServer samples provide examples of parsing and using layout JSON.

## Getting started

### Dependencies

Requires Cinder on macOS or Windows. This block depends on the [Cinder-Asio](https://github.com/BanTheRewind/Cinder-Asio) block, which uses the standalone Asio library that ships with Cinder. I originally recommended Jean-Pierre Mouilleseaux's fork, which is no longer available on GitHub ([archived page](https://web.archive.org/web/20210114192638/https://github.com/pizthewiz/Cinder-Asio)).

The block was written in late 2015 and early 2016, and last changed in May 2016, against Cinder's master branch (0.9.1dev, between the 0.9.0 and 0.9.1 releases). It hasn't been tested with later Cinder releases.

### Installation

From your Cinder `blocks` directory:

```sh
git clone https://github.com/kitschpatrol/Cinder-OpenPixelControl.git
git clone https://github.com/BanTheRewind/Cinder-Asio.git Asio
```

Then add **OpenPixelControl** to a project with [TinderBox](https://libcinder.org/docs/guides/tinderbox/index.html). The block manifest lists Cinder-Asio (`asio.bantherewind.com`) as a requirement.

The sample projects expect Cinder-Asio in a `blocks/Asio` directory, which is why the second command clones it under that name.

## Usage

Create a client, set LED colors, and call `update()` to send them to an OPC server:

```cpp
#include "OpcClient.h" // Include before Cinder's headers, as Cinder-Asio requires on Windows

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"

class MyApp : public ci::app::App {
public:
	void setup() override {
		mOpcClient = kp::opc::Client::create("127.0.0.1", 7890);
	}

	void update() override {
		// Pulse 32 LEDs between black and red
		const float brightness = 0.5f + 0.5f * std::sin(static_cast<float>(getElapsedSeconds()));
		for (int i = 0; i < 32; i++) {
			mOpcClient->setLED(i, ci::Color(brightness, 0.0f, 0.0f));
		}

		// Nothing is sent to the server until update() is called
		mOpcClient->update();
	}

private:
	kp::opc::ClientRef mOpcClient;
};

CINDER_APP(MyApp, ci::app::RendererGl)
```

A server works the other way around. Create one for a given number of LEDs and read their colors back as they arrive:

```cpp
#include "OpcServer.h"

// In setup(), simulate a 32-LED strip listening on port 7890
mOpcServer = kp::opc::Server::create(32, 7890);

// In draw()
for (const ci::Color &color : mOpcServer->getLeds()) {
	// Draw each LED
}
```

The [OpcClient.h](src/OpcClient.h) and [OpcServer.h](src/OpcServer.h) API:

| Method                                                                           | Description                                                                                                                                                                                |
| -------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `Client::create(host, port, isLongConnection, isBandwidthConserved, colorOrder)` | Connect to an OPC server. Defaults to `localhost`, port `7890`, a connection that stays open between messages, sending only when a color has changed, and `kp::opc::RED_GREEN_BLUE` order. |
| `setLED(index, color)`                                                           | Set the `ci::Color8u` color of the LED at `index`. The message grows to include the highest index set.                                                                                     |
| `update()`                                                                       | Send the LED colors to the server. If `isBandwidthConserved` is true, only sends when a color has changed since the last `update()` call.                                                  |
| `getNumMessagesSent()`                                                           | Number of messages written to the server.                                                                                                                                                  |
| `Server::create(numLeds, port, colorOrder)`                                      | Listen for an OPC client and keep the colors of `numLeds` LEDs. Defaults to port `7890` and `kp::opc::RED_GREEN_BLUE` order.                                                               |
| `getLeds()`                                                                      | A copy of the current LED colors as `ci::Color` values.                                                                                                                                    |
| `getNumMessagesReceived()`                                                       | Number of valid messages received.                                                                                                                                                         |

Set `isLongConnection` to false to close and reopen the connection after every message. The other supported color order is `kp::opc::BLUE_RED_GREEN`.

In Xcode, Cinder-Asio needs to be added as the first include to the precompiled header file. See the samples' `_Prefix.pch` files for an example. If you forget this step, you will see "No member named 'error' in namespace 'asio::placeholders'" errors and similar.

On macOS, if you're running your OPC client or server in the background, [App Nap](https://developer.apple.com/library/archive/documentation/Performance/Conceptual/power_efficiency_guidelines_osx/AppNap.html) will slow down and eventually completely stop network transmissions.

To disable App Nap, add the following to your app's prepareSettings function:

```cpp
settings->setPowerManagementEnabled(true);
```

The sample projects are already set accordingly.

## Examples

Each sample includes an Xcode project in its `xcode` directory and a Visual Studio 2013 solution in its `vc2013` directory. The projects expect this block to be inside Cinder's `blocks` directory. The clients connect to `127.0.0.1` and the servers listen on port `7890`.

### OpcBasicClient

The [basic client](samples/OpcBasicClient/src/OpcBasicClientApp.cpp) pulses a rainbow through 32 LEDs and shows the number of messages sent.

### OpcBasicServer

The [basic server](samples/OpcBasicServer/src/OpcBasicServerApp.cpp) simulates a 32-LED strip, draws each LED as a circle filled with its received color, and shows the number of messages received.

Run both the OpcBasicClient and OpcBasicServer samples simultaneously on the same machine to see OPC commands passing from client to (virtual) server.

### OpcLayoutClient

The [layout client](samples/OpcLayoutClient/src/OpcLayoutClientApp.cpp) loads 3,000 LED positions from [layout.json](samples/OpcLayoutClient/assets/layout.json), draws a spinning, pulsing red square, and sends the color under each LED position to the server.

### OpcLayoutServer

The [layout server](samples/OpcLayoutServer/src/OpcLayoutServerApp.cpp) loads the same [layout.json](samples/OpcLayoutServer/assets/layout.json) and draws each LED at its position in the received color. Run it alongside OpcLayoutClient.

## Background

### Implementation notes

This implementation is certainly not the leanest out there, but it tries to leverage Cinder's abstractions where possible. (e.g., `ci::Color`, `asio::io_service`)

The client and server run on the app's `io_service()` through Cinder-Asio's `TcpClient` and `TcpServer`. The client keeps a single OPC message (a four-byte header followed by three bytes per LED) and updates it in place with each `setLED()` call. The server collects received bytes in a `std::deque` and parses complete messages off the front, so messages can span several reads or share one.

### Other projects

- Shortly after finishing this block, I discovered James Hurlbut's [FadeCandyCinderClient](https://github.com/jhurlbut/FadeCandyCinderClient), which includes a very similar implementation of the client, and also ports over some of the neat visual effect classes from the Fadecandy project.

### Known issues

- Channels are currently unsupported. The client always sends to channel 0, and the server logs and discards messages for any other channel or for commands other than "set pixel colors".
- The block hasn't been tested on Windows. Cinder-Asio says `CinderAsio.h` must be included before Cinder on Windows, but the samples include Cinder's headers first.
- Support for layout JSON is limited to the parsing in the layout samples.
- After every message it parses, the server logs a "Too short" error, because it checks the empty queue for another message. A message whose header is split across reads logs the same error.
- When `setLED()` grows the message, LEDs between the previous highest index and the new one hold uninitialized data until they are set.
- `update()` discards pending changes if the client isn't connected yet, so a static frame set before the connection opens (or between short connections) isn't sent until a color changes.
- While no server is running, the client retries the connection immediately and indefinitely.
- The client detects a refused connection or a broken pipe by comparing Asio's error messages with "Connection refused" and "Broken pipe". Other platforms may word these errors differently, which would prevent the client from reconnecting.

## Maintainers

[kitschpatrol](https://github.com/kitschpatrol)

## Acknowledgments

The [OpenPixelControl](https://web.archive.org/web/20260704010252/http://openpixelcontrol.org/) spec and reference implementation were created by [Ka-Ping Yee](https://github.com/zestyping).

[Micah Elizabeth Scott](https://scanlime.org/) has done a ton of significant work on top of the OPC protocol for her [Fadecandy](https://web.archive.org/web/20230923191147/https://github.com/scanlime/fadecandy/) project, including hardware controllers and OPC wrapper libraries for a number of frameworks and platforms.

<!-- contributing -->

## Contributing

[Issues](https://github.com/kitschpatrol/Cinder-OpenPixelControl/issues) are welcome and appreciated.

Please open an issue to discuss changes before submitting a pull request. Unsolicited PRs (especially AI-generated ones) are unlikely to be merged.

This repository uses [MDAT](https://github.com/kitschpatrol/mdat) for readme placeholder expansion.

<!-- /contributing -->

<!-- license -->

## License

[MIT](license.txt) © Eric Mika

<!-- /license -->
