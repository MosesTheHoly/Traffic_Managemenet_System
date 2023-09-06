#define _USE_MATH_DEFINES // Required for PI
#include <windows.h>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/PolygonMode> // New
#include <osg/ShapeDrawable>
#include <osg/Shape>
#include <osg/Material>
#include <osg/Switch> // New
#include "TrafficLightControl.h" // New
#include "TrafficLightFacarde.h" // New
#include "raaBoundCalculator.h" // New
#include "raaFacarde.h" // New
#include "raaFinder.h" // New
#include "raaAnimationPointFinder.h" // New
#include "raaCollisionTarget.h" // New
#include "raaTrafficSystem.h"
#include "raaCarFacarde.h"
#include <iostream>
#include <cmath>

using namespace std;
raaCarFacarde::raaCarFacarde(osg::Node* pWorldRoot, osg::Node* pPart, osg::AnimationPath * ap, double dSpeed) : raaAnimatedFacarde(pPart, ap, dSpeed)
{
	raaTrafficSystem::addTarget(this); // adds the car to the traffic system (static class) which holds a reord of all the dynamic parts in the system
    this->initial_speed = dSpeed;
    //Get cars initial speed
    this->m_dSpeed = initial_speed;
    this->animationPath = ap;
    //this->name = pPart->getName();
    cout << "CAR NAME: " << pPart->getName() << endl;
    setCarColourAndSize();

}

raaCarFacarde::~raaCarFacarde()
{
	raaTrafficSystem::removeTarget(this); // removes the car from the traffic system (static class) which holds a reord of all the dynamic parts in the system
}


void raaCarFacarde::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    // Do this here so the car doesnt have to keep checking its position again and again. Just once
    updateRotation();
    updateBoxCorners();
    increaseCarSize(true);

    osg::Vec3f thisPos = getWorldDetectionPoint();
    float carRot = getCarRotation();

    //Set cars speed back to normal first
    this->m_dSpeed = initial_speed;

    // All cars start at 0, 0, 45 first. Needs to skip that first part otherwise they never move
    if (thisPos[0] != 0 && thisPos[1] != 0) {
        cout << endl << endl;
        cout << "----------------------------------------------------------" << endl;
        cout << "----------------------------------------------------------" << endl;
        std::cout << pPart->getName() << std::endl;
        cout << "This cars position: X: " << thisPos[0] << " | Y: " << thisPos[1] << " | Z: " << thisPos[2] << endl;
        cout << "This cars rotation: " << carRot << endl;
        cout << "This cars inital speed: " << initial_speed << endl;
        cout << "This cars speed: " << this->m_dSpeed << endl;
        cout << "----------------------------------------------------------" << endl;
        float distanceThreshold = 300.0f;
        float closestDistance = FLT_MAX;
        list<raaCollisionTarget*> possibleCars;
        //Distance between cars
        for (auto it = raaTrafficSystem::colliders().begin(); it != raaTrafficSystem::colliders().end(); it++)
        {
            auto collider = *it;

            if (collider == this) //cannot collide with itself
            {
                continue;
            }

            auto colliderPos = collider->getWorldDetectionPoint();

            // Only X and Z because the cars dont move up and down
            float dx = thisPos[0] - colliderPos[0];
            float dz = thisPos[2] - colliderPos[2];
            float distance = sqrt((dx * dx) + (dz * dz));

            if (distance < distanceThreshold && m_dSpeed > collider->getCarSpeed()) {
                if (isCarAligned(collider->getCarRotation())) {
                    cout << "Possible Collision with " << collider->getCarName() << endl;
                    closestDistance = distance;
                    possibleCars.push_back(collider);
                }
            }
        }

        if (possibleCars.size() > 0)
        {
            osg::Vec2Array* thisCorners = getBoxCorners(); // Only need to compute this once
            bool intersecting = false;
            if (getCarName() == "car4") {
                cout << endl;
            }
            else if (getCarName() == "car3") {
                cout << endl;
            }
            for (auto it = possibleCars.begin(); it != possibleCars.end(); it++) {
                auto collider = *it;
                if (isIntersecting(thisCorners, collider->getBoxCorners())) {
                    intersecting = true;
                    m_dSpeed = collider->getCarSpeed();
                    break;
                }
            }
        }

        distanceThreshold = 200.0f;
        bool isStopLight = false;
        for (auto it = raaTrafficSystem::trafficLights().begin(); it != raaTrafficSystem::trafficLights().end(); it++) //traffic light logic
        {
            TrafficLightFacarde* trafficLight = *it;
            osg::Vec3f trafficLightLocation = trafficLight->root()->getBound().center();

            // If traffic light is 1 (Red) or 2 (Amber) then do the checks
            if (trafficLight->m_iTrafficLightStatus < 3) {
                float dx = thisPos[0] - trafficLightLocation[0];
                float dz = thisPos[2] - trafficLightLocation[2];
                float distance = sqrt((dx * dx) + (dz * dz));

                if (distance < distanceThreshold) {
                    if (isLightAligned(trafficLight->_rotation)) {
                        isStopLight = true;
                    }
                }
            }
        }
        if (isStopLight) {
            m_dSpeed = 0;
        }
        

    }
    increaseCarSize(false);

    raaAnimationPathCallback::operator()(node, nv);
}

