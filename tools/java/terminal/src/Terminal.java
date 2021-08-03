/**
 * WiFi terminal for
 * XtsTiExpanderV2
 * <p>
 * Xtase - fgalliat @Aug2021
 */

public class Terminal {


    public static void main(String[] args) throws Exception {
        GUI.getInstance().createGUI();

        while (true) {
            ExpanderClient.getInstance().loop();
            Utils.delay(200);
        }

    }
}