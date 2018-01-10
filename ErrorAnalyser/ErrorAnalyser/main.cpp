#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>

using namespace std;

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    return split(s, delim, elems);
}

void main()
{
	double erreur;
	unsigned int nbFrame;
	string line;
	string input = "";

	while (true)
	{
		erreur = 0;
		nbFrame = 0;

		cout << "Nom du fichier : ";
		cin >> input;
		
		if (input == "exit")
			break;

		vector<string> output;

		ifstream myfile (input);

		if (myfile.is_open())
		{
			while ( myfile.good() )
			{
				getline(myfile, line);

				if (line != "")
				{
					output = split(line, ' ');

					erreur += ::atof(output[4].c_str());
					++nbFrame;
				}
			}	
			myfile.close();

			cout << "Erreur totale pour " << nbFrame << " frames = " << erreur << endl;
			cout << "Erreur totale moyenne = " << erreur/nbFrame << endl << endl;
		}
		else 
		{
			cout << "Unable to open file" << endl << endl; 
		}
	}
}