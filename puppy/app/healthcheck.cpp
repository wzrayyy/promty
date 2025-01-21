#include "httplib.hpp"

int main() {
    auto res = httplib::Client("127.0.0.1", 80).Get("/health");
    return !(res && res->status == 200);
}
