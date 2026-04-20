#include "HttpUpload.hpp"

/* Constructors & Destructors */
HttpUpload::HttpUpload(void)
{
}

HttpUpload::HttpUpload(const std::map<std::string, std::string> headers, std::string filePath, size_t clientMaxBody) : _headers(headers)
{
	_state = body_state::parsing_boundary;;
	_filePath = filePath + "/";

	auto search = _headers.find("Content-Length");
	if ((search == _headers.end())) // todo: From HTTP/1.1 onward -> _headers_map.find("Transfer-Encoding") != _headers_map.end()
		throw MissingLengthException();
	is_number(search->second);
	_contentLength = std::stoul(search->second);

	auto boundary_header = _headers.find("Content-Type");
	if (boundary_header == _headers.end())
		throw BadRequestException();
	_rawBoundary = extractBoundary(boundary_header->second);
	_boundary = _rawBoundary + "\r\n";
	_boundaryClosig = "\r\n" + _rawBoundary;
	if (_contentLength > clientMaxBody && clientMaxBody != 0)
		throw BodyTooLargeException();
}

HttpUpload::~HttpUpload()
{
	if (_file.is_open())
	{
		_file.close();
		std::remove((_filePath + _filename).c_str());
	}
}

/* Member Functions */
std::string HttpUpload::extractBoundary(std::string value)
{
	if (value.substr(0, 30).compare("multipart/form-data; boundary=") != 0)
		throw BadRequestException();
	std::string boundary = value.substr(30);
	return ("--" + boundary);
}

std::string HttpUpload::extractFilename(std::string& value)
{
	std::string filename;
	size_t fileNamePos = value.find("filename=");

	if (fileNamePos == std::string::npos)
		throw BadRequestException();
	fileNamePos += 9;
	if (value[fileNamePos] != '\"')
		throw BadRequestException();
	fileNamePos++;
	for (size_t i = fileNamePos; i < value.size(); i++)
	{
		if (value[i] == ';' && fileNamePos != i && value[i - 1] == '\"')
		{
			filename = value.substr(fileNamePos, (i - 1 - fileNamePos));
			return (filename);
		}
		if (i == value.size() - 1 && fileNamePos != i && value[i] == '\"')
		{
			filename = value.substr(fileNamePos, (i - fileNamePos));
			return (filename);
		}
	}
	throw BadRequestException();
}

void HttpUpload::writeToFile(std::string data)
{
	_file << data;
	if (_file.fail())
	{
		throw InternelErrorException();
	}
}

// read body boundery
bool  HttpUpload::parsingBoundary(std::string& data)
{
	size_t pos = data.find(_boundary);

	if (pos == 0)
	{
		if (_boundary.size() == data.size())
			data.erase();
		else
			data = data.substr(_boundary.size());
		return (true);
	}
	if (data.size() > _boundary.size())
		throw BadRequestException();
	return (false);
}

// read headers
bool HttpUpload::parsingHeaders(std::string& data)
{
	bool isProcessCompleted = false;

	while (true)
	{
		size_t posEnd = data.find("\r\n\r\n");  // check if i got the full header
		if (posEnd == std::string::npos)
			break;

		size_t lineLength = data.find_first_of("\r\n"); // find the end of the line 
		if (lineLength == std::string::npos)
			throw BadRequestException();

		std::string line = data.substr(0, lineLength); // extract the line
		size_t pos = line.find(':');
		if (pos == std::string::npos)
			throw BadRequestException();

		std::string key = line.substr(0, pos); // extract the key
		if (key.empty())
			throw BadRequestException();

		std::string value = line.substr(pos + 1); // extract the value

		if (key == "Content-Disposition") // extract the filename
		{
			_filename = extractFilename(value);
		}

		data = data.substr(lineLength + 2);
		if (posEnd == lineLength)
		{
			data = data.substr(2);
			isProcessCompleted = true;
			break;
		}
	}
	return (isProcessCompleted);
}

void HttpUpload::openFile()
{
	if (_filename.empty())
		throw BadRequestException();
	for (size_t i = 0; i < _filename.size(); i++)
	{
		if (_filename[i] == '/')
			throw BadRequestException();
	}
	Logger::log<LogLevel::trace>("HttpUpload::openFile() - Upload file: ", _filePath + _filename);
	_file = std::ofstream(_filePath + _filename);
	if (_file.fail())
		throw InternelErrorException();
}

body_state HttpUpload::saveData(std::string& data)
{
	size_t pos = 0;
	size_t i = 0;

	while (i < data.size())
	{
		while (i + pos < data.size() && data[i + pos] == _boundaryClosig[pos])
			pos++;
		if (pos == _boundaryClosig.size()) // boundary found
		{
			if (i != 0)
			{
				writeToFile(data.substr(0, i));
			}
			std::string tmp = data.substr(i + pos);
			if (tmp.find("\r\n") == 0) // check if the normal boundary (\r\n); if so, process the next file
			{
				data = data.substr(i + pos + 2);
				_file.close();
				return (body_state::parsing_headers);
			}
			if (tmp.find("--\r\n") == 0) // check if closing boundary (--\r\n)
			{
				_file.close();
				return (body_state::done);
			}
			else
			{
				return (body_state::save_data);
			}
		}
		if (i + pos == data.size()) // possible boundary found, but it's at the end of the data string, so we have to receive more data
		{
			writeToFile(data.substr(0, i));
			data = data.substr(i);
			return (body_state::save_data);
		}
		pos = 0;
		i++;
	}
	writeToFile(data);
	data.erase();
	return (body_state::save_data);
}

/*
// curl -X POST http://localhost:8085/upload \
//   -F "file1=@test.txt" \
//   -F "file2=@b.png"s
*/
bool HttpUpload::uploadBody(std::string& data)
{
	_bytesRead += data.size();
	if (_bytesRead > _contentLength)
		throw BodyTooLargeException();
	_buffer.append(data, 0, data.size());
	data.erase();
	while (_state != body_state::done)
	{
		if (_state == body_state::parsing_boundary)
		{
			if (parsingBoundary(_buffer))
				_state = body_state::parsing_headers;
			else
				break;
		}

		if (_state == body_state::parsing_headers)
		{
			if (parsingHeaders(_buffer))
			{
				openFile();
				_state = body_state::save_data;
			}
			else
				break;
		}

		if (_state == body_state::save_data)
		{
			_state = saveData(_buffer);
			if (_state == body_state::save_data)
				break;
		}
	}
	if (_bytesRead == _contentLength && _state == body_state::done)
		return true; // todo: need to reset bytes_read? can same client get here a second time?
	return false;
}

/* Basic Operators */

/* Getters & Setters */


const char* HttpUpload::BadRequestException::what() const noexcept
{
	return ("Http upload bad request error\n");
}

const char* HttpUpload::MissingLengthException::what() const noexcept
{
	return ("Http upload missing length error\n");
}

const char* HttpUpload::BodyTooLargeException::what() const noexcept
{
	return ("Http upload body too large error\n");
}

const char* HttpUpload::InternelErrorException::what() const noexcept
{
	return ("Http upload internel server error\n");
}
