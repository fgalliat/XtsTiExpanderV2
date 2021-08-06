import java.io.File;

/**
 * WiFi terminal for
 * XtsTiExpanderV2
 * <p>
 * Xtase - fgalliat @Aug2021
 */

public class Terminal {

    protected static boolean ISRlocked = false;

    public static void lockISR() {
        ISRlocked = true;
    }

    public static void unlockISR() {
        ISRlocked = false;
    }

    public static void main(String[] args) throws Exception {

        if (args != null && args.length > 0) {
            GUI.getInstance().setCliMode(true);
            if ("recv".equals(args[0])) {
                if (args.length > 2) {
                    ExpanderClient.getInstance().connect(args[1]);
                    ExpanderClient.getInstance().consumeHello(); // consume Hello message
                    ExpanderClient.getInstance().getVarFromExpander(args[2]);
                    ExpanderClient.getInstance().disconnect();
                } else {
                    System.out.println("Usage expander.jar recv <ExpanderIP> <TiVar>");
                    System.out.println(" to get a TiVar from Expander storage");
                }
            } else if ("send".equals(args[0])) {
                if (args.length > 2) {
                    boolean sendToTi = false;
                    if (args.length > 3) {
                        sendToTi = "true".equals(args[3]);
                    }
                    File tiVarFile = new File(args[2]);
                    boolean legacyFile = ExpanderClient.defineIfNative(tiVarFile);
                    ExpanderClient.getInstance().connect(args[1]);
                    ExpanderClient.getInstance().consumeHello(); // consume Hello message
                    ExpanderClient.getInstance().sendVarToExpander(tiVarFile, legacyFile, sendToTi);
                    ExpanderClient.getInstance().disconnect();
                } else {
                    System.out.println("Usage expander.jar send <ExpanderIP> <TiVarFile> <bool:SendToTi>");
                    System.out.println(" to send a TiVar file to Expander storage");
                    System.out.println(" & optionally to connected Ti unit");
                }
            } else {
                System.out.println("Usage expander.jar recv <ExpanderIP> <TiVar>");
                System.out.println("Usage expander.jar send <ExpanderIP> <TiVar> <bool:SendToTi>");
            }

            return;
        }


        GUI.getInstance().createGUI();

        while (true) {
            if (!ISRlocked) {
                ExpanderClient.getInstance().loop();
            }
            Utils.delay(200);
        }

    }
}