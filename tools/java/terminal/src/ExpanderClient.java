import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;

public class ExpanderClient {

    protected static ExpanderClient instance;

    protected ExpanderClient() {
    }

    public static ExpanderClient getInstance() {
        if (instance == null) {
            instance = new ExpanderClient();
        }
        return instance;
    }

    public static final int port = 23;
    protected Socket sk = null;
    protected InputStream in = null;
    protected OutputStream out = null;


    public boolean connect(String host) throws Exception {
        sk = new Socket(host, port);
        in = sk.getInputStream();
        out = sk.getOutputStream();
        return true;
    }

    public void disconnect() {
        try {
            out.close();
        } catch (Exception ex) {
        }
        try {
            in.close();
        } catch (Exception ex) {
        }
        try {
            sk.close();
        } catch (Exception ex) {
        }
        sk = null;
    }

    public boolean isConnected() {
        return sk != null;
    }

    public boolean loop() {
        if (!isConnected()) {
            return false;
        }
        try {
            while (in.available() > 0) {
                GUI.getInstance().addTextToConsole("" + ((char) in.read()));
            }
        } catch (Exception ex) {
            GUI.getInstance().addTextToConsole("(!!) " + ex.toString());
        }
        return true;
    }

    public int write(int ch) {
        try {
            out.write(ch);
            return 1;
        } catch (Exception ex) {
            return 0;
        }
    }

}
