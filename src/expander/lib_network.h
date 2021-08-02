/**
 * XtsTiExpander V2
 * 
 * Network stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

class Network {
  public:
    Network();
    ~Network();

    void start();
    void stop();
    bool addConfig(char* ssid, char* psk);
    char* getIP();
    char* getNetname();
    bool isConnected();
};
