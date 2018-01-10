#include "Outils.h"
#include <iostream>
#include <fstream>
#include <sstream>
namespace Outils
{
	CvMat* Ransac(std::vector<Point> &points1, std::vector<Point> &points2)
	{
		// La taille des deux vecteurs doivent être identiques
		if (points1.size() == points2.size())
		{
			unsigned int taille = points1.size();
			int ind = 0;
			int t = 0;

			float *mat1 = new float[taille*2];
			float *mat2 = new float[taille*2];

			for (unsigned int i = 0; i < taille; ++i)
			{
				mat1[t] = (float)points1.at(i).x;
				mat1[t+1] = (float)points1.at(i).y;

				mat2[t] = (float)points2.at(i).x;
				mat2[t+1] = (float)points2.at(i).y;

				t += 2;
			}

			CvMat mImgGauche, mImgDroite;
			cvInitMatHeader(&mImgGauche,   taille, 2, CV_32FC1, mat1);
			cvInitMatHeader(&mImgDroite,   taille, 2, CV_32FC1, mat2);

			CvMat *h1 = cvCreateMat(3, 3, CV_32FC1);
			cvFindHomography(&mImgGauche, &mImgDroite, h1, CV_RANSAC);

			return h1;
		}

		else
			return NULL;
	}

	CvMat* Ransac(std::vector<KeyPt> &points1, std::vector<KeyPt> &points2)
	{
		// La taille des deux vecteurs doivent être identiques
		if (points1.size() == points2.size())
		{
			unsigned int taille = points1.size();
			int ind = 0;
			int t = 0;

			float *mat1 = new float[taille*2];
			float *mat2 = new float[taille*2];

			for (unsigned int i = 0; i < taille; ++i)
			{
				mat1[t] = (float)points1.at(i).getPosition().x;
				mat1[t+1] = (float)points1.at(i).getPosition().y;

				mat2[t] = (float)points2.at(i).getPosition().x;
				mat2[t+1] = (float)points2.at(i).getPosition().y;

				t += 2;
			}

			CvMat mImgGauche, mImgDroite;
			cvInitMatHeader(&mImgGauche,   taille, 2, CV_32FC1, mat1);
			cvInitMatHeader(&mImgDroite,   taille, 2, CV_32FC1, mat2);
			delete[] mat1;
			delete[] mat2;
			CvMat *h1 = cvCreateMat(3, 3, CV_32FC1);
			cvFindHomography(&mImgGauche, &mImgDroite, h1, CV_RANSAC);

			return h1;
		}

		else
			return NULL;
	}

	std::vector<CvMat*> CV_Ransac_Simple(std::vector<Point> points1, std::vector<Point> points2)
	{
		std::vector<Point> pt1, pt2;
		int tailleMin = 0;
		int tailleMax = 0;
		bool minIsLeft = false;

		if (points1.size() <= points2.size())
		{
			tailleMin = points1.size(); 
			tailleMax = points2.size();
			minIsLeft = true;
		}
		else
		{
			tailleMin = points2.size();
			tailleMax = points1.size();
			minIsLeft = false;
		}

		std::vector<CvMat*> mats;
		int ind = 0;
		// Pour chaque décalage de point entre le nombre de points de gauche par rapport à celui de droite
		float dist = 0;
		float distTemp;
		int t = 0;
		int indDist = 0;
		float *matGauche = new float[tailleMin*2];
		float *matDroite = new float[tailleMin*2];

		for (int i = 0; i < tailleMin; ++i)
		{
			if (minIsLeft)
			{
				float xVal, yVal;
				matGauche[t] = (float)points1.at(i).x;
				matGauche[t+1] = (float)points1.at(i).y;

				xVal = (float)points1.at(i).x -(float)points2.at(0).x;
				yVal = (float)points1.at(i).y -(float)points2.at(0).y;

				dist = sqrtf(xVal*xVal + yVal*yVal);

				for (int j = 1; j < tailleMax; ++j)
				{
					xVal = (float)points1.at(i).x -(float)points2.at(j).x;
					yVal = (float)points1.at(i).y -(float)points2.at(j).y;

					distTemp = sqrtf(xVal*xVal + yVal*yVal);

					if (distTemp < dist)
					{
						dist = distTemp;
						indDist = j;
					}
				}

				matDroite[t] = (float)points2.at(indDist).x;
				matDroite[t+1] = (float)points2.at(indDist).y;
			}

			else
			{
				float xVal, yVal;
				matGauche[t] = (float)points2.at(i).x;
				matGauche[t+1] = (float)points2.at(i).y;

				xVal = (float)points2.at(i).x -(float)points1.at(0).x;
				yVal = (float)points2.at(i).y -(float)points1.at(0).y;

				dist = sqrtf(xVal*xVal + yVal*yVal);

				for (int j = 1; j < tailleMax; ++j)
				{
					xVal = (float)points2.at(i).x -(float)points1.at(j).x;
					yVal = (float)points2.at(i).y -(float)points1.at(j).y;

					distTemp = sqrtf(xVal*xVal + yVal*yVal);

					if (distTemp < dist)
					{
						dist = distTemp;
						indDist = j;
					}
				}

				matDroite[t] = (float)points1.at(indDist).x;
				matDroite[t+1] = (float)points1.at(indDist).y;
			}

			t += 2;

		
		}

		CvMat mImgGauche, mImgDroite;
		cvInitMatHeader(&mImgGauche,   tailleMin, 2, CV_32FC1, matGauche);
		cvInitMatHeader(&mImgDroite,   tailleMin, 2, CV_32FC1, matDroite);

		CvMat *h1 = cvCreateMat(3, 3, CV_32FC1);

		if (minIsLeft)
		{
			cvFindHomography(&mImgGauche, &mImgDroite, h1, CV_RANSAC);
			//cvFindFundamentalMat(&mImgGauche, &mImgDroite, h1, CV_FM_RANSAC);
		}

		else
		{
			cvFindHomography(&mImgDroite, &mImgGauche, h1, CV_RANSAC);
			//cvFindFundamentalMat(&mImgDroite, &mImgGauche, h1, CV_FM_RANSAC);
		}
	
		mats.push_back(h1);	
		return mats;
	}

	void drawImage (std::vector<cv::Point> points, slImage3ch &output)
	{
		// Empty output image
		output = 0;

		for (unsigned int  i = 0; i < points.size(); ++i)
		{
			circle(output, points[i], 2, CV_RGB(0, 0, 255), 1, 8, 0);
		}
	}

	std::vector<cv::Point> extractPoints(std::vector<slKeyPoints> keysPoints)
	{
		std::vector<cv::Point> points;

		for (unsigned int i = 0; i < keysPoints.size(); ++i)
		{
			for (slKeyPoints::const_iterator itPoint = keysPoints[i].begin();
			itPoint != keysPoints[i].end(); itPoint++)
			{
				points.push_back(Point(itPoint->first));
			}
		}

		return points;
	}

