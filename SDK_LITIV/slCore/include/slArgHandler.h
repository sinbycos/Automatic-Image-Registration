/*!	\file	slArgHandler.h
 *	\brief	All classes for arguments handling
 *
 *	This is the declaration of all the classes needed to handle
 *	the arguments used in SDK-LITIV projects.
 *
 *	Please, take a look at namespace slAH.
 *
 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
 *	\date		April 2010
 */

#ifndef _SLARGHANDLER_H_
#define	_SLARGHANDLER_H_


#define NOT_MANDATORY false
#define MANDATORY true


#include <map>
#include <vector>

#include "slException.h"


//!	Argument handler utils
/*!
 *	slAH is a set of classes working together to provide argument handling.
 *	These classes are organized as in the following figure:
 *	\dot
 *	digraph slAH {
 *		rankdir=BT;
 *		node [shape=record, fontname=Helvetica, fontsize=10];
 *
 *		slValVect [ URL="\ref slValVect"];
 *		slSyntax [ URL="\ref slSyntax"];
 *		slParamSpec [ URL="\ref slParamSpec"];
 *		slParameters [ URL="\ref slParameters"];
 *		slParamSpecMap [ URL="\ref slParamSpecMap"];
 *		slArgHandler [ URL="\ref slArgHandler"];
 *		slArgProcess [ URL="\ref slArgProcess"];
 *
 *		slParamSpecMap -> slParamSpec [arrowhead="open"];
 *		slParamSpec -> slParamSpecMap [arrowhead="open"];
 *		slParamSpec -> slSyntax [arrowhead="open"];
 *
 *		slArgProcess -> slArgHandler [arrowhead="open"];
 *		slArgHandler -> slParameters [arrowhead="open"];
 *		slParameters -> slValVect [arrowhead="open"];
 *		slParameters -> slParamSpec [arrowhead="open", style="dashed"];
 *
 *		slArgProcess -> slArgHandler -> slParamSpecMap [color="blue4"];
 *	}
 *	\enddot
 *	Long story short: <tt>;-)</tt>
 *	- A slArgProcess is a slArgHandler which is a slParamSpecMap.
 *	- A slParamSpecMap contains the specification of many switched parameters:
 *	one slParamSpec per switch (like \c -s or \c --switch).
 *	- Typically, a slParamSpec contains zero, one or many slSyntax, one for each
 *	component of the expected values corresponding to the specified parameter.
 *	- A slParamSpec also contains sub-parameters in a slParamSpecMap for
 *	expected values in a slValVect.
 *	- A slArgHandler, according to its slParamSpec objects inherited from
 *	slParamSpecMap, parses a range of arguments in \c argv.
 *	- The resulting parameters are stored in a slParameters object which
 *	contains a slValVect for each recognized switch.
 *	- A slParameters object also stores other arguments.
 *	- Finally, a slArgProcess can manage many different compute nodes with
 *	as much slArgHandler objects that could be reused for similar compute nodes.
 *
 *	\section ex1 Example 1
 *	Here is an example of what kind of command-line we could parse with
 *	an instance of slArgProcess properly configured:
 *	\code
 *	executable -a param1 val1 -b -c p2 p3 node1: -d 234 -e node2: -f fic.avi : val2 -z 321 -y
 *	\endcode
 *	- Program name: \c executable
 *	- All global parameters: <tt>-a param1 val1 -b -c p2 p3 val2 -z 321 -y</tt>
 *	- Global switches: <tt>-a -b -c -z -y</tt>
 *	- Number of values for parameter \c -c: 2, i.e. \c p2 and \c p3
 *	- Other global arguments: <tt>val1 val2</tt>
 *	- The compute nodes: \c node1 and \c node2
 *	- \em node1 parameters: <tt>-d 234 -e</tt>
 *	- \em node2 parameters: <tt>-f fic.avi</tt>
 *
 *	In this example, we would have three different slArgHandler: one for
 *	\c node1, one for \c node2, and a last one which is slArgProcess
 *	for the global parameters.
 *	As seen in the previous example, a compute node starts with \c tag: and
 *	ends with the end of \c argv, with the beginning of another compute node or
 *	with a colon (<tt>:</tt>).
 *	When a colon is found, the parser stores the following parameters
 *	as global parameters in slArgProcess until another compute node starts.
 *
 *	The <em>other arguments</em> are standalone arguments in \c argv that are
 *	not associated to any known switch (like -s or --switch).
 *	Even unknown switches are interpreted as <em>other arguments</em>.
 *	This let us support command-line syntaxes with an undefined number of
 *	arguments, like a list of filenames, images, etc.
 *
 *	The previous command-line could be parsed by the following slArgProcess object:
 *	\code
 *	slArgProcess argProcess("executable");
 *	slArgHandler argH1("node1"), argH2("node2");
 *
 *	argProcess
 *		.addGlobal(slParamSpec("-a", "Algorithm", MANDATORY) << slSyntax("algo_name"))
 *		.addGlobal(slParamSpec("-b", "Bold"))
 *		.addGlobal(slParamSpec("-c", "Center") << slSyntax("x") << slSyntax("y"))
 *		.addGlobal(slParamSpec("-z", "Z position") << slSyntax("z-pos", "321"))
 *		.addGlobal(slParamSpec("-y", "Flip vertically"));
 *
 *	argH1
 *		<< (slParamSpec("-d", "Delta max") << slSyntax("delta", "256"))
 *		<< (slParamSpec("-e", "Show errors"));
 *	argH2
 *		<< (slParamSpec("-f", "Filename") << slSyntax("filename") << MANDATORY);
 *
 *	argProcess << argH1 << argH2;
 *	\endcode
 *	Then, this is how we may use it:
 *	\code
 *	argProcess.printUsage();
 *	argProcess.parse(argc, argv);
 *
 *	slParameters globalParams(argProcess.getParameters(""));
 *	slParameters node1Params(argProcess.getParameters("node1"));
 *	slParameters node2Params(argProcess.getParameters("node2"));
 *
 *	globalParams.getDefValue("-z");
 *	for (unsigned int i = 0; i < globalParams.getNbOtherArgs(); i++) {
 *		globalParams.getOtherArg(i);
 *	}
 *
 *	bool showErrors = node1Params.isParsed("-e");
 *	string filename = node2Params.getValue("-f");
 *	\endcode
 *
 *	\section ex2 Example 2
 *	If two instances of the same compute node is needed, the following
 *	command-line remains valid in slArgProcess:
 *	\code
 *	executable bgSub: -a tempAVG bgSub: -a tempAVG -s 64
 *	\endcode
 *
 *	In this example, a single slArgHandler, named \c bgSub, would have a quota
 *	of two instances of slParameters.
 *	In other words, the programmer could specify a quota of two, and this quota
 *	will be automatically verified at the end of the parsing process where two
 *	slParameters objects must be stored in slArgHandler \c bgSub.
 *
 *	Here is how we build a slArgProcess object for the previous command-line:
 *	\code
 *	slArgProcess argProcess("executable");
 *	slArgHandler argH("bgSub", 2);
 *
 *	argH
 *		<< (slParamSpec("-a", "Algo name", MANDATORY) << slSyntax("algo"))
 *		<< (slParamSpec("-s", "Size filter") << slSyntax("nbPix", "256"));
 *
 *	argProcess << argH;
 *	\endcode
 *	Then, we would use it like this:
 *	\code
 *	argProcess.parse(argc, argv);
 *	for (unsigned int n = 0; n < 2; n++) {
 *		slParameters params(argProcess.getParameters("bgSub", n));
 *		string algo = params.getValue("-a");
 *	}
 *	\endcode
 *
 *	\see	slArgProcess, slArgHandler, slArgHandler.h, slComputeNode
 *	\see	File <em>doc/slArgHandler.docx</em> for the first available documentation.
 */
