/*!	\file	slArgHandler.cpp
 *	\brief	Implementation of all classes for arguments handling
 *
 *	This is the implementation of all the classes needed to
 *	handle the arguments used in SDK-LITIV projects.
 *
 *	\see	slAH
 *	\author	Pier-Luc St-Onge, Michael Eilers-Smith
 *	\date	April 2010
 */

#include <algorithm>
#include <iostream>

#include "slArgHandler.h"


using namespace slAH;
using namespace std;


/******************************************************************************
-------------------------- VALVECT IMPLEMENTATION -----------------------------
******************************************************************************/


/****************************************************************************
 * Description    :  Constructor by default
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slValVect::slValVect()
{
}


/****************************************************************************
 * Description    :  add()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slValVect& slValVect::operator<<(const std::string& value)
{
	contVal_.push_back(value);

	return *this;
}


/****************************************************************************
 * Description    : getSyntax()
 * Parameters     : No
 * Return value   : No
 ***************************************************************************/
string slValVect::getValue(unsigned int index) const 
{
	if (index >= contVal_.size()) {
		return string();
	}

	return contVal_[index]; 
}


/****************************************************************************
 * Description    :  getSize()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
unsigned int slValVect::size() const
{
	return contVal_.size();
}


const string& slValVect::operator[](unsigned int index) const 
{
	if (index >= contVal_.size()) {
		throw slExceptionArgHandler("slValVect::operator[]: index >= size()");
	}

	return contVal_[index]; 
}


/****************************************************************************
 * Description    : operator<
 * Parameters     : No
 * Return value   : No
 ***************************************************************************/
bool slValVect::operator<(const slValVect& valVect) const
{
   return contVal_ < valVect.contVal_;
}


std::ostream& slValVect::operator>>(std::ostream &ostr) const
{
	for (std::vector<std::string>::const_iterator iter = contVal_.begin();
			iter != contVal_.end(); iter++)
	{
		ostr << " " << *iter;
	}

	return ostr;
}


std::ostream& slAH::operator<<(std::ostream &ostr, const slValVect& valVect)
{
	return valVect >> ostr;
}




/******************************************************************************
------------------------ SLPARAMETERS IMPLEMENTATION --------------------------
******************************************************************************/


slParameters::slParameters()
{
}


void slParameters::savePool(const std::map<std::string, const slParamSpec*>& pool)
{
	contPool_ = pool;
}


void slParameters::setValue(const std::string& paramSwitch, const slValVect& values)
{
	contParsedValues_[paramSwitch] = values;
}


bool slParameters::isParsed(const std::string& paramSwitch) const
{
	return (contParsedValues_.find(paramSwitch) != contParsedValues_.end());
}


slParameters& slParameters::operator<<(const std::string& otherArg)
{
	otherArgs_.push_back(otherArg);

	return *this;
}


const string& slParameters::getDescription(const std::string& paramSwitch) const
{
	map<string, const slParamSpec*>::const_iterator itVal = contPool_.find(paramSwitch);

	if (itVal == contPool_.end()) {
		throw slExceptionArgHandler(string("slParameters::getDescription(): switch \"") +
			paramSwitch + "\" not defined");
	}

	return (itVal->second)->getDescription();
}


const string& slParameters::getDefValue(const std::string& paramSwitch, unsigned int index) const
{
	map<string, const slParamSpec*>::const_iterator itVal = contPool_.find(paramSwitch);

	if (itVal == contPool_.end()) {
		throw slExceptionArgHandler(string("slParameters::getDefValue(): switch \"") +
			paramSwitch + "\" not defined");
	}

	return (itVal->second)->getDefault(index);
}


const string& slParameters::getValue(const std::string& paramSwitch, unsigned int index) const
{
	map<string, slValVect>::const_iterator itVal = contParsedValues_.find(paramSwitch);

	// If not parsed, get default value
	if (itVal == contParsedValues_.end()) {
		return getDefValue(paramSwitch, index);
	}

	return (itVal->second)[index];
}


unsigned int slParameters::getNbOtherArgs() const
{
	return otherArgs_.size();
}


