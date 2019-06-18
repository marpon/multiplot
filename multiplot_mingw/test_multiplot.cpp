
#define __TEST_THIS_MODULE__


#include "multiplot.h"
using namespace multiplot;


int main()
{

// create a multiplot Window at position (x,y)=(50,50) and width=640, height=480
    /*
    Multiplot m(50, 50, 640, 480);
    	//m.linewidth(1);	// first trace has line-width of 2
    	//m.pointsize(0);

    for (int x = 0; x<100; x++)
    	{
    		// plot a sine wave
    		m.plot(float(x), sin(0.1f * x));
    		//m[1].plot(float(x),  cos(0.1f * x));

    		if (!m.check()) { break; }
    		m.sleep(20);
    		m.redraw();
    	}

    	keep_alive(m);*/
    test_module();
    return 0;

}
