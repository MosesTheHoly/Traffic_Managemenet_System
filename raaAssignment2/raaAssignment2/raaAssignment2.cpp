#define _USE_MATH_DEFINES // Required for PI
#include "raaAnimatedFacarde.h"
#include "raaAnimationPointFinder.h"
#include "raaAssetLibrary.h"
#include "raaCarFacarde.h"
#include "raaFacarde.h"
#include "raaFinder.h"
#include "raaInputController.h"
#include "raaRoadTileFacarde.h"
#include "raaSwitchActivator.h"
#include "raaTrafficSystem.h"
#include "TrafficLightControl.h"
#include "TrafficLightFacarde.h"
#include <osg/AnimationPath>
#include <osg/Material>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osgDB/ReadFile>
#include <osgGA/DriveManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <iostream>
#include <windows.h>

#include "raaBoundCalculator.h" // New
#include <osg/ComputeBoundsVisitor>


using namespace std;
using namespace osg;

typedef vector<raaAnimationPointFinder>raaAnimationPointFinders;
Group* g_pRoot = 0; // root of the sg
float g_fTileSize = 472.441f; // width/depth of the standard road tiles
string g_sDataPath = "../../Data/";
const unsigned int sizeOfLand = 11;
bool landUsageArray[sizeOfLand][sizeOfLand];

enum raaRoadTileType
{
	Normal,
	LitTJunction,
	LitXJunction,
};

/* Realised there is a built in OSG function that does this already, no need for it
float degToRad(float _degrees) {
	return _degrees * (M_PI / 180);
}
*/

void updateLandUsageArray(int xUnit, int yUnit) {
	int centerOfArray = sizeOfLand / 2;
	int _x = centerOfArray + xUnit;
	int _y = centerOfArray + yUnit;

	landUsageArray[_x][_y] = true;
}

void addRoadTile(string sAssetName, string sPartName, int xUnit, int yUnit, float fRot, Group* pParent)
{
	raaFacarde* pFacarde = new raaRoadTileFacarde(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot);
	pParent->addChild(pFacarde->root());
	updateLandUsageArray(xUnit, yUnit);
}