namespace slAH
{
	//!	A vector of values
	/*!
	 *	This class is a vector of strings (std::string) which are values
	 *	corresponding to the components of a parameter.
	 *	It could be used to save parsed parameters or to specify expected
	 *	values for sub-parameters.
	 *
	 *	Here is an example of how to create and fill a slValVect:
	 *	\code
	 *	slValVect() << "string1" << "string2"
	 *	\endcode
	 *
	 *	\see	slAH for overview of all classes,
	 *			slParamSpec for a complete parameter specification,
	 *			slParameters for parsed parameters
	 *
	 *	\author	Pier-Luc St-Onge, Michael Eilers-Smith
	 *	\date	April 2010
	 */
	class SLCORE_DLL_EXPORT slValVect 
	{
	public:
		slValVect();	//!< To initialize an empty vector of parameter values

		// Insert functions

		slValVect& operator<<(const std::string& value);	//!< Insert operator: appends a value

		// Get functions

		unsigned int size() const;							//!< Returns the number of components
		std::string getValue(unsigned int index = 0) const;	//!< Returns the value of a component or an empty string
		const std::string& operator[](unsigned int index) const;	//!< Returns the value of a component or raises an exception

		// Other operators

		bool operator<(const slValVect& valVect) const;		// Needed for a map
		std::ostream& operator>>(std::ostream &ostr) const;	// Needed for ostr <<

