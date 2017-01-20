// tCommand.cpp
//
// Parses a command line. A command line takes the form:
// program.exe [arg1 arg2 arg3 ...]
// Arguments are separated by spaces. An argument must be enclosed in quotes (single or double) if it has a space in it.
// Use escape sequences to put either type of quote inside. If you need to specify paths, I suggest using forward
// slashes, although backslashes will work so long as the filename does not have a single or double quote next.
//
// An argument may be an 'option' or 'parameter'.
// An option is a combination of a 'flag' specified using a single or double hyphen, and zero or more option arguments.
// A parameter is just a single string.
//
// Example:
// mycopy.exe -R --overwrite fileA.txt -pat fileB.txt --log log.txt
//
// The fileA.txt and fileB.txt in the above example are parameters (assuming overwrite does not take any option
// arguments). The order in which parameters are specified is important. fileA.txt is the first parameter, and fileB.txt
// is the second. Options on the other hand can be specified in any order. All options take a specific number (zero or
// more) of option arguments. If an option takes zero arguments you can only test for its presence (or lack of).
//
// The '--log log.txt' is an option with a single option argument, log.txt. Single character flags specified with a
// single hyphen may be combined. The -pat in the example expands to -p -a -t. It is suggested not to combine flags
// when options take arguments as only the last flag would get them.
//
// Variable argument counts are not supported but you may list the same option more than once. Eg.
// -i filea.txt -i fileb.txt etc is valid.
//
// A powerful feature of the design of this parsing system is separation of concerns. In a typical system the knowledge
// of all the different command line parameters and options is needed in a single place, often in main() where argc and
// argv are passed in. These values need to somehow be passed all over the place in a large system. With tCommand you
// specify which options and parameters you care about only in the cpp file you are working in.
//
// To use the command line class, you start by registering your options and parameters. This is done using the tOption
// and tParam types to create static objects. After main calls the parse function, your objects get populated
// appropriately. For example,
//
// FileA.cpp:
// tParam FromFile(1, "FromFile");	// The 1 means this is the first parameter. The description is optional.
// tParam ToFile(2, "ToFile");		// The 2 means this is the second parameter. The description is optional.
// tOption("log", 'l', 1, "Specify log file");	// The 1 means there is one option argument to --log or -l.
//
// FileB.cpp:
// tOption ProgramOption('p', 0, "Program mode.");
// tOption AllOption('a', "ALL", 0, "Include all widgets.");
// tOption TimeOption("time", 't', 0, "Print timestamp.");
//
// Main.cpp:
// tParse(argc, argv);
//
// Internal Processing. The first step is the expansion of combined single hyphen options. Next the parameters and
// options are parsed out. For each registered tOption and tParam object, its members are set to reflect the current
// command line when the tParse call is made. You may have more than one tOption that responds to the same option name.
// You may have more than one tParam that responds to the same parameter number.
//
// Copyright (c) 2017 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#include "System/tCommand.h"
#include "System/tFile.h"


namespace tCommand
{
	// Any single-hyphen combined arguments are expanded here. Ex. -abc becomes -a -b -c.
	void ExpandArgs(tList<tStringItem>& args);

	// The aggregate-list construction syntax is guaranteed to happen before normal construction. This allows the lists
	// of modes and options to be properly constructed before they are populated by any globally constructed tCommand
	// and tOption objects. @todo Actually, I need to check this with the C++11 standard at some point.
	tList<tParam> Params = { false };
	tList<tOption> Options = { false };
	tString Program;
	tString Empty;
}


tCommand::tParam::tParam(int paramNumber, const char* paramDesc) :
	ParamNumber(paramNumber),
	Param(),
	Description()
{
	if (paramDesc)
		Description = tString(paramDesc);
	Params.Append(this);
}


tCommand::tOption::tOption(const char* description, char shortName, const char* longName, int numArgs) :
	ShortName(shortName),
	LongName(longName),
	Description(description),
	NumFlagArgs(numArgs),
	Args(),
	Present(false)
{
	Options.Append(this);
}


