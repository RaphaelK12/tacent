// tTimer.cpp
//
// Simple timer class. Like a stopwatch. Supports keeping track of time in a number of different units. Accuracy is all
// up to you - you call the update function. This code does not access low-level timer hardware.
//
// Copyright (c) 2005, 2017, 2019, 2020 Tristan Grimmer.
// Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby
// granted, provided that the above copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
// INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
// AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
// PERFORMANCE OF THIS SOFTWARE.

#ifdef PLATFORM_WIN
#include <Windows.h>
#endif
#include "System/tTime.h"
#include "System/tPrint.h"
#include "System/tFile.h"


#ifdef PLATFORM_WIN


namespace tSystem
{
	static int64 QPCStartCount = tSystem::tGetHardwareTimerCount();
	static int64 QPCFrequency = tSystem::tGetHardwareTimerFrequency();
}


uint64 tSystem::tGetTimeLocal()
{
	SystemTime sysTime;
	GetLocalTime(&sysTime);

	FileTime fileTime;
	tStd::tMemset(&fileTime, 0, sizeof(fileTime));
	SystemTimeToFileTime(&sysTime, &fileTime);

	uint64 retVal;
	retVal = fileTime.dwHighDateTime;
	retVal <<= 32;
	retVal |= fileTime.dwLowDateTime;
	return retVal;
}


uint64 tSystem::tGetTimeUTC()
{
	FileTime utcTime;
	GetSystemTimeAsFileTime(&utcTime);

	uint64 retVal;
	retVal = utcTime.dwHighDateTime;
	retVal <<= 32;
	retVal |= utcTime.dwLowDateTime;

	return retVal;
}


uint64 tSystem::tGetTimeGMT()
{
	return tGetTimeUTC();
}


tString tSystem::tConvertTimeToString(uint64 time, tTimeFormat format)
{
	FileTime fileTime;
	fileTime.dwHighDateTime = (time >> 32);
	fileTime.dwLowDateTime = (time & 0x00000000FFFFFFFF);
	SystemTime st;
	FileTimeToSystemTime(&fileTime, &st);

	tString timeStr;
	switch (format)
	{
		case tTimeFormat::Standard:
			tsPrintf
			(
				timeStr,
				"%04d-%02d-%02d %02d:%02d:%02d",
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond
			);
			break;

		case tTimeFormat::Extended:
		{
			tString day;
			switch (st.wDayOfWeek)
			{
				case 0: day = "Sunday";			break;
				case 1: day = "Monday";			break;
				case 2: day = "Tuesday";		break;
				case 3: day = "Wednesday";		break;
				case 4: day = "Thursday";		break;
				case 5: day = "Friday";			break;
				case 6: day = "Saturday";		break;
			}

			tString month;
			switch (st.wMonth)
			{
				case 1:  month = "January";		break;
				case 2:  month = "February";	break;
				case 3:  month = "March";		break;
				case 4:  month = "April";		break;
				case 5:  month = "May";			break;
				case 6:  month = "June";		break;
				case 7:  month = "July";		break;
				case 8:  month = "August";		break;
				case 9:  month = "September";	break;
				case 10: month = "October";		break;
				case 11: month = "November";	break;
				case 12: month = "December";	break;
			}

			tsPrintf
			(
				timeStr,
				"%s %s %d %d - %02d:%02d:%02d",
				day.ConstText(), month.ConstText(), st.wDay, st.wYear,
				st.wHour, st.wMinute, st.wSecond
			);

			break;
		}

		case tTimeFormat::Short:
		{
			tString day("NaD");
			switch (st.wDayOfWeek)
			{
				case 0: day = "Sun"; break;
				case 1: day = "Mon"; break;
				case 2: day = "Tue"; break;
				case 3: day = "Wed"; break;
				case 4: day = "Thu"; break;
				case 5: day = "Fri"; break;
				case 6: day = "Sat"; break;
			}

			tString month("NaM");
			switch (st.wMonth)
			{
				case 1:  month = "Jan"; break;
				case 2:  month = "Feb";	break;
				case 3:  month = "Mar"; break;
				case 4:  month = "Apr"; break;
				case 5:  month = "May"; break;
				case 6:  month = "Jun"; break;
				case 7:  month = "Jul"; break;
				case 8:  month = "Aug"; break;
				case 9:  month = "Sep"; break;
				case 10: month = "Oct"; break;
				case 11: month = "Nov"; break;
				case 12: month = "Dec"; break;
			}

			tsPrintf
			(
				timeStr,
				"%s %s %2d %4d %2d:%02d:%02d",
				day.ConstText(), month.ConstText(), st.wDay, st.wYear,
				st.wHour, st.wMinute, st.wSecond
			);

			break;
		}
	}

	return timeStr;
}
#endif // PLATFORM_WIN


