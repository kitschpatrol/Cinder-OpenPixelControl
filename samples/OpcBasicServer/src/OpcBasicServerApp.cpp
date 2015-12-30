#include "cinder/Utilities.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "OpcServer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpcBasicServerApp : public App {
public:
	static void prepareSettings(Settings *settings);
	void setup() override;
	void update() override;
	void draw() override;

private:
	kp::opc::ServerRef mOpcServer;
};

void OpcBasicServerApp::prepareSettings(Settings *settings) {
	settings->setPowerManagementEnabled(true); // Prevent App Nap
	settings->setTitle("OPC Basic Server Sample App");
}

void OpcBasicServerApp::setup() {
	// Create OPC Server simulating a 32 LED strip.
	// Listen on port 7890.
	mOpcServer = kp::opc::Server::create(32, 7890);
}

void OpcBasicServerApp::update() {
}

void OpcBasicServerApp::draw() {
	gl::clear(Color(0, 0, 0));

	// Draw LEDs as circles
	const int numLeds = mOpcServer->getLeds().size();
	const float spacing = getWindowWidth() / static_cast<float>(numLeds);
	const float radius = (spacing * 0.8) / 2.0;

	int i = 0;
	for (const Color8u &color : mOpcServer->getLeds()) {
		vec2 center = vec2(((i + 1) * spacing) - radius, getWindowHeight() / 2.0);

		// Fill with received color
		gl::color(color);
		gl::drawSolidCircle(center, radius);

		// White outline
		gl::color(Color("white"));
		gl::drawStrokedCircle(center, radius);
		i++;
	}

	// Draw stats
	TextBox textBox = TextBox().size(ivec2(200, 15)).text("Messages received: " + toString(mOpcServer->getNumMessagesReceived()));
	gl::pushMatrices();
	gl::translate(10.0, 10.0);
	gl::color(Color("white"));
	gl::draw(gl::Texture2d::create(textBox.render()));
	gl::popMatrices();
}

CINDER_APP(OpcBasicServerApp, RendererGl(RendererGl::Options().msaa(4)), OpcBasicServerApp::prepareSettings)