void addTrafficLightSystem(string sAssetName, string sPartName, int xUnit, int yUnit, float fRot, Group* pParent)
{
	float _x = g_fTileSize * xUnit; // Converts grid number to actual coord
	float _y = g_fTileSize * yUnit; // E.g. 2 == 944.882f;

	Group* pTrafficLight = new Group();
	pParent->addChild(pTrafficLight);

	TrafficLightControl* pJunction = new TrafficLightControl(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), 
		Vec3(
			_x, 
			_y, 
			0.0f), fRot, 1.0f);

	pParent->addChild(pJunction->root());

	float _displacement = 200.0f;
	float _angleInRadians = DegreesToRadians(fRot); // Radians is the preferred measurement for this formula

	float _cos = cos(_angleInRadians);
	float _sin = sin(_angleInRadians);

	float _xND = _x - _displacement;
	float _xPD = _x + _displacement;
	float _yND = _y - _displacement;
	float _yPD = _y + _displacement;

	if (sAssetName == "roadXJunction") {
		TrafficLightFacarde* pFacarde1 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".1"), Vec3(_xND, _yND, 0.0f), 0, 0.08f);
		TrafficLightFacarde* pFacarde2 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".2"), Vec3(_xND, _yPD, 0.0f), 0 - 90.0f, 0.08f);
		TrafficLightFacarde* pFacarde3 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".3"), Vec3(_xPD, _yPD, 0.0f), 0 + 180.0f, 0.08f);
		TrafficLightFacarde* pFacarde4 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".4"), Vec3(_xPD, _yND, 0.0f), 0 + 90.0f, 0.08f);

		pJunction->addTrafficLight(pFacarde1);
		pJunction->addTrafficLight(pFacarde2);
		pJunction->addTrafficLight(pFacarde3);
		pJunction->addTrafficLight(pFacarde4);
		pTrafficLight->addChild(pFacarde1->root());
		pTrafficLight->addChild(pFacarde2->root());
		pTrafficLight->addChild(pFacarde3->root());
		pTrafficLight->addChild(pFacarde4->root());
	}
	else if (sAssetName == "roadTJunction") {
		// Quite complicated. https://en.wikipedia.org/wiki/Rotation_(mathematics) For actual formula
		// I changed it a little as I am not rotating around 0,0. I am rotating around a pivot
		// Pivot is actually the default x and y of the object, but the lights are moved around it
		// So to get the rotation for the lights, I minus pivot from the lights
		// That makes it pivot around 0,0
		float _px1 = _cos * (_xND - _x) - _sin * (_yND - _y) + _x;
		float _py1 = _sin * (_xND - _x) + _cos * (_yND - _y) + _y;
		
		
		TrafficLightFacarde* pFacarde1 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".1"), 
			Vec3(
				_px1, 
				_py1, 
				0.0f), fRot, 0.08f);

		// The rest of them, I put into the method to reduce the lines of code...
		TrafficLightFacarde* pFacarde2 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".2"), 
			Vec3(
				_cos * (_xND - _x) - _sin * (_yPD - _y) + _x,
				_sin * (_xND - _x) + _cos * (_yPD - _y) + _y,
				0.0f), fRot - 90.0f, 0.08f);

		TrafficLightFacarde* pFacarde3 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".3"), 
			Vec3(
				_cos * (_xPD - _x) - _sin * (_yPD - _y) + _x,
				_sin * (_xPD - _x) + _cos * (_yPD - _y) + _y,
				0.0f), fRot + 180.0f, 0.08f);

		pJunction->addTrafficLight(pFacarde1);
		pJunction->addTrafficLight(pFacarde2);
		pJunction->addTrafficLight(pFacarde3);
		pTrafficLight->addChild(pFacarde1->root());
		pTrafficLight->addChild(pFacarde2->root());
		pTrafficLight->addChild(pFacarde3->root());
	}
	else if (sAssetName == "roadStraight") {
		TrafficLightFacarde* pFacarde1 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".1"), Vec3(
			_cos * (_xND - _x) - _sin * (_yPD - _y) + _x,
			_sin * (_xND - _x) + _cos * (_yPD - _y) + _y,
			0.0f), fRot - 90.0f, 0.08f);

		TrafficLightFacarde* pFacarde2 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".2"), Vec3(
			_cos * (_xPD - _x) - _sin * (_yND - _y) + _x,
			_sin * (_xPD - _x) + _cos * (_yND - _y) + _y,
			0.0f), fRot + 90.0f, 0.08f);


		pJunction->addTrafficLight(pFacarde1);
		pJunction->addTrafficLight(pFacarde2);
		pTrafficLight->addChild(pFacarde1->root());
		pTrafficLight->addChild(pFacarde2->root());
	}
	else if (sAssetName == "roadCurve") {
		TrafficLightFacarde* pFacarde1 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".1"), Vec3(
			_cos * (_xPD - _x) - _sin * (_yND - _y) + _x,
			_sin * (_xPD - _x) + _cos * (_yND - _y) + _y,
			0.0f), fRot + 90.0f, 0.08f);

		TrafficLightFacarde* pFacarde2 = new TrafficLightFacarde(raaAssetLibrary::getClonedAsset("trafficLight", sPartName + ".2"), Vec3(
			_cos * (_xPD - _x) - _sin * (_yPD - _y) + _x,
			_sin * (_xPD - _x) + _cos * (_yPD - _y) + _y,
			0.0f), fRot + 180.0f, 0.08f);

		pJunction->addTrafficLight(pFacarde1);
		pJunction->addTrafficLight(pFacarde2);
		pTrafficLight->addChild(pFacarde1->root());
		pTrafficLight->addChild(pFacarde2->root());
	}

	updateLandUsageArray(xUnit, yUnit);

}

void fillLand(Group* pParent)
{
	Group* group = new Group();


	ref_ptr<ShapeDrawable> grassBase = new ShapeDrawable(new Box(Vec3(), g_fTileSize * 30, g_fTileSize * 30, 1.0f));
	grassBase->setColor(Vec4(0.541f, 0.769f, 0.369f, 1));

	MatrixTransform* translation = new MatrixTransform;
	Vec3d _translate(0.0f, 0.0f, -1.0f);

	translation->setMatrix(Matrix::translate(_translate));
	translation->addChild(grassBase);

	group->addChild(translation);
	pParent->addChild(group);

	for (int i = 0; i < sizeOfLand; i++) {
		for (int y = 0; y < sizeOfLand; y++) {
			if (landUsageArray[i][y] == false) {
				// Draw some land here...
			}
		}
	}
	//raaFacarde* pFacarde = new raaRoadTileFacarde(raaAssetLibrary::getNamedAsset(sAssetName, sPartName), Vec3(g_fTileSize * xUnit, g_fTileSize * yUnit, 0.0f), fRot);
	//pParent->addChild(pFacarde->root());
}