	private:
		std::vector<std::string> contVal_;
	};


	//! Operator: ostr << valVect
	/*!
	 *	This operator is useful for writing to stdout or any output stream.
	 *
	 *	\see	slValVect
	 */
	SLCORE_DLL_EXPORT std::ostream& operator<<(std::ostream &ostr, const slValVect &valVect);


	//!	Syntax for one component of a parameter
	/*!
	 *	It contains the syntax of the component and a default value for that
	 *	component.  These are std::string attributes.
	 *
	 *	For example, if we want to specify a color in the HSV color system,
	 *	the corresponding syntax could be something like:
	 *	\code
	 *	--hsv h s v
	 *	\endcode
	 *
	 *	The way to specify such parameter with three components is as follow:
	 *	\code
	 *	slParamSpec("--hsv") << slSyntax("h", "0") << slSyntax("s", "0") << slSyntax("v", "255")
	 *	\endcode
	 *	In the previous example, the default color would be white: <tt>hsv(0, 0, 255)</tt>.
	 *
	 *	\see	slAH for overview of all classes,
	 *			slParamSpec for a single parameter specification
	 *
	 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
	 *	\date		April 2010
	 */
	class SLCORE_DLL_EXPORT slSyntax
	{
	public:
		//! Creates the syntax of a component with an optional default value
		slSyntax(const std::string& synt, const std::string& defValue = "");

		const std::string& getSyntax() const;	//!< Returns the syntax of the component
		const std::string& getDefault() const;	//!< Returns the default value or an empty string

	private:
		std::string syntax_;
		std::string default_;
	};


	class slParamSpecMap;	// Needed for slParamSpec declaration


	//!	A complete parameter specification
	/*!
	 *	With this class, it is possible to specify a parameter according to:
	 *	- a switch (-s, --switch)
	 *	- a description (text)
	 *	- zero, one or more components with their default values (slSyntax)
	 *	- requirement of the program (mandatory or not).  Default: not mandatory
	 *
	 *	Here are three different ways to fill a slParamSpec instance:
	 *	\code
	 *	slParamSpec("-a", "Algorithm name") << MANDATORY << slSyntax("algo_name")
	 *	slParamSpec("-a", "Algorithm name") << slSyntax("algo_name") << true
	 *	slParamSpec("-a", "Algorithm name", MANDATORY) << slSyntax("algo_name")
	 *	\endcode
	 *
	 *	For some expected values, it is possible to specify sub-parameters
	 *	that are "private" to these values.  Example:
	 *	\code
	 *	slParamSpec("-a", "Algo...").setSubParamSpec(
	 *		slValVect() << "algo1",
	 *		slParamSpecMap() << (slParamSpec("-r", "Rotation") << slSyntax("degrees"))
	 *	);
	 *	\endcode
	 *	In this example, if someone writes <tt>-a algo1</tt>, the switch
	 *	\c -r becomes available after \c -a is parsed.
	 *
	 *	Here is a complete example:
	 *	\code
	 *	slParamSpec specAlgo("-a", "Algorithm name", MANDATORY);
	 *	specAlgo << slSyntax("algo_name");
	 *	specAlgo.setSubParamSpec(
	 *		slValVect() << "algo1",
	 *		slParamSpecMap() << (slParamSpec("-r", "Rotation") << slSyntax("degrees"))
	 *	);
	 *	specAlgo.printParamUsage(1); // With one tabulation
	 *	\endcode
	 *
	 *	\see	slAH for overview of all classes,
	 *			slSyntax for parameter component syntax,
	 *			slValVect for expected values,
	 *			slParamSpecMap for sub-parameters
	 *
	 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
	 *	\date		April 2010
	 */
	class SLCORE_DLL_EXPORT slParamSpec 
	{
	public:
		//! Creates a parameter with a switch, a description and the mandatory flag
		slParamSpec(const std::string& paramSwitch = "", const std::string& desc = "",
			const bool mandatory = NOT_MANDATORY);

