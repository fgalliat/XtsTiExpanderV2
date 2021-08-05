import java.io.*;
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
            while (in.available() > 0 && !Terminal.ISRlocked) {
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

    protected int min(int a, int b) {
        return a < b ? a : b;
    }

    public boolean getVarFromExpander(String varName) throws Exception {
        if (varName == null || varName.isEmpty()) {
            return false;
        }


        GUI.getInstance().lockInput(true);
        Terminal.lockISR();
        Utils.delay(50);

        out.write(("/recv " + varName + "\n").getBytes(StandardCharsets.UTF_8));

        while (true) {
            while (in.available() <= 0) {
                Utils.delay(10);
            }
            int r = in.read();
            GUI.getInstance().addTextToConsole( ""+(char)r );
            if ((char) r == '\n') {
                break;
            }
        }

        GUI.getInstance().addTextToConsole("Waiting '" + varName + "'\n");

        while (in.available() <= 0) {
            Utils.delay(20);
        }

        int b0 = in.read();
        int b1 = in.read();

        if ((char) b0 == '/' && (char) b1 == '!') {
            // BEWARE w/ that test ....
            GUI.getInstance().addTextToConsole("/!");
            GUI.getInstance().lockInput(false);
            Terminal.unlockISR();
            return false;
        }

        long varSize = (b0 << 8) + b1; // +2 contains CHK
        int varType = in.read();

        GUI.getInstance().addTextToConsole("Fetching '" + varName + "' (" + Integer.toHexString(varType) + ") " + varSize + "bytes\n");

        File f = new File("./tivars/" + varName + "." + Integer.toHexString(varType));
        f.getParentFile().mkdirs();

        FileOutputStream fout = new FileOutputStream(f);

        byte[] buff = new byte[128];
        for (int i = 0; i < varSize; i++) {
            int recv = in.read(buff, 0, min( 128, (int)(varSize - i) ));
            fout.write(buff, 0, recv);
        }

        fout.flush();
        fout.close();

        GUI.getInstance().addTextToConsole("-EOF-");

        GUI.getInstance().lockInput(false);
        Terminal.unlockISR();
        return true;
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

            fin.skip(LEGACY_VAR_DATA_OFFSET - LEGACY_VAR_TYPE_OFFSET - 1);
            // varSize -= LEGACY_VAR_DATA_OFFSET;
            varSize = fin.read() << 8; // need to read 2 bytes @86 [MSB]
            varSize += fin.read(); // [LSB]
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

        GUI.getInstance().lockInput(true);
        Terminal.lockISR();
        Utils.delay(50);

        GUI.getInstance().addTextToConsole("[Begin garbage[");
        while (in.available() > 0) {
            int ch = in.read();
            GUI.getInstance().addTextToConsole("" + ((char) ch));
        } // read potential previous echo messages
        GUI.getInstance().addTextToConsole("]\n");

        String expFileName = varName + "." + Integer.toHexString(varType);

        GUI.getInstance().addTextToConsole("cmd:" + "/send\n");
        // GUI.getInstance().addTextToConsole("nam:" + expFileName + "\n");
        GUI.getInstance().addTextToConsole("nam:" + varName + "\n");
        GUI.getInstance().addTextToConsole("typ:" + Integer.toHexString(varType) + "\n"); // -> uint8_t
        GUI.getInstance().addTextToConsole("siz:" + (varSize - 2) + "\n"); // -> uint16_t -- -2 only for display
        GUI.getInstance().addTextToConsole("tiS:" + (sendToTiToo ? "1" : "0") + "\n");

        out.write("/send\n".getBytes(StandardCharsets.UTF_8));
        out.flush();

        while (in.available() <= 0) {
            Utils.delay(10);
        }
        GUI.getInstance().addTextToConsole("[Command garbage[");
        while (in.available() > 0) {
            int ch = in.read();
            GUI.getInstance().addTextToConsole("" + ((char) ch));
        } // read potential previous echo messages
        GUI.getInstance().addTextToConsole("]\n");


        out.write((varName + "\n").getBytes(StandardCharsets.UTF_8));
//        Utils.delay(5);

        write(varType);
//        Utils.delay(5);

        write((int) (varSize >> 8));
        write((int) (varSize % 256));
//        Utils.delay(5);

        write(sendToTiToo ? 0x01 : 0x00);
        out.flush();
//        Utils.delay(5);

        GUI.getInstance().addTextToConsole("Waiting for Expander CTS\n");
        while (in.available() <= 0) {
            Utils.delay(5);
        } // wait for RTS
        int cts = in.read();
        if (cts != 0x01) {
            GUI.getInstance().addTextToConsole("Oups CTS may be not valid [" + ((int) cts) + "]\n");
            write(0x03); // abord
        } else {
            GUI.getInstance().addTextToConsole("Received Expander CTS\n");
            write(0x02);
        }

        final int blocLen = 128; // must be < 256 / as same as arduino side
        byte[] buff = new byte[blocLen];
        int read;
        int i = 0;
        while (i < varSize) {
            read = fin.read(buff, 0, min(blocLen, fin.available()));
//            GUI.getInstance().debugDatas(buff, read);

            write(read); // send len to copy
            out.write(buff, 0, read);
//            out.flush();

            while (in.available() <= 0) {
                Utils.delay(1);
            }
            int handshake = in.read();
            if (handshake != 0x01) {
                GUI.getInstance().addTextToConsole("Oups HANDSHAKE may be not valid [" + ((int) handshake) + "]\n");
            }

            i += read;
        }
        fin.close();

        GUI.getInstance().addTextToConsole("-EOF-\n");

        Terminal.unlockISR();
        GUI.getInstance().lockInput(false);

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