/* OLD ONE
Node* buildAnimatedVehicleAsset()
{
	Group* pGroup = new Group();

	Geode* pGB = new Geode(); // Add drawable to this later

	// The car drawable, the shapes that will make it
	ShapeDrawable* pGeomB = new ShapeDrawable(
		new Box(Vec3(0.0f, 0.0f, 0.0f), 100.0f, 60.0f, 40.0f)
	);

	
	Material* pMat = new Material();
	Vec4 redCol = *new Vec4(1.0f, 0.0f, 0.0f, 1.0f);
	Vec4 zeroCol = *new Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	Vec4 oneCol = *new Vec4(1.0f, 1.0f, 1.0f, 1.0f);

	pMat->setAmbient(Material::FRONT_AND_BACK, redCol);
	pMat->setDiffuse(Material::FRONT_AND_BACK, oneCol);
	pMat->setSpecular(Material::FRONT_AND_BACK, oneCol);
	

	Material* pMat = new Material();
	pMat->setAmbient(Material::FRONT_AND_BACK, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pMat->setDiffuse(Material::FRONT_AND_BACK, Vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pMat->setSpecular(Material::FRONT_AND_BACK, Vec4(1.0f, 1.0f, 1.0f, 1.0f));

	pGroup->addChild(pGB);

	pGB->addDrawable(pGeomB);

	pGB->getOrCreateStateSet()->setAttribute(pMat, StateAttribute::ON || StateAttribute::OVERRIDE);
	pGB->getOrCreateStateSet()->setAttributeAndModes(new PolygonMode(PolygonMode::FRONT_AND_BACK, PolygonMode::FILL), StateAttribute::ON || StateAttribute::OVERRIDE);

	return pGroup;
}
*/

/// <summary>
/// Drawable gets stored inside a geode, then geode gets stored in the MatrixTransform
/// </summary>
/// <param name="shape">Drawable</param>
/// <param name="matrix">Transformation matrix</param>
/// <returns></returns>
MatrixTransform* createTransformNode(Drawable* shape, const Matrix& matrix) {
	Geode* geode = new Geode;
	geode->addDrawable(shape);

	//MatrixTransform* trans = new MatrixTransform;
	ref_ptr<MatrixTransform> trans = new MatrixTransform;

	trans->addChild(geode);
	trans->setMatrix(matrix);

	return trans.release();
}