const string& slParameters::getOtherArg(unsigned int index) const
{
	if (index >= otherArgs_.size()) {
		throw slExceptionArgHandler("slParameters::getOtherArg(): index >= size()");
	}

	return otherArgs_[index];
}


void slParameters::printParameters() const
{
	printParameters(cout);
}


void slParameters::printParameters(std::ostream &ostr) const
{
	for (map<string, slValVect>::const_iterator it = contParsedValues_.begin();
		it != contParsedValues_.end(); it++)
	{
		ostr << "\t" << it->first << it->second << endl;
	}

	if (!otherArgs_.empty()) {
		ostr << "\t: ";

		for (vector<string>::const_iterator it = otherArgs_.begin();
			it != otherArgs_.end(); it++)
		{
			ostr << *it << " ";
		}

		ostr << endl;
	}
}




/******************************************************************************
-------------------------- SYNTAX IMPLEMENTATION ------------------------------
******************************************************************************/

/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  - name (string): the name of the sample
 * Return value	:  No
 ***************************************************************************/
slSyntax::slSyntax(const std::string& synt, const std::string &defValue)
: syntax_(synt), default_(defValue) 
{
}


/****************************************************************************
 * Description    :  getSyntax()
                     Returns the syntax
 * Parameters     :  No
 * Return value	:  The syntax
 ***************************************************************************/
const string& slSyntax::getSyntax() const
{
	return syntax_;
}


/****************************************************************************
 * Description    :  getDefValue()
                     Returns the syntax's definition value
 * Parameters     :  No
 * Return value	:  The syntax's definition value
 ***************************************************************************/
const string& slSyntax::getDefault() const 
{ 
	return default_; 
}




/******************************************************************************
------------------------- PARAMSPEC IMPLEMENTATION ----------------------------
******************************************************************************/


/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slParamSpec::slParamSpec(const std::string& paramSwitch, const std::string& desc, const bool mandatory)
: switch_(paramSwitch), description_(desc), mandatory_(mandatory)
{
}


/****************************************************************************
 * Description    :  getTag()
                     Returns the parameter's tag.
 * Parameters     :  No
 * Return value   :  The parameter tag
 ***************************************************************************/
const string& slParamSpec::getSwitch() const 
{ 
	return switch_; 
}


/****************************************************************************
 * Description    :  getDescription()
                     Return the parameter's description
 * Parameters     :  No
 * Return value   :  The parameter description
 ***************************************************************************/
const string& slParamSpec::getDescription() const 
{ 
	return description_; 
}


/****************************************************************************
 * Description    :  isMandatory()
                     Gives the parameter's state (required or not)
 * Parameters     :  No
 * Return value   :  The parameter state
 ***************************************************************************/
bool slParamSpec::isMandatory() const 
{
	return mandatory_; 
}


/****************************************************************************
 * Description    :  getSyntaxSize()
                     Returns the size of the parameter's syntax
 * Parameters     :  No
 * Return value   :  The size of the parameter syntax
 ***************************************************************************/
unsigned int slParamSpec::getSyntaxSize() const 
{ 
	return syntaxVector_.size();
}


/****************************************************************************
 * Description    :  getDefValue()
                     Returns the definition value of the parameter's vector 
                     of syntax.
 * Parameters     :  - index (int): the index of the syntax in the vector
 * Return value   :  The parameter's syntax definition value
 ***************************************************************************/
const string& slParamSpec::getDefault(unsigned int index) const 
{
	if (index >= syntaxVector_.size()) {
		throw slExceptionArgHandler("slParamSpec::getDefault(): index >= size()");
	}

	return syntaxVector_[index].getDefault(); 
}


const slParamSpecMap* slParamSpec::getSubParamSpec(const slValVect& values) const
{
	map<slValVect, slParamSpecMap>::const_iterator it = contValParams_.find(values);

	return (it != contValParams_.end() ? &it->second : NULL);
}