int64 tSystem::tGetHardwareTimerFrequency()
{
	#ifdef PLATFORM_WIN
	int64 freq;
	QueryPerformanceFrequency((LargeInteger*)&freq);
	return freq;
	#endif
}


int64 tSystem::tGetHardwareTimerCount()
{
	#ifdef PLATFORM_WIN
	int64 count;
	QueryPerformanceCounter((LargeInteger*)&count);
	return count;
	#endif
}


void tSystem::tSleep(int milliSeconds)
{
	::Sleep(milliSeconds);
}


float tSystem::tGetTime()
{
	int64 currCount = tGetHardwareTimerCount();

	// This is organized like this to keep precision as high as possible.
	int64 elapsedCount = currCount - QPCStartCount;
	int64 wholeSeconds = elapsedCount / QPCFrequency;
	int64 remainder = elapsedCount % QPCFrequency;
	return float(wholeSeconds) + float(remainder)/float(QPCFrequency);
}


double tSystem::tGetTimeDouble()
{
	int64 currCount = tGetHardwareTimerCount();

	// This is organized like this to keep precision as high as possible.
	int64 elapsedCount = currCount - QPCStartCount;
	int64 wholeSeconds = elapsedCount / QPCFrequency;
	int64 remainder = elapsedCount % QPCFrequency;
	return double(wholeSeconds) + double(remainder)/double(QPCFrequency);
}


void tSystem::tTimer::Update(float timeElapsed, tUnit::tTime unit)
{
	if (!Running)
		return;

	if (unit == tUnit::tTime::Unspecified)
		unit = UnitInternal;

	Time += Convert(timeElapsed, unit, UnitInternal);
}


float tSystem::tTimer::GetTime(tUnit::tTime unit) const
{
	if (unit == tUnit::tTime::Unspecified)
		unit = UnitInternal;

	return float(Convert(Time, UnitInternal, unit));
}


void tSystem::tTimer::PrintHighPrecisionConversionTable(const char* outputFile)
{
	double inSeconds[tUnit::tTime::NumTimeUnits] =
	{
		5.39E-44,					// PlankTime
		6.97E-24,					// Chronon
		1.0E-18,					// Atosecond
		1.0E-15,					// Femtosecond
		1.0E-12,					// Picosecond
		1.0E-9,						// Nanosecond
		1.0E-6,						// Microsecond
		1.0E-3,						// Millisecond
		1.0/60.0,					// Tick
		1.0,						// Second
		0.3,						// She
		3.0 + 1.0/3.0,				// Helek
		60.0,						// Minute
		3600.0,						// Hour
		86400.0,					// Day
		604800.0,					// Week
		1209600.0,					// Fortnight
		31536000.0,					// Year
		31536000.0,					// Annum
		3153600000.0,				// Century
		31536000000.0,				// Millennium
		7884000000000000.0,			// GalacticYear
	};

	tFileHandle file = tSystem::tOpenFile(outputFile, "wt");

	tfPrintf(file, "//                     ");
	for (int n = 0; n < int(tUnit::tTime::NumTimeUnits); n++)
	{
		int count = tfPrintf(file, "%s", tUnit::GetUnitName(tUnit::tTime(n)));
		for (int space = 0; space < (25-count); space++)
			tfPrintf(file, " ");
	}
	tfPrintf(file, "\n");

	for (int from = 0; from < int(tUnit::tTime::NumTimeUnits); from++)
	{
		int count = tfPrintf(file, "/* %s */ ", tUnit::GetUnitName(tUnit::tTime(from)));
		for (int space = 0; space < (21-count); space++)
			tfPrintf(file, " ");

		tfPrintf(file, "{");
		for (int to = 0; to < int(tUnit::tTime::NumTimeUnits); to++)
		{
			// From -> to. t = f * t/f.
			tfPrintf(file, " %019.16E,", inSeconds[from]/inSeconds[to]);
		}
		tfPrintf(file, " },\n");
	}
	tSystem::tCloseFile(file);
};


