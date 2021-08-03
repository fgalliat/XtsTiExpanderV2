import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.util.Locale;

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

    /**
     * @param f              : dataFile
     * @param legacyFileMode : for .92x, v2x ... files (skip 86 bytes)
     * @param sendToTiToo    : auto send to ti after storing on expander
     * @return
     */
    public boolean sendVarToExpander(File f, boolean legacyFileMode, boolean sendToTiToo) throws Exception {
        if (f == null || !f.exists()) {
            return false;
        }
        FileInputStream fin = new FileInputStream(f);

        int varType = -1;
        String varName = null;

        long varSize = f.length();
        if (legacyFileMode) {
            final int LEGACY_VAR_NAME_OFFSET = 64; // 0x40
            final int LEGACY_VAR_TYPE_OFFSET = 72; // 0x48
            final int LEGACY_VAR_DATA_OFFSET = 86; // 0x56

            if (varSize <= LEGACY_VAR_DATA_OFFSET) {
                fin.close();
                throw new IllegalArgumentException("Wrong native file (<= " + LEGACY_VAR_DATA_OFFSET + " bytes)");
            }

            fin.skip(LEGACY_VAR_NAME_OFFSET);
            byte[] vn = new byte[8];
            fin.read(vn);
            varName = "";
            for (int i = 0; i < vn.length; i++) {
                if (vn[i] == 0x00) {
                    break;
                }
                varName += (char) vn[i];
            }

            // fin.skip(LEGACY_VAR_TYPE_OFFSET);
            varType = fin.read();

            fin.skip(LEGACY_VAR_DATA_OFFSET - LEGACY_VAR_TYPE_OFFSET);
            varSize -= LEGACY_VAR_DATA_OFFSET;
        } else {
            varType = nameToTiVarType(f.getName());
            varName = nameToTiVarName(f.getName());
        }

        if (varType <= 0) {
            fin.close();
            throw new IllegalArgumentException("Wrong variable type");
        }

        if (varName.isEmpty()) {
            fin.close();
            throw new IllegalArgumentException("Wrong varName (for " + f.getName() + ")");
        }

        Terminal.lockISR();

        String expFileName = varName + "." + Integer.toHexString(varType);

        GUI.getInstance().addTextToConsole("cmd:" + "/send\n");
        // GUI.getInstance().addTextToConsole("nam:" + expFileName + "\n");
        GUI.getInstance().addTextToConsole("nam:" + varName + "\n");
        GUI.getInstance().addTextToConsole("typ:" + Integer.toHexString(varType) + "\n"); // -> uint8_t
        GUI.getInstance().addTextToConsole("siz:" + (varSize - 2) + "\n"); // -> uint16_t -- -2 only for display
        GUI.getInstance().addTextToConsole("tiS:" + (sendToTiToo ? "1" : "0") + "\n");

        out.write("/send\n".getBytes(StandardCharsets.UTF_8));
        out.flush();

        //Utils.delay(50);
        while( in.available() < 0 ) { Utils.delay(10); }
        while( in.available() > 0 ) {
            int ch = in.read();
            GUI.getInstance().addTextToConsole( ""+((char)ch) );
        } // read potential echo messages


        out.write((varName + "\n").getBytes(StandardCharsets.UTF_8));
        out.write(varType);
        out.write((int) (varSize >> 8)); out.write((int) (varSize % 256));
        out.write(sendToTiToo ? 0x01 : 0x00);
        out.flush();
        // Utils.delay(50);

        while( in.available() < 0 ) { Utils.delay(10); } // wait for RTS
        in.read();

        final int blocLen = 64;
        byte[] buff = new byte[blocLen];
        int read;
        for (int i = 0; i < varSize; i += blocLen) {
            read = fin.read(buff, 0, blocLen);

            //out.write(buff, 0, read);
            for(int ii=0; ii < read; ii++) {
                out.write( buff[ii] );
            }

            // Utils.delay(2);
            while( in.available() < 0 ) { Utils.delay(10); }
            while( in.available() > 0 ) { in.read(); } // wread handshake
        }
        fin.close();

        GUI.getInstance().addTextToConsole("-EOF-");

        Terminal.unlockISR();

        return true;
    }

    public static int nameToTiVarType(String name) {
        if (!name.contains(".")) {
            return -1;
        }
        String ext = name.substring(name.indexOf(".") + 1);
        try {
            return Integer.parseInt(ext, 16);
        } catch (Exception ex) {
            return -1;
        }
    }

    public static String nameToTiVarName(String name) {
        if (name.contains("/") || name.contains("\\")) {
            name = new File(name).getName();
        }

        if (name.contains(".")) {
            name = name.substring(0, name.indexOf("."));
        }

        name = name.toLowerCase(Locale.ROOT);

        if (name.length() > 8) {
            name = name.substring(0, 8);
        }
        return name;
    }

}
