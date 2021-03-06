#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Request {
    private:

        std::string extractDocument(std::string& str);
        std::string extractGet(std::string& str);

        std::string decodeURL(std::string& str);
        void splitString(const std::string& str, char c, std::vector<std::string>& strVec);
    public:
        bool isRequestGood;
        std::string Method;
        std::string Document;
        std::string DocumentOriginal;
        std::string GetParameters;
        std::string PostParameters;
        std::string Version;
        std::vector<std::string> Headers;
        std::string UserAgent;
        std::string Host;
        std::string Body;

        Request(const std::string& request);
};

#endif
