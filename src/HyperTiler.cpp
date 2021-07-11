#include "TileUtils.hpp"
#include "TileConversion.hpp"

#include <string>
#include "jsonUtils.hpp"
#include "Config.hpp"

#include "WebHighLevel.hpp"

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
    }

    return shouldShutdown;
}
#else
void my_handler(int s) {
    std::cout << "Halting from Ctrl+C\n";
    Running = false;
    ContinueProgram = false;
}
#endif

void SetupSignalHandler() {
#ifdef _WIN32
    ::SetConsoleCtrlHandler(CtrlHandler, TRUE);
#else
   struct sigaction sigIntHandler;

   sigIntHandler.sa_handler = my_handler;
   sigemptyset(&sigIntHandler.sa_mask);
   sigIntHandler.sa_flags = 0;

   sigaction(SIGINT, &sigIntHandler, NULL);
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
    AddJsonPost<Config>(svr, "/process",
        [](json const& j, js::ErrorStack& er, Config& state) {
            try {
                state = j;
            } catch(js::ErrorStack const& ex) {
                er = ex;
            }
        },
        [](Config const& state) -> json {
            if (Running) {
                return json("Already Running");
            } else {
                CurrentConfig = state;
                Running = true;
                ConversionThread = std::thread([] {
                    Convert(CurrentConfig, AddLogItem, Running);
                });
                return json("Begun");
            }
        }
    );
    
    AddEmptyPost(svr, "/cancel", [](){
        if (Running) return;

        Running = false;
        htAssert(ConversionThread.joinable());
        ConversionThread.join();
    });

    AddJsonPost<uint64_t>(svr, "/status",
        [](json const& j, js::ErrorStack& er, uint64_t& state) { // Parse json input
            js::LoadNamed(j, er, 0, "beginMicros", state);
        },
        [](uint64_t const& state) -> json { // After parsing
            json::array_t ar;
            std::chrono::system_clock::time_point lockTIme;
            vector<Log> logs = GetLogs(ProgramStart + std::chrono::microseconds(state), lockTIme);

            for (auto &l : logs) {
                ar.push_back({
                    {"time", std::chrono::duration_cast<std::chrono::microseconds>(l.time - ProgramStart).count()},
                    {"content", l.item->content()}
                });
            }

            return json {
                {"time", std::chrono::duration_cast<std::chrono::microseconds>(lockTIme - ProgramStart).count()},
                {"logs", ar}
            };
        }
    );

    {
        struct CheckExistenceStruct {
            string formatStr;
            ivec3 coord;
        };
        
        AddJsonPost<CheckExistenceStruct>(svr, "/exists",
            [](json const& j, js::ErrorStack& er, CheckExistenceStruct& state) {
                js::LoadNamed(j, er, 0, "formatStr", state.formatStr);
                js::LoadNamed(j, er, 0, "coord", state.coord);
            },
            [](CheckExistenceStruct const& state) -> json {
                return TileExists(state.formatStr, state.coord);
            }
        );
    }

    path const resourceDir = path(FRONT_DIR);

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

    // Wait for a request to stop the program
    while (ContinueProgram) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    svr.stop();
    webThread.join();

    if (ConversionThread.joinable()) ConversionThread.join();

    return 0;
}