/// <summary>
/// Gets the cars center X,Y,Z
/// </summary>
/// <returns>osg::Vec3f that is the cars location</returns>
osg::Vec3f raaCarFacarde::getWorldDetectionPoint()
{
    return this->m_pRoot->getBound().center(); // should return the world position of the detection point for this subtree
}

osg::Vec3f raaCarFacarde::getWorldCollisionPoint()
{
    //osg::Vec3f detectionPoint = getWorldDetectionPoint();

	return osg::Vec3(); // should return the world position of the collision point for this subtree
}

/// <summary>
/// Clockwise order starting from front left
/// 0 = Front Left
/// 1 = Front Right
/// 2 = Back Right
/// 3 = Back Left
/// </summary>
/// <returns></returns>
osg::Vec2Array* raaCarFacarde::getBoxCorners() {
    return this->corners;
}

void raaCarFacarde::updateBoxCorners() {
    // Require the center of the car first in world space
    osg::Vec3f center = getWorldDetectionPoint();
    float carRot = osg::DegreesToRadians(getCarRotation());

    float widthHalf = height / 2.0f;
    float heightHalf = width / 2.0f;

    float sinAngle = sinf(carRot);
    float cosAngle = cosf(carRot);

    osg::Vec4f corners = osg::Vec4f();

    osg::Vec2Array* vecArray = new osg::Vec2Array;

    // containerX and Y are just to hold the values to make it readable

    // Front Left:
    float containerX = center[0] + (widthHalf * cosAngle) - (heightHalf * sinAngle);
    float containerY = center[1] + (widthHalf * sinAngle) + (heightHalf * cosAngle);
    vecArray->push_back(osg::Vec2f(containerX, containerY));

    // Front Right:
    containerX = center[0] + (widthHalf * cosAngle) + (heightHalf * sinAngle);
    containerY = center[1] + (widthHalf * sinAngle) - (heightHalf * cosAngle);
    vecArray->push_back(osg::Vec2f(containerX, containerY));

    // Back Right:
    containerX = center[0] - (widthHalf * cosAngle) + (heightHalf * sinAngle);
    containerY = center[1] - (widthHalf * sinAngle) - (heightHalf * cosAngle);
    vecArray->push_back(osg::Vec2f(containerX, containerY));

    // Back Left 
    containerX = center[0] - (widthHalf * cosAngle) - (heightHalf * sinAngle);
    containerY = center[1] - (widthHalf * sinAngle) + (heightHalf * cosAngle);
    vecArray->push_back(osg::Vec2f(containerX, containerY));

    /*
    cout << endl << endl;
    cout << "(" << vecArray->at(0)[0] << ", " << vecArray->at(0)[1] << ")" << endl;
    cout << "(" << vecArray->at(1)[0] << ", " << vecArray->at(1)[1] << ")" << endl;
    cout << "(" << vecArray->at(2)[0] << ", " << vecArray->at(2)[1] << ")" << endl;
    cout << "(" << vecArray->at(3)[0] << ", " << vecArray->at(3)[1] << ")" << endl << endl;
    */
    this->corners = vecArray;
}

