#include <iostream>
#include <fstream>
#include <json.hpp>
#include <getopt.h>

using json = nlohmann::json;
using namespace std;

enum MERGE_STRATEGY {
	THEIRS,
	OURS
};

void
merge_json_objects(
	MERGE_STRATEGY strategy,
	json *init_json,
	json *merge_json)
{
	// If initial type is different then merge
	// apply strategy
	if (init_json->type() != merge_json->type()
	    || init_json->type() == json::value_t::null
	    || init_json->type() == json::value_t::boolean
	    || init_json->type() == json::value_t::number_integer
	    || init_json->type() == json::value_t::number_float
	    || init_json->type() == json::value_t::string)
	{
		if (strategy == MERGE_STRATEGY::THEIRS)
		{
			*init_json = json(*merge_json);
		}
		return;
	}

	if (init_json->type() == json::value_t::object)
	{
		for (json::iterator it = merge_json->begin(); it != merge_json->end(); ++it)
		{
			auto item = init_json->find(it.key());

			// If corresponding key found in initial object, merge it !
			// Otherwise insert new key
			if (item != init_json->end())
			{
				merge_json_objects(strategy, &item.value(), &it.value());
			}
			else
			{
				(*init_json)[it.key()] = it.value();
			}
		}
	}
	else if (init_json->type() == json::value_t::array)
	{
		for (json::iterator it = merge_json->begin(); it != merge_json->end(); ++it)
		{
			init_json->push_back(it.value());
		}
	}
}

void
merge_json_streams(
	MERGE_STRATEGY strategy,
	istream& init_stream,
	istream& merge_stream,
	ostream& dst_stream)
{
	auto init_json = json::parse(init_stream);
	auto merge_json = json::parse(merge_stream);

	merge_json_objects(strategy, &init_json, &merge_json);

	dst_stream << init_json.dump(4);
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

	merge_json_streams(strategy, ancestor_stream, merge_stream, buffer_stream);

	output_stream.open(argv[optind], ofstream::out);
	assert(output_stream.is_open());
	output_stream << buffer_stream.str();

	ancestor_stream.close();
	merge_stream.close();
	output_stream.close();

	return 0;
}

