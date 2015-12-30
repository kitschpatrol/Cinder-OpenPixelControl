#include "cinder/Json.h"
#include "cinder/Log.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "OpcServer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpcLayoutServerApp : public App {
public:
	static void prepareSettings(Settings *settings);
	void setup() override;
	void update() override;
	void draw() override;

private:
	kp::opc::ServerRef mOpcServer;
	vector<vec2> mLayoutPositions;
};

void OpcLayoutServerApp::prepareSettings(Settings *settings) {
	settings->setWindowSize(800, 800);
	settings->setPowerManagementEnabled(true); // Prevent App Nap
	settings->setTitle("OPC Layout Server Sample App");
}

void OpcLayoutServerApp::setup() {

	// Load layout JSON and store positions by LED index. Ignore the Z axis.
	JsonTree layoutData = JsonTree(loadAsset("layout.json"));
	for (auto &led : layoutData) {
		const vec2 position = vec2(led["point"].getValueAtIndex<float>(0), led["point"].getValueAtIndex<float>(1));
		mLayoutPositions.push_back(position);
	}

	mOpcServer = kp::opc::Server::create(mLayoutPositions.size(), 7890);
}

void OpcLayoutServerApp::update() {
}

void OpcLayoutServerApp::draw() {
	gl::clear(Color(0, 0, 0));

	// Draw the LEDs per the layout, setting color from received messages.
	int i = 0;
	for (const Color8u &color : mOpcServer->getLeds()) {
		const ivec2 center = mLayoutPositions[i];

		// LED outline

		if (color == Color("black")) {
			// Fill with placeholder color
			gl::color(Color(0.1, 0.1, 0.1));
		} else {
			// Fill with received color
			gl::color(color);
		}
		gl::drawSolidCircle(center, 3.0);

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

CINDER_APP(OpcLayoutServerApp, RendererGl, OpcLayoutServerApp::prepareSettings)
