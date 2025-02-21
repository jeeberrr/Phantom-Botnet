#include <iostream>
#include "Headers/ClientUtils.h"

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
	std::cerr << ips << '\n' << "You have " << ipcount << " ips available." << '\n';

	std::cerr << "Enter your command (type help for command list and args)\n(FORMAT: Command argname arg argname arg argname arg): ";
	std::string command{};
	std::cin >> command;

	sock.Start();
	Command::Run(command, ips, sock);


}