// To keep error as low as possible while converting, we explicitly specify the conversions between all possible unit
// combinations. This is better than converting to a norm, and then to the destination.
const double tSystem::tTimer::UnitConversionTable[tUnit::tTime::NumTimeUnits][tUnit::tTime::NumTimeUnits] =
{
	//                     PlankTime                Chronon                  Attosecond               Femtosecond              Picosecond               Nanosecond               Microsecond              Millisecond              Tick                     Second                   She                      Helek                    Minute                   Hour                     Day                      Week                     Fortnight                Year                     Annum                    Century                  Millennium               GalacticYear             
	/* PlankTime */      { 1.0000000000000000E+000, 7.7331420373027263E-021, 5.3899999999999997E-026, 5.3899999999999996E-029, 5.3899999999999998E-032, 5.3899999999999992E-035, 5.3899999999999999E-038, 5.3899999999999998E-041, 3.2339999999999997E-042, 5.3899999999999995E-044, 1.7966666666666664E-043, 1.6169999999999997E-044, 8.9833333333333332E-046, 1.4972222222222221E-047, 6.2384259259259255E-049, 8.9120370370370367E-050, 4.4560185185185184E-050, 1.7091577879249111E-051, 1.7091577879249111E-051, 1.7091577879249112E-053, 1.7091577879249110E-054, 6.8366311516996447E-060, },
	/* Chronon */        { 1.2931354359925789E+020, 1.0000000000000000E+000, 6.9699999999999993E-006, 6.9699999999999989E-009, 6.9699999999999996E-012, 6.9699999999999993E-015, 6.9700000000000007E-018, 6.9700000000000003E-021, 4.1820000000000000E-022, 6.9699999999999997E-024, 2.3233333333333334E-023, 2.0909999999999998E-024, 1.1616666666666666E-025, 1.9361111111111109E-027, 8.0671296296296291E-029, 1.1524470899470898E-029, 5.7622354497354492E-030, 2.2101725012683914E-031, 2.2101725012683914E-031, 2.2101725012683916E-033, 2.2101725012683915E-034, 8.8406900050735662E-040, },
	/* Attosecond */     { 1.8552875695732842E+025, 1.4347202295552369E+005, 1.0000000000000000E+000, 1.0000000000000000E-003, 1.0000000000000002E-006, 1.0000000000000001E-009, 1.0000000000000002E-012, 1.0000000000000001E-015, 6.0000000000000001E-017, 1.0000000000000001E-018, 3.3333333333333337E-018, 2.9999999999999999E-019, 1.6666666666666668E-020, 2.7777777777777779E-022, 1.1574074074074075E-023, 1.6534391534391536E-024, 8.2671957671957680E-025, 3.1709791983764591E-026, 3.1709791983764591E-026, 3.1709791983764589E-028, 3.1709791983764589E-029, 1.2683916793505836E-034, },
	/* Femtosecond */    { 1.8552875695732842E+028, 1.4347202295552370E+008, 1.0000000000000000E+003, 1.0000000000000000E+000, 1.0000000000000000E-003, 9.9999999999999995E-007, 1.0000000000000001E-009, 9.9999999999999998E-013, 6.0000000000000009E-014, 1.0000000000000001E-015, 3.3333333333333336E-015, 2.9999999999999999E-016, 1.6666666666666667E-017, 2.7777777777777778E-019, 1.1574074074074075E-020, 1.6534391534391535E-021, 8.2671957671957675E-022, 3.1709791983764592E-023, 3.1709791983764592E-023, 3.1709791983764591E-025, 3.1709791983764591E-026, 1.2683916793505836E-031, },
	/* Picosecond */     { 1.8552875695732841E+031, 1.4347202295552368E+011, 9.9999999999999988E+005, 9.9999999999999989E+002, 1.0000000000000000E+000, 1.0000000000000000E-003, 9.9999999999999995E-007, 1.0000000000000001E-009, 6.0000000000000000E-011, 9.9999999999999998E-013, 3.3333333333333335E-012, 2.9999999999999998E-013, 1.6666666666666667E-014, 2.7777777777777775E-016, 1.1574074074074074E-017, 1.6534391534391534E-018, 8.2671957671957670E-019, 3.1709791983764586E-020, 3.1709791983764586E-020, 3.1709791983764587E-022, 3.1709791983764586E-023, 1.2683916793505833E-028, },
	/* Nanosecond */     { 1.8552875695732840E+034, 1.4347202295552369E+014, 1.0000000000000000E+009, 1.0000000000000000E+006, 1.0000000000000001E+003, 1.0000000000000000E+000, 1.0000000000000000E-003, 9.9999999999999995E-007, 6.0000000000000008E-008, 1.0000000000000001E-009, 3.3333333333333338E-009, 3.0000000000000000E-010, 1.6666666666666667E-011, 2.7777777777777779E-013, 1.1574074074074075E-014, 1.6534391534391536E-015, 8.2671957671957678E-016, 3.1709791983764588E-017, 3.1709791983764588E-017, 3.1709791983764587E-019, 3.1709791983764586E-020, 1.2683916793505836E-025, },
	/* Microsecond */    { 1.8552875695732838E+037, 1.4347202295552366E+017, 9.9999999999999988E+011, 9.9999999999999988E+008, 1.0000000000000000E+006, 9.9999999999999989E+002, 1.0000000000000000E+000, 1.0000000000000000E-003, 5.9999999999999995E-005, 9.9999999999999995E-007, 3.3333333333333333E-006, 2.9999999999999999E-007, 1.6666666666666667E-008, 2.7777777777777777E-010, 1.1574074074074074E-011, 1.6534391534391534E-012, 8.2671957671957671E-013, 3.1709791983764584E-014, 3.1709791983764584E-014, 3.1709791983764586E-016, 3.1709791983764588E-017, 1.2683916793505834E-022, },
	/* Millisecond */    { 1.8552875695732840E+040, 1.4347202295552369E+020, 1.0000000000000000E+015, 1.0000000000000000E+012, 1.0000000000000000E+009, 1.0000000000000000E+006, 1.0000000000000001E+003, 1.0000000000000000E+000, 6.0000000000000005E-002, 1.0000000000000000E-003, 3.3333333333333335E-003, 2.9999999999999997E-004, 1.6666666666666667E-005, 2.7777777777777776E-007, 1.1574074074074074E-008, 1.6534391534391535E-009, 8.2671957671957675E-010, 3.1709791983764586E-011, 3.1709791983764586E-011, 3.1709791983764587E-013, 3.1709791983764590E-014, 1.2683916793505834E-019, },
	/* Tick */           { 3.0921459492888067E+041, 2.3912003825920613E+021, 1.6666666666666666E+016, 1.6666666666666666E+013, 1.6666666666666666E+010, 1.6666666666666666E+007, 1.6666666666666668E+004, 1.6666666666666668E+001, 1.0000000000000000E+000, 1.6666666666666666E-002, 5.5555555555555559E-002, 5.0000000000000001E-003, 2.7777777777777778E-004, 4.6296296296296296E-006, 1.9290123456790122E-007, 2.7557319223985891E-008, 1.3778659611992945E-008, 5.2849653306274306E-010, 5.2849653306274306E-010, 5.2849653306274313E-012, 5.2849653306274313E-013, 2.1139861322509723E-018, },
	/* Second */         { 1.8552875695732839E+043, 1.4347202295552367E+023, 9.9999999999999987E+017, 9.9999999999999987E+014, 1.0000000000000000E+012, 9.9999999999999988E+008, 1.0000000000000000E+006, 1.0000000000000000E+003, 6.0000000000000000E+001, 1.0000000000000000E+000, 3.3333333333333335E+000, 2.9999999999999999E-001, 1.6666666666666666E-002, 2.7777777777777778E-004, 1.1574074074074073E-005, 1.6534391534391535E-006, 8.2671957671957675E-007, 3.1709791983764586E-008, 3.1709791983764586E-008, 3.1709791983764586E-010, 3.1709791983764586E-011, 1.2683916793505835E-016, },
	/* She */            { 5.5658627087198520E+042, 4.3041606886657104E+022, 2.9999999999999994E+017, 2.9999999999999994E+014, 3.0000000000000000E+011, 3.0000000000000000E+008, 3.0000000000000000E+005, 3.0000000000000000E+002, 1.8000000000000000E+001, 2.9999999999999999E-001, 1.0000000000000000E+000, 8.9999999999999997E-002, 5.0000000000000001E-003, 8.3333333333333331E-005, 3.4722222222222220E-006, 4.9603174603174601E-007, 2.4801587301587301E-007, 9.5129375951293758E-009, 9.5129375951293758E-009, 9.5129375951293758E-011, 9.5129375951293754E-012, 3.8051750380517500E-017, },
	/* Helek */          { 6.1842918985776139E+043, 4.7824007651841229E+023, 3.3333333333333330E+018, 3.3333333333333330E+015, 3.3333333333333335E+012, 3.3333333333333335E+009, 3.3333333333333335E+006, 3.3333333333333335E+003, 2.0000000000000000E+002, 3.3333333333333335E+000, 1.1111111111111112E+001, 1.0000000000000000E+000, 5.5555555555555559E-002, 9.2592592592592596E-004, 3.8580246913580246E-005, 5.5114638447971785E-006, 2.7557319223985893E-006, 1.0569930661254863E-007, 1.0569930661254863E-007, 1.0569930661254863E-009, 1.0569930661254863E-010, 4.2279722645019450E-016, },
	/* Minute */         { 1.1131725417439703E+045, 8.6083213773314209E+024, 5.9999999999999992E+019, 5.9999999999999992E+016, 6.0000000000000000E+013, 6.0000000000000000E+010, 6.0000000000000000E+007, 6.0000000000000000E+004, 3.6000000000000000E+003, 6.0000000000000000E+001, 2.0000000000000000E+002, 1.8000000000000000E+001, 1.0000000000000000E+000, 1.6666666666666666E-002, 6.9444444444444447E-004, 9.9206349206349206E-005, 4.9603174603174603E-005, 1.9025875190258751E-006, 1.9025875190258751E-006, 1.9025875190258752E-008, 1.9025875190258752E-009, 7.6103500761035007E-015, },
	/* Hour */           { 6.6790352504638226E+046, 5.1649928263988526E+026, 3.6000000000000000E+021, 3.5999999999999995E+018, 3.6000000000000000E+015, 3.6000000000000000E+012, 3.6000000000000000E+009, 3.6000000000000000E+006, 2.1600000000000000E+005, 3.6000000000000000E+003, 1.2000000000000000E+004, 1.0800000000000000E+003, 6.0000000000000000E+001, 1.0000000000000000E+000, 4.1666666666666664E-002, 5.9523809523809521E-003, 2.9761904761904760E-003, 1.1415525114155251E-004, 1.1415525114155251E-004, 1.1415525114155251E-006, 1.1415525114155251E-007, 4.5662100456621005E-013, },
	/* Day */            { 1.6029684601113173E+048, 1.2395982783357247E+028, 8.6400000000000000E+022, 8.6400000000000000E+019, 8.6400000000000000E+016, 8.6400000000000000E+013, 8.6400000000000000E+010, 8.6400000000000000E+007, 5.1840000000000000E+006, 8.6400000000000000E+004, 2.8800000000000000E+005, 2.5920000000000000E+004, 1.4400000000000000E+003, 2.4000000000000000E+001, 1.0000000000000000E+000, 1.4285714285714285E-001, 7.1428571428571425E-002, 2.7397260273972603E-003, 2.7397260273972603E-003, 2.7397260273972603E-005, 2.7397260273972604E-006, 1.0958904109589040E-011, },
	/* Week */           { 1.1220779220779221E+049, 8.6771879483500723E+028, 6.0479999999999997E+023, 6.0480000000000000E+020, 6.0480000000000000E+017, 6.0480000000000000E+014, 6.0480000000000000E+011, 6.0480000000000000E+008, 3.6288000000000000E+007, 6.0480000000000000E+005, 2.0160000000000000E+006, 1.8144000000000000E+005, 1.0080000000000000E+004, 1.6800000000000000E+002, 7.0000000000000000E+000, 1.0000000000000000E+000, 5.0000000000000000E-001, 1.9178082191780823E-002, 1.9178082191780823E-002, 1.9178082191780821E-004, 1.9178082191780822E-005, 7.6712328767123292E-011, },
	/* Fortnight */      { 2.2441558441558443E+049, 1.7354375896700145E+029, 1.2095999999999999E+024, 1.2096000000000000E+021, 1.2096000000000000E+018, 1.2096000000000000E+015, 1.2096000000000000E+012, 1.2096000000000000E+009, 7.2576000000000000E+007, 1.2096000000000000E+006, 4.0320000000000000E+006, 3.6288000000000000E+005, 2.0160000000000000E+004, 3.3600000000000000E+002, 1.4000000000000000E+001, 2.0000000000000000E+000, 1.0000000000000000E+000, 3.8356164383561646E-002, 3.8356164383561646E-002, 3.8356164383561642E-004, 3.8356164383561644E-005, 1.5342465753424658E-010, },
	/* Year */           { 5.8508348794063083E+050, 4.5245337159253948E+030, 3.1535999999999997E+025, 3.1535999999999996E+022, 3.1536000000000000E+019, 3.1536000000000000E+016, 3.1536000000000000E+013, 3.1536000000000000E+010, 1.8921600000000000E+009, 3.1536000000000000E+007, 1.0512000000000000E+008, 9.4608000000000000E+006, 5.2560000000000000E+005, 8.7600000000000000E+003, 3.6500000000000000E+002, 5.2142857142857146E+001, 2.6071428571428573E+001, 1.0000000000000000E+000, 1.0000000000000000E+000, 1.0000000000000000E-002, 1.0000000000000000E-003, 4.0000000000000002E-009, },
	/* Annum */          { 5.8508348794063083E+050, 4.5245337159253948E+030, 3.1535999999999997E+025, 3.1535999999999996E+022, 3.1536000000000000E+019, 3.1536000000000000E+016, 3.1536000000000000E+013, 3.1536000000000000E+010, 1.8921600000000000E+009, 3.1536000000000000E+007, 1.0512000000000000E+008, 9.4608000000000000E+006, 5.2560000000000000E+005, 8.7600000000000000E+003, 3.6500000000000000E+002, 5.2142857142857146E+001, 2.6071428571428573E+001, 1.0000000000000000E+000, 1.0000000000000000E+000, 1.0000000000000000E-002, 1.0000000000000000E-003, 4.0000000000000002E-009, },
	/* Century */        { 5.8508348794063081E+052, 4.5245337159253946E+032, 3.1536000000000000E+027, 3.1535999999999999E+024, 3.1536000000000000E+021, 3.1536000000000000E+018, 3.1536000000000000E+015, 3.1536000000000000E+012, 1.8921600000000000E+011, 3.1536000000000000E+009, 1.0512000000000000E+010, 9.4608000000000000E+008, 5.2560000000000000E+007, 8.7600000000000000E+005, 3.6500000000000000E+004, 5.2142857142857147E+003, 2.6071428571428573E+003, 1.0000000000000000E+002, 1.0000000000000000E+002, 1.0000000000000000E+000, 1.0000000000000001E-001, 3.9999999999999998E-007, },
	/* Millennium */     { 5.8508348794063081E+053, 4.5245337159253946E+033, 3.1535999999999999E+028, 3.1535999999999997E+025, 3.1536000000000000E+022, 3.1536000000000000E+019, 3.1536000000000000E+016, 3.1536000000000000E+013, 1.8921600000000000E+012, 3.1536000000000000E+010, 1.0512000000000000E+011, 9.4608000000000000E+009, 5.2560000000000000E+008, 8.7600000000000000E+006, 3.6500000000000000E+005, 5.2142857142857145E+004, 2.6071428571428572E+004, 1.0000000000000000E+003, 1.0000000000000000E+003, 1.0000000000000000E+001, 1.0000000000000000E+000, 3.9999999999999998E-006, },
	/* GalacticYear */   { 1.4627087198515771E+059, 1.1311334289813487E+039, 7.8839999999999993E+033, 7.8839999999999996E+030, 7.8839999999999997E+027, 7.8839999999999993E+024, 7.8840000000000000E+021, 7.8840000000000000E+018, 4.7304000000000000E+017, 7.8840000000000000E+015, 2.6280000000000000E+016, 2.3652000000000000E+015, 1.3140000000000000E+014, 2.1900000000000000E+012, 9.1250000000000000E+010, 1.3035714285714285E+010, 6.5178571428571424E+009, 2.5000000000000000E+008, 2.5000000000000000E+008, 2.5000000000000000E+006, 2.5000000000000000E+005, 1.0000000000000000E+000, }
};


double tSystem::tTimer::Convert(double time, tUnit::tTime unitFrom, tUnit::tTime unitTo)
{
	tAssert((unitFrom != tUnit::tTime::Unspecified) && (unitTo != tUnit::tTime::Unspecified));
	if (unitFrom == unitTo)
		return time;

	return (UnitConversionTable[int(unitFrom)][int(unitTo)])*time;
}
