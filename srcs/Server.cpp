#include "../includes/Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Server::Server(const std::string config_text) :
	name("default_server"), 
	port(80), 
	body(100) {
	std::map<std::string, std::string> map;

	//! Temporary
	Config config(config_text);

	//! Need to use and parse nicely in Request and Response
	name = config.getServerName();
	port = config.getPort();
	body = config.getBody();
	_locations = config.getLocations();
	//std::cout << _locations << std::endl; //* DEBUG
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Server::~Server() {
	if (_fd_socket)
		close(_fd_socket);
	while(!_locations.empty() && _locations.size() > 0)
	{
		delete _locations.back();
		_locations.pop_back();
	}
}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

std::ostream &			operator<<( std::ostream & o, Server const & i ) {
	o << M << "\n----Server----\n";
	o << "|Name: " << i.getName() << "\n";
	o << "|Port: " << i.getPort() << "\n";
	o << "|Body: " << i.getBody() << "\n";
	o << "-----End-----\n" << C;
	return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void Server::mySocket(void) {
	// memset(_client_socket, 0, MAX_CLIENT * sizeof(int));
	for (int i = 0; i < MAX_CLIENT; i++) {
        _client_socket[i] = 0;
    }

	if ((_fd_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		throw std::runtime_error("Socked failed");

	this->sock_address.sin_family = AF_INET;
    this->sock_address.sin_port = htons(this->port);
    this->sock_address.sin_addr.s_addr = INADDR_ANY;
}

void Server::myBind(void) {
	int opt = 1;
	if (setsockopt(_fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
		throw std::runtime_error("setsockopt failed");
	}
	if (bind(_fd_socket, (sockaddr *) &this->sock_address, sizeof(this->sock_address)) < 0)
		throw std::runtime_error("Bind failed");
}

void Server::myListen(void) {
	if (listen(_fd_socket, MAX_CLIENT) < 0)
		throw std::runtime_error("Listen failed");

	std::ostringstream ss;
	ss << "\n*** Listening on ADDRESS: " 
		<< inet_ntoa(sock_address.sin_addr) 
		<< " PORT: " << ntohs(sock_address.sin_port) 
		<< " ***\n\n";
	std::cout << ss.str() << std::endl; //* DEBUG
}

void Server::process(void) {
	FD_ZERO(&_readfds);
    FD_ZERO(&_writefds);

    FD_SET(_fd_socket, &_readfds);
    FD_SET(_fd_socket, &_writefds);
	_max_sd = _fd_socket;

	for (int i = 0; i < MAX_CLIENT; i++) {
		_sd = _client_socket[i];

		// Si le socket est valide, on l’ajoute au set
		if (_sd > 0) {
			FD_SET(_sd, &_readfds);
			FD_SET(_sd, &_writefds);
		}
		if (_sd > _max_sd)
			_max_sd = _sd;

		// On identifie le plus grand descripteur pour `select()`
	}

	if ((select(_max_sd + 1, &_readfds, &_writefds, NULL, NULL) < 0) && (errno != EINTR)) {
		std::cerr << "Erreur avec select(), errno: " << strerror(errno) << std::endl;
	}

	if (FD_ISSET(_fd_socket, &_readfds)) {
		int addrlen = sizeof(this->sock_address);
		if ((_new_socket = accept(_fd_socket, (sockaddr *) &this->sock_address, (socklen_t *)&addrlen)) < 0)
			throw std::runtime_error("Accept failed");
		for (int i = 0; i < MAX_CLIENT; i++) {
			if (_client_socket[i] == 0) {
				_client_socket[i] = _new_socket;
				break;
			}
		}
	}

	for (int i = 0; i < MAX_CLIENT; i++) {
        _sd = _client_socket[i];
		if (FD_ISSET(_sd, &_readfds)) {
			_request = new Request(_client_socket[i], *this, _sd);
			std::cout << *_request << std::endl;
			if (!_request->getSucced()) {
				close(_sd);
				_client_socket[i] = 0;
			}
			else if (FD_ISSET(_sd, &_writefds)) {
				_response = new Response(_client_socket[i], *_request, *this, _sd);
				close(_sd);
				_client_socket[i] = 0;
			}
		}
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

const std::string &Server::getName(void) const { return (this->name); }
const unsigned int &Server::getPort(void) const { return (this->port); } 
const unsigned int &Server::getBody(void) const { return (this->body); }
const std::vector<Location *> &Server::getLocations(void) const { return (this->_locations); }

/* ************************************************************************** */