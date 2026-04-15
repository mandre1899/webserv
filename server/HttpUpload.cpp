#include "HttpUpload.hpp"

/* Constructors & Destructors */
HttpUpload::HttpUpload(const std::map<std::string, std::string> headers, std::string filePath) : _headers(headers)
{
	_state = body_state::parsing_boundary;;
	_filePath = filePath + "/"; // todo: is this necessary?

	auto search = _headers.find("Content-Length");
	if ((search == _headers.end())) // todo:  Ab HTTP/1.1 -> _headers_map.find("Transfer-Encoding") != _headers_map.end()
		throw UploadErrorException();
	is_number(search->second);
	_contentLength = std::stoul(search->second);

	auto boundary_header = _headers.find("Content-Type");
	if (boundary_header == _headers.end())
		throw UploadErrorException();
	_rawBoundary = extractBoundary(boundary_header->second);
	_boundary = _rawBoundary + "\r\n";
	_boundaryClosig = _rawBoundary + "--\r\n";
}

HttpUpload::HttpUpload(void)
{}

//HttpUpload::HttpUpload(HttpUpload&& src)
//{
//
//}

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
		throw UploadErrorException();
	std::string boundary = value.substr(30);
	return ("--" + boundary);
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
		throw UploadErrorException();
	return (false);
}

// read headers
bool HttpUpload::parsingHeaders(std::string& data)
{
	bool isProcessCompleted = false;

	while (true)
	{
		size_t posEnd = data.find("\r\n\r\n"); // check if i got the full header
		if (posEnd == std::string::npos)
			return isProcessCompleted;

		size_t subLen = data.find_first_of("\r\n"); // find the sub data
		if (subLen == std::string::npos)
			throw UploadErrorException();

		std::string sub = data.substr(0, subLen); // extract find the sub data
		std::cout << "sub: " << sub << std::endl;
		size_t pos = sub.find(':');
		if (pos == std::string::npos)
			throw UploadErrorException();

		std::string key = sub.substr(0, pos); // extract the key
		if (key.empty())
			throw UploadErrorException();

		std::string value = sub.substr(pos + 1);

		if (key == "Content-Disposition") // get file name
		{
			size_t fileNamePos = value.find("filename="); // todo: more validation
			if (fileNamePos == std::string::npos)
				throw UploadErrorException();
			fileNamePos += 9;
			if (value[fileNamePos] != '\"')
				throw UploadErrorException();
			fileNamePos++;
			for (size_t i = fileNamePos; i < value.size(); i++)
			{
				if (value[i] == ';' && fileNamePos != i && value[i - 1] == '\"')
				{
					_filename = value.substr(fileNamePos, (i - 1 - fileNamePos));
					break;
				}
				if (i == value.size() - 1 && fileNamePos != i && value[i] == '\"')
				{
					_filename = value.substr(fileNamePos, (i - fileNamePos));
					break;
				}
			}
		}

		data = data.substr(subLen + 2);
		if (posEnd == subLen)
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
		throw UploadErrorException();
	std::cout << _filePath + _filename << " <- file path + file name" << std::endl; //todo: remove
	_file = std::ofstream(_filePath + _filename);
}

body_state HttpUpload::saveData(std::string& data)
{
	std::string buffer;
	size_t pos = 0;
	size_t i = 0;

	buffer.append(data, 0, data.size());
	data = buffer;
	while (i < data.size())
	{
		while (i + pos < data.size() && data[i + pos] == _rawBoundary[pos])
			pos++;
		if (pos == _rawBoundary.size())
		{
			if (i != 0)
			{
				_file << data.substr(0, i);
			}
			if (i + pos + 3 < data.size() && data[i + pos] == '-' && data[i + pos + 1] == '-') // add check for \r\n check if closing boundery
			{
				_file.close();
				return (body_state::done);
			}
			else if (i + pos + 1 < data.size() && data[i + pos] == '\r' && data[i + pos + 1] == '\n') // check if normal boundery
			{
				data = data.substr(i + _rawBoundary.size() + 2);
				buffer.erase();
				_file.close();
				std::cout << "check data find: " << data << std::endl;
				return (body_state::parsing_headers);
			}
		}
		if (i + pos == data.size())
		{
			buffer = data.substr(i);
			data = data.substr(0, i);
			std::cout << "check buffer: " << buffer << std::endl;
			std::cout << "check data: " << data << std::endl;
			break;
		}
		pos = 0;
		i++;
	}
	_file << data;
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
	std::cout << data << std::endl;
	_bytesRead += data.size();
	if (_bytesRead > _contentLength)
		throw UploadErrorException();
	_buffer.append(data, 0, data.size());

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
		return true; // need to reset bytes_read? can same client get here a second time?
	return false;
}

/* Basic Operators */

/* Getters & Setters */


const char* HttpUpload::UploadErrorException::what() const noexcept
{
	return ("Http parsing error\n");
}
