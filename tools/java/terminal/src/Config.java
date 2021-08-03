public class Config {

    protected static Config instance;

    protected Config() {
    }

    public static Config getInstance() {
        if (instance == null) {
            instance = new Config();
        }
        return instance;
    }

    public String getLastHostname() {
        // FIXME : persists
        return "192.168.1.21";
    }

    public void setLastHostname(String hostname) {
        // FIXME : persists
    }

    public String getLastDir() {
        // FIXME : persists
        return "./";
    }

    public void setLastDire(String directory) {
        // FIXME : persists
    }

}
