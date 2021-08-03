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
        GUI.getInstance().createGUI();

        while (true) {
            if (!ISRlocked) {
                ExpanderClient.getInstance().loop();
            }
            Utils.delay(200);
        }

    }
}