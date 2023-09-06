#include "TrafficLightControl.h"

TrafficLightControl::TrafficLightControl(osg::Node* pPart, osg::Vec3 vTrans, float fRot, float fScale) : raaNodeCallbackFacarde(pPart, vTrans, fRot, fScale)
{
	timeCountGlobal = -1;
	timeCountLocal = 0;
	count = 0;
	isGreen = true;
}

TrafficLightControl::~TrafficLightControl()
{

}

void TrafficLightControl::operator() (osg::Node* node, osg::NodeVisitor* nv)
{
	int noOfLights = m_lTrafficLights.size();
	
	// Will only run the first time, so that I can have them set to green first. Instead of all red at the beginning.
	if (timeCountGlobal == -1) {
		if (noOfLights >= 3) { // If its the 3 traffic light or 4, then only one can be green
			TrafficLightFacarde* selectedFacarde = m_lTrafficLights.front();
			selectedFacarde->setGreenTrafficLight();
		}
		else { // Otherwise, set them all to green
			for (trafficLightList::iterator it = m_lTrafficLights.begin(); it != m_lTrafficLights.end(); it++)
			{
				TrafficLightFacarde* selectedFacarde = *it;
				selectedFacarde->setGreenTrafficLight();
				isGreen = true;
			}
		}
		timeCountGlobal = 0;
	}

	// If the time is over 250, then keep running the code in here. Thats important so I can continue the animations inside here
	if (timeCountGlobal >= 250) {
		if (noOfLights >= 3) { // Like the comment before. 3 or more traffic lights = 1 on. Otherwise, all on.
			iterator = std::next(m_lTrafficLights.begin(), count); // Iterator that is based on the count. Count starts at 0, so it makes sure not to skip the first.
			TrafficLightFacarde* selectedFacarde = *iterator;

			// Iterate through each light in the group
			for (trafficLightList::iterator it = m_lTrafficLights.begin(); it != m_lTrafficLights.end(); it++)
			{
				TrafficLightFacarde* tempFacarde = *it;
				// If this is the same as the selectedFacarde, then do skip this iteration
				// I dont want to turn the selectedFacarde off
				if (tempFacarde == selectedFacarde) {
					continue;
				}

				tempFacarde->setRedTrafficLight();
			}

			// Local time so I can have a finer timer for the change in lights.
			// Same as timeCountGlobal, keep running the code over 50 for the animation
			if (timeCountLocal >= 50) {
				switch (selectedFacarde->m_iTrafficLightStatus) {
					// Each of the case's is annotated on what they mean. Change them from that to the selected light color
				case 1: // Red Light
					selectedFacarde->setAmberTrafficLight();
					break;
				case 2: // Amber Light
					selectedFacarde->setGreenTrafficLight();
					break;
				case 3: // Green Light
					// If its green, then increase the counter so we can select the next facarde
					count++;
					// If counter is out of index range, then go back to 0
					if (count >= noOfLights) {
						count = 0;
					}
					// Reset the global timer so we can restart the main timer
					timeCountGlobal = 0;
					break;
				}
				// Reset the local timer so that 
				timeCountLocal = 0;
			}
			timeCountLocal++; // Time local should increment here so that it doesnt increment if global timer isnt reached.
		}
		else {
			for (trafficLightList::iterator it = m_lTrafficLights.begin(); it != m_lTrafficLights.end(); it++)
			{
				changeTrafficLight(*it);
			}
			// If its on amber, then it doesnt need to take up as much time as red or green do.
			if (m_lTrafficLights.front()->m_iTrafficLightStatus == 2) {
				timeCountGlobal = 150;
			}
			else {
				timeCountGlobal = 0;
			}
		}
	}
	timeCountGlobal++;
}

void TrafficLightControl::changeTrafficLight(TrafficLightFacarde* pTrafficLight)
{
	pTrafficLight->m_iTrafficLightStatus++;
	if (pTrafficLight->m_iTrafficLightStatus > 3)
	{
		pTrafficLight->m_iTrafficLightStatus = 1;
	}

	if (pTrafficLight->m_iTrafficLightStatus == 1)
	{
		pTrafficLight->setRedTrafficLight();
	}
	if (pTrafficLight->m_iTrafficLightStatus == 2)
	{
		pTrafficLight->setAmberTrafficLight();
	}
	if (pTrafficLight->m_iTrafficLightStatus == 3)
	{
		pTrafficLight->setGreenTrafficLight();
	}
}

void TrafficLightControl::addTrafficLight(TrafficLightFacarde* pTrafficLight)
{
	m_lTrafficLights.push_back(pTrafficLight);
}