		// Get functions

		const std::string& getSwitch() const;		//!< Returns the switch of the parameter
		const std::string& getDescription() const;	//!< Returns the description
		bool isMandatory() const;					//!< Returns true if parameter is mandatory in \c argv, false otherwise

		unsigned int getSyntaxSize() const;			//!< Returns the number of components
		const std::string& getDefault(unsigned int index = 0) const;	//!< Returns the default value of one component

		//! Returns the slParamSpecMap pointer corresponding to a slValVect or NULL
		const slParamSpecMap* getSubParamSpec(const slValVect& values) const;

		// Set functions

		slParamSpec& operator<<(bool mandatory);			//!< Sets the "mandatory" flag
		slParamSpec& operator<<(const slSyntax& syntax);	//!< Appends a component syntax

		//! Add sub-parameters for an expected slValVect
		slParamSpec& setSubParamSpec(const slValVect& values, const slParamSpecMap& paramMap);

		//! Print global usage with a customizable margin
		void printParamUsage(const unsigned int nbTab) const;

	private:
		std::string switch_;		// -s, --switch
		std::string description_;	// Description
		bool mandatory_;			// Needed or not

		std::vector<slSyntax> syntaxVector_;	// Values expected: -s 640 480 -> 2 values, a syntax for each
		std::map<slValVect, slParamSpecMap> contValParams_;	// Even more parameters for some expected values
	};


	//!	Contains all parsed parameters and other information
	/*!
	 *	There are parsed parameters that are classified according to their
	 *	corresponding switch (see slParamSpec).
	 *	There are also "other arguments", which could be an unlimited
	 *	list of items like filenames, numbers, etc.
	 *
	 *	An instance of slParameters is created and filled in function
	 *	slArgHandler::parse().
	 *	It could be retrieved by slArgHandler::getParameters() or
	 *	slArgProcess::getParameters().
	 *
	 *	With function isParsed(), we know if a parameter has been set by \c argv.
	 *	If not parsed, we can manually get the default value for one component
	 *	of a parameter with function getDefValue().
	 *	The function getValue() returns the value of one component of the
	 *	parameter if it is parsed.  If not, getValue() will call automatically
	 *	getDefValue().
	 *
	 *	For other arguments, it is possible to retrieve them with function
	 *	getOtherArg().  The total number of other arguments is available with
	 *	function getNbOtherArgs().
	 *
	 *	\see	slAH for overview of all classes,
	 *			slValVect for parsed values,
	 *			slArgHandler, slArgProcess, slParamSpec
	 *
	 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
	 *	\date		April 2010
	 */
	class SLCORE_DLL_EXPORT slParameters
	{
	public:
		slParameters();	//!< Constructor used by slArgHandler

		// Insert functions

		void savePool(const std::map<std::string, const slParamSpec*>& pool);

		void setValue(const std::string& paramSwitch, const slValVect& values);
		bool isParsed(const std::string& paramSwitch) const;	//!< Returns true if the parameter is parsed, false otherwise

		slParameters& operator<<(const std::string& otherArg);

		// Get functions

		const std::string& getDescription(const std::string& paramSwitch) const;

		//! Returns the default value of one component of the parameter or the empty string
		const std::string& getDefValue(const std::string& paramSwitch, unsigned int index = 0) const;

		//! Returns the parsed value or the default value if the parameter is not parsed
		const std::string& getValue(const std::string& paramSwitch, unsigned int index = 0) const;

		unsigned int getNbOtherArgs() const;						//!< Returns number of other arguments
		const std::string& getOtherArg(unsigned int index) const;	//!< Returns an other argument or raises an exception

		//! Prints parsed values (including other arguments)
		void printParameters() const;