/// <summary>
/// Was using SAT but now modified to check line and rectangle intersection
/// Checks if the two front corners built as a line 
/// Using this instead of the built in bound().intersects() as this is faster in 2D than 3D
/// </summary>
/// <returns>True if intersecting, false otherwise</returns>
bool raaCarFacarde::isIntersecting(osg::Vec2Array* box1Corners, osg::Vec2Array* box2Corners) {
    float p0x = box1Corners->at(0)[0];
    float p0y = box1Corners->at(0)[1];

    float p1x = box1Corners->at(1)[0];
    float p1y = box1Corners->at(1)[1];

    float a1 = p1y - p0y;
    float b1 = p0x - p1x;
    float c1 = a1 * p0x + b1 * p0y;

    for (int i = 0; i < 4; i++) {

        float p2x = box2Corners->at(i)[0];
        float p2y = box2Corners->at(i)[1];
        float p3x;
        float p3y;
        if (i == 3) {
            p3x = box2Corners->at(0)[0];
            p3y = box2Corners->at(0)[1];
        }
        else {
            p3x = box2Corners->at(i + 1)[0];
            p3y = box2Corners->at(i + 1)[1];
        }

        float a2 = p3y - p2y;
        float b2 = p2x - p3x;
        float c2 = a2 * p2x + b2 * p2y;

        // (A1 * B2) - (A2 * B1)
        float denominator = a1 * b2 - a2 * b1;
        if (denominator == 0) {
            //cout << "denominator: " << denominator << endl;
            continue;
        }


        float intersectX = ((b2 * c1) - (b1 * c2)) / denominator;
        float intersectY = ((a1 * c2) - (a2 * c1)) / denominator;
        float rx0 = (intersectX - p0x) / (p1x - p0x);
        float ry0 = (intersectY - p0y) / (p1y - p0y);
        float rx1 = (intersectX - p2x) / (p3x - p2x);
        float ry1 = (intersectY - p2y) / (p3y - p2y);

        /*
        cout << "p0x: " << p0x << " | p0y: " << p0y << endl;
        cout << "p1x: " << p1x << " | p1y: " << p1y << endl;
        cout << "p2x: " << p2x << " | p2y: " << p2y << endl;
        cout << "p3x: " << p3x << " | p3y: " << p3y << endl;
        cout << "intersectX: " << intersectX << " | intersectY : " << intersectY << endl;
        cout << "rx0: " << rx0 << " | ry0: " << ry0 << " | rx1: " << rx1 << " | ry1: " << ry1 << endl;
        */
        if (((rx0 >= 0 && rx0 <= 1) || (ry0 >= 0 && ry0 <= 1)) &&
            ((rx1 >= 0 && rx1 <= 1) || (ry1 >= 0 && ry1 <= 1))) {
            return true;
        }
    }

    return false;
}

/// <summary>
/// Returns the standard formula of lines Ax+By=C
/// </summary>
/// <param name="p1"></param>
/// <param name="p2"></param>
/// <returns>
/// 0 = A
/// 1 = B
/// 2 = C
/// </returns>
osg::Vec3f raaCarFacarde::lineEquationFrom2Points(osg::Vec2f p1, osg::Vec2f p2) {
    osg::Vec3f result;

    result[0] = p2[1] - p1[1];
    result[1] = p1[0] - p2[0];
    result[2] = result[0] * p1[0] + result[1] * p1[1];

    return result;
}

osg::Vec2f raaCarFacarde::getAxis(osg::Vec2f _1, osg::Vec2f _2) {
    osg::Vec2f normal = _1 - _2;
    normal = osg::Vec2f(-normal[1], normal[0]);
    return normal;
}

/// <summary>
/// Gets the minimum and maximum projection of all corners
/// </summary>
/// <param name="corners"></param>
/// <param name="axis"></param>
/// <returns></returns>
osg::Vec2f raaCarFacarde::findMinMax(osg::Vec2Array* corners, osg::Vec2f axis) {
    osg::Vec2f minMax = osg::Vec2f(0, 0);
    
    minMax[1] = getDot(corners->at(0), axis); // Max
    minMax[0] = minMax[0]; // Min

    for (int i = 1; i < corners->size(); i++) {
        float projection = getDot(corners->at(i), axis);
        if (projection < minMax[0])
            minMax[0] = projection; // New Minimum
        else if (projection > minMax[1])
            minMax[1] = projection; // New Maximum
    }
    return minMax;
}

float raaCarFacarde::getDot(osg::Vec2f v1, osg::Vec2f v2) {
    float product = (v1[0] * v2[0]) + (v1[1] * v2[1]);
    return product;
}

/// <summary>
/// Gets the cars X Rotation as that is the only one that matters. Returns in degrees
/// 0 is east, 90 is north etc...
/// </summary>
/// <returns>Float that contains the X Rotation in degrees</returns>
float raaCarFacarde::getCarRotation() {
    return _rotation;
}

