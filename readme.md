# Cinder-OpenPixelControl

**A [Cinder](https://libcinder.org) block providing a basic implementation of the [OpenPixelControl](http://openpixelcontrol.org) protocol for controlling large numbers of LEDs in real-time over TCP/IP. Networking provided by the [Cinder-Asio block](https://github.com/BanTheRewind/Cinder-Asio).**


## Getting Started

Run both the OpcBasicClient and OpcBasicServer samples simultaneously on the same machine to see OPC commands passing from client to (virtual) server.

The utility of a server implementation is somewhat dubious, since that role is usually taken care of by the micro controller (or similar) that's communicating directly with the LEDs — Cinder's much more likely to come in handy creating OPC data rather than consuming it. Nevertheless, the server can be useful for creating simulators and testing layouts. (Similar conceptually to the gl_server example included in the official OpenPixelControl library.)

This library foregoes direct support for JSON layout files, but the OpcLayoutClient and OpcLayoutServer samples provide examples of parsing and using layout JSON.

Channels are currently unsupported.

This implementation is certainly not the leanest out there, but it tries to leverage Cinder's abstractions where possible. (e.g. ci::Color, asio::io_service, etc.) 

## Origin

The [OpenPixelControl](http://openpixelcontrol.org) spec and reference implementation was created by [Ka-Ping Yee](https://github.com/zestyping).

[Micah Elizabeth Scott](https://github.com/scanlime) has done a ton of significant work on top of the OPC protocol for her [Fadecandy](https://github.com/scanlime/fadecandy) project, including hardware controllers and OPC wrapper libraries for a number of frameworks and platforms.

## Dependencies

This block depends on the Cinder-Asio block. I recommend [Jean-Pierre Mouilleseaux fork](https://github.com/pizthewiz/Cinder-Asio/).

## Known Issues

### Xcode

Cinder-Asio needs to be added as the first include to the precompiled header file. See the examples for an example. If you forget this step, you will see "No member named 'error' in namespace 'asio::placeholders'" errors and similar.

### Mac
If you're running your OPC client or server in the background, [App Nap](https://developer.apple.com/library/mac/documentation/Performance/Conceptual/power_efficiency_guidelines_osx/AppNap.html) will slow down and eventually completely stop network transmissions.

To disable App Nap, add the following to your app's prepareSettings function:

		settings->setPowerManagementEnabled(true);

The sample projects are already set accordingly.


##TODO

- Test on Windows.
- Support for channels.
- Better support for layout JSON.