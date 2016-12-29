/*
    This file is part of git-merge-drivers.

    git-merge-drivers is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    git-merge-drivers is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this git-merge-drivers.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <fstream>
#include <json.hpp>

using json = nlohmann::json;
using namespace std;

void
merge(
	istream& our_stream,
	istream& older_stream,
	istream& their_stream,
	ostream& dst_stream)
{
	auto our = json::parse(our_stream);
	auto older = json::parse(older_stream);
	auto their = json::parse(their_stream);

	dst_stream << our.patch(json::diff(older, their)).dump(4);
}

void
usage(char *program)
{
	cerr << "Usage: " << program << " <current> <older> <other>" << endl;
}

int main(
	int argc,
	char *argv[])
{
	// Validate number of arguments or show usage.
	if (argc < 4)
	{
		cerr << "Too few arguments" << endl;
		usage(argv[0]);
		exit(1); 
	}

	// 3 input streams, 1 buffer and 1 output stream
	ifstream our_stream;
	ifstream older_stream;
	ifstream their_stream;
	ofstream output_stream;
	ostringstream buffer_stream;

	buffer_stream = ostringstream();

	our_stream.open(argv[1]);
	if (!our_stream.is_open())
	{
		cerr << "Cannot open current file " << argv[1] << endl;
		exit(1);
	}

	older_stream.open(argv[2], fstream::in);
	if (!older_stream.is_open())
	{
		cerr << "Cannot open older file " << argv[2] << endl;
		exit(1);
	}

	their_stream.open(argv[3], fstream::in);
	if (!their_stream.is_open())
	{
		cerr << "Cannot open other file " << argv[3] << endl;
		exit(1);
	}
	
	// Parse and merge JSON streams
	merge(our_stream, older_stream, their_stream, buffer_stream);

	// Close input streams
	our_stream.close();
	older_stream.close();
	their_stream.close();

	// (Re)open our stream in output and dump buffer
	output_stream.open(argv[1], ofstream::out);
	if (!output_stream.is_open())
	{
		cerr << "Cannot open output file " << argv[1] << endl;
		exit(1);
	} 
	output_stream << buffer_stream.str();

	// Close output stream
	output_stream.close();

	return 0;
}