	std::vector<cv::Point> extractPoints(std::vector<KeyPoint> keysPoints)
	{
		std::vector<cv::Point> points;

		for (unsigned int i = 0; i < keysPoints.size(); ++i)
		{

			points.push_back(keysPoints.at(i).pt);

		}

		return points;
	}

	std::vector<KeyPoint> extractKeyPoints(std::vector<slKeyPoints> keysPoints)
	{
		std::vector<KeyPoint> points;
		KeyPoint keyTemp;
		double minRef;
		double min;
	
		for (unsigned int i = 0; i < keysPoints.size(); ++i)
		{
			for (slKeyPoints::const_iterator itPoint = keysPoints[i].begin();
			itPoint != keysPoints[i].end(); itPoint++)
			{
				keyTemp.pt = Point(itPoint->first);

				minRef = 100000;

				for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors(); itNb != itPoint->second.endNeighbors(); itNb++)
				{
					Point p1, p2, p3;
					p1 = Point(itPoint->first);
					p2 = Point(*itNb);

					p3 = p1-p2;
					min = norm(p3);
					if (min < minRef)
						minRef = min;
				}
				keyTemp.size = (float)minRef;
				points.push_back(keyTemp);
			}
		}

		return points;
	}

	std::vector<slKeyPoints> extractKeysPoints(const slContours &contours, const slBlobAnalyzer *ba)
	{
		// Empty output image
		//output.create(fg.size());
		//output = 0;

		std::vector<slKeyPoints> keyPoints;

		// For each external contour
		for (slContours::const_iterator contour = contours.begin();
			!contour.isNull(); contour = contour.next())
		{
			if (ba->hasKeyPoints(contour)) {
				//paintKeyPoints(ba->getKeyPoints(contour), CV_RGB(0, 255, 0), output);
				keyPoints.push_back(ba->getKeyPoints(contour));

				// For each hole or internal contour
				//for (slContours::const_iterator child = contour.child();
				//	!child.isNull(); child = child.next())
				//{
				//	if (ba->hasKeyPoints(child)) {
				//		keyPoints.push_back(ba->getKeyPoints(child));
				//		//paintKeyPoints(ba->getKeyPoints(child), CV_RGB(255, 0, 0), output);
				//	}
				//}
			}
		}
		return keyPoints;
	}

	void paint(const slImage1ch fg, const slContours &contours, const slBlobAnalyzer *ba, slImage3ch &output)
	{
		// Empty output image
		output.create(fg.size());
		output = 0;

		// For each external contour
		for (slContours::const_iterator contour = contours.begin();
			!contour.isNull(); contour = contour.next())
		{
			if (ba->hasKeyPoints(contour)) {
				paintKeyPoints(ba->getKeyPoints(contour), CV_RGB(0, 255, 0), output);

				// For each hole or internal contour
				/*for (slContours::const_iterator child = contour.child();
					!child.isNull(); child = child.next())
				{
					if (ba->hasKeyPoints(child)) {
						paintKeyPoints(ba->getKeyPoints(child), CV_RGB(255, 0, 0), output);
					}
				}*/
			}
		}
	}

	void paintKeyPoints(const slKeyPoints &kPt, cv::Scalar color, slImage3ch &output)
	{
		for (slKeyPoints::const_iterator itPoint = kPt.begin();
			itPoint != kPt.end(); itPoint++)
		{
			circle(output, Point(itPoint->first), 2, CV_RGB(0, 0, 255), 1, 8, 0);
			for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors();
				itNb != itPoint->second.endNeighbors(); itNb++)
			{
				if (CvPoint2fLessThan()(itPoint->first, *itNb)) {
					line(output, Point(itPoint->first), Point(*itNb), color);
					//circle(output, Point(itPoint->first), 2, CV_RGB(0, 0, 255), 1, 8, 0);
				}
			}
		}
	}

	void paintKeyPoints(const slImage1ch fg, std::vector<std::vector<KeyPt>> vecKeyPts, cv::Scalar vertexColor, cv::Scalar edgeColor, slImage3ch &output)
	{
		output.create(fg.size());
		output = 0;

		for (unsigned int k = 0; k < vecKeyPts.size(); ++k)
		{
			for (unsigned int l = 0; l < vecKeyPts.at(k).size(); ++l)
			{
				if (l == 0)
					circle(output, vecKeyPts.at(k).at(l).getPosition(), 2, CV_RGB(255, 0, 0), 1, 8, 0);
				else
					circle(output, vecKeyPts.at(k).at(l).getPosition(), 2, vertexColor, 1, 8, 0);
				if (l < vecKeyPts.at(k).size() - 1)
					line(output, vecKeyPts.at(k).at(l).getPosition(), vecKeyPts.at(k).at(l+1).getPosition(), edgeColor);
				else
					line(output, vecKeyPts.at(k).at(l).getPosition(), vecKeyPts.at(k).at(0).getPosition(), edgeColor);
			}
		}
	}

	

	std::vector<KeyPoint> convertPoints2KeyPoints(std::vector<cv::Point> points)
	{
		std::vector<KeyPoint> kps;
		KeyPoint kp;
		for (unsigned int i = 0; i < points.size(); ++i)
		{
			kp.pt = points.at(i);
			kp.angle = -1;
			kp.size = 20;
			//kp.response = 0;
			kps.push_back(kp);
		}

		return kps;
	}
	std::vector<Point> pointsTransform(std::vector<Point> points, cv::Mat transMat)
	{
		std::vector<Point> modifiedPoints;
		Point2f src;
		Point dst;

		for (unsigned int i = 0; i < points.size(); ++i)
		{
			src = points.at(i);

			vector<Point3f> vec; 

			// On change le point 2D en point 3D
			vec.push_back(Point3f(src.x,src.y,1)); 

			// On applique une transposé pour avoir la bonne orientation de la matrice avant la multiplication
			Mat srcMat = Mat(vec).reshape(1).t(); 
			Mat dstMat = transMat*srcMat; // Faire la multiplication 

			// Suite < la multiplication, on obtient un vecteur [a, b, c] où c est différent de 1
			// Il faut normaliser afin de ramener en un vecteur 2D
			// Pour ce faire, on divise a et b par c -> a = a/c et b = b/c
			// dstMat.at<float>(0,0) = a,  dstMat.at<float>(1,0) = b  et dstMat.at<float>(2,0) = c
			dst = (Point) Point2f(dstMat.at<float>(0,0) / dstMat.at<float>(2,0), dstMat.at<float>(1,0) / dstMat.at<float>(2,0)); 
			//std::cout<<dst.x<<" "<<dst.y<<std::endl; 
			modifiedPoints.push_back(dst);
		}
		return modifiedPoints;
	}

	std::vector<KeyPt> pointsTransform(std::vector<KeyPt> points, cv::Mat transMat)
	{
		std::vector<KeyPt> modifiedPoints;
		Point2f src;
		Point2f dst;

		for (unsigned int i = 0; i < points.size(); ++i)
		{
			src = points.at(i).getPosition();

			vector<Point3f> vec; 

			// On change le point 2D en point 3D
			vec.push_back(Point3f(src.x,src.y,1)); 

			// On applique une transposé pour avoir la bonne orientation de la matrice avant la multiplication
			Mat srcMat = Mat(vec).reshape(1).t(); 
			Mat dstMat = transMat*srcMat; // Faire la multiplication

			// Suite < la multiplication, on obtient un vecteur [a, b, c] où c est différent de 1
			// Il faut normaliser afin de ramener en un vecteur 2D
			// Pour ce faire, on divise a et b par c -> a = a/c et b = b/c
			// dstMat.at<float>(0,0) = a,  dstMat.at<float>(1,0) = b  et dstMat.at<float>(2,0) = c
			dst = (Point) Point2f(dstMat.at<float>(0,0) / dstMat.at<float>(2,0), dstMat.at<float>(1,0) / dstMat.at<float>(2,0)); 
			points.at(i).setPosition(dst);
			//std::cout<<dst.x<<" "<<dst.y<<std::endl; 
			modifiedPoints.push_back(points.at(i));
		}
		return modifiedPoints;
	}

	std::vector<std::vector<KeyPt>> pointsTransform(std::vector<std::vector<KeyPt>> points, cv::Mat transMat)
	{
		std::vector<std::vector<KeyPt>> modifiedPoints;
		Point2f src;
		Point2f dst;

		for (unsigned int i = 0; i < points.size(); ++i)
		{
			for (unsigned int j = 0; j < points.at(i).size(); ++j)
			{
				src = points.at(i).at(j).getPosition();

				vector<Point3f> vec; 

				// On change le point 2D en point 3D
				vec.push_back(Point3f(src.x,src.y,1)); 

				// On applique une transposé pour avoir la bonne orientation de la matrice avant la multiplication
				Mat srcMat = Mat(vec).reshape(1).t(); 
				Mat dstMat = transMat*srcMat; // Faire la multiplication

				// Suite < la multiplication, on obtient un vecteur [a, b, c] où c est différent de 1
				// Il faut normaliser afin de ramener en un vecteur 2D
				// Pour ce faire, on divise a et b par c -> a = a/c et b = b/c
				// dstMat.at<float>(0,0) = a,  dstMat.at<float>(1,0) = b  et dstMat.at<float>(2,0) = c
				dst = (Point) Point2f(dstMat.at<float>(0,0) / dstMat.at<float>(2,0), dstMat.at<float>(1,0) / dstMat.at<float>(2,0)); 
				points.at(i).at(j).setPosition(dst);
				//std::cout<<dst.x<<" "<<dst.y<<std::endl; 
				
			}
			modifiedPoints.push_back(points.at(i));
		}
		return modifiedPoints;
	}

	float computeMeanEuclideanError(std::vector<Point> points1, std::vector<Point> points2, unsigned int frameNumber)
	{
		float result = 0;
		float diffx;
		float diffy;

		for (unsigned int i = 0; i < points1.size(); ++i)
		{
			diffx = (float)points1.at(i).x - (float)points2.at(i).x;
			diffy = (float)points1.at(i).y - (float)points2.at(i).y;
			result += sqrt(diffx*diffx + diffy*diffy);
		}

		result = result/points1.size();
		ofstream log;

		log.open("ErrorLog.txt", ios::out | ios::app);
		stringstream ss;
		ss << "Frame - " << frameNumber << " : " << result << "\n";
		log << ss.str() ;
		log.close();

		// result = somme des distances entre chaque point
		return result;
	}

	float computeMeanEuclideanError(std::vector<KeyPt> points1, std::vector<KeyPt> points2, unsigned int frameNumber)
	{
		float result = 0;
		float diffx;
		float diffy;

		for (unsigned int i = 0; i < points1.size(); ++i)
		{
			diffx = (float)points1.at(i).getPosition().x - (float)points2.at(i).getPosition().x;
			diffy = (float)points1.at(i).getPosition().y - (float)points2.at(i).getPosition().y;
			result += sqrt(diffx*diffx + diffy*diffy);
		}

		result = result/points1.size();
		ofstream log;

		log.open("ErrorLog.txt", ios::out | ios::app);
		stringstream ss;
		ss << "Frame - " << frameNumber << " : " << result << "\n";
		log << ss.str() ;
		log.close();

		// result = somme des distances entre chaque point
		return result;
	}
	
	void descriptorMatchLSS(slImage3ch im1, slImage3ch im2, std::vector<Point> &points1, std::vector<Point> &points2)
	{
		slWindow g1("AA"), g2("BB");

		g1.show(im1);
		g2.show(im2);

		waitKey();
		// Initialisation
		SelfSimDescriptor ssd;
		std::vector<std::vector<float>> LSSDescriptors1, LSSDescriptors2;
		std::vector<float> d1, d2;
		std::vector<Point> vTemp;
		
		float distance = 0;
		// Pour chauqe point clé de l'image 1, on extrait le descripteur LSS
		for (unsigned int i = 0; i < points1.size(); ++i)
		{
			vTemp.push_back(points1.at(i));
			ssd.compute(im1, d1, cv::Size(im1.size().width, im1.size().height), vTemp);
			
			LSSDescriptors1.push_back(d1);
			d1.clear();
			vTemp.clear();
		}
		
		// Pour chauqe point clé de l'image 1, on extrait le descripteur LSS
		for (unsigned int j = 0; j < points2.size(); ++j)
		{
			vTemp.push_back(points2.at(j));
			ssd.compute(im2, d2, cv::Size(im2.size().width, im2.size().height), vTemp);
			
			LSSDescriptors2.push_back(d2);	
			d1.clear();
			vTemp.clear();
		}
		
		/*slWindow winGraph6("IR"), winGraph7("VISIBLE");
		winGraph6.show(im1);
		winGraph7.show(im2);

		waitKey();*/

		// On fait une copie des points2
		std::vector<Point> pts2 = points2;
		
		// Réinitialiser les points2 pour les réassigner selon les correspondances avec les points1
		points2.clear();

		// Pour chaque descripteur des points1, on trouve le match des points2
		for (int ind1 = 0; ind1 < LSSDescriptors1.size(); ++ind1)
		{
			float distMin = 9999.0;
			float distTemp = 0.0;
			int indexMin = 0;

			for (int ind2 = 0; ind2 < LSSDescriptors2.size(); ++ind2)
			{
				distTemp = LSSDescriptorDistance(LSSDescriptors1.at(ind1), LSSDescriptors2.at(ind2));
				if (distTemp < distMin)
				{
					distMin = distTemp;
					indexMin = ind2;
				}
			}
			points2.push_back(pts2.at(indexMin));
		}
	}

	float LSSDescriptorDistance(std::vector<float> descriptor1, std::vector<float> descriptor2)
	{
		float diff, result;
		result = 0;
		
		// Calcul de la distance entre chaque élément des deux descripteurs de 80 éléments
		for (unsigned int i = 0; i < descriptor1.size(); ++i)
		{
			diff = descriptor1.at(i) - descriptor2.at(i);
			result += diff * diff;
		}
		
		// Retourner le résultat
		return sqrt(result);
	}

	std::vector<KeyPt> convert2KeyPt(std::vector<slKeyPoint> &sortedKeysPoints)
	{
		std::vector<KeyPt> keyPts;
		std::vector<cv::Point2f> pointsVoisins;
		cv::Point2f bbMin, bbMax;
		std::pair<cv::Point2f, cv::Point2f> bb;
		KeyPt key;

		// Obternir la position du centroïde pour le calcul de la convexité
		cv::Point2f centroid = getCentroid(sortedKeysPoints);

		// Déterminer les dimentions du boundingbox des points (du blob)
		computeBlobBoundingBox(bbMin, bbMax, sortedKeysPoints);
		bb.first = bbMin;
		bb.second = bbMax;
		
		for (unsigned int i = 0; i < sortedKeysPoints.size(); ++i)
		{
			key.setBoundingBox(bb);
			key.setCentroid(centroid);				// Ajout du centroid du blob
			key.setPosition(sortedKeysPoints.at(i).position());	// Position du point
			//key.determinerSecteur(bbMin, bbMax);				// Le secteur où se trouve le point par rapport au bounding box du blob

			for (CvPt2fVector_t::const_iterator itNb = sortedKeysPoints.at(i).beginNeighbors(); itNb != sortedKeysPoints.at(i).endNeighbors(); itNb++)
				pointsVoisins.push_back(Point2f(*itNb));

			//std::cout << "Position : (" << key.getPosition().x << ", " << key.getPosition().y << ")" << std::endl << std::endl;

			key.setVoisins(pointsVoisins);			// Ajoute les 2 voisins (précédent et suivant) du point
			pointsVoisins.clear();

			//key.determinerDistanceCentroid();		// Déterminer la distance entre le point et le centroid
			key.determinerConvexite();				// Déterminer si le somet est convexe ou concave
			key.determinerDistancesVoisins();		// Déterminer les distances entre le point et les voisins
			key.determinerAngle();					// Déterminer l'angle du sommet
			//key.determinerDirectionAngle();				// Déterminer la direction de l'angle vers le centroïde

			// On ajoute le point créé dans le vecteur de points
			keyPts.push_back(key);
			key.clear();
		}

		//std::cout << "=====================================" << std::endl << std::endl;

		return keyPts;
	}

	std::vector<KeyPt> convert2KeyPt(std::vector<std::vector<KeyPt>> &sortedKeysPoints)
	{
		std::vector<KeyPt> keyPts;
		std::vector<cv::Point2f> pointsVoisins;
		cv::Point2f bbMin, bbMax;
		std::pair<cv::Point2f, cv::Point2f> bb;
		cv::Point2f centroid;
		KeyPt key;

		for (unsigned int ind = 0; ind < sortedKeysPoints.size(); ++ind)
		{
			for (unsigned int ind2 = 0; ind2 < sortedKeysPoints.at(ind).size(); ++ind2)
			{
				// Si c'est le premier point
				if (ind2 == 0)
				{
					int pos = sortedKeysPoints.at(ind).size() - 1;
					// Le dernier point = premier voisin
					pointsVoisins.push_back(sortedKeysPoints.at(ind).at(pos).getPosition());
					pointsVoisins.push_back(sortedKeysPoints.at(ind).at(ind2 + 1).getPosition());

					sortedKeysPoints.at(ind).at(ind2).setVoisins(pointsVoisins);
				}

				// Si c'est le dernier point
				else if (ind2 == sortedKeysPoints.at(ind).size() - 1)
				{
					// Voisin suivant = premier point
					pointsVoisins.push_back(sortedKeysPoints.at(ind).at(ind2 - 1).getPosition());
					pointsVoisins.push_back(sortedKeysPoints.at(ind).at(0).getPosition());

					sortedKeysPoints.at(ind).at(ind2).setVoisins(pointsVoisins);
					
				}

				// Dans tous les autre cas
				else
				{
					pointsVoisins.push_back(sortedKeysPoints.at(ind).at(ind2 - 1).getPosition());
					pointsVoisins.push_back(sortedKeysPoints.at(ind).at(ind2 + 1).getPosition());

					sortedKeysPoints.at(ind).at(ind2).setVoisins(pointsVoisins);
				}

				pointsVoisins.clear();
			}
		}





		


		
		
		for (unsigned int it = 0; it < sortedKeysPoints.size(); ++it)
		{
			// Déterminer les dimentions du boundingbox des points (du blob)
			computeBlobBoundingBox(bbMin, bbMax, sortedKeysPoints.at(it));
			bb.first = bbMin;
			bb.second = bbMax;

			// Obternir la position du centroïde pour le calcul de la convexité
			//std::vector<cv::Point2f> centroid = getCentroid(sortedKeysPoints);
			centroid = getCentroid(sortedKeysPoints.at(it));

			for (unsigned int i = 0; i < sortedKeysPoints.at(it).size(); ++i)
			{
				// Faire le ménage des distancesVoisins
				sortedKeysPoints.at(it).at(i).clearDistancesVoisins();

				sortedKeysPoints.at(it).at(i).setBoundingBox(bb);

				//sortedKeysPoints.at(it).at(i).setCentroid(centroid);				// Ajout du centroid du blob
				//sortedKeysPoints.at(it).at(i).determinerSecteur(bbMin, bbMax);				// Le secteur où se trouve le point par rapport au bounding box du blob

				//sortedKeysPoints.at(it).at(i).determinerDistanceCentroid();		// Déterminer la distance entre le point et le centroid
				sortedKeysPoints.at(it).at(i).determinerConvexite();				// Déterminer si le somet est convexe ou concave
				sortedKeysPoints.at(it).at(i).determinerDistancesVoisins();		// Déterminer les distances entre le point et les voisins
				sortedKeysPoints.at(it).at(i).determinerAngle();					// Déterminer l'angle du sommet
				//sortedKeysPoints.at(it).at(i).determinerDirectionAngle();				// Déterminer la direction de l'angle vers le centroïde

				// On ajoute le point créé dans le vecteur de points
				keyPts.push_back(sortedKeysPoints.at(it).at(i));
			}
		}
		//std::cout << "=====================================" << std::endl << std::endl;

		return keyPts;
	}

	void computeBlobBoundingBox(cv::Point2f &bbMin, cv::Point2f &bbMax, std::vector<slKeyPoint> &sortedKeysPoints)
	{
		bbMin = sortedKeysPoints.at(0).position();
		bbMax = sortedKeysPoints.at(0).position();

		// Déterminer les dimentions du boundingbox des points
		for (unsigned int i = 1; i < sortedKeysPoints.size(); ++i)
		{
			if (sortedKeysPoints.at(i).position().x < bbMin.x)
				bbMin.x = sortedKeysPoints.at(i).position().x;

			if (sortedKeysPoints.at(i).position().y < bbMin.y)
				bbMin.y = sortedKeysPoints.at(i).position().y;


			if (sortedKeysPoints.at(i).position().x > bbMax.x)
				bbMax.x = sortedKeysPoints.at(i).position().x;

			if (sortedKeysPoints.at(i).position().y > bbMax.y)
				bbMax.y = sortedKeysPoints.at(i).position().y;
		}
	}

	void computeBlobBoundingBox(cv::Point2f &bbMin, cv::Point2f &bbMax, std::vector<KeyPt> &sortedKeysPoints)
	{
		bbMin = sortedKeysPoints.at(0).getPosition();
		bbMax = sortedKeysPoints.at(0).getPosition();

		// Déterminer les dimentions du boundingbox des points
		for (unsigned int i = 1; i < sortedKeysPoints.size(); ++i)
		{
			if (sortedKeysPoints.at(i).getPosition().x < bbMin.x)
				bbMin.x = sortedKeysPoints.at(i).getPosition().x;

			if (sortedKeysPoints.at(i).getPosition().y < bbMin.y)
				bbMin.y = sortedKeysPoints.at(i).getPosition().y;


			if (sortedKeysPoints.at(i).getPosition().x > bbMax.x)
				bbMax.x = sortedKeysPoints.at(i).getPosition().x;

			if (sortedKeysPoints.at(i).getPosition().y > bbMax.y)
				bbMax.y = sortedKeysPoints.at(i).getPosition().y;
		}
	}

	std::vector<slKeyPoint> sortKeyPoints(slKeyPoints &keysPoints)
	{
		std::vector<slKeyPoint> sortedKeyPoints;
		slKeyPoints::const_iterator itPoint = keysPoints.begin();

		// On ajoute le premier point
		sortedKeyPoints.push_back(itPoint->second);
		std::vector<cv::Point2f> pointsVoisins;

		for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors(); itNb != itPoint->second.endNeighbors(); itNb++)
			pointsVoisins.push_back(Point2f(*itNb));

		// Le premier voisin représente le dernier point du vecteur
		cv::Point2f lastPoint = pointsVoisins.at(0);
		
		// Le deuxième voisin représente le prochain point du vecteur
		cv::Point2f nextPoint = pointsVoisins.at(1);

		pointsVoisins.clear();

		while (nextPoint != lastPoint)
		{
			itPoint = keysPoints.find(nextPoint);
			sortedKeyPoints.push_back(itPoint->second);

			for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors(); itNb != itPoint->second.endNeighbors(); itNb++)
				pointsVoisins.push_back(Point2f(*itNb));
		
			// Le deuxième voisin représente le prochain point du vecteur
			nextPoint = pointsVoisins.at(1);
			pointsVoisins.clear();
		}

		// On ajoute le dernier point
		itPoint = keysPoints.find(nextPoint);
		sortedKeyPoints.push_back(itPoint->second);

		return sortedKeyPoints;
	}

	void convertAndSortKeyPoints(std::vector<slKeyPoints> &keys, std::vector<std::vector<KeyPt>> &vecNewKeyPts)
	{
		// Pour chaque blob
		for (unsigned int i = 0; i < keys.size(); ++i)
		{
			bool bonBlob = true;
			// Pour chaque point d'un blob
			for (slKeyPoints::const_iterator itPoint = keys.at(i).begin(); itPoint != keys.at(i).end(); itPoint++)
			{
				int temp = 0;
				// On compte le nombre de voisin
				for (CvPt2fVector_t::const_iterator itNb = itPoint->second.beginNeighbors(); itNb != itPoint->second.endNeighbors(); itNb++)
					++temp;

				// Si un des points du blob a plus que deux voisin, ce n'est pas un bon blob
				if (temp > 2)
				{
					bonBlob = false;
					break;
				}
			}

			// Si le blob est bon : chaque point n'a pas plus de 2 voisins, on trie les points et on le convertit en bon format
			if (bonBlob)
				vecNewKeyPts.push_back(convert2KeyPt(sortKeyPoints(keys.at(i))));
		}
	}

	float rad2Deg(float radian)
	{
		return radian*180/3.14159265;
	}

	float deg2Rad(float degree)
	{
		return degree*3.14159265/180;
	}

	std::vector<cv::Point2f> getCentroid(std::vector<std::vector<KeyPt>> keyPoints)
	{
		// Initialisation
		std::vector<cv::Point2f> centroids;
		cv::Point2f centroid;

		// Pour chaque blob
		for (unsigned int i = 0; i < keyPoints.size(); ++i)
		{
			// Pour chaque point d'un blob, on calcule la somme en X et en Y
			for (unsigned int j = 0; j < keyPoints.at(i).size(); ++j)
			{
				centroid.x += keyPoints.at(i).at(j).getPosition().x;
				centroid.y += keyPoints.at(i).at(j).getPosition().y;
			}

			// On divise par le nombre de points du blob pour obtenir la valeur du centroïde
			centroid.x = centroid.x/keyPoints.at(i).size();
			centroid.y = centroid.y/keyPoints.at(i).size();

			// On ajoute le centroïde dans le vecteur;
			centroids.push_back(centroid);
		}

		return centroids;
	}

	cv::Point2f getCentroid(std::vector<KeyPt> keyPoints)
	{
		// Initialisation
		cv::Point2f centroid;

		// Pour chaque point du blob, on calcule la somme en X et en Y
		for (unsigned int i = 0; i < keyPoints.size(); ++i)
		{	
			centroid.x += keyPoints.at(i).getPosition().x;
			centroid.y += keyPoints.at(i).getPosition().y;
		}

		// On divise par le nombre de points du blob pour obtenir la valeur du centroïde
		centroid.x = centroid.x/keyPoints.size();
		centroid.y = centroid.y/keyPoints.size();

		return centroid;
	}

	cv::Point2f getCentroid(std::vector<slKeyPoint> &keyPoints)
	{
		// Initialisation
		cv::Point2f centroid;

		// Pour chaque point du blob, on calcule la somme en X et en Y
		for (unsigned int i = 0; i < keyPoints.size(); ++i)
		{	
			centroid.x += keyPoints.at(i).position().x;
			centroid.y += keyPoints.at(i).position().y;
		}

		// On divise par le nombre de points du blob pour obtenir la valeur du centroïde
		centroid.x = centroid.x/keyPoints.size();
		centroid.y = centroid.y/keyPoints.size();

		return centroid;
	}

	float determinerAngle(cv::Point2f vect1, cv::Point2f vect2, cv::Point2f vect3)
	{
		// On calcule d'abord la distance entre les 2 voisins
		cv::Point2f vecTemp = vect1 - vect3;
		float distanceEntreV1_V3 = sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );

		vecTemp = vect1 - vect2;
		float distanceEntreV1_V2 = sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );

		vecTemp = vect3 - vect2;
		float distanceEntreV3_V2 = sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );

		float angle = acos
					(    
						(distanceEntreV1_V2*distanceEntreV1_V2 
						+ distanceEntreV3_V2*distanceEntreV3_V2 
						- distanceEntreV1_V3 * distanceEntreV1_V3) / (2 * distanceEntreV1_V2*distanceEntreV3_V2)
					);

		// Conversion de radian ver degré
		return angle*(float)180/(float)3.14159265;
	}

	void matchBlobs(std::vector<std::vector<KeyPt>> vecKeyPoints1, std::vector<std::vector<KeyPt>> vecKeyPoints2, std::vector<Point> &matchedPoints1, std::vector<Point> &matchedPoints2)
	{
		// Pour chaque blob de la première image
		for (unsigned int i = 0; i < vecKeyPoints1.size(); ++i)
		{
			// Pour chaque blob de la deuxième image
			for (unsigned int j = 0; j < vecKeyPoints2.size(); ++j)
			{
				matchPoints(vecKeyPoints1.at(i), vecKeyPoints2.at(j), matchedPoints1, matchedPoints2);
			}
		}
	}

	void matchBlobs(std::vector<std::vector<KeyPt>> vecKeyPoints1, std::vector<std::vector<KeyPt>> vecKeyPoints2, std::vector<std::vector<KeyPt>> &matchedPoints1, std::vector<std::vector<KeyPt>> &matchedPoints2)
	{
		std::vector<KeyPt> matchedPts1, matchedPts2, keyPoints2;

		for (unsigned int it = 0; it < vecKeyPoints2.size(); ++it)
		{
			keyPoints2.insert(keyPoints2.begin(), vecKeyPoints2.at(it).begin(), vecKeyPoints2.at(it).end());
		}

		// Pour chaque blob de la première image
		for (unsigned int i = 0; i < vecKeyPoints1.size(); ++i)
		{
			matchPoints(vecKeyPoints1.at(i), keyPoints2, matchedPts1, matchedPts2);
			
			// On ajoute un vecteur de matches pour le blob i de la première image
			matchedPoints1.push_back(matchedPts1);
			matchedPoints2.push_back(matchedPts2);

			matchedPts1.clear();
			matchedPts2.clear();
		}
	}

	void matchBlobs(std::vector<std::vector<KeyPt>> vecKeyPoints1, std::vector<std::vector<KeyPt>> vecKeyPoints2, std::vector<KeyPt> &matchedPoints1, std::vector<KeyPt> &matchedPoints2)
	{
		// Pour chaque blob de la première image
		for (unsigned int i = 0; i < vecKeyPoints1.size(); ++i)
		{
			// Pour chaque blob de la deuxième image
			for (unsigned int j = 0; j < vecKeyPoints2.size(); ++j)
			{
				matchPoints(vecKeyPoints1.at(i), vecKeyPoints2.at(j), matchedPoints1, matchedPoints2);
			}
		}
	}

	void matchPoints(std::vector<KeyPt> keyPoints1, std::vector<KeyPt> keyPoints2, std::vector<Point> &matchedPoints1, std::vector<Point> &matchedPoints2)
	{
		float distance;
		float erreurAngle;

		std::vector<int> index;
		std::vector<float> distances;
		std::vector<float> erreurAngles;


		// Pour chaque point du premier blob
		for (unsigned int i = 0; i < keyPoints1.size(); ++i)
		{
			// Pour chaque point du deuxième blob
			for (unsigned int j = 0; j < keyPoints2.size(); ++j)
			{
				distance = computeDistance(keyPoints1.at(i).getPosition(), keyPoints2.at(j).getPosition());
				
				// Si les points ne sont pas trop éloigné
				if (distance <= RADIUS_MIN)
				{
					// Si les deux points ne sont pas dans des secteurs diagonalement opposés
					//if (keyPoints1.at(i).getSecteur() + keyPoints2.at(j).getSecteur() != 0)
					//{
						// Si les deux points ont la même convexité
						if (keyPoints1.at(i).getConvexite() == keyPoints2.at(j).getConvexite())
						{
							//erreurAngle = abs(keyPoints1.at(i).getAngle() - keyPoints2.at(j).getAngle());
							// Si la différence entre l'amplitude des angle est plus faible qu'un certain nombre de degré
							if (erreurAngle < ERREUR_ANGLE_MAX	)
							{
								index.push_back(j);
								distances.push_back(distance);
								erreurAngles.push_back(erreurAngle);
							}
						}
					//}
				}
			}

			// S'il y a plus qu'un match pour un point i
			if (index.size() > 1)
			{
				unsigned int ind = 0;
				float score = 0;
				float scoreMin = 1000;
				for (unsigned int k = 0; k < index.size(); ++k)
				{
					score = (distances.at(k)/RADIUS_MIN) * 2 + (erreurAngles.at(k)/ERREUR_ANGLE_MAX);
					if (score < scoreMin)
					{
						scoreMin = score;
						ind = index.at(k);
					}
				}

				matchedPoints1.push_back(keyPoints1.at(i).getPosition());
				matchedPoints2.push_back(keyPoints2.at(ind).getPosition());
			}

			else if (index.size() == 1)
			{
				matchedPoints1.push_back(keyPoints1.at(i).getPosition());
				matchedPoints2.push_back(keyPoints2.at(index.at(0)).getPosition());
			}

			// Réinitialisation
			index.clear();
			distances.clear();
			erreurAngles.clear();
		}
	}

	void matchPoints(std::vector<KeyPt> keyPoints1, std::vector<KeyPt> keyPoints2, std::vector<KeyPt> &matchedPoints1, std::vector<KeyPt> &matchedPoints2)
	{
		float distance;
		float erreurAngle;

		std::vector<int> index;
		std::vector<float> distances;
		std::vector<float> erreurAngles;

		// Pour chaque point du premier blob
		for (unsigned int i = 0; i < keyPoints1.size(); ++i)
		{
			// Pour chaque point du deuxième blob
			for (unsigned int j = 0; j < keyPoints2.size(); ++j)
			{
				distance = computeDistance(keyPoints1.at(i).getPosition(), keyPoints2.at(j).getPosition());
				
				// Si les points ne sont pas trop éloigné
				if (distance <= RADIUS_MIN)
				{
					// Si les deux points ne sont pas dans des secteurs diagonalement opposés
					//if (keyPoints1.at(i).getSecteur() + keyPoints2.at(j).getSecteur() != 0)
					//{
						// Si les deux points ont la même convexité
						if (keyPoints1.at(i).getConvexite() == keyPoints2.at(j).getConvexite())
						{
							erreurAngle = abs(keyPoints1.at(i).getAngle() - keyPoints2.at(j).getAngle());

							// Si la différence entre l'amplitude des angle est plus faible que 45 degré
							if (erreurAngle < ERREUR_ANGLE_MAX	)
							{
								index.push_back(j);
								distances.push_back(distance);
								erreurAngles.push_back(erreurAngle);						
							}
						}
					//}
				}
			}

			// S'il y a plus qu'un match pour un point i
			if (index.size() > 1)
			{
				unsigned int ind = 0;
				float score = 0;
				float scoreMin = 1000;
				for (unsigned int k = 0; k < index.size(); ++k)
				{
					score = (distances.at(k)/RADIUS_MIN) * 2 + (erreurAngles.at(k)/ERREUR_ANGLE_MAX);
					if (score < scoreMin)
					{
						scoreMin = score;
						ind = index.at(k);
					}
				}

				matchedPoints1.push_back(keyPoints1.at(i));
				matchedPoints2.push_back(keyPoints2.at(ind));
			}

			else if (index.size() == 1)
			{
				matchedPoints1.push_back(keyPoints1.at(i));
				matchedPoints2.push_back(keyPoints2.at(index.at(0)));
			}

			
			// Réinitialisation
			index.clear();
			distances.clear();
			erreurAngles.clear();
		}
	}

	float computeDistance(cv::Point2f pt1, cv::Point2f pt2)
	{
		cv::Point2f vecTemp = pt1 - pt2;
		return sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );
	}

	Point2f findBlobCentroid(Mat &src_gray)
	{
		Mat canny_output;
		Mat threshold_output;
		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		Point2f centroid;
		centroid.x = 0;
		centroid.y = 0;

		bool termine = false;

		int taille = 0;

		// Détection des contours en utilisant le filtre de Canny
		//Canny( src_gray, canny_output, thresh, thresh*2, 3 );

		double tr = threshold(src_gray,threshold_output,200,255,0);

		// Déterminer les contours
		findContours( threshold_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

		// Obtenir les moments
		vector<Moments> mu;
		for( unsigned int i = 0; i < contours.size(); ++i )
		{ 
			if (contours[i].size() >= 3)
				mu.push_back(moments( contours[i], false ));

			//for (unsigned int k = 0; k < contours[i].size(); ++k)
				//circle(src_gray, Point(contours[i][k]), 2, CV_RGB(0, 255, 0), 1, 8, 0);
		}

		// Obtenir les centres de masse
		vector<Point2f> mc( mu.size() );
		for( unsigned int j = 0; j < mu.size(); ++j )
		{ 
			mc[j] = Point2f( mu[j].m10/mu[j].m00 , mu[j].m01/mu[j].m00 );

			
			std::cout << "Centroid " << j << " (" << mc[j].x << ", " << mc[j].y << ")" << std::endl;
			
			circle(src_gray, Point(mc[j]), 2, CV_RGB(0, 0, 0), 3, 8, 0);
		} 


		for( unsigned int k = 0; k < mc.size(); ++k )
		{
			centroid.x += mc[k].x;
			centroid.y += mc[k].y;
		}

		centroid.x /= mc.size();
		centroid.y /= mc.size();

		circle(src_gray, Point(centroid), 2, CV_RGB(0, 0, 255), 5, 8, 0);
		//std::cout << "Point (" << centroid.x << ", " << centroid.y << ")" << std::endl;

		return centroid;
	}

	void update_map( slImage3ch src, Mat &dest, Mat map_x, Mat map_y)
	{
		
		for( int y = 0; y < src.rows; y++ )
		{ 
			for( int x = 0; x < src.cols; x++ )
			{
				// Si ce n'est pas un pixel blanc ou noir
				if (src.at<Vec3b>(y,x)[0] != 255 && src.at<Vec3b>(y,x)[0] != 0)
				{
					// Translation
					dest.at<Vec3b>(y,x - src.at<Vec3b>(y,x)[0]/3)[0] = src.at<Vec3b>(y,x)[0];
					dest.at<Vec3b>(y,x - src.at<Vec3b>(y,x)[0]/3)[1] = src.at<Vec3b>(y,x)[1];
					dest.at<Vec3b>(y,x - src.at<Vec3b>(y,x)[0]/3)[2] = src.at<Vec3b>(y,x)[2];
				}
			}
		}
	}

	void switchBlackAndWhite(Mat &src)
	{
		
		for( int y = 0; y < src.rows; y++ )
		{ 
			for( int x = 0; x < src.cols; x++ )
			{
				// Si c'est un pixel blanc, on le remet à noir
				if ( src.at<Vec3b>(y,x)[0] == 255)
				{
					// Translation
					src.at<Vec3b>(y,x)[0] = 0;
					src.at<Vec3b>(y,x)[1] = 0;
					src.at<Vec3b>(y,x)[2] = 0;
				}

				// Si c'est un pixel autre que blanc, on le met blanc
				else
				{
					// Translation
					src.at<Vec3b>(y,x)[0] = 255;
					src.at<Vec3b>(y,x)[1] = 255;
					src.at<Vec3b>(y,x)[2] = 255;
				}
			}
		}
	}

	void transformGroundTruth(Mat src, Mat &dest, string pathFile, string pathVisForegroundFile)
	{
		ifstream myfile (pathFile);
		string line;
		//int posX, posY;
		int disparity;

		/*if (myfile.is_open())
		{
			while ( myfile.good() )
			{
				getline(myfile, line);

				if (line == pathVisForegroundFile)
				{
					getline(myfile, line);		// valeur en X
					posX = ::atoi(line.c_str());

					getline(myfile, line);		// valeur en Y
					posY = ::atoi(line.c_str());

					getline(myfile, line);		// disparité
					disparity = ::atoi(line.c_str());

					cout << "Valeur X : " << posX << endl;
					cout << "Disparity : " << disparity << endl;
					
					for( int y = 0; y < src.rows; y++ )
					{ 
						// Translation
						dest.at<Vec3b>(y,posX - disparity)[0] = src.at<Vec3b>(y,posX)[0];
						dest.at<Vec3b>(y,posX - disparity)[1] = src.at<Vec3b>(y,posX)[1];
						dest.at<Vec3b>(y,posX - disparity)[2] = src.at<Vec3b>(y,posX)[2];

						dest.at<Vec3b>(y,posX)[0] = 0;
						dest.at<Vec3b>(y,posX)[1] = 0;
						dest.at<Vec3b>(y,posX)[2] = 0;
					}
				}
			}
			myfile.close();
		}
		else 
		{
			cout << "Unable to open file" << endl << endl; 
		}*/

		disparity = 11;
		for (int posX = 120; posX < 219; posX++)
		{
			for( int y = 0; y < src.rows; y++ )
			{ 
				// Translation
				dest.at<Vec3b>(y,posX - disparity)[0] = src.at<Vec3b>(y,posX)[0];
				dest.at<Vec3b>(y,posX - disparity)[1] = src.at<Vec3b>(y,posX)[1];
				dest.at<Vec3b>(y,posX - disparity)[2] = src.at<Vec3b>(y,posX)[2];

				dest.at<Vec3b>(y,posX)[0] = 0;
				dest.at<Vec3b>(y,posX)[1] = 0;
				dest.at<Vec3b>(y,posX)[2] = 0;
			}
		}

		disparity = 13;
		for (int posX = 223; posX < 302; posX++)
		{
			for( int y = 0; y < src.rows; y++ )
			{ 
				//if (y >= 290)
					//disparity = 11;
				//else
					//disparity = 20;
				// Translation
				dest.at<Vec3b>(y,posX - disparity)[0] = src.at<Vec3b>(y,posX)[0];
				dest.at<Vec3b>(y,posX - disparity)[1] = src.at<Vec3b>(y,posX)[1];
				dest.at<Vec3b>(y,posX - disparity)[2] = src.at<Vec3b>(y,posX)[2];

				dest.at<Vec3b>(y,posX)[0] = 0;
				dest.at<Vec3b>(y,posX)[1] = 0;
				dest.at<Vec3b>(y,posX)[2] = 0;
			}
		}

		disparity = 20;
		for (int posX = 310; posX < 385; posX++)
		{
			for( int y = 0; y < src.rows; y++ )
			{ 
				// Translation
				dest.at<Vec3b>(y,posX - disparity)[0] = src.at<Vec3b>(y,posX)[0];
				dest.at<Vec3b>(y,posX - disparity)[1] = src.at<Vec3b>(y,posX)[1];
				dest.at<Vec3b>(y,posX - disparity)[2] = src.at<Vec3b>(y,posX)[2];

				dest.at<Vec3b>(y,posX)[0] = 0;
				dest.at<Vec3b>(y,posX)[1] = 0;
				dest.at<Vec3b>(y,posX)[2] = 0;
			}
		}

		/*disparity = 14;
		for (int posX = 307; posX < 375; posX++)
		{
			for( int y = 0; y < src.rows; y++ )
			{ 
				// Translation
				dest.at<Vec3b>(y,posX - disparity)[0] = src.at<Vec3b>(y,posX)[0];
				dest.at<Vec3b>(y,posX - disparity)[1] = src.at<Vec3b>(y,posX)[1];
				dest.at<Vec3b>(y,posX - disparity)[2] = src.at<Vec3b>(y,posX)[2];

				dest.at<Vec3b>(y,posX)[0] = 0;
				dest.at<Vec3b>(y,posX)[1] = 0;
				dest.at<Vec3b>(y,posX)[2] = 0;
			}
		}*/
		/*disparity = 14;
		for (int posX = 0; posX < src.cols; posX++)
		{
			for( int y = 0; y < src.rows; y++ )
			{ 
				Point2f pt1(183.325, 199.399);
				Point2f pt2(210.532, 206.0);

				Point2f pt3(posX, y);
				cv::Point2f vecTemp = pt1 - pt3;
				cv::Point2f vecTemp2 = pt2 - pt3;

				float d = sqrt( (vecTemp.x*vecTemp.x) + (vecTemp.y*vecTemp.y) );
				float d2 = sqrt( (vecTemp2.x*vecTemp2.x) + (vecTemp2.y*vecTemp2.y) );

				if (d < d2)
					disparity = 17;
				else
					disparity = 11;

				// Translation
				dest.at<Vec3b>(y,posX - disparity)[0] = src.at<Vec3b>(y,posX)[0];
				dest.at<Vec3b>(y,posX - disparity)[1] = src.at<Vec3b>(y,posX)[1];
				dest.at<Vec3b>(y,posX - disparity)[2] = src.at<Vec3b>(y,posX)[2];

				dest.at<Vec3b>(y,posX)[0] = 0;
				dest.at<Vec3b>(y,posX)[1] = 0;
				dest.at<Vec3b>(y,posX)[2] = 0;
			}
		}*/
	}

	int computeBlobsSurface(Mat src)
	{
		int surface = 0;

		for( int y = 0; y < src.rows; y++ )
		{ 
			for( int x = 0; x < src.cols; x++ )
			{
				// Si c'est un pixel blanc, on le remet à noir
				if ( src.at<Vec3b>(y,x)[0] > 200)
				{
					++surface;
				}
			}
		}

		return surface;
	}

	std::vector<Point2f> computeTranslate(std::vector<std::vector<KeyPt>> modifiedPoints, std::vector<std::vector<KeyPt>> points2Compare)
	{
		std::vector<Point2f> vecTranslate;
		Point2f translate;

		// Pour chaque blob
		for (unsigned int i = 0; i < modifiedPoints.size(); ++i)
		{
			translate.x = 0;
			translate.y = 0;

			// Pour chaque point du blob, faire la somme des distances entre les points
			for (unsigned int j = 0; j < modifiedPoints.at(i).size(); ++j)
			{
				translate += points2Compare.at(i).at(j).getPosition() - modifiedPoints.at(i).at(j).getPosition();
			}

			// On divise par le nombre de point pour avoir la translation moyenne
			translate.x = translate.x/modifiedPoints.at(i).size();
			translate.y = translate.y/modifiedPoints.at(i).size();
			
			// Ajout dans le vecteur à retourner
			vecTranslate.push_back(translate);
		}

		return vecTranslate;
	}

	void substractRegion(Mat &src, Mat region2Substract)
	{
		for (int x = 0; x < region2Substract.cols; x++)
		{
			for( int y = 0; y < region2Substract.rows; y++ )
			{ 
				// Si le pixel est blanc (appartient à la région d'intérêt), on le soustrait dans l'autre image
				if (region2Substract.at<Vec3b>(y,x)[0] > 200)
				// Translation
				src.at<Vec3b>(y,x)[0] = 0;
				src.at<Vec3b>(y,x)[1] = 0;
				src.at<Vec3b>(y,x)[2] = 0;
			}
		}
	}

	int countWhitePixel(Mat &src)
	{
		int count = 0;

		for (int x = 0; x < src.cols; x++)
		{
			for( int y = 0; y < src.rows; y++ )
			{ 
				// Si le pixel est blanc (appartient à la région d'intérêt), on le soustrait dans l'autre image
				if (src.at<Vec3b>(y,x)[0] > 200)
					++count;
			}
		}

		return count;
	}


	void writeMatToFile(cv::Mat& m, const std::string& filename)
{
    ofstream fout(filename);
	
    if(!fout)
    {
        cout<<"File Not Opened"<<endl;  return;
    }

    for(int i=0; i<m.rows; i++)
    {
        for(int j=0; j<m.cols; j++)
        {
            fout<<m.at<float>(i,j)<<"\t";
        }
        fout<<endl;
    }
	
	fout.close();
	
	
}



}

