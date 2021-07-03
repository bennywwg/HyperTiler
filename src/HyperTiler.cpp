#include "TileUtils.hpp"
#include "TileConversion.hpp"

#include <string>
#include "jsonUtils.hpp"
#include "Config.hpp"

#include "httplib.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#include <signal.h>
#endif

#pragma optimize("", off)

using namespace HyperTiler;

httplib::Server svr;
std::chrono::system_clock::time_point ProgramStart;
Config CurrentConfig;
std::atomic_bool Running = false;
std::atomic_bool ContinueProgram = true;
std::thread ConversionThread;
std::mutex Mut;
vector<Log> Logs;

#ifdef _WIN32
HANDLE g_hTerminateEvent;
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType) {
    bool shouldShutdown = false;

    string reason = "";

    switch (fdwCtrlType) {
    case CTRL_C_EVENT:
        shouldShutdown = true;
        reason = "Ctrl+C";
        break;

    case CTRL_CLOSE_EVENT:
        shouldShutdown = true;
        reason = "Ctrl+Close";
        break;

    case CTRL_BREAK_EVENT:
        shouldShutdown = true;
        reason = "Ctrl+Break";
        break;

    case CTRL_LOGOFF_EVENT:
        shouldShutdown = true;
        reason = "Logoff";
        break;

    case CTRL_SHUTDOWN_EVENT:
        shouldShutdown = true;
        reason = "Shutdown";
        break;

    default:
        shouldShutdown = false;
        break;
    }

    if (shouldShutdown) {
        std::cout << "Halting from " << reason << "\n";
        std::cout << "Cleaning up ...\n";
        Running = false;
        ContinueProgram = false;
        ::SetEvent(g_hTerminateEvent);
    }

    return shouldShutdown;
}
#endif

void SetupSignalHandler() {
#ifdef _WIN32
    g_hTerminateEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    ::SetConsoleCtrlHandler(CtrlHandler, TRUE);
#endif
}

void AddLogItem(Log log) {
    std::lock_guard<std::mutex> lock(Mut);
    Logs.push_back(log);
}

vector<Log> GetLogs(std::chrono::system_clock::time_point beginTime, std::chrono::system_clock::time_point &lockTIme) {
    std::lock_guard<std::mutex> lock(Mut);
    lockTIme = std::chrono::system_clock::now();
    vector<Log> res;
    for (Log log : Logs) {
        if (log.time >= beginTime) res.push_back(log);
    }
    return res;
}

void HandleWebRequests() {
    svr.Post("/process", [](const httplib::Request& req, httplib::Response& res) {
        if (Running) {
            res.set_content("Already Running", "text/plain");
            return;
        }

        try {
            json parsed = json::parse(req.body);
            Config conf = parsed;

            {
                if (Running) {
                    std::cout << "Already processing, ignoring request\n";
                    return;
                }

                CurrentConfig = conf;
                Running = true;
                htAssert(!ConversionThread.joinable());
                ConversionThread = std::thread([] {
                    Convert(CurrentConfig, AddLogItem, Running);
                });
            }
        } catch (json::exception const& ex) {
            std::cout << ex.what() << "\n";
        } catch (js::ErrorStack const& ex) {
            std::cout << ex.what() << "\n";
        }
    });
    
    svr.Post("/cancel", [](const httplib::Request& req, httplib::Response& res) {
        {
            if (Running) return;

            Running = false;
            htAssert(ConversionThread.joinable());
            ConversionThread.join();
        }
    });
    
    svr.Post("/status", [](const httplib::Request& req, httplib::Response& res) {
        uint64_t beginMicros = 0;
        try {
            json parsed = json::parse(req.body);

            beginMicros = parsed["beginMicros"].get<uint64_t>();
        } catch (json::exception) {
            std::cout << "malformed status request\n";
            res.status = 400;
            return;
        }

        json::array_t ar;
        std::chrono::system_clock::time_point lockTIme;
        vector<Log> logs = GetLogs(ProgramStart + std::chrono::microseconds(beginMicros), lockTIme);
        for (auto &l : logs) {
            ar.push_back({
                {"time", std::chrono::duration_cast<std::chrono::microseconds>(l.time - ProgramStart).count()},
                {"content", l.item->content()}
            });
        }

        res.set_content(json{
            {"time", std::chrono::duration_cast<std::chrono::microseconds>(lockTIme - ProgramStart).count()},
            {"logs", ar}
        }.dump(), "application/json");
    });

    svr.Post("/exists", [](const httplib::Request& req, httplib::Response& res) {
        json parsed;
        string formatStr;
        ivec3 coord;
        try {
            parsed = json::parse(req.body);
        } catch (json::exception) {
            std::cout << "malformed exists request\n";
            res.status = 400;
            return;
        }

        js::ErrorStack er;
        js::LoadNamed(parsed, er, 0, "formatStr", formatStr);
        js::LoadNamed(parsed, er, 0, "coord", coord);

        if (!er.empty()) {
            res.status = 400;
            return;
        }

        res.set_content(json(TileExists(formatStr, coord)).dump(), "application/json");
    });
    
    path const resourceDir = path(INSTALL_DIR) / path("front/public");

    std::cout << "Starting from resource directory " << resourceDir << "\n";

    if (!svr.set_mount_point("/", resourceDir.string().c_str())) {
        std::cout << "Failed to mount web resources\n";
        exit(0);
    }

    svr.listen("127.0.0.1", 5000);
}

int main(int /*argc*/, char** /*argv*/) {
    string str = ((json)ImageEncoding()).dump();

    ProgramStart = std::chrono::system_clock::now();

    SetupSignalHandler();

    std::thread webThread(HandleWebRequests);

    //Config conf;
    //conf.DatasetConfig.InputURIFormat = string("https://spkit.org/datasets/remapped/{x3}_{y3}_{z3}.hgt");
    ////conf.DatasetConfig.InputURIFormat = string("http://192.168.1.52:4432//datasets/remapped/{x3}_{y3}_{z3}.hgt");
    //conf.DatasetConfig.InputEncoding.SwapEndian = true;
    //
    //conf.DatasetConfig.OutputURIFormat = string("./{x3}_{y3}_{z3}.png");
    //conf.DatasetConfig.OutputEncoding.Encoding = FormatEncoding::PNG;
    //conf.DatasetConfig.OutputEncoding.SwapEndian = false;
    //
    //conf.SpatialConfig.BeginOutputLevel = 3;
    //conf.SpatialConfig.EndOutputLevel = 4;
    //conf.SpatialConfig.InputTileSize = ivec2(1201, 1201);
    //conf.SpatialConfig.OutputTileSize = ivec2(512, 512);
    //conf.SpatialConfig.OutputPixelRange = DiscreteAABB2<int>(0, 0, 216180 * 2, 216180);
    //conf.SpatialConfig.OutputPixelRange = DiscreteAABB2<int>(0, 0, 1201 * 4, 1201 * 4) + (ivec2(0, 90) * 1201 + ivec2(12010));
    //
    //Convert(conf, AddLogItem);

    // Wait for a request to stop the program
#ifdef _WIN32
    ::WaitForSingleObject(g_hTerminateEvent, INFINITE);
#endif

    svr.stop();
    webThread.join();

    if (ConversionThread.joinable()) ConversionThread.join();

    return 0;
}