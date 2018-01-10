/*!	\file	slParamGenerator.h
 *	\brief	Contains class slParamGenerator
 *
 *	\author		Pier-Luc St-Onge
 *	\date		March 2012
 */

#ifndef SLPARAMGENERATOR_H
#define SLPARAMGENERATOR_H


#include "slArgHandler.h"

#include <iostream>


//!	Parameter generator to test multiple values for each component
/*!
 *	This class is used to generate some command line arguments (\c argv)
 *	for a parameter of multiple components.
 *	A parameter could be boolean (present or not) or having from
 *	one to many numerical components.
 *	The numerical components could be integers or floats, defined
 *	individually for each component.
 *
 *	A parameter generator is made of a switch, like \c -s, and a number of
 *	components.
 *	Then, for each component, there is a string format (\c printf() format),
 *	the range of its value, the current value, the current delta for the value
 *	and the minimum delta value.
 *	When a parameter is generated (including the switch and all components), only one
 *	component is modified from its current value by adding or subtracting its delta.
 *	In the case of boolean parameters, the switch will be present or not.
 *
 *	Of course, it is possible to update one component's current value
 *	by adding or subtracting its delta.
 *	Finally, it is also possible to reduce by half all the deltas.
 *	At some point, it may be necessary to refine more precisely the search in
 *	the neighborhood of the current values.
 *
 *	Example of many configured slParamGenerator instances on command line (from a \c .bat file):
 *	\code
 *	:bgSub -a tempAVG ^
 *	bgSubParam: -s -sf -n 3 -f "%%0.3f %%0.3f %%0.3f" -r "0.2 0.2 0.2" "0.8 0.8 0.8" -v "0.5 0.5 0.5" -d "0.1 0.1 0.1" "0.001 0.001 0.001" ^
 *	bgSubParam: -s -e  -n 1 -f "%%d"                  -r "0"           "255"         -v "15"          -d "16"          "1" ^
 *	\endcode
 *	In order to parse this command line:
 *	\code
 *	 	slArgProcess argProcess;
 *		slArgHandler argHandler("bgSubParam", 0);
 *	
 *		slParamGenerator::fillParamSpecs(argHandler);
 *		argProcess << argHandler;
 *	
 *		try {
 *			// All bgSubParam nodes will be parsed automatically
 *			argProcess.parse(argc, argv);
 *
 *			// Classify all common parameters
 *			const slParameters &globalParams = argProcess.getParameters("");
 *			vector<string> global_argv, bgSub_argv;
 *			string nodeMode = "";
 *	
 *			global_argv.push_back("run");
 *	
 *			for (unsigned int ind = 0; ind < globalParams.getNbOtherArgs(); ind++) {
 *				string globalArg = globalParams.getOtherArg(ind);
 *	
 *				// If a compute node is found, put ':' at the end
 *				if (globalArg[0] == ':') {
 *					nodeMode = globalArg.substr(1);
 *					globalArg = nodeMode + ":";
 *				}
 *	
 *				if (nodeMode == "bgSub") {
 *					bgSub_argv.push_back(globalArg);
 *				}
 *				else {
 *					global_argv.push_back(globalArg);
 *				}
 *			}
 *		}
 *		catch (const slException &err) {
 *			argProcess.printUsage();
 *			cerr << err.getMessage() << endl;
 *		}
 *	\endcode
 *	How to create and configure the generators:
 *	\code
 *	 	// Modifiable bgSub arguments
 *		int nbNodes = argProcess.getNumberOfNodes("bgSubParam");
 *		vector<slParamGenerator> generators;
 *	
 *		for (int node = 0; node < nbNodes; node++) {
 *			slParamGenerator paramGenerator;
 *	
 *			paramGenerator.setParameters(argProcess.getParameters("bgSubParam", node));
 *			generators.push_back(paramGenerator);
 *		}
 *	\endcode
 *	Suggested main loop:
 *	\code
 *	 	// Main loops
 *		bool newDeltas = true;
 *	
 *		while (newDeltas) {
 *			bool bestValuesFound = true;
 *	
 *			while (bestValuesFound) {
 *				// Test all parameters and all their components once
 *				bestValuesFound = doWork(generators, bestValues, global_argv, bgSub_argv);
 *			}
 *	
 *			newDeltas = false;
 *	
 *			for (unsigned int gen = 0; gen < generators.size(); gen++) {
 *				if (generators[gen].updateDeltas()) newDeltas = true;
 *			}
 *		}
 *	\endcode
 *	In the main function:
 *	\code
 *		bool bestValuesFound = false;
 *	
 *		// For each generator
 *		for (unsigned int gen = 0; gen < generators.size(); gen++) {
 *			unsigned int nbComp = generators[gen].getNbComp();
 *	
 *			// For each component
 *			for (unsigned int compInd = 0; compInd < nbComp; compInd++) {
 *				// For each sign (-1 and 1)
 *				for (int sign = -1; sign <= 1; sign += 2) {
 *					try {
 *						// Generate all arguments
 *						vector<string> test_argv;
 *						test_argv.insert(test_argv.end(), global_argv.begin(), global_argv.end());
 *						test_argv.insert(test_argv.end(), bgSub_argv.begin(), bgSub_argv.end());
 *						for (unsigned int ind = 0; ind < generators.size(); ind++) {
 *							generators[ind].generate(compInd, ind == gen ? sign : 0, test_argv);
 *						}
 *				
 *						// Fill the argv
 *						vector<char*> argv;
 *						for (unsigned int ind = 0; ind < test_argv.size(); ind++) {
 *							argv.push_back((char*)test_argv[ind].c_str());
 *						}
 *						argv.push_back(NULL);
 *	
 *						// Do work
 *						Type values = fctMain(argv.size() - 1, &argv[0]);
 *	
 *						if (values < bestValues) {
 *							generators[gen].updateComp(compInd, sign);
 *							bestValuesFound = true;
 *							bestValues = values;
 *							break;	// No need to go the other sign
 *						}
 *					}
 *					catch (const slException &err) {
 *						cerr << "Canceled case: " << err.getMessage() << endl;
 *					}// Try/catch
 *				}// for (int sign = -1; sign <= 1; sign += 2)
 *			}// for (unsigned int compInd = 0; compInd < nbComp; compInd++)
 *		}// for (unsigned int gen = 0; gen < generators.size(); gen++)
 *	
 *		return bestValuesFound;
 *	\endcode
 *
 *	\see		slAH::slParamSpec, slAH::slParamSpecMap, slAH::slArgHandler
 *	\author		Pier-Luc St-Onge
 *	\date		March 2012
 */