Node* buildAnimatedVehicleAsset()
{
	Vec3d _axis100(1, 0, 0);
	Vec3d _axis010(0, 1, 0);
	Vec3d _axis001(0, 0, 1);

	Group* group = new Group();

	Group* car = new Group();

	// Creating all the shapes needed
	ref_ptr<ShapeDrawable> mainRodShape = new ShapeDrawable(new Cylinder(Vec3(), 4.0f, 100.0f));
	ref_ptr<ShapeDrawable> wheelRodShape = new ShapeDrawable(new Cylinder(Vec3(), 4.0f, 60.0f));
	ref_ptr<ShapeDrawable> wheelShape = new ShapeDrawable(new Cylinder(Vec3(), 25.0f, 10.0f));
	ref_ptr<ShapeDrawable> bodyMain = new ShapeDrawable(new Box(Vec3(), 160.0f, 40.0f, 50.0f));
	ref_ptr<ShapeDrawable> bodyTop = new ShapeDrawable(new Box(Vec3(), 60.0f, 50.0f, 50.0f));
	ref_ptr<ShapeDrawable> bodyWindshield = new ShapeDrawable(new Cylinder(Vec3(), 30.0f, 49.6f));
	
	bodyMain->setName("BodyMain");
	bodyTop->setName("BodyTop");


	bodyWindshield->setColor(Vec4(0.435f, 1.0f, 0.98f, 1));
	bodyTop->setColor(Vec4(0, 0.004f, 1, 1));
	bodyMain->setColor(Vec4(0, 0.004f, 1, 1));
	wheelShape->setColor(Vec4(0.102f, 0.102f, 0.102f, 1));
	wheelRodShape->setColor(Vec4(0.651f, 0.651f, 0.651f, 1));
	mainRodShape->setColor(Vec4(0.651f, 0.651f, 0.651f, 1));

	MatrixTransform* body = createTransformNode(bodyMain.get(), Matrix::translate(0.0f, 20.0f, 0.0f));
	body->addChild(createTransformNode(bodyTop.get(), Matrix::translate(20.0f, 20.0f, 0.0f)));
	body->addChild(createTransformNode(bodyWindshield.get(), Matrix::translate(40.0f, 15.0f, 0.0f)));

	MatrixTransform* wheel1 = createTransformNode(wheelShape.get(), Matrix::translate(0.0f, 0.0f, -30.0f));
	MatrixTransform* wheel2 = createTransformNode(wheelShape.get(), Matrix::translate(0.0f, 0.0f, 30.0f));

	MatrixTransform* wheelRod1 = createTransformNode(wheelRodShape.get(), Matrix::translate(50.0f, 0.0f, 0.0f));
	wheelRod1->addChild(wheel1);
	wheelRod1->addChild(wheel2);

	// Instead of repeating the same code, I cast a clone to a MatrixTransform using this.
	// Has to be cast due to clone returning an object, not a MatrixTranform.
	// SHALLOW_COPY basically points to the same object. Where as DEEP_COPY actually copies the whole object.
	MatrixTransform* wheelRod2 = static_cast<MatrixTransform*>(wheelRod1->clone(CopyOp::SHALLOW_COPY));
	wheelRod2->setMatrix(Matrix::translate(-50.0f, 0.0f, 0.0f));

	MatrixTransform* wheelMainRod = createTransformNode(mainRodShape.get(), Matrix::rotate(DegreesToRadians(90.0f), _axis010));
	car->addChild(wheelRod1);
	car->addChild(wheelRod2);
	car->addChild(wheelMainRod);
	car->addChild(body);

	MatrixTransform* rotation = new MatrixTransform;

	// Move the car up a little so the wheels aren't below the ground
	Vec3d _translate(0.0f, 0.0f, 25.0f);

	// Matrix::rotate is in radians. 1.5708 is 90degrees
	rotation->setMatrix(Matrix::rotate(DegreesToRadians(90.0f), _axis100) * Matrix::translate(_translate));
	

	rotation->addChild(car);

	group->addChild(rotation);

	return group;
}

AnimationPath* createAnimationPath(raaAnimationPointFinders apfs, Group* pRoadGroup)
{
	float fAnimTime = 0.0f;
	AnimationPath* ap = new AnimationPath();

	for (int i = 0; i < apfs.size(); i++)
	{
		float fDistance = 0.0f;
		Vec3 vs;
		Vec3 ve;

		vs.set(apfs[i].translation().x(), apfs[i].translation().y(), apfs[i].translation().z());

		if (i == apfs.size() - 1)
			ve.set(apfs[0].translation().x(), apfs[0].translation().y(), apfs[0].translation().z());
		else
			ve.set(apfs[i + 1].translation().x(), apfs[i + 1].translation().y(), apfs[i + 1].translation().z());

		float fXSqr = pow((ve.x() - vs.x()), 2);
		float fYSqr = pow((ve.y() - vs.y()), 2);
		float fZSqr = pow((ve.z() - vs.z()), 2);

		fDistance = sqrt(fXSqr + fYSqr);
		ap->insert(fAnimTime, AnimationPath::ControlPoint(apfs[i].translation(), apfs[i].rotation()));
		fAnimTime += (fDistance / 10.0f);
	}

	return ap;
}

