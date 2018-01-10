#ifndef __KEY_PT_H__
#define __KEY_PT_H__

#include "slBlobAnalyzer.h"
#include <iostream>

#define S_GAUCHE_HAUT 1
#define S_DROITE_HAUT 10

#define S_DROITE_BAS -1
#define S_GAUCHE_BAS -10

#define CONVEXE 5
#define CONCAVE 6

using namespace std;

class KeyPt
{
public:
	KeyPt();
	KeyPt(cv::Point2f position, cv::Point2f centroid, int secteur, int convexite, float angle, cv::Point2f directionAngle, std::vector<float> distancesVoisins, std::pair<cv::Point2f, cv::Point2f> boundingBox);
	
	~KeyPt();

	void determinerSecteur(cv::Point2f ptMin, cv::Point2f ptMax);
	void determinerConvexite();
	void determinerDistanceCentroid();
	void determinerDistancesVoisins();
	void determinerAngle();
	void determinerDirectionAngle();
	//void determinerDirectionAngle();
	void clearVoisins();
	void clearDistancesVoisins();

	// GET
	cv::Point2f getPosition(){return position_;}
	cv::Point2f getCentroid(){return centroid_;}
	int getSecteur(){return secteur_;}
	int getConvexite(){return convexite_;}
	std::vector<cv::Point2f> getVoisins(){return voisins_;}
	float getAngle(){return angle_;}
	cv::Point2f getDirectionAngle(){return directionAngle_;}
	//cv::Point2f getDirectionAngle(){return directionAngle_;}
	float getDistancesCentroid(){return distanceCentroid_;}	
	std::vector<float> getDistancesVoisins(){return distancesVoisins_;}	
	std::pair<cv::Point2f, cv::Point2f> getBoundingBox(){return boundingBox_;}	

	// SET
	void setPosition(cv::Point2f position){position_ = position;}
	void setCentroid(cv::Point2f centroid){centroid_ = centroid;}
	void setSecteur(int secteur){secteur_ = secteur;}
	void setConvexite(int convexite){convexite_ = convexite;}
	void setVoisins(std::vector<cv::Point2f> voisins){voisins_ = voisins;}
	void setAngle(float angle){angle_ = angle;}
	void setDirectionAngle(cv::Point2f directionAngle){directionAngle_ = directionAngle;}
	//void setDirectionAngle(cv::Point2f directionAngle){directionAngle_ = directionAngle;}
	void setDistancesCentroid(float distancesCentroid){distancesCentroid = distancesCentroid;}
	void setDistancesVoisins(std::vector<float> distancesVoisins){distancesVoisins_ = distancesVoisins;}
	void setBoundingBox(std::pair<cv::Point2f, cv::Point2f> boundingBox){boundingBox_ = boundingBox;}
	void clear();
private:
	cv::Point2f position_;
	cv::Point2f centroid_;
	int secteur_;
	int convexite_;
	std::vector<cv::Point2f> voisins_;
	float angle_;
	cv::Point2f directionAngle_;
	//cv::Point2f directionAngle_;
	float distanceCentroid_;
	std::vector<float> distancesVoisins_;
	std::pair<cv::Point2f, cv::Point2f> boundingBox_;
};

#endif