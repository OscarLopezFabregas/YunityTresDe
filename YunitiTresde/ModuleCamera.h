#ifndef _MODULECAMERA_
#define _MODULECAMERA_

#include "Module.h"
#include "Mathgeolib\include\Geometry\Frustum.h"


class ModuleCamera :
	public Module
{

public:
	ModuleCamera();
	~ModuleCamera();

	bool Init();

	update_status PreUpdate();
	update_status Update();
	void SetFOV(float degrees);
	void SetAspectRatio();
	void SetPlaneDistances();
	void SetPosition();
	//void LookAt(int x, int y, int z);
	float *GetProjectionMatrix();
	float *GetViewMatrix();

	bool CleanUp();

public:



private:

	Frustum frustum_;
	float camSpeed;
	float aspectRatio;
};



#endif // !_MODULECAMERA_