void buildRoad(Group* pRoadGroup)
{
	addTrafficLightSystem("roadXJunction", "tile0", 0, 0, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "tile1", 0, -1, 90.0f, pRoadGroup);

	addTrafficLightSystem("roadCurve", "tile2", 0, -2, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "tile3", 1, -2, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile4", 2, -2, 0.0f, pRoadGroup);

	addTrafficLightSystem("roadTJunction", "tile5", 3, -2, 0.0f, pRoadGroup);

	addRoadTile("roadCurve", "tile6", 3, -3, 90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile7", 2, -3, 0.0f, pRoadGroup);

	addTrafficLightSystem("roadStraight", "tile8", 1, -3, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "tile9", 0, -3, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile10", -1, -3, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile11", -2, -3, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile12", -2, -2, 90.0f, pRoadGroup);

	addTrafficLightSystem("roadXJunction", "tile13", -2, -1, 0.0f, pRoadGroup);

	addRoadTile("roadStraight", "tile14", -3, -1, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile15", -4, -1, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile16", -4, 0, 90.0f, pRoadGroup);

	addTrafficLightSystem("roadStraight", "tile17", -4, 1, 90.0f, pRoadGroup);

	addRoadTile("roadCurve", "tile18", -4, 2, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile19", -3, 2, 0.0f, pRoadGroup);

	addTrafficLightSystem("roadTJunction", "tile20", -2, 2, 90.0f, pRoadGroup);

	addRoadTile("roadStraight", "tile21", -2, 1, 90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile22", -2, 0, 90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile23", -1, 2, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile24", 0, 2, 180.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile25", 0, 1, 90.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile26", -1, -1, 90.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile27", -1, 0, -90.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile28", 1, 0, 0.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile29", 2, 0, 0.0f, pRoadGroup);
	addRoadTile("roadCurve", "tile30", 3, 0, 180.0f, pRoadGroup);
	addRoadTile("roadStraight", "tile31", 3, -1, 90.0f, pRoadGroup);
}


void createCarOne(Group* pRoadGroup)
{
	string carName = "car1";
	raaAnimationPointFinders apfs;
	AnimationPath* ap = new AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile20", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile23", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile23", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile24", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile24", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile24", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile25", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile25", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 7, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 13, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 0, pRoadGroup));

	ap = createAnimationPath(apfs, pRoadGroup);

	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getClonedAsset("vehicle0", carName), ap, 100.0);
	g_pRoot->addChild(pCar->root());
}

void createCarTwo(Group* pRoadGroup)
{
	string carName = "car2";
	raaAnimationPointFinders apfs;
	AnimationPath* ap = new AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile19", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 7, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 14, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 10, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile25", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile25", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile24", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile24", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile24", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile23", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile23", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 9, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile20", 7, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile21", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile22", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 6, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 7, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile14", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile15", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile15", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile15", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile16", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile17", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile17", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile18", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile18", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile18", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile19", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile19", 0, pRoadGroup));

	ap = createAnimationPath(apfs, pRoadGroup);
	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getClonedAsset("vehicle0", carName), ap, 50.0);
	g_pRoot->addChild(pCar->root());
}

void createCarThree(Group* pRoadGroup)
{
	string carName = "car3";
	raaAnimationPointFinders apfs;
	AnimationPath* ap = new AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile5", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile5", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile31", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile31", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile30", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile30", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile30", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile29", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile29", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile28", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile28", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 7, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 12, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile12", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile12", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile10", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile10", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile9", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile9", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile8", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile8", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile7", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile7", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile5", 8, pRoadGroup));

	ap = createAnimationPath(apfs, pRoadGroup);

	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getClonedAsset("vehicle0", carName), ap, 30.0);
	g_pRoot->addChild(pCar->root());
}

void createCarFour(Group* pRoadGroup)
{
	string carName = "car4";
	raaAnimationPointFinders apfs;
	AnimationPath* ap = new AnimationPath();

	apfs.push_back(raaAnimationPointFinder("tile30", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile29", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile29", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile28", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile28", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile0", 7, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile27", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile26", 5, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 11, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 12, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile13", 4, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile12", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile12", 3, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile11", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile10", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile10", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile9", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile9", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile8", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile8", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile7", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile7", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile6", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile5", 8, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile5", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile31", 2, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile31", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile30", 0, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile30", 1, pRoadGroup));
	apfs.push_back(raaAnimationPointFinder("tile30", 2, pRoadGroup));

	ap = createAnimationPath(apfs, pRoadGroup);
	raaCarFacarde* pCar = new raaCarFacarde(g_pRoot, raaAssetLibrary::getNamedAsset("vehicle0", carName), ap, 10.0);
	g_pRoot->addChild(pCar->root());
}

