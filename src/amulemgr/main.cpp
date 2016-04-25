#include "Client.h"
#include "Server.h"
#include <iostream>
#include <string>
using namespace std;

/*
 * Usage: rpc_client keyWord waitSeconds
 * Bug: 客户端和服务端的调用参数反了
*/
int main(int argc, char** argv) {
	if (argc != 3) {
		cout << "invalid parameters" << endl;
		return -1;
	}
	AW::uint32 waitSeconds;
	AW::string keyword = argv[1];
	stringstream ss;
	ss << argv[2];
	ss >> waitSeconds;

	AW::clientStart("127.0.0.1", [&](shared_ptr<AW::SocketType> socket) -> void {
		AW::Client<AW::string, AW::string> echo(socket, t("echo"));
		AW::Client<std::vector<AW::string>, AW::uint32, AW::string> searchByKeyword(socket, t("searchByKeyword"));


		//cout << echo("hello");
		auto results = searchByKeyword(waitSeconds, keyword);
		for (int i = 0; i < results.size(); ++i) {
			cout << results[i] << endl;
		}
	});
}