/****************************************************************************
 * Description    :  setMandatory()
                     Sets the parameter's description
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slParamSpec& slParamSpec::operator<<(bool mandatory)
{
	mandatory_ = mandatory;

	return *this;
}


/****************************************************************************
 * Description    :  setSyntaxVector()
                     Sets the parameter's syntax vector
 * Parameters     :  - sVect (sVect): the syntax vector
 * Return value   :  No
 ***************************************************************************/
slParamSpec& slParamSpec::operator<<(const slSyntax& syntax)
{
	syntaxVector_.push_back(syntax);

	return *this;
}


/****************************************************************************
 * Description    :  addValues
 * Parameters     :  - nTab (short): the number of tabs desired between
                                     each usage
 * Return value   :  No
 ***************************************************************************/
slParamSpec& slParamSpec::setSubParamSpec(const slValVect& values, const slParamSpecMap& paramMap)
{
	if (contValParams_.find(values) != contValParams_.end()) {
		throw slExceptionArgHandler("slParamSpec::addSubParamMap(): values already exist");
	}

	if (values.size() != syntaxVector_.size()) {
		throw slExceptionArgHandler("slParamSpec::addSubParamMap(): values size inadequate");
	}

	contValParams_[values] = paramMap;

	return *this;
}


/****************************************************************************
 * Description    :  printUsage
 * Parameters     :  - nTab (short): the number of tabs desired between
                                     each usage
 * Return value   :  No
 ***************************************************************************/
void slParamSpec::printParamUsage(const unsigned int nbTab) const
{
	for (unsigned int ind = 0; ind < nbTab; ind++) {
		cout << "    ";
	}

	// Print syntax
	cout << (mandatory_ ? "  " : "[ ") << switch_;
	for (vector<slSyntax>::const_iterator iter = syntaxVector_.begin();
			iter != syntaxVector_.end(); iter++)
	{
		cout << " " << iter->getSyntax();
	}
	cout << (mandatory_ ? "  " : " ]") << "\t\\ # " << description_;

	// Print default values if needed
	if (!mandatory_ && syntaxVector_.size() > 0) {
		cout << " (";
		for (vector<slSyntax>::const_iterator iter = syntaxVector_.begin();
				iter != syntaxVector_.end(); iter++)
		{
			if (iter != syntaxVector_.begin()) {
				cout << " ";
			}
			cout << iter->getDefault();
		}
		cout << ")";
	}
	cout << endl;

	// Print sub parameters syntax
	for (map<slValVect, slParamSpecMap>::const_iterator itSpecSet = contValParams_.begin();
			itSpecSet != contValParams_.end(); itSpecSet++)
	{
		for (unsigned int ind = 0; ind <= nbTab; ind++) {
			cout << "    ";
		}
		cout << "  \\ # if \"" << switch_ << itSpecSet->first << "\"" << endl;

		itSpecSet->second.printParamUsage(nbTab + 2);
	}
}




/******************************************************************************
------------------------ PARAMSPECMAP IMPLEMENTATION --------------------------
******************************************************************************/


/****************************************************************************
 * Description    :  add()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slParamSpecMap& slParamSpecMap::operator<<(const slParamSpec& paramSpec)
{
	contParamSpec_[paramSpec.getSwitch()] = paramSpec;

	return *this;
}


/****************************************************************************
 * Description    :  getContParam()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slParamSpecMap::addToPool(std::map<std::string, const slParamSpec*>& pool) const
{
	for (map<string, slParamSpec>::const_iterator itSpec = contParamSpec_.begin();
			itSpec != contParamSpec_.end(); itSpec++) {
		if (pool.find(itSpec->first) != pool.end())
		{
			throw slExceptionArgHandler(itSpec->first + " is already in pool.");
		}

		pool[itSpec->first] = &itSpec->second;
	}
}


/****************************************************************************
 * Description    :  printUsage()
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slParamSpecMap::printParamUsage(const unsigned int nbTab) const
{
	// Print mandatory parameters first
	for (map<string, slParamSpec>::const_iterator itSpec = contParamSpec_.begin();
			itSpec != contParamSpec_.end(); itSpec++)
	{
		if (itSpec->second.isMandatory()) {
			itSpec->second.printParamUsage(nbTab);
		}
	}

	// Print non-mandatory parameters second
	for (map<string, slParamSpec>::const_iterator itSpec = contParamSpec_.begin();
			itSpec != contParamSpec_.end(); itSpec++)
	{
		if (!itSpec->second.isMandatory()) {
			itSpec->second.printParamUsage(nbTab);
		}
	}
}




/******************************************************************************
------------------------ SLARGHANDLER IMPLEMENTATION --------------------------
******************************************************************************/

