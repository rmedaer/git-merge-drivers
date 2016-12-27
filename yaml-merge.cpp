#include <iostream>
#include <fstream>
#include <string.h>
#include <yaml.h>
#include <getopt.h>

using namespace std;

enum MERGE_STRATEGY {
	THEIRS,
	OURS
};

void
merge_yaml_nodes(
	MERGE_STRATEGY strategy,
	YAML::Node *current_yaml,
	YAML::Node *merge_yaml)
{
	if (current_yaml->Type() != merge_yaml->Type()
	    || current_yaml->Type() == YAML::NodeType::Undefined
	    || current_yaml->Type() == YAML::NodeType::Null
	    || current_yaml->Type() == YAML::NodeType::Scalar)
	{
		if (strategy == MERGE_STRATEGY::THEIRS)
		{
			*current_yaml = YAML::Node(*merge_yaml);
		}
		return;
	}

	if (current_yaml->Type() == YAML::NodeType::Map)
	{
		for (YAML::const_iterator it = merge_yaml->begin(); it != merge_yaml->end(); it++)
		{
			string key = it->first.as<string>();
			YAML::Node value = it->second;
			YAML::Node item = (*current_yaml)[key];

			if (item.Type() == YAML::NodeType::Undefined)
			{
				(*current_yaml)[key] = YAML::Node(value);
			}
			else
			{
				merge_yaml_nodes(strategy, &item, &value); 
			}
		}
	}
	else if (current_yaml->Type() == YAML::NodeType::Sequence)
	{
		for (YAML::const_iterator it = merge_yaml->begin(); it != merge_yaml->end(); it++)
		{
			current_yaml->push_back(YAML::Node(*it));
		}
	}
}

void
merge_yaml_streams(
	MERGE_STRATEGY strategy,
	istream& current_stream,
	istream& merge_stream,
	ostream& output_stream)
{
	auto current_yaml = YAML::Load(current_stream);
	auto merge_yaml = YAML::Load(merge_stream);

	merge_yaml_nodes(strategy, &current_yaml, &merge_yaml);

	YAML::Emitter out;
	out.SetIndent(4);
	out.SetOutputCharset(YAML::EscapeNonAscii);
	out << current_yaml;
	output_stream << out.c_str();
}

static struct option long_options[] =
{
	{ "strategy", required_argument, 0, 's' }
};

int main(
	int argc,
	char *argv[])
{
	MERGE_STRATEGY strategy = MERGE_STRATEGY::THEIRS;
	char c = 0;
	while (c != -1)
	{
		int option_index = 0;
		c = getopt_long(argc, argv, "s:", long_options, &option_index);

		switch (c)
		{
			case 's':
				if (strcasecmp(optarg, "theirs") == 0)
				{
					strategy = MERGE_STRATEGY::THEIRS;
				}
				else if (strcasecmp(optarg, "ours") == 0)
				{
					strategy = MERGE_STRATEGY::OURS;
				}
				else
				{
					cerr << "Unknown strategy " << optarg << endl;
					abort();
				}
				break;
			case -1:
				break;
			default:
				abort();
		}
	}
	
	assert(argc - optind >= 2);

	auto ancestor_stream = ifstream(argv[optind], fstream::in);
	auto merge_stream = ifstream(argv[optind + 1], fstream::in);
	auto buffer_stream = ostringstream();
	ofstream output_stream;

	assert(ancestor_stream);
	assert(merge_stream);

	merge_yaml_streams(strategy, ancestor_stream, merge_stream, buffer_stream);

	output_stream.open(argv[optind], ofstream::out);
	assert(output_stream.is_open());
	output_stream << buffer_stream.str();

	ancestor_stream.close();
	merge_stream.close();
	output_stream.close();

	return 0;
}

