#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::vector<int>> readDimacs(std::string filepath) {

	std::ifstream dimacsFile(filepath);

	std::string text;

	while (std::getline(dimacsFile, text)) {
		std::istringstream iss(text);
		std::vector<int> clause{ std::istream_iterator<int>{iss}, std::istream_iterator<int>{} };
	}
}
