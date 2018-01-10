#include "KeyPt.h"


KeyPt::KeyPt()
{
}

KeyPt::KeyPt(cv::Point2f position, cv::Point2f centroid, int secteur, int convexite, float angle, cv::Point2f directionAngle, std::vector<float> distancesVoisins, std::pair<cv::Point2f, cv::Point2f> boundingBox)
{
	position_ = position;
	centroid_ = centroid;
	secteur_ = secteur;
	convexite_ = convexite;
	angle_ = angle;
	directionAngle_ = directionAngle;
	distancesVoisins_ = distancesVoisins;
	boundingBox_ = boundingBox;
}

KeyPt::~KeyPt()
{
	clear();
}

void KeyPt::clear()
{
	voisins_.clear();
	distancesVoisins_.clear();
	angle_ = 0;
}

void KeyPt::determinerSecteur(cv::Point2f ptMin, cv::Point2f ptMax)
{
	float halfDisX = (ptMax.x - ptMin.x)/2;
	float halfDisY = (ptMax.y - ptMin.y)/2;

	float disPointX = position_.x - ptMin.x;
	float disPointY = position_.y - ptMin.y;

	if (disPointX < halfDisX && disPointY < halfDisY)
		secteur_ = S_GAUCHE_HAUT;
	
	else if (disPointX >= halfDisX && disPointY < halfDisY)
		secteur_ = S_DROITE_HAUT;

	else if (disPointX >= halfDisX && disPointY >= halfDisY)
		secteur_ = S_DROITE_BAS;

	else if (disPointX < halfDisX && disPointY >= halfDisY)
		secteur_ = S_GAUCHE_BAS;
}

void KeyPt::determinerConvexite()
{
	cv::Vec3f va = cv::Vec3f(position_.x - voisins_.at(0).x, position_.y - voisins_.at(0).y, 0.0);
	cv::Vec3f vb = cv::Vec3f(voisins_.at(1).x - position_.x, voisins_.at(1).y - position_.y, 0.0);
	
	cv::Vec3f vc = va.cross(vb);
	if (vc(2) > 0)
	{
		convexite_ = CONCAVE;
		//std::cout << "CONCAVE :  x : " << vc(0) << "   y : " << vc(1) << "   z : " << vc(2) << std::endl;
	}
	else
	{
		convexite_ = CONVEXE;
		//std::cout << "CONVEXE :  x : " << vc(0) << "   y : " << vc(1) << "   z : " << vc(2) << std::endl;
	}
}

void KeyPt::determinerDistanceCentroid()
{
	cv::Point2f vecTemp = position_ - centroid_;
	distanceCentroid_ = sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );
}

void KeyPt::determinerDistancesVoisins()
{
	cv::Point2f vecTemp;

	for (unsigned int i = 0; i < voisins_.size(); ++i)
	{
		vecTemp = position_ - voisins_.at(i);
		distancesVoisins_.push_back( sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) ) );
	}
}

void KeyPt::determinerAngle()
{
	// On calcule d'abord la distance entre les 2 voisins
	cv::Point2f vecTemp = voisins_.at(0) - voisins_.at(1);
	float distanceEntreVoisins = sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );
	angle_ = acos
				(    
					(distancesVoisins_.at(0)*distancesVoisins_.at(0) 
					+ distancesVoisins_.at(1)*distancesVoisins_.at(1) 
					- distanceEntreVoisins * distanceEntreVoisins) / (2 * distancesVoisins_.at(0)*distancesVoisins_.at(1))
				);

	// Conversion de radian ver degré
	angle_ = angle_*(float)180/(float)3.14159265;
	//std::cout << "angle " << angle_ << std::endl << std::endl;
}

void KeyPt::determinerDirectionAngle()
{
	// Application de la formule qui calcule la bissectrice d'un angle à partir de 3 points
	// Soit 3 points : P, R et Q
	// Le vecteur normalisé de la bissectrice de l'angle formé par les 2 vecteurs PR et PQ est donné comme suit :
	// Bissectrice normalisé = ( PQ/Norm(PQ) + PR/Norm(PR) ) / Norm( PQ/Norm(PQ) + PR/Norm(PR) )
	// Ce qui suit est une application de cette formule
	cv::Point2f PQ, PR;

	PQ = voisins_.at(1) - position_;
	PR = voisins_.at(0) - position_;

	float distPQ = sqrt( (PQ.x*PQ.x) + (PQ.y*PQ.y) );
	float distPR = sqrt( (PR.x*PR.x) + (PR.y*PR.y) );

	PQ.x = PQ.x/distPQ;
	PQ.y = -PQ.y/distPQ;	// On invers les Y, car l'origine (0,0) de l'axe 2D se trouve au coin haut gauche

	PR.x = PR.x/distPR;
	PR.y = -PR.y/distPR;	// On invers les Y, car l'origine (0,0) de l'axe 2D se trouve au coin haut gauche

	directionAngle_ = PQ + PR;

	float distDirectionAngle = sqrt( (directionAngle_.x*directionAngle_.x) + (directionAngle_.y*directionAngle_.y) );

	// Normaliser le vecteur de la bissectrice
	directionAngle_.x = directionAngle_.x/distDirectionAngle;
	directionAngle_.y = directionAngle_.y/distDirectionAngle;

	//std::cout << "Ditection angle : (" << directionAngle_.x << ", " << directionAngle_.y << ")" << std::endl << std::endl;
}

/*void KeyPt::determinerDirectionAngle()
{
	cv::Point2f vecTemp = centroid_ - position_;

	directionAngle_.x = vecTemp.x/distanceCentroid_;
	directionAngle_.y = vecTemp.y/distanceCentroid_;
}*/

void KeyPt::clearVoisins()
{
	voisins_.clear();
}
void KeyPt::clearDistancesVoisins()
{
	distancesVoisins_.clear();
}