/*!
 *	\param name could be the executable name or the compute node type.
 *	\param quota is the minimum number of compute nodes of the same type to be
 *		parsed on the command-line with an slArgProcess
 */
slArgHandler::slArgHandler(const std::string& name, unsigned int quota)
: name_(name), quota_(quota)
{
}


/****************************************************************************
 * Description    :  Constructor for copy
 * Parameters     :  - execName (string): the name of the executable with
                                          the arguments.
 * Return value   :  No
 ***************************************************************************/
slArgHandler::slArgHandler(const slArgHandler& c)
: slParamSpecMap(c)
{
	name_ = c.name_;
	quota_ = c.quota_;
}


/****************************************************************************
 * Description    :  Destructor
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slArgHandler::~slArgHandler()
{
	contParameters_.clear();
}


/****************************************************************************
 * Description    :  getName()
                     Returns the object's parameter specifications map 
 * Parameters     :  No
 * Return value   :  A pointer towards the parameter specifications map
 ***************************************************************************/
string slArgHandler::getName() const
{
	return name_;
}


unsigned int slArgHandler::getQuota() const
{
	return quota_;
}


void slArgHandler::clearParameters()
{
	contParameters_.clear();
}


/*!
 *	\param argc is the number of arguments in \c argv.  Note: it counts
 *		\c argv[0] which is the executable path or the compute node
 *		<tt>type:</tt>.  The arguments are parsed from \c argv[1] through
 *		<tt>argv[argc-1]</tt>.
 *	\param argv is a pointer to the first \c char* in a range of arguments.
 */
void slArgHandler::parse(unsigned int argc, char** argv)
{
	unsigned int argIndex = 1;				// index of first free parameter or switch
	map<string, const slParamSpec*> pool;	// Pool of compatible parameters, some mandatory
	slParameters parameters;				// Recorded parameters

	// Initialize all expected parameters without sub-parameters
	this->addToPool(pool);

	while (argIndex < argc)
	{
		// Get first arg which could be a free parameter or a switch
		string arg = argv[argIndex];
		argIndex++;

		// If arg is a switch
		if (pool.find(arg) != pool.end()) {
			const slParamSpec* paramSpec = pool[arg];
			unsigned int nbValues = paramSpec->getSyntaxSize();

			// Make sure enough arguments are provided
			if (argIndex + nbValues > argc) {
				throw slExceptionArgHandler(string("Not enough values for ") + arg);
			}

			// Make sure the switch is not already parsed
			if (parameters.isParsed(arg)) {
				throw slExceptionArgHandler(arg + " is already parsed.");
			}

			// Read all values for that switch
			slValVect values;

			for (unsigned int indVal = 0; indVal < nbValues; indVal++) {
				values << argv[argIndex];
				argIndex++;
			}

			// Save the switch and its values
			parameters.setValue(arg, values);

			// Get the sub parameters for this vector of values
			const slParamSpecMap *psMap = paramSpec->getSubParamSpec(values);

			if (psMap != NULL) {
				psMap->addToPool(pool);
			}
		}
		else {	// arg is simply a free parameter
			parameters << arg;
		}
	}

	// We need to verify that all mandatory parameters in pool are defined
	for (map<string, const slParamSpec*>::const_iterator itPool = pool.begin();
		itPool != pool.end(); itPool++)
	{
		// If parameter in pool is mandatory
		if (itPool->second->isMandatory()) {
			// If parameter is not parsed, throw an error
			if (!parameters.isParsed(itPool->first)) {
				throw slExceptionArgHandler("(for " + name_ + ") " +
					itPool->second ->getDescription() +
					" (" + itPool->first + ") is mandatory"); 
			}
		}
	}

	parameters.savePool(pool);
	contParameters_.push_back(parameters);
}


