# include "../inc/Webserv.hpp"

void Response::getReq(Request * req) {
    std::cout << "void Response::getReq(Request * req)" << std::endl;
}

void Response::headReq(Request * req) {
    std::cout << "void Response::headReq(Request * req)" << std::endl;
}

void Response::putReq(Request * req) {
    std::cout << "void Response::putReq(Request * req)" << std::endl;
}

void Response::postReq(Request * req) {
    std::cout << "void Response::postReq(Request * req)" << std::endl;
}

void Response::connectReq(Request * req) {
    std::cout << "void Response::connectReq(Request * req)" << std::endl;
}

void Response::traceReq(Request * req) {
    std::cout << "void Response::traceReq(Request * req)" << std::endl;
}

void Response::optionsReq(Request * req) {
    std::cout << "void Response::optionsReq(Request * req)" << std::endl;
}

void Response::deleteReq(Request * req) {
    std::cout << "void Response::deleteReq(Request * req)" << std::endl;
}
