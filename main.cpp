
#include <boost/asio.hpp>
#include <boost/asio/spawn.hpp>

#include <iostream>

// g++ -std=c++14 main.cpp -omain -lboost_system -lboost_context -lboost_coroutine -pthread

int main() {
	using error_code = boost::system::error_code;
	using context    = boost::asio::yield_context;
	using io_service = boost::asio::io_service;
	using strand     = boost::asio::io_service::strand;
	using endpoint   = boost::asio::ip::tcp::endpoint;
	using socket     = boost::asio::ip::tcp::socket;
	using acceptor   = boost::asio::ip::tcp::acceptor;

	io_service ios;
	boost::asio::spawn(
		 ios
		,[&ios](context y) {
			acceptor acc(ios, endpoint(boost::asio::ip::tcp::v4(), 55555));
			while ( 1 ) {
				error_code ec;
				socket sock(ios);
				strand str(ios);
				acc.async_accept(sock, y[ec]);

				if ( ec ) continue;

				std::cout << "new connection from "
					<< sock.remote_endpoint().address().to_string()
				<< std::endl;

				boost::asio::spawn(
					 str
					,[so=std::move(sock)/*, st=std::move(str)*/](context y) mutable {
						while ( 1 ) try {
							char buf[1024];
							auto n = so.async_read_some(boost::asio::buffer(buf), y);
							boost::asio::async_write(so, boost::asio::buffer(buf, n), y);
						} catch(const std::exception &ex) {
							std::cerr << "[exception]: " << ex.what() << std::endl;
							break;
						}
					}
				);
			}
		}
	);

	ios.run();
}
