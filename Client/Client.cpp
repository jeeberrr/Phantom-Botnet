#include <iostream>
#include "ClientUtils.h"

int count(std::string string) {

	int count = 1;
	for (char c : string) {
		if (c == '\n') {
			count++;
		}
	}

	return count;

}

int main() {

	std::string url{};
	Socket sock{ Socket() };

start:

	std::cout << "Welcome, enter your IP web api link: ";
	std::cin >> url;

	std::string ips{ ApiConnect(url) };
	int ipcount{ count(ips) };
	std::cerr << ips << '\n' << "You have " << ipcount++ << " ips available.";

	entercomm:

		std::cerr << "\nEnter your command (type help for command list and args, or exit to exit the program)\n(FORMAT: Command {argname: arg argname: arg argname: arg} ): ";
		std::string command{};
		std::cin >> command;
		std::transform(command.begin(), command.end(), command.begin(), [](unsigned char c) {return std::tolower(c);});

		if (command == "help") {
			std::cout << '\n' << "Commands:\nping (args ip and time (in minutes) )\nweb (args link (or ip) time (in minutes) and https (1 or 0 for true and false) )"; goto entercomm;
		}
		else if (command == "exit") {
			exit(0);
		}
		else {
			sock.Start();
			try {
				sock.Send(ips, command);
			}
			catch (...) {
				std::cerr << "\nError, did you enter the command correctly?";
			}
		}

		return 1;

}
