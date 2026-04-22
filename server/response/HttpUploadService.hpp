#ifndef HTTPUPLOAD_SERVICE_HPP
#define HTTPUPLOAD_SERVICE_HPP

#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include "utils/helper.hpp"
#include "utils/Logger.hpp"

enum class body_state {
	parsing_boundary,
	parsing_headers,
	save_data,
	done
};

class HttpUploadService {
private:
	body_state _state;
	std::string _buffer;
	size_t _bytesRead = 0;
	const std::map<std::string, std::string> _headers;
	size_t _contentLength;
	std::string _rawBoundary;
	std::string _boundary;
	std::string _boundaryClosig;
	std::string _filePath;
	std::string _filename;
	std::ofstream _file;

	/* Member Functions */
	std::string extractBoundary(std::string value);
	std::string extractFilename(std::string& value);
	void writeToFile(std::string data);
	bool parsingBoundary(std::string& line);
	bool parsingHeaders(std::string& line);
	void openFile();
	body_state saveData(std::string& line);

public:
	/* Constructors & Destructors */
	HttpUploadService(void);
	HttpUploadService(const std::map<std::string, std::string> headers, std::string filePath, size_t clientMaxBody);
	HttpUploadService(const HttpUploadService& src) = delete;
	HttpUploadService(HttpUploadService&& src) = delete;
	~HttpUploadService();

	/* Member Functions */
	bool uploadBody(std::string& data);

	/* Basic Operators */
	HttpUploadService& operator=(const HttpUploadService& src) = delete;
	HttpUploadService& operator=(HttpUploadService&& src) = delete;

	/* Getters & Setters */

	class BadRequestException : public std::exception {
	public:
		const char* what() const noexcept override;
	};
	class MissingLengthException : public std::exception {
	public:
		const char* what() const noexcept override;
	};
	class BodyTooLargeException : public std::exception {
	public:
		const char* what() const noexcept override;
	};
	class InternelErrorException : public std::exception {
	public:
		const char* what() const noexcept override;
	};

};

#endif // HTTPUPLOAD_HPP