void raaCarFacarde::updateRotation() {
    // Get the world matrices of this car and convert it to euler
    // Just because euler is easier for me to work with than quaternion
    osg::Matrixf m = m_pRoot->getWorldMatrices()[0];
    osg::Quat quat = m.getRotate();

    // I had this in a osg::Vec3f before, but because I only need X, I did this
    // Didn't want to remove the other ones just in case they are needed...
    float xRot;
    //float yRot;
    //float zRot;

    // Convert the quaternion to euler angles
    xRot = atan2f(2 * (quat[0] * quat[1] + quat[2] * quat[3]), 1 - 2 * (quat[1] * quat[1] + quat[2] * quat[2]));

    // Convert the angle to degrees
    xRot = osg::RadiansToDegrees(xRot);

    _rotation = xRot;
}

bool raaCarFacarde::isCarAligned(float colliderRot) {
    float threshold = 45.0f; // Maximum difference is 90 degrees
    float thisRot = getCarRotation();

    if (colliderRot > thisRot - threshold && colliderRot < thisRot + threshold) {
        return true;
    }
    else {
        return false;
    }
    return true;
}

bool raaCarFacarde::isLightAligned(float colliderRot) {
    float threshold = 10.0f;
    float thisRot = roundf(getCarRotation());

    // Rounding to the nearest whole number to avoid casting from -180.0 to -179
    colliderRot = roundf(colliderRot);
    if (colliderRot == -90.0f) {
        colliderRot = 270.0f;
    }
    else if (colliderRot == -180.0f) {
        colliderRot = 180.0f;
    }

    if (thisRot == -90.0f) {
        thisRot = 270.0f;
    }
    else if (thisRot == -180.0f) {
        thisRot = 180.0f;
    }
    // 0 South
    // 90 East
    // 180 North
    // 270 West

    // 0 East
    // 90 North
    // 180 West
    // 270 South

    if (thisRot == 0.0f && colliderRot == 270.0f || 
        thisRot == 90.0f && colliderRot == 0.0f ||
        thisRot == 180.0f && colliderRot == 90.0f ||
        thisRot == 270.0f && colliderRot == 180.0f) {
        return true;
    }
    return false;
}

float raaCarFacarde::getCarSpeed() {
    return this->m_dSpeed;
}

string raaCarFacarde::getCarName() {
    return pPart->getName();
}

/// <summary>
/// Randomly assigns a colour to a car. Only works if the car has the "BodyMain" and "BodyTop" nodes
/// </summary>
void raaCarFacarde::setCarColourAndSize() {
    raaFinder<osg::Node> _bm("BodyMain", pPart);
    raaFinder<osg::Node> _bt("BodyTop", pPart);

    if (_bt.node() && _bm.node()) {
        bodyMainNode = static_cast<osg::ShapeDrawable*>(_bm.node());
        bodyTopNode = static_cast<osg::ShapeDrawable*>(_bt.node());
        // Seed rand() with the current time, otherwise will always run the same
        srand(time(NULL));

        // Create random numbers between 0 and 1 to 2 decimal places
        float one = floorf(((float)(rand()) / (float)(RAND_MAX)) * 100) / 100;
        float two = floorf(((float)(rand()) / (float)(RAND_MAX)) * 100) / 100;
        float three = floorf(((float)(rand()) / (float)(RAND_MAX)) * 100) / 100;

        // Set the same colour to each of the vehicles
        carColour = osg::Vec4(one, two, three, 1);
        bodyMainNode->setColor(carColour);
        bodyTopNode->setColor(carColour);

        carColour = bodyMainNode->getColor();

        osg::BoundingBox box = bodyMainNode->getBoundingBox();
        setSize(box);
    }
    else {
        raaBoundCalculator box(m_pRoot);
        setSize(box.bound());
    }
}

void raaCarFacarde::setSize(osg::BoundingBox box) {
    float xDim = box.xMax() - box.xMin();
    float yDim = box.yMax() - box.yMin();
    float zDim = box.zMax() - box.zMin();

    height = xDim + 20.0f;
    width = zDim;
}

void raaCarFacarde::increaseCarSize(bool toIncrease) {
    if (toIncrease) {
        width += 20.0f;
    }
    else {
        width -= 20.0f;
    }
}

