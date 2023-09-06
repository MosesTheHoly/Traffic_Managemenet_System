#pragma once

#include <windows.h>
#include <osg/Vec3f>
#include <osg/Array>
#include <iostream>

// a pure virtual base class (interface) to provide support for collidable objects

class raaCollisionTarget
{
public:
	virtual osg::Vec3f getWorldDetectionPoint()=0;
	virtual osg::Vec3f getWorldCollisionPoint()=0;
	virtual osg::Vec2Array* getBoxCorners()=0;
	virtual float getCarRotation()=0;
	virtual float getCarSpeed()=0;
	virtual std::string getCarName()=0;

};

