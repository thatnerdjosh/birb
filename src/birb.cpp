#include "Birb.hpp"
#include <filesystem>
#include <fstream>
#include <vector>
#include <iostream>

void pkg_source::print()
{
	std::cout 	<< "Name: \t" 	<< name << "\n"
				<< "URL: \t" 	<< url 	<< "\n"
				<< "Path: \t" 	<< path << "\n";
}

bool pkg_source::is_valid()
{
	return (!name.empty() || !url.empty() || !path.empty());
}

std::vector<pkg_source> birb::get_pkg_sources()
{
	/* Read the birb-sources.conf file line by line */
	std::vector<std::string> repository_lines = birb::read_file(PKG_SOURCE_CONFIG_PATH);

	std::vector<pkg_source> sources;
	std::vector<std::string> line(3);
	for (size_t i = 0; i < repository_lines.size(); ++i)
	{
		line = birb::split_string(repository_lines[i], ";");

		pkg_source s;
		s.name = line[0];
		s.url  = line[1];
		s.path = line[2];

		sources.push_back(s);
	}

	return sources;
}

std::vector<std::string> birb::get_pkg_source_list()
{
	return birb::read_file(PKG_SOURCE_CONFIG_PATH);
}

pkg_source birb::locate_pkg_repo(const std::string& pkg_name, const std::vector<pkg_source>& package_sources)
{
	/* Loop through all of the repositories and try to find
	 * the seed.sh file for the given package */
	std::string seed_path;
	for (pkg_source s : package_sources)
	{
		seed_path = s.path + "/" + pkg_name + "/seed.sh";
		if (std::filesystem::exists(seed_path) && std::filesystem::is_regular_file(seed_path))
			return s;
	}

	return pkg_source();
}

std::vector<std::string> birb::split_string(std::string text, std::string delimiter)
{
	std::vector<std::string> result;

	/* Split the string */
	size_t pos = 0;
	while ((pos = text.find(delimiter)) != std::string::npos)
	{
		result.push_back(text.substr(0, pos));
		text.erase(0, pos + delimiter.length());
	}

	if (!text.empty())
		result.push_back(text);

	return result;
}

std::vector<std::string> birb::read_file(std::string file_path)
{
	std::ifstream file(file_path);

	if (!file.is_open())
	{
		std::cout << "File [" << file_path << " can't be opened!\n";
		exit(2);
	}

	std::string line;
	std::vector<std::string> lines;

	/* Read the file */
	while (std::getline(file, line))
	{
		/* Ignore empty lines and lines starting with '#' */
		if (line.size() == 0 || line.at(0) == '#')
			continue;

		lines.push_back(line);
	}

	file.close();

	return lines;
}

std::string birb::read_pkg_variable(std::string pkg_name, std::string var_name, std::string repo_path)
{
   	/* Check if the result is already in the cache*/
	std::string key = pkg_name + var_name;
	if (!var_cache[key].empty())
		return var_cache[key];

	std::string pkg_path = repo_path + "/" + pkg_name + "/seed.sh";

	/* Read data from the package file */
	std::string var_line;
	std::ifstream pkg_file(pkg_path);

	if (!pkg_file.is_open())
	{
		//std::cout << "File [" << pkg_path << "] can't be opened!\n";
		return "";
	}

	while (std::getline(pkg_file, var_line))
	{
		/* Check if we have located the dependency line */
		if (var_line.substr(0, var_name.size() + 2) == var_name + "=\"")
		{
			/* We are done with the file, stop reading it */
			pkg_file.close();

			/* Break the file reading loop */
			break;
		}
	}

	/* Clean up the string */
	var_line.erase(0, var_name.size() + 2);
	var_line.erase(var_line.size() - 1, 1);

	/* Cache the result */
	var_cache[key] = var_line;

	return var_line;
}

