#include <string>
#include <vector>
#include <iostream>
#include <fstream>

class fileMang {

private:

	typedef struct file {
		std::string content;
		std::string name;
		int sl;
		int fl;
		bool filled = false;
		file* next;
	}*filePtr;

	typedef struct dataNode {
		std::string fileName;
		int fileSize = -1;
		file* pointToFile;
	};

	filePtr Filehead;
	filePtr Filecurr;

public:

	int count = 1;
	int startingMemLocation = 0;

	fileMang();

	std::vector<dataNode*> dataVector;
	std::vector<int> deleteInfoVector;
	std::vector<std::string> addedClients;
	void fcreateFile(std::string name);
	std::string fopenFile(std::string name, std::string mode, std::string text, std::string tId);
	std::string fcloseFile(std::string name, std::string tId);
	void freadFrom(std::string fileName, int form, int size);
	void fdeleteFile(std::string fileName);
	std::string ffileAlreadyOpen(std::string tId,std::string name);
	void fmoveFile(std::string sName, std::string dName);
	void fprintDeleteVector();
	void fprintMap();
	int fifFileExists(std::string fileName);

	std::string fprintToCheck();


	//void fwriteatt(std::string sName, int location);


};
