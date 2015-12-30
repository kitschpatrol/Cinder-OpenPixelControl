#include "cinder/Json.h"
#include "cinder/Log.h"
#include "cinder/Rand.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "OpcClient.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpcLayoutClientApp : public App {
public:
	static void prepareSettings(Settings *settings);
	void setup() override;
	void update() override;
	void draw() override;

private:
	kp::opc::ClientRef mOpcClient;
	vector<vec2> mLayoutPositions;
};

void OpcLayoutClientApp::prepareSettings(Settings *settings) {
	settings->setWindowSize(800, 800);
	settings->setPowerManagementEnabled(true); // Prevent App Nap
	settings->setTitle("OPC Layout Client Sample App");
}

void OpcLayoutClientApp::setup() {
	// Load layout JSON and store positions by LED index. Ignore the Z axis.
	JsonTree layoutData = JsonTree(loadAsset("layout.json"));
	for (auto &led : layoutData) {
		const vec2 position = vec2(led["point"].getValueAtIndex<float>(0), led["point"].getValueAtIndex<float>(1));
		mLayoutPositions.push_back(position);
	}

	mOpcClient = kp::opc::Client::create("127.0.0.1", 7890);
}

void OpcLayoutClientApp::update() {
	mOpcClient->update();
}

void OpcLayoutClientApp::draw() {
	gl::clear(Color(0, 0, 0));

	// Draw some content to be sampled and sent out via OPC
	// Using an FBO would be more hygienic.
	float scaleFactor = sin(getElapsedFrames() / 30.0) * 7;

	gl::pushMatrices();
	gl::translate(400, 400);
	gl::rotate(getElapsedFrames() / 30.0);
	gl::scale(scaleFactor, scaleFactor);
	gl::color(Color("red"));
	gl::drawSolidRect(Rectf(-50, -50, 50, 50));
	gl::popMatrices();

	// Copy what we've drawn so far into surface for pixel inspection...
	Surface windowImage = copyWindowSurface(getWindowBounds());

	// Send drawing out as sampled points
	int i = 0;
	for (vec2 &position : mLayoutPositions) {
		// Sample color under each point in the layout
		Color8u sampledColor = Color8u(windowImage.getPixel(position));

		// Set the corresponding LED
		mOpcClient->setLED(i, sampledColor);

		// Draw the sampled point for reference
		gl::color(Color("white"));
		gl::drawSolidRect(Rectf(position - vec2(0.5, 0.5), position + vec2(0.5, 0.5)));

		i++;
	}

	// Draw stats
	TextBox textBox = TextBox().size(ivec2(200, 15)).text("Messages sent: " + toString(mOpcClient->getNumMessagesSent()));
	gl::pushMatrices();
	gl::translate(10.0, 10.0);
	gl::color(Color("white"));
	gl::draw(gl::Texture2d::create(textBox.render()));
	gl::popMatrices();
}

CINDER_APP(OpcLayoutClientApp, RendererGl, OpcLayoutClientApp::prepareSettings)