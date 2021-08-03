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

    String lastDir = "./";


    public String getLastHostname() {
        // FIXME : persists
        return "192.168.1.21";
    }

    public void setLastHostname(String hostname) {
        // FIXME : persists
    }

    public String getLastDir() {
        // FIXME : persists
        return lastDir;
    }

    public void setLastDir(String directory) {
        // FIXME : persists
        lastDir = directory;
    }

}
