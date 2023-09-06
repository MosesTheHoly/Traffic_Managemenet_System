#pragma once

#include <windows.h>
#include <osg/switch>
#include <osg/ShapeDrawable>
#include <list>

#include "raaAnimatedFacarde.h"
#include "raaCollisionTarget.h"

// a facarde for the cars in the scene - note this also inherets from collision target to provide support for collision management

class raaCarFacarde: public raaAnimatedFacarde, public raaCollisionTarget
{
public:
	raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath* ap, double dSpeed);
	virtual ~raaCarFacarde();
	void operator()(osg::Node* node, osg::NodeVisitor* nv) override;
	osg::AnimationPath* animationPath;
	virtual osg::Vec3f getWorldDetectionPoint(); // from raaCollisionTarget
	virtual osg::Vec3f getWorldCollisionPoint(); // from raaCollisionTarget
	virtual float getCarRotation(); // added into raaCollisionTarget
	virtual void updateRotation(); // added into raaCollisionTarget
	virtual osg::Vec2Array* getBoxCorners();
	virtual void updateBoxCorners();
	virtual float getCarSpeed(); // added into raaCollisionTarget
	virtual std::string getCarName(); // added into raaCollisionTarget

protected:
	bool isIntersecting(osg::Vec2Array* box1Corners, osg::Vec2Array* box2Corners);
	osg::ShapeDrawable* bodyMainNode;
	osg::ShapeDrawable* bodyTopNode;
	osg::Vec4f carColour;
	double initial_speed;
	double _rotation;
	void setCarColourAndSize();
	bool isCarAligned(float collider);
	bool isLightAligned(float collider);
	void setSize(osg::BoundingBox box);
	osg::Vec2Array* corners;
	void increaseCarSize(bool toIncrease);
	float height;
	float width;
	osg::Vec3f lineEquationFrom2Points(osg::Vec2f p1, osg::Vec2f p2);
	osg::Vec2f getAxis(osg::Vec2f _1, osg::Vec2f _2);
	osg::Vec2f findMinMax(osg::Vec2Array* corners, osg::Vec2f axis);
	float getDot(osg::Vec2f v1, osg::Vec2f v2);
};

