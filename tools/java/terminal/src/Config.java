import java.io.FileOutputStream;
import java.io.FileReader;
import java.util.Date;
import java.util.Properties;

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

    protected Properties props = null;

    public String getLastHostname() {
        String lastHost = readProp("host.lastHost");
        if (lastHost == null) {
            lastHost = "192.168.1.21";
            setLastHostname(lastHost);
        }
        return lastHost;
    }

    public void setLastHostname(String hostname) {
        storeProp("host.lastHost", hostname);
    }

    public String getLastDir() {
        String lastDir = readProp("path.lastDir");
        if (lastDir == null) {
            lastDir = "./";
            setLastDir(lastDir);
        }
        return lastDir;
    }

    public void setLastDir(String directory) {
        storeProp("path.lastDir", directory);
    }

    protected String readProp(String prop) {
        if (props == null) {
            props = new Properties();
            try {
                FileReader reader = new FileReader("./config.prop");
                props.load(reader);
                reader.close();
            } catch (Exception ex) {
                // props = null;
            }
        }
        return props.getProperty(prop);
    }

    protected void storeProp(String prop, String propValue) {
        if (props == null) {
            props = new Properties();
            try {
                FileReader reader = new FileReader("./config.prop");
                props.load(reader);
                reader.close();
            } catch (Exception ex) {
                // props = null;
            }
        }
        props.setProperty(prop, propValue);
        try {
            FileOutputStream fout = new FileOutputStream("./config.prop");
            props.store(fout, "Updated @ " + new Date().toString());
            fout.close();
        } catch (Exception ex) {
        }
    }


}