tCommand::tOption::tOption(const char* description, const char* longName, char shortName, int numArgs) :
	ShortName(shortName),
	LongName(longName),
	Description(description),
	NumFlagArgs(numArgs),
	Args(),
	Present(false)
{
	Options.Append(this);
}


tCommand::tOption::tOption(const char* description, char shortName, int numArgs) :
	ShortName(shortName),
	LongName(),
	Description(description),
	NumFlagArgs(numArgs),
	Args(),
	Present(false)
{
	Options.Append(this);
}


tCommand::tOption::tOption(const char* description, const char* longName, int numArgs) :
	ShortName(),
	LongName(longName),
	Description(description),
	NumFlagArgs(numArgs),
	Args(),
	Present(false)
{
	Options.Append(this);
}


const tString& tCommand::tOption::ArgN(int n) const
{
	for (tStringItem* arg = Args.First(); arg; arg = arg->Next(), n--)
		if (n <= 1)
			return *arg;

	return Empty;
}


void tCommand::tParse(int argc, char** argv)
{
	if (argc <= 0)
		return;

	// Set the program name as typed in the command line.
	Program = argv[0];

	// Create a single line string of all the separate argv's. Arguments with quotes and spaces will come in as
	// distinct argv's, but they all get combined here. I don't believe two consecutive spaces will work.
	tString line;
	for (int a = 1; a < argc; a++)
	{
		char* arg = argv[a];
		if (!arg || (tStd::tStrlen(arg) == 0))
			continue;

		line += arg;
		if (a < (argc - 1))
			line += " ";
	}

	tParse(line);
}


void tCommand::ExpandArgs(tList<tStringItem>& args)
{
	tList<tStringItem> expArgs;
	while (tStringItem* arg = args.Remove())
	{
		if ((arg->Length() < 2) || ((*arg)[0] != '-') || (((*arg)[0] == '-') && ((*arg)[1] == '-')))
		{
			expArgs.Append(arg);
			continue;
		}

		for (int flag = 1; flag < arg->Length(); flag++)
		{
			tString newArg = "-" + tString((*arg)[flag]);
			expArgs.Append(new tStringItem(newArg));
		}

		delete arg;
	}

	// Repopulate args.
	while (tStringItem* arg = expArgs.Remove())
		args.Append(arg);
}


void tCommand::tParse(const char* commandLine)
{
	tString line(commandLine);

	// Mark both kinds of escaped quotes that may be present. These may be found when the caller
	// wants a quote inside a string on the command line.
	line.Replace("\\'", tStd::SeparatorAStr);
	line.Replace("\\\"", tStd::SeparatorBStr);

	// Mark the spaces inside normal (non escaped) quotes.
	bool inside = false;
	for (char* ch = line.Text(); *ch; ch++)
	{
		if ((*ch == '\'') || (*ch == '\"'))
			inside = !inside;

		if (!inside)
			continue;

		if (*ch == ' ')
			*ch = tStd::SeparatorC;
	}

	line.Remove('\'');
	line.Remove('\"');
	tList<tStringItem> args;
	tStd::tExplode(args, line, ' ');

	ExpandArgs(args);

	// Now that the arguments are exploded into separate elements we replace the separators with the correct characters.
	int c = 0;
	for (tStringItem* arg = args.First(); arg; arg = arg->Next())
	{
		arg->Replace(tStd::SeparatorA, '\'');
		arg->Replace(tStd::SeparatorB, '\"');
		arg->Replace(tStd::SeparatorC, ' ');
	}

	// Process all options.
	for (tStringItem* arg = args.First(); arg; arg = arg->Next())
	{
		for (tOption* option = Options.First(); option; option = option->Next())
		{
			if ( (*arg == tString("--") + option->LongName) || (*arg == tString("-") + option->ShortName) )
			{
				option->Present = true;
				for (int optArgNum = 0; optArgNum < option->NumFlagArgs; optArgNum++)
				{
					arg = arg->Next();
					option->Args.Append(new tStringItem(*arg));
				}
			}
		}
	}

	// Now we're going to create a list of just the parameters by skipping any options as we encounter them.
	// For any option that we know about we'll also have to skip its option arguments.
	tList<tStringItem> commandLineParams;
	for (tStringItem* arg = args.First(); arg; arg = arg->Next())
	{
		tStringItem* candidate = arg;

		// This loop skips any options for the current arg.
		for (tOption* option = Options.First(); option; option = option->Next())
		{
			if (*(arg->Text()) == '-')
			{
				candidate = nullptr;
				tString flagArg = *arg;
				if ( (flagArg == tString("--") + option->LongName) || (flagArg == tString("-") + option->ShortName) )
				{
					for (int optArgNum = 0; optArgNum < option->NumFlagArgs; optArgNum++)
						arg = arg->Next();
				}
			}
		}

		if (candidate)
			commandLineParams.Append(new tStringItem(*candidate));
	}

	// Process all parameters.
	int paramNumber = 1;
	for (tStringItem* arg = commandLineParams.First(); arg; arg = arg->Next(), paramNumber++)
	{
		for (tParam* param = Params.First(); param; param = param->Next())
		{
			if (param->ParamNumber == paramNumber)
				param->Param = *arg;
		}
	}
}


