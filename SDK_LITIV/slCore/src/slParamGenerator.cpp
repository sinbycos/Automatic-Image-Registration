#include "slParamGenerator.h"

#include <sstream>


using namespace std;
using namespace slAH;


#define ARG_SWITCH "-s"
#define ARG_SIZE "-n"
#define ARG_FORMAT "-f"

#define ARG_RANGE "-r"
#define ARG_VALUE "-v"
#define ARG_DELTA "-d"


void slParamGenerator::fillParamSpecs(slAH::slParamSpecMap& paramSpecMap)
{
	paramSpecMap
		<< (slParamSpec(ARG_SWITCH, "Switch name", MANDATORY) << slSyntax("[\"]-switch[\"]"))
		<< (slParamSpec(ARG_SIZE, "Number of components", MANDATORY) << slSyntax("0..n"))
		<< (slParamSpec(ARG_FORMAT, "Format") << slSyntax("\"%d [%0.6f ...]\""))
		<< (slParamSpec(ARG_RANGE, "Range [min,max]")
			<< slSyntax("\"min1 [min2 ...]\"", "0") << slSyntax("\"max1 [max2 ...]\"", "1"))
		<< (slParamSpec(ARG_VALUE, "Initial values") << slSyntax("\"val1 [val2 ...]\"", "0"))
		<< (slParamSpec(ARG_DELTA, "Jump length")
			<< slSyntax("\"max1 [max2 ...]\"", "1") << slSyntax("\"min1 [min2 ...]\"", "1"))
		;
}


slParamGenerator::slParamGenerator()
{
	setPresent(true);
	setNbComp(0);
}


void slParamGenerator::setPresent(bool present)
{
	present_ = present;
}


void slParamGenerator::setSwitch(const std::string &switchName)
{
	switchName_ = switchName;
}


void slParamGenerator::setNbComp(unsigned int nbComponents)
{
	nbComponents_ = nbComponents;
}


void slParamGenerator::setFormats(const std::string &formats)
{
	istringstream iss(formats);

	component_types_.resize(nbComponents_);
	formats_.resize(nbComponents_);

	for (unsigned int ind = 0; ind < nbComponents_; ind++) {
		iss >> formats_[ind];

		char printfType = (!formats_[ind].empty() ? formats_[ind].back() : '\0');
		component_types_[ind] = (printfType == 'd' || printfType == 'i' ? INT_COMP : FLOAT_COMP);
	}
}


void slParamGenerator::setRange(const std::vector<double> &minValues,
	const std::vector<double> &maxValues)
{
	if (minValues.size() == nbComponents_ && maxValues.size() == nbComponents_) {
		minValues_ = minValues;
		maxValues_ = maxValues;
	}
	else {
		throw slException("slParamGenerator::setRange(): vectors of invalide size");
	}
}


void slParamGenerator::setValues(const std::vector<double> &curValues)
{
	if (curValues.size() == nbComponents_) {
		curValues_ = curValues;
	}
	else {
		throw slException("slParamGenerator::setValues(): vector of invalide size");
	}
}


void slParamGenerator::setDeltas(const std::vector<double> &curDeltas,
	const std::vector<double> &minDeltas)
{
	if (curDeltas.size() == nbComponents_ && minDeltas.size() == nbComponents_) {
		curDeltas_ = curDeltas;
		minDeltas_ = minDeltas;
	}
	else {
		throw slException("slParamGenerator::setDeltas(): vectors of invalide size");
	}
}


void slParamGenerator::setParameters(const slAH::slParameters& parameters)
{
	// Switch name
	setSwitch(parameters.getValue(ARG_SWITCH));

	// Parameter size and format
	setNbComp((unsigned int)atoi(parameters.getValue(ARG_SIZE).c_str()));
	setFormats(parameters.getValue(ARG_FORMAT));

	// All numerical values
	setRange(	parseNum(parameters.getValue(ARG_RANGE, 0)),
				parseNum(parameters.getValue(ARG_RANGE, 1)));
	setValues(	parseNum(parameters.getValue(ARG_VALUE, 0)));
	setDeltas(	parseNum(parameters.getValue(ARG_DELTA, 0)),
				parseNum(parameters.getValue(ARG_DELTA, 1)));
}


std::vector<double> slParamGenerator::parseNum(const std::string &numbers)
{
	istringstream iss(numbers);
	vector<double> values;

	values.resize(nbComponents_);

	for (unsigned int ind = 0; ind < nbComponents_; ind++) {
		iss >> values[ind];
	}

	return values;
}


