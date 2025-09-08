#include <iostream>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]) {
	if (argc != 2) {
		cout << "Wrong count of arguments\n";
		return 1;
	}
	ifstream file(argv[1]);
	if (file.is_open()) {
		string text;
		while (file >> text) {
			cout << text << "\n";
		}
	}
	else {
		cout << "Could not open the file \'" << argv[1] << "\'";
		return 2;
	}
	file.close();
	return 0;
}