class SLCORE_DLL_EXPORT slParamGenerator
{
public:
	slParamGenerator();		//!< Constructor

	static void fillParamSpecs(slAH::slParamSpecMap& paramSpecMap);	//!< To get the parameters' syntax

	// Set Functions

	void setPresent(bool present);							//!< For boolean parameters
	void setSwitch(const std::string &switchName);			//!< The switch
	void setNbComp(unsigned int nbComponents);				//!< Number of components (0 for boolean)
	void setFormats(const std::string &formats);			//!< Expects alls formats, one for each component, in one string: "%d %0.4f ..."

	void setRange(	const std::vector<double> &minValues,
					const std::vector<double> &maxValues);	//!< Components' minimum and maximum values
	void setValues(	const std::vector<double> &curValues);	//!< Components' current value
	void setDeltas(	const std::vector<double> &curDeltas,
					const std::vector<double> &minDeltas);	//!< Current deltas, minimum deltas (when current is divided by 2)

	void setParameters(const slAH::slParameters& parameters);	//!< Complete configuration
	void showParameters(std::ostream &ostr = std::cout) const;	//!< Show current values

	// Get Functions

	int getNbComp() const;	//!< Returns the number of numerical components.  Returns 1 for boolean parameters
	void generate(unsigned int comp, int sign, std::vector<std::string> &argv);	//!< Appends the generated arguments to argv.  Comp: index of component to modify.  Sign: subtract or add delta
	void updateComp(unsigned int comp, int sign);	//!< Update the current value of one component, subtract or add its delta
	bool updateDeltas();							//!< Divide all deltas by 2.  Returns true if one component's delta is modified

private:
	std::vector<double> parseNum(const std::string &numbers);

private:
	enum component_t {FLOAT_COMP, INT_COMP};

private:
	bool present_;
	std::string switchName_;
	unsigned int nbComponents_;

	std::vector<component_t> component_types_;
	std::vector<std::string> formats_;

	std::vector<double> minValues_, maxValues_;
	std::vector<double> curValues_;
	std::vector<double> curDeltas_, minDeltas_;

};


#endif	// SLPARAMGENERATOR_H

