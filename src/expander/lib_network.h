/**
 * XtsTiExpander V2
 * 
 * Network stack
 * 
 * Xtase - fgalliat @Aug 2021
 */

class Network {
  private:
    char curIp[16+1];
    char curSSID[64+1];
    bool connected = false;

    void loopTelnet();
  public:
    Network();
    ~Network();

    void start();
    void stop();
    void loop();

    bool addConfig(char* ssid, char* psk);
    void eraseConfig();
    char* getIP();
    char* getNetname();
    bool isConnected();
};