void tCommand::tPrintUsage()
{
	tString usage =
R"U5AG3(
Arguments are separated by spaces. An argument must be enclosed in quotes
(single or double) if it has a space in it. Use escape sequences to put either
type of quote inside. If you need to specify paths, I suggest using forward
slashes, although backslashes will work so long as the filename does not have
a single or double quote next.

An argument may be an 'option' or 'parameter'.
An option is a combination of a 'flag' specified using a single or double
hyphen, and zero or more option arguments. A parameter is a single string.

Example:
mycopy.exe -R --overwrite fileA.txt -pat fileB.txt --log log.txt

The fileA.txt and fileB.txt in the above example are parameters (assuming
overwrite does not take any option arguments). The order in which parameters
are specified is important. fileA.txt is the first parameter, and fileB.txt is
the second. Options on the other hand can be specified in any order. All
options take a specific number (zero or more) of option arguments. If an
option takes zero arguments you can only test for its presence (or lack of).

The '--log log.txt' is an option with a single option argument, log.txt.
Single character flags specified with a single hyphen may be combined. The
-pat in the example expands to -p -a -t. It is suggested not to combine flags
when options take arguments as only the last flag would get them.

Variable argument counts are not supported but you may list the same option
more than once. Eg. -i filea.txt -i fileb.txt etc is valid.


)U5AG3";

	tString exeName = "Program.exe";
	if (!tCommand::Program.IsEmpty())
		exeName = tSystem::tGetFileName(tCommand::Program);

	tPrintf("USAGE: %s [options] ", exeName.Pod());

	// Support 256 parameters.
	bool printedParamNum[256];
	tStd::tMemset(printedParamNum, 0, sizeof(printedParamNum));
	for (tParam* param = Params.First(); param; param = param->Next())
	{
		if ((param->ParamNumber < 256) && !printedParamNum[param->ParamNumber])
		{
			if (!param->Description.IsEmpty())
				tPrintf("%s ", param->Description.Pod());
			else
				tPrintf("param%d ", param->ParamNumber);
			printedParamNum[param->ParamNumber] = true;
		}
	}
	tPrintf("\n\n%s", usage.Pod());

	tPrintf("Options:\n");
	for (tOption* option = Options.First(); option; option = option->Next())
	{
		if (!option->LongName.IsEmpty())
		{
			tPrintf("--%s ", option->LongName.Pod());
			for (int a = 0; a < option->NumFlagArgs; a++)
				tPrintf("arg%c ", '1'+a);
			tPrintf(": %s\n", option->Description.Pod());
		}

		if (!option->ShortName.IsEmpty())
		{
			tPrintf("-%s ", option->ShortName.Pod());
			for (int a = 0; a < option->NumFlagArgs; a++)
				tPrintf("arg%c ", '1'+a);
			tPrintf(": %s\n", option->Description.Pod());
		}
	}

	tPrintf("\n\n");
}
