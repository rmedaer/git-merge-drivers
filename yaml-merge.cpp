#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <fstream>
#include <cassert>
#include <yaml.h>

using namespace std;

void
replace_substring(
	string str,
	const string& pattern,
	const string& replace)
{
	assert(not pattern.empty());

	for (
		size_t pos = str.find(pattern);
		pos != string::npos;
		str.replace(pos, pattern.size(), replace),
		pos = str.find(pattern, pos + replace.size())
	);
}

string
escape_string_path(
	string str)
{
	replace_substring(str, "~", "~0");
	replace_substring(str, "/", "~1");
	return str;
}

bool
compare_nodes(
	const YAML::Node& node1,
	const YAML::Node& node2)
{
	if (node1.is(node2))
		return true;

	if (node1.Type() != node2.Type())
		return false;

	switch(node1.Type())
	{
		case YAML::NodeType::Map:
		case YAML::NodeType::Sequence:
		case YAML::NodeType::Undefined:
			return false;
		case YAML::NodeType::Null:
			return true;
		case YAML::NodeType::Scalar:
			return node1.as<string>().compare(node2.as<string>()) == 0;
	}
}

YAML::Node
diff(
	const YAML::Node& source,
	const YAML::Node& target,
	const string& path = "")
{
	YAML::Node result = YAML::Load("[]");

	if (compare_nodes(source, target))
	{
		return result;
	}
	else if (source.Type() == YAML::NodeType::Map && target.Type() == YAML::NodeType::Map)
	{
		// First, look at existing key in target object.
		//   - if key found, recursively call diff function.
		//   - if key NOT found, add remove instruction in result array.
		for (YAML::const_iterator it = source.begin(); it != source.end(); it++)
		{
			YAML::Node value = it->second;
			string key = it->first.as<string>();
			string esc_key = escape_string_path(key);

			// Key not found in target .. remove it
			if (!target[key])
			{
				YAML::Node item;
				item["op"] = "remove";
				item["path"] = path + "/" + esc_key;
				result.push_back(item);
			}
			else
			{
				YAML::Node rec_result = diff(value, target[key], path + "/" + esc_key);
				for (YAML::const_iterator rec_it = rec_result.begin(); rec_it != rec_result.end(); rec_it++)
				{
					result.push_back(*rec_it);
				}
			}
		}

		// Walk target object to add new keys in result array
		for (YAML::const_iterator it = target.begin(); it != target.end(); it++)
		{
			YAML::Node value = it->second;
			string key = it->first.as<string>();
			string esc_key = escape_string_path(key);

			if (!source[key])
			{
				YAML::Node item;
				item["op"] = "add";
				item["path"] = path + "/" + esc_key;
				item["value"] = value;
				result.push_back(item);
			}
		}
	}
	else if (source.Type() == YAML::NodeType::Sequence && target.Type() == YAML::NodeType::Sequence)
	{
		size_t i = 0;
		while (i < source.size() && i < target.size())
		{
			YAML::Node rec_result = diff(source[i], target[i], path + "/" + to_string(i));
			for (YAML::const_iterator rec_it = rec_result.begin(); rec_it != rec_result.end(); rec_it++)
			{
				result.push_back(*rec_it);
			}
			i++;
		}

		while (i < target.size())
		{
			YAML::Node item;
			item["op"] = "add";
			item["path"] = path + "/" + to_string(i);
			item["value"] = target[i];
			result.push_back(item);
			i++;
		}
	}
	else
	{
		YAML::Node item;
		item["op"] = "replace";
		item["path"] = path;
		item["value"] = target;
		result.push_back(item);
	}

	return result;
}

bool
is_number(
	const string& str)
{
	return find_if(
	    str.begin(),
	    str.end(),
	    [](char c) { return !isdigit(c); }) == str.end();
}

YAML::Node
patch(
	YAML::Node& origin,
	const YAML::Node& patches)
{
	assert(patches.IsSequence());

	for (size_t i = 0; i < patches.size(); i++)
	{
		YAML::Node patch = patches[i];
		assert(patch["op"]);
		assert(patch["path"]);

		string op = patch["op"].as<string>();
		string path = patch["path"].as<string>();

		vector<YAML::Node> tree = { origin };

		string part;
		stringstream ss;

		// Create string stream from patch path
		ss.str(path);

		// Walk through each part of the path
		while (getline(ss, part, '/'))
		{
			if (part.empty())
				continue;

			YAML::Node next;
			if (is_number(part))
				next = tree.back()[stoi(part)];
			else
				next = tree.back()[part];

			tree.push_back(next);
		}

		if (op.compare("add") == 0 || op.compare("replace") == 0)
		{
			tree.back() = patch["value"];
		}
		else if (op.compare("remove") == 0)
		{
			tree.at(tree.size() - 2).remove(part);
		}
		else if (op.compare("move") == 0)
		{
			throw "Not implemented";
		}
		else if (op.compare("copy") == 0)
		{
			throw "Not implemented";
		}
		else if (op.compare("test") == 0)
		{
			throw "Not implemented";
		}
	}

	return origin;
}

void
merge(
	istream& our_stream,
	istream& older_stream,
	istream& their_stream,
	ostream& dst_stream)
{
	auto our = YAML::Load(our_stream);
	auto older = YAML::Load(older_stream);
	auto their = YAML::Load(their_stream);

	dst_stream << patch(our, diff(older, their));
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

