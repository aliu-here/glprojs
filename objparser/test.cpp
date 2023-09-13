#include <vector>
#include <string>
#include <iostream>
#include <fstream>

	std::vector<std::string> split (const std::string &s, char delim) 
	{
    		std::vector<std::string> result;
    		std::stringstream ss (s);
    		std::string item;

    		while (getline (ss, item, delim)) {
        		result.push_back (item);
		}
		return result;
	}
	
int main()
{
    std::vector<std::string> test = split("asdklfqwerffqw/eqetqtcewcfrwev/qv//", '/');
    for (int i=0; i<test.size(); i++)
    {
        std::cout << test[i] << "," ;
    }
}
