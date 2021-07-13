#include "Util.hpp"
#include "jsonUtils.hpp"
#include "httplib.hpp"

namespace HyperTiler {
    // For parse errors, push onto 
    template<typename T>
    using ParseJsonFunc = std::function<void(json const&, js::ErrorStack&, T&)>;

    template<typename T, typename R = json>
    using PostParseFunc = std::function<R(T const&)>;

    // Parse input into json
    // Parse the json into user format
    // Then call postFunc if nothing failed
    template<typename T>
    void AddJsonPost(httplib::Server & svr, string const& path, ParseJsonFunc<T> const& parseFunc, PostParseFunc<T> const& postFunc) {
        svr.Post(path.c_str(), [parseFunc, postFunc](const httplib::Request& req, httplib::Response& res) {
            json j;
            try {
                j = json::parse(req.body);
            } catch (json::exception const& ex) {
                res.status = 400;
                res.set_content(ex.what(), "text/plain");
                return;
            }

            T state;
            js::ErrorStack er;
            try {
                parseFunc(j, er, state);
            } catch (json::exception const& ex) {
                res.status = 500;
                res.set_content(string("parseFunc unexpectedly threw json error:\n") + ex.what(), "text/plain");
                return;
            }

            if (!er.empty()) {
                res.status = 400;
                res.set_content(er.what(), "text/plain");
                return;
            }

            json jres = postFunc(state);

            res.status = 200;
            res.set_content(jres.dump(), "application/json");
        });
    }

    void AddEmptyPost(httplib::Server& svr, string const& path, std::function<void()> const& func) {
        svr.Post(path.c_str(), [func](const httplib::Request& req, httplib::Response& res) {
            func();

            res.status = 200;
            res.set_content(json({}).dump(), "application/json");
        });
    }

    template<typename T>
    void AddDataPost(httplib::Server & svr, string const& path, string const& mime, ParseJsonFunc<T> const& parseFunc, PostParseFunc<T, vector<uint8_t>> const& postFunc) {
        svr.Post(path.c_str(), [parseFunc, postFunc, mime](const httplib::Request& req, httplib::Response& res) {
            json j;
            try {
                j = json::parse(req.body);
            } catch (json::exception const& ex) {
                res.status = 400;
                res.set_content(ex.what(), "text/plain");
                return;
            }

            T state;
            js::ErrorStack er;
            try {
                parseFunc(j, er, state);
            } catch (json::exception const& ex) {
                res.status = 500;
                res.set_content(string("parseFunc unexpectedly threw json error:\n") + ex.what(), "text/plain");
                return;
            }

            if (!er.empty()) {
                res.status = 400;
                res.set_content(er.what(), "text/plain");
                return;
            }

            vector<uint8_t> bres = postFunc(state);

            res.status = 200;
            res.set_content(reinterpret_cast<char*>(bres.data()), bres.size(), mime.c_str());
        });
    }

    #define EASY_POST(svr, path, T, parseBody, postBody)\
    AddJsonPost<T>(svr, path, [](json const& j, js::ErrorStack& er, T& state) parseBody, [](T const& state) -> json postBody)
}