int main(int argc, char** argv)
{
	raaAssetLibrary::start();
	raaTrafficSystem::start();

	osgViewer::Viewer viewer;

	for (int i = 0; i < argc; i++)
	{
		if (string(argv[i]) == "-d") g_sDataPath = argv[++i];
	}

	// The root of the scene
	g_pRoot = new Group();
	g_pRoot->ref();

	// Build asset library - instances or clones of parts can be created from this
	raaAssetLibrary::loadAsset("roadStraight", g_sDataPath + "roadStraight.osgb");
	raaAssetLibrary::loadAsset("roadCurve", g_sDataPath + "roadCurve.osgb");
	raaAssetLibrary::loadAsset("roadTJunction", g_sDataPath + "roadTJunction.osgb");
	raaAssetLibrary::loadAsset("roadXJunction", g_sDataPath + "roadXJunction.osgb");
	raaAssetLibrary::loadAsset("trafficLight", g_sDataPath + "raaTrafficLight.osgb");
	raaAssetLibrary::insertAsset("vehicle0", buildAnimatedVehicleAsset());

	//raaAssetLibrary::loadAsset("vehicle1", g_sDataPath + "car-delta.OSGB"); // Always wrong rotation...


	// add a group node to the scene to hold the road sub-tree
	Group* pRoadGroup = new Group();
	g_pRoot->addChild(pRoadGroup);

	// Create road
	buildRoad(pRoadGroup);
	fillLand(g_pRoot);

	// Add car one
	createCarOne(pRoadGroup);
	createCarTwo(pRoadGroup);
	createCarThree(pRoadGroup);
	createCarFour(pRoadGroup);


	#pragma region osg setup stuff

	GraphicsContext::Traits* pTraits = new GraphicsContext::Traits();
	pTraits->x = 20;
	pTraits->y = 20;
	pTraits->width = 1200;
	pTraits->height = 960;
	pTraits->windowDecoration = true;
	pTraits->doubleBuffer = true;
	pTraits->sharedContext = 0;

	GraphicsContext* pGC = GraphicsContext::createGraphicsContext(pTraits);
	osgGA::KeySwitchMatrixManipulator* pKeyswitchManipulator = new osgGA::KeySwitchMatrixManipulator();
	pKeyswitchManipulator->addMatrixManipulator('1', "Trackball", new osgGA::TrackballManipulator());
	pKeyswitchManipulator->addMatrixManipulator('2', "Flight", new osgGA::FlightManipulator());
	pKeyswitchManipulator->addMatrixManipulator('3', "Drive", new osgGA::DriveManipulator());
	viewer.setCameraManipulator(pKeyswitchManipulator);
	Camera* pCamera = viewer.getCamera();

	// Set the camera position, I prefer a top down view at the start instead of side profile
	Vec3d eye(0.0, 0.0, 10000.0); // Original [2] was 10000.0
	Vec3d center(0.0, 0.0, 0.0);
	Vec3d up(0.0, 10.0, 0.0);
	viewer.getCameraManipulator()->setHomePosition(eye, center, up);

	pCamera->setGraphicsContext(pGC);
	pCamera->setViewport(new Viewport(0, 0, pTraits->width, pTraits->height));

	// add own event handler - this currently switches on an off the animation points
	viewer.addEventHandler(new raaInputController(g_pRoot));

	// add the state manipulator
	viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));

	// add the thread model handler
	viewer.addEventHandler(new osgViewer::ThreadingHandler);

	// add the window size toggle handler
	viewer.addEventHandler(new osgViewer::WindowSizeHandler);

	// add the stats handler
	viewer.addEventHandler(new osgViewer::StatsHandler);

	// add the record camera path handler
	viewer.addEventHandler(new osgViewer::RecordCameraPathHandler);

	// add the LOD Scale handler
	viewer.addEventHandler(new osgViewer::LODScaleHandler);

	// add the screen capture handler
	viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

	// set the scene to render
	viewer.setSceneData(g_pRoot);

	viewer.realize();

	return viewer.run();
	#pragma endregion
}