		//! Prints parsed values (including other arguments)
		void printParameters(std::ostream &ostr) const;

	private:
		std::map<std::string, const slParamSpec*> contPool_;	// To keep a link to the slParamSpec in pool

		std::map<std::string, slValVect> contParsedValues_;	// This is where parsed switches are stored
		std::vector<std::string> otherArgs_;				// These are other arguments
	};


	//!	Contains all parameters' and their sub-parameters' specs.
	/*!
	 *	This class is needed by slParamSpec where there are sub-parameters.
	 *	So, one slParamSpecMap may contain many slParamSpec, and these may
	 *	contain many slParamSpecMap which are associated to expected values.
	 *	In a slParamSpecMap, the slParamSpec are classified by their
	 *	corresponding switch (like \c -s).
	 *
	 *	slParamSpecMap is also the base class of slArgHandler.
	 *
	 *	Here is an example where two slParamSpec are stored:
	 *	\code
	 *	slParamSpecMap()
	 *		<< (slParamSpec("-a", "Algorithm name") << slSyntax("name"))
	 *		<< (slParamSpec("-s", "Size") << slSyntax("w", "640") << slSyntax("h", "480"));
	 *	\endcode
	 *
	 *	\see	slAH for overview of all classes,
	 *			slParamSpec for complete parameter specification,
	 *			slArgHandler
	 *
	 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
	 *	\date		April 2010
	 */
	class SLCORE_DLL_EXPORT slParamSpecMap
	{
	public:
		//! Inserts an slParamSpec object
		slParamSpecMap& operator<<(const slParamSpec& paramSpec);

		// For parsing purposes
		void addToPool(std::map<std::string, const slParamSpec*>& pool) const;

		// Prints recursively the usage of all parameters
		void printParamUsage(const unsigned int nbTab) const;

	protected:
		std::map<std::string, slParamSpec> contParamSpec_;	// paramSwitch -> specification
	};


	//!	This is the argument parser engine
	/*!
	 *	Because this class inherits from slParamSpecMap, it contains a set of
	 *	slParamSpec instances which are classified by their corresponding
	 *	switch (-s, --switch, etc.).
	 *	So, slArgHandler works as slParamSpecMap, with all the mecanisms for
	 *	sub-parameters.
	 *
	 *	Usually, a slArgHandler is associated to one or many similar
	 *	slComputeNode objects.
	 *	In fact, one slArgHandler instance could parse many \c argv.
	 *	The result of each parsed \c argv is stored in a vector of
	 *	slParameters objects.
	 *	So, when used with a slArgProcess, the slArgHandler object could be
	 *	constructed with a compute node name and a quota.  Example:
	 *	\code
	 *	slArgHandler argHandler("bgSub", 2); // Minimum quota of 2
	 *	// Insert in slArgProcess object, then parse with the slArgProcess
	 *	\endcode
	 *
	 *	It is also possible to use slArgHandler alone when the command-line
	 *	syntax is simple, i.e. a simple list of switches and other arguments.
	 *	Here is an example:
	 *	\code
	 *	slArgHandler argHandler("executable");
	 *	argHandler
	 *		<< (slParamSpec("-a", "Algorithm name") << slSyntax("name"))
	 *		<< (slParamSpec("-s", "Size") << slSyntax("w", "640") << slSyntax("h", "480"));
	 *	argHandler.printUsage();
	 *	argHandler.parse(argc, argv);
	 *	argHandler.printParameters();
	 *	\endcode
	 *
	 *	To retrieve parameters, we first need to have access to the
	 *	slParameters object containing all parsed parameters of \c argv.
	 *	This is done by calling function getParameters().
	 *	Then, with the slParameters object, we can retrieve the switchable
	 *	parameters and the other arguments.
	 *
	 *	\see	slAH for overview of all classes,
	 *			slParamSpecMap, slArgProcess, slParameters, slComputeNode
	 *
	 *	\author		Pier-Luc St-Onge, Michael Eilers-Smith
	 *	\date		April 2010
	 */
	class SLCORE_DLL_EXPORT slArgHandler: public slParamSpecMap
	{
	public:
		//! Constructor for standalone slArgHandler or for compute node argument handler
		slArgHandler(const std::string& name = "", unsigned int quota = 1);