void slArgHandler::testQuota() const
{
	if (contParameters_.size() < quota_) {
		throw slExceptionArgHandler(string("slArgHandler::testQuota(): "
			"missing at least one \"") + name_ + "\" compute node");
	}
}


unsigned int slArgHandler::getNumberOfNodes() const
{
	return contParameters_.size();
}


const slParameters& slArgHandler::getParameters(unsigned int index) const
{
	if (index >= contParameters_.size()) {
		throw slExceptionArgHandler("slArgHandler::getParameters(): (in " + name_ + ") bad index");
	}

	return contParameters_[index];
}


void slArgHandler::printParameters(unsigned int index) const
{
	if (index >= contParameters_.size()) {
		throw slExceptionArgHandler("slArgHandler::printParameters(): (in " + name_ + ") bad index");
	}

	contParameters_[index].printParameters();
}


/****************************************************************************
 * Description    :  verifyMandVal()
                     Clears the pool of parameter specifications and the
                     Argument Handler objects that created it.
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slArgHandler::printUsage() const
{
	cout << "------------------------------------------------------------------" << endl;
	cout << name_ << " \\" << endl;
	printParamUsage(1);
}




/******************************************************************************
-------------------- SLPROCESSARGHANDLER IMPLEMENTATION -----------------------
******************************************************************************/


/****************************************************************************
 * Description    :  Constructor with parameters
 * Parameters     :  - execName (string): the name of the executable with
                                          the arguments.
 * Return value   :  No
 ***************************************************************************/
slArgProcess::slArgProcess(const std::string& execName)
: slArgHandler(execName)
{
}


/****************************************************************************
 * Description    :  Constructor for copy
 * Parameters     :  - execName (string): the name of the executable with
                                          the arguments.
 * Return value   :  No
 ***************************************************************************/
slArgProcess::slArgProcess(const slArgProcess& c)
: slArgHandler(c)
{
	contArgHandler_ = c.contArgHandler_;
}


/****************************************************************************
 * Description    :  Destructor
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
slArgProcess::~slArgProcess()
{
	contArgHandler_.clear();
}


/****************************************************************************
 * Description    :  getExecName()
                     Returns the object's executable name 
 * Parameters     :  No
 * Return value   :  The object's executable name 
 ***************************************************************************/
string slArgProcess::getExecName() const
{
	return name_;
}


slArgProcess& slArgProcess::addGlobal(const slParamSpec& paramSpec)
{
	slParamSpecMap::operator<<(paramSpec);

	return *this;
}


/****************************************************************************
 * Description    :  addArgHandler()
                     Returns the object's parameter specifications map 
 * Parameters     :  No
 * Return value   :  A pointer towards the parameter specifications map
 ***************************************************************************/
slArgProcess& slArgProcess::operator<<(const slArgHandler& argHandler)
{
	contArgHandler_[argHandler.getName()] = argHandler;

	return *this;
}


/****************************************************************************
 * Description    :  parse()
                     Parses the basic parameters and sends their respective
                     contents to their respective Argument Handler.
 * Parameters     :  - argc (int): the number of arguments
                     - argv (char**) : a pointer to an array of character
                                       chains.
 * Return value   :  No
 ***************************************************************************/
