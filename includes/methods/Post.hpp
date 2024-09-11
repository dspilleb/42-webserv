#ifndef POST_HPP
# define POST_HPP

# include "../macro.hpp"

struct file {
	std::string 	contentDisposition;
	std::string 	filename;
	std::string 	contentType;
	std::ofstream 	output_file;
};

class Post {

	private:
		const int 		_client_fd;
		const Request 	&_request;
		const Server 	&_server;
		ssize_t			_body_size;

		std::string		_remaining_content;
		std::string		_boundary;
		std::string		_header;
		size_t			&_status_code;
		ClientState 		_state;
		file			_file;

		void 		output_Content_Body(std::string &new_content);
		std::string get_data_in_header(const std::string &header, const std::string &first_delimiter, const std::string &end_delimiter);
		void		throw_and_set_status(const size_t status_code, std::string message);


		//file data handling
		void	get_file_infos( void );
		void	clear_file_infos ( void );

	public:

		ClientState	get_state( void ) {return (_state);};
		void		decide_action ( std::string &new_content);
		Post(const int client_fd, Request &request, Server &server);
		~Post();
};

std::ostream &			operator<<( std::ostream & o, Post const & i );

#endif /* ************************************************************ POST_H */