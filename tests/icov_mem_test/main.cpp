#include <iostream>
#include <vector>
#include <string>
#include "renderer/clientcontroller.h"
#include "network/client/implementation/client.h"
#include "omnidirectional_layer/implementation/BlockDecoder.h"

using namespace std;

///Error code
enum ErrorCode
{
    UNKNOWN_ERROR = -1, ///< Unknown error
    OK = 0, ///< No error
    BAD_USAGE, ///< Bad usage in command line
};

/**
 * @brief main
 *
 * @return
 * 0 if OK
 * 1 for bad usage
 *
 */
int main(int argc, char *argv[])
{

   // int* tab = new int[5];


    ClientController ctrl;
    ctrl.start();
    ctrl.setPoV(25, 32);
    ctrl.update();


    const float xAngle = ctrl.getClient().getDecoder().getRenderer().getRequest().getAngleX();
	const float yAngle = ctrl.getClient().getDecoder().getRenderer().getRequest().getAngleY();
    
    cout << "CHECK: " << xAngle << ", " << yAngle << endl;

    ctrl.setPoV(12, 46);
    ctrl.update();

    return ErrorCode::OK; // ALL FINE !
}