void slArgProcess::parse(unsigned int argc, char** argv)
{
	unsigned int begin = 0;
	string nodeName = "";
	vector<char*> vargv;

	for (map<string, slArgHandler>::iterator it = contArgHandler_.begin();
		it != contArgHandler_.end(); it++)
	{
		it->second.clearParameters();
	}
	clearParameters();

	for (unsigned int argIndex = begin; argIndex < argc; argIndex++) {
		string arg = argv[argIndex];

		// If beginning of a new compute node
		if (*arg.rbegin() == ':') {
			// Parse compute node parameters
			if (!nodeName.empty()) {
				contArgHandler_[nodeName].parse(argIndex - begin, &argv[begin]);
			}

			// New beginning
			begin = argIndex;

			// New compute node or global mode (if empty)
			nodeName = arg.substr(0, arg.size() - 1);

			if (!nodeName.empty() && contArgHandler_.find(nodeName) == contArgHandler_.end()) {
				throw slExceptionArgHandler(string("slArgProcess::parse(): \"") +
					nodeName + "\" invalid compute node name");
			}
		}
		// If global mode
		else if (nodeName.empty()) {
			// Record char pointer
			vargv.push_back(argv[argIndex]);
		}
	}

	// Parse last compute node parameters
	if (!nodeName.empty()) {
		contArgHandler_[nodeName].parse(argc - begin, &argv[begin]);
	}

	for (map<string, slArgHandler>::const_iterator it = contArgHandler_.begin();
		it != contArgHandler_.end(); it++)
	{
		it->second.testQuota();
	}

	slArgHandler::parse(vargv.size(), &vargv[0]);
}


unsigned int slArgProcess::getNumberOfNodes(const std::string &nodeName) const
{
	if (nodeName.empty()) {
		return 1;
	}
	
	map<string, slArgHandler>::const_iterator itArgH = contArgHandler_.find(nodeName);

	if (itArgH == contArgHandler_.end()) {
		throw slExceptionArgHandler(string("slArgProcess::getNumberOfNodes(): \"") +
			nodeName + "\" is an invalid node name");
	}

	return itArgH->second.getNumberOfNodes();
}


const slParameters& slArgProcess::getParameters(const std::string &nodeName, unsigned int index) const
{
	if (nodeName.empty()) {
		return slArgHandler::getParameters(index);
	}
	
	map<string, slArgHandler>::const_iterator itArgH = contArgHandler_.find(nodeName);

	if (itArgH == contArgHandler_.end()) {
		throw slExceptionArgHandler(string("slArgProcess::getParameters(): \"") +
			nodeName + "\" is an invalid node name");
	}

	return itArgH->second.getParameters(index);
}


void slArgProcess::printParameters(unsigned int index) const
{
	slArgHandler::printParameters();

	for (map<string, slArgHandler>::const_iterator it = contArgHandler_.begin();
		it != contArgHandler_.end(); it++)
	{
		for (unsigned int ind = 0; ind < it->second.getQuota(); ind++) {
			cout << "    " << it->first << ":";
			if (it->second.getQuota() > 1) {
				cout << " (" << ind + 1 << "/" << it->second.getQuota() << ")";
			}
			cout << endl;

			it->second.printParameters(ind);
		}
	}
}


/****************************************************************************
 * Description    :  printUsage()
                     Clears the pool of parameter specifications and the
                     Argument Handler objects that created it.
 * Parameters     :  No
 * Return value   :  No
 ***************************************************************************/
void slArgProcess::printUsage() const
{
	cout << "------------------------------------------------------------------" << endl;
	cout << name_ << " <globalParams> ";

	for (map<string, slArgHandler>::const_iterator itArg = contArgHandler_.begin();
			itArg != contArgHandler_.end(); itArg++)
	{
		if (itArg->second.getQuota() == 0) {
			cout << "[" << itArg->first << ": <" << itArg->first << "Params>] ";
			continue;
		}

		for (unsigned int repeat = 0; repeat < itArg->second.getQuota(); repeat++) {
			cout << itArg->first << ": <" << itArg->first << "Params> ";
		}
	}

	cout << "[: <globalParams>]" << endl << endl << "<globalParams> = \\" << endl;
	printParamUsage(1);

	for (map<string, slArgHandler>::const_iterator itArg = contArgHandler_.begin();
			itArg != contArgHandler_.end(); itArg++)
	{
		cout << endl << "<" << itArg->first << "Params> = \\";

		if (itArg->second.getQuota() > 1) {
			cout << "\t# Note: at least " << itArg->second.getQuota()
				<< " \"" << itArg->first << ":\" nodes are needed";
		}

		cout << endl;
		itArg->second.printParamUsage(1);
	}
}


