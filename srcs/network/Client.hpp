/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vshchuki <vshchuki@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/19 12:27:08 by dnikifor          #+#    #+#             */
/*   Updated: 2024/08/03 15:30:30 by vshchuki         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../request/Request.hpp"
#include "../response/Response.hpp"
#include <limits>
#include <string>

#include <chrono>

// Forward declaration of the Request class
class Request;
// Forward declaration of the Response class
class Response;

class Client
{
	public:
		enum class ClientState
		{
			READING,
			READY_TO_WRITE,
			BUILDING,
			WRITING,
			FINISHED_WRITING
		};

		enum class CGIState
		{
			INIT,
			FORKED,
			FINISHED_SET,
			FINISHED
		};
	
	private:
		int											_fd;
		pid_t										_pid;
		int											_parentPipe[2];
		int											_childPipe[2];
		std::string									_CGIString;
		// Request*									_request;
		std::shared_ptr<Request>					_request;
		// Response*									_response;
		std::shared_ptr<Response>					_response;
		std::string									_respBody;
		ClientState									_state;
		CGIState									_stateCGI;

		std::string									_requestString;
		int											_emptyLinePos;
		int											_emptyLinesSize;
		size_t										_contentLengthNum;
		bool										_isHeadersRead;
		bool										_isBodyRead;
		size_t										_maxClientBodyBytes;

		std::string									_responseString;
		size_t										_totalBytesWritten;
		std::chrono::system_clock::time_point		_cgiStart;

	public:
		Client();
		~Client();

		int											getFd();
		pid_t										getPid();
		int											getChildPipe(int index);
		int*										getChildPipeWhole();
		int											getParentPipe(int index);
		int*										getParentPipeWhole();
		std::string									getCGIString();
		std::string&								getRespBody();
		// Request*									getRequest();
		std::shared_ptr<Request>					getRequest();
		// Response*									getResponse();
		std::shared_ptr<Response>					getResponse();
		ClientState									getState();
		CGIState									getCGIState();
		std::string									getRequestString();
		bool										getIsHeadersRead();
		bool										getIsBodyRead();
		int											getEmptyLinePos();
		int											getEmptyLinesSize();
		size_t										getContentLengthNum();
		size_t										getMaxClientBodyBytes();
		std::string									getResponseString();
		size_t										getTotalBytesWritten();
		std::chrono::system_clock::time_point		getCgiStart();
		
		void										setFd(int fd);
		void										setPid(pid_t pid);
		void										setParentPipe(int index, int fd);
		void										setChildPipe(int index, int fd);
		void										setCGIString(const std::string& cgiString);
		// void										setRequest(Request* request);
		void										setRequest(std::shared_ptr<Request> request);
		// void										setResponse(Response* response);
		void										setResponse(std::shared_ptr<Response> response);
		void										setState(ClientState state);
		void										setCGIState(CGIState state);
		void										setRequestString(const std::string& requestString);
		void										setEmptyLinePos(int emptyLinePos);
		void										setEmptyLinesSize(int emptyLinesSize);
		void										setContentLengthNum(size_t contentLengthNum);
		void										setIsHeadersRead(bool isHeadersRead);
		void										setIsBodyRead(bool isBodyRead);
		void										setMaxClientBodyBytes(size_t maxClientBodyBytes);
		void										setResponseString(const std::string& responseString);
		void										setTotalBytesWritten(size_t totalBytesWritten);
		void										setCgiStart(std::chrono::system_clock::time_point cgiStart);
};
