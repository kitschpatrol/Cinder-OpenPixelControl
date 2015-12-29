//#include "CinderAsio.h" // Needs to be included before Cinder on Windows? TBD.

#include "cinder/Utilities.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "OpcClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpcBasicClientApp : public App {
public:
	static void prepareSettings(Settings *settings);
	void setup() override;
	void update() override;
	void draw() override;

private:
	kp::opc::ClientRef mOpcClient;
};

void OpcBasicClientApp::prepareSettings(Settings *settings) {
	settings->setPowerManagementEnabled(true);
	settings->setTitle("OPC Basic Client Sample App");
}

void OpcBasicClientApp::setup() {
	// Create OPC client, providing the IP and port for OPC server.
	// Node additional (defaulted) arguments in create function.
	mOpcClient = kp::opc::Client::create("127.0.0.1", 7890);
}

void OpcBasicClientApp::update() {
	const int numLeds = 32;

	// Pulse a rainbow through the LEDs...
	for (int i = 0; i < numLeds; i++) {
		// Set the color of an LED at a particular index
		// Note that nothing's actually sent to the server till mOpcClient-->update() is called.
		mOpcClient->setLED(i, ci::Color(CM_HSV, fmod((i + getElapsedSeconds() * 20.0) / (float)numLeds, 1.0), 1.0, 1.0));
	}

	// Update syncs LED data to the server if necessary
	mOpcClient->update();
}

void OpcBasicClientApp::draw() {
	gl::clear(Color(0, 0, 0));
	// Nothing to see here

	// Draw stats
	TextBox textBox = TextBox().size(ivec2(200, 15)).text("Messages sent: " + toString(mOpcClient->getNumMessagesSent()));
	gl::pushMatrices();
	gl::translate(10.0, 10.0);
	gl::draw(gl::Texture2d::create(textBox.render()));
	gl::popMatrices();
}

CINDER_APP(OpcBasicClientApp, RendererGl, OpcBasicClientApp::prepareSettings)