void slParamGenerator::showParameters(std::ostream &ostr) const
{
	const int BUFSIZE = 50;
	char buffer[BUFSIZE];

	// Switch name and number of components
	ostr << switchName_ << " [" << nbComponents_ << "] ";

	if (nbComponents_ > 0) {
		// Show components' format
		ostr << "(" << (component_types_[0] == INT_COMP ? "i" : "f") << ":" << formats_[0];
		for (unsigned int ind = 1; ind < nbComponents_; ind++) {
			ostr << " " << (component_types_[ind] == INT_COMP ? "i" : "f") << ":" << formats_[ind];
		}
		ostr << ")\n\t[ ";

		// Min <= Value <= Max
		for (unsigned int ind = 0; ind < nbComponents_; ind++) {
			int j = 0;

			if (component_types_[ind] == INT_COMP) {
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), (int)minValues_[ind]);
				ostr << buffer << "<=";
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), (int)curValues_[ind]);
				ostr << buffer << "<=";
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), (int)maxValues_[ind]);
				ostr << buffer << " ";
			}
			else {
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), minValues_[ind]);
				ostr << buffer << "<=";
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), curValues_[ind]);
				ostr << buffer << "<=";
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), maxValues_[ind]);
				ostr << buffer << " ";
			}
		}
		ostr << "]\n\t+/- ";

		// Delta -> MinDelta
		for (unsigned int ind = 0; ind < nbComponents_; ind++) {
			int j = 0;

			if (component_types_[ind] == INT_COMP) {
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), (int)curDeltas_[ind]);
				ostr << buffer << "->";
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), (int)minDeltas_[ind]);
				ostr << buffer << " ";
			}
			else {
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), curDeltas_[ind]);
				ostr << buffer << "->";
				j += sprintf_s(buffer, BUFSIZE - j, formats_[ind].c_str(), minDeltas_[ind]);
				ostr << buffer << " ";
			}
		}
	}
	else {
		ostr << "false <= (" << (present_ ? "true": "false") << ") <= true";
	}

	ostr << endl;
}


int slParamGenerator::getNbComp() const
{
	return (nbComponents_ > 0 ? nbComponents_ : 1);
}


void slParamGenerator::generate(unsigned int comp, int sign, std::vector<std::string> &argv)
{
	if (nbComponents_ > 0) {
		const int BUFSIZE = 50;
		char buffer[BUFSIZE];
		vector<double> testValues = curValues_;

		// Modify requested component
		if (sign != 0) {
			testValues[comp] += sign * curDeltas_[comp];

			if (testValues[comp] < minValues_[comp] || testValues[comp] > maxValues_[comp]) {
				ostringstream oss;
				oss << "slParamGenerator::generate(): component " << comp
					<< " (=" << testValues[comp] << ")" << " for " << switchName_
					<< " out of range [" << minValues_[comp] << ", " << maxValues_[comp] << "]";
				throw slException(oss.str().c_str());
			}

			cout << "Case: " << switchName_ << "[" << comp << "] = " << testValues[comp] << endl;
		}

		// Push back the switch and all its components
		argv.push_back(switchName_);

		for (unsigned int ind = 0; ind < nbComponents_; ind++) {
			if (component_types_[ind] == INT_COMP) {
				sprintf_s(buffer, BUFSIZE, formats_[ind].c_str(), (int)testValues[ind]);
			}
			else {
				sprintf_s(buffer, BUFSIZE, formats_[ind].c_str(), testValues[ind]);
			}

			argv.push_back(buffer);
		}
	}
	else {
		// If parameter is present or forced to be present
		if (sign == 1 || sign == 0 && present_) {
			argv.push_back(switchName_);
			cout << switchName_ << " present" << endl;
		}
		else {
			cout << switchName_ << " absent" << endl;
		}
	}
}


void slParamGenerator::updateComp(unsigned int comp, int sign)
{
	if (nbComponents_ > 0) {
		curValues_[comp] += sign * curDeltas_[comp];
		cout << switchName_ << "[" << comp << "] = " << curValues_[comp] << endl;
	} else {
		present_ = (sign > 0);
		cout << switchName_ << (present_ ? " present" : " absent")  << endl;
	}
}


bool slParamGenerator::updateDeltas()
{
	bool newDeltas = false;

	for (unsigned int ind = 0; ind < curDeltas_.size(); ind++) {
		double tmp = curDeltas_[ind];

		curDeltas_[ind] /= 2;

		if (curDeltas_[ind] < minDeltas_[ind]) {
			curDeltas_[ind] = minDeltas_[ind];
		}

		if (curDeltas_[ind] != tmp) {
			newDeltas = true;
			cout << "Delta(" << switchName_ << "[" << ind << "]) = " << curDeltas_[ind] << endl;
		}
	}

	return newDeltas;
}

