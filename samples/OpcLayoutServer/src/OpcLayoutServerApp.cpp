#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OpcLayoutServerApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
};

void OpcLayoutServerApp::setup()
{
}

void OpcLayoutServerApp::mouseDown( MouseEvent event )
{
}

void OpcLayoutServerApp::update()
{
}

void OpcLayoutServerApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP( OpcLayoutServerApp, RendererGl )
