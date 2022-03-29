#include <fstream>
#include <iostream>

int main()
{
  std::ifstream in("data/someData.txt");

  if (!in)
  {
    std::cerr << "Can not open file!";
    return EXIT_FAILURE;
  }

  std::string message;

  if (!(in >> message))
  {
    std::cerr << "Can not read file content!";
    return EXIT_FAILURE;
  }

  std::cout << message << std::endl;

  return EXIT_SUCCESS;
}