		slArgHandler(const slArgHandler& c);

		virtual ~slArgHandler();

		// Get functions

		std::string getName() const;	//!< Returns the executable name or compute node name
		unsigned int getQuota() const;	//!< Returns the quota for the compute node

		// Parsing functions

		void clearParameters();
		virtual void parse(unsigned int argc, char** argv);	//!< Parses a range of arguments in argv[]

		void testQuota() const;

		//! Returns the number of slParameters objects
		unsigned int getNumberOfNodes() const;

		//! Returns parsed parameters for one range of arguments
		const slParameters& getParameters(unsigned int index = 0) const;

		//! Prints one set of parsed parameters to stdout
		virtual void printParameters(unsigned int index = 0) const;
		virtual void printUsage() const;	//!< Prints to stdout the usage of the executable or the compute node

	protected:
		std::string name_;		// Name of the compute node or executable
		unsigned int quota_;

		std::vector<slParameters> contParameters_;
	};


	//!	This is the multi-argument parser engine
	/*!
	 *	This class inherits from slArgHandler because it must be able to parse
	 *	global parameters.
	 *	So, it works like a slArgHandler, except for the function
	 *	operator<<(const slParamSpec&) that is replaced by addGlobal().
	 *
	 *	Furthermore, it contains many slArgHandler instances.
	 *	This is useful when one wants to specify different sets of parameters
	 *	for different types of compute nodes.
	 *	Finally, for each slArgHandler, it is possible to specify how many
	 *	times it must be used, i.e. the minimum number of compute nodes of the
	 *	corresponding type.
	 *	Note: a slArgHandler could also correspond to something else than a
	 *	compute node if we want to.
	 *
	 *	Here is a complete example:
	 *	\code
	 *	slArgProcess argProcess("executable");
	 *	argProcess
	 *		.addGlobal(slParamSpec("-a", "Algorithm name") << slSyntax("name"))
	 *		.addGlobal(slParamSpec("--help", "To print the usage"));
	 *
	 *	slArgHandler argHandler("bgSub", 2); // Minimum quota of 2
	 *	argProcess << argHandler;
	 *
	 *	argProcess.printUsage();
	 *	argProcess.parse(argc, argv);
	 *	argProcess.printParameters();
	 *	\endcode
	 *
	 *	To retrieve parameters, we first need to have access to the
	 *	slParameters object containing all parsed parameters of \c argv.
	 *	This is done by calling function getParameters(), and by giving
	 *	the name of the compute node (without the colon <tt>:</tt>) or an empty
	 *	string for the global parameters.
	 *	Then, with the slParameters object, we can retrieve the switchable
	 *	parameters and the other arguments.
	 *
	 *	\see	slAH for overview of all classes,
	 *			slArgHandler, slParameters, slComputeNode
	 *
	 *	\author		Michael Eilers-Smith, Pier-Luc St-Onge
	 *	\date		April 2010
	 */
	class SLCORE_DLL_EXPORT slArgProcess: public slArgHandler
	{
	public:
		//! Default constructor
		slArgProcess(const std::string& execName = "executable");
		slArgProcess(const slArgProcess& c);

		virtual ~slArgProcess();

	public:
		std::string getExecName() const;	//!< Returns the executable name

		//! Because slParamSpecMap::operator<<() is unreachable, VS2008 needs this function
		slArgProcess& addGlobal(const slParamSpec& paramSpec);

		//! To insert a new slArgHandler
		slArgProcess& operator<<(const slArgHandler& argHandler);

		//! Parses a complete \c argv, the one received by \c main()
		void parse(unsigned int argc, char** argv);

		//! Returns the exact number of parsed nodeName compute nodes
		unsigned int getNumberOfNodes(const std::string &nodeName) const;

		//! Returns parsed parameters for one range of arguments
		const slParameters& getParameters(const std::string &nodeName, unsigned int index = 0) const;

		//! Prints one set of parsed parameters to stdout
		virtual void printParameters(unsigned int index = 0) const;
		virtual void printUsage() const;	//!< Prints to stdout the usage of the executable

	private:
		std::map<std::string, slArgHandler> contArgHandler_;	// slComputeNode name -> its slArgHandler
	};

};	// namespace slAH


#endif	// _SLARGHANDLER_H_


