import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyAdapter;
import java.awt.event.KeyEvent;

public class GUI {

    protected JFrame win = null;
    protected JTextField hostField;
    protected JTextArea output;
    protected JTextField editLine;

    protected static GUI instance;

    protected GUI() {
    }

    public static GUI getInstance() {
        if (instance == null) {
            instance = new GUI();
        }
        return instance;
    }

    public String getHostValue() {
        return hostField.getText() == null ? null : hostField.getText().trim();
    }

    public void addTextToConsole(String txt) {
        output.append(txt);
        String curText = output.getText();
        int lastCur = curText == null ? 0 : curText.length();
        output.setSelectionStart(lastCur);
        output.setSelectionEnd(lastCur);
    }


    public JFrame createGUI() {
        win = new JFrame("XtsTiExpanderV2 Terminal v1.0");

        hostField = new JTextField(Config.getInstance().getLastHostname());

        JPanel mainPane = new JPanel();
        mainPane.setLayout(new BorderLayout());

        JPanel actionPane = new JPanel();
        final TermButton connBtn = new TermButton("connect", new TermButtonAction() {
            @Override
            public void run() {
                try {
                    if ("connect".equals(getSource().getText())) {
                        ExpanderClient.getInstance().connect(getHostValue());
                        getSource().setText("disconnect");
                    } else {
                        ExpanderClient.getInstance().disconnect();
                        getSource().setText("connect");
                    }
                } catch (Exception ex) {
                    addTextToConsole(ex.toString());
                }
            }
        }, true, null);
        actionPane.add(connBtn);

        actionPane.add(hostField);
        actionPane.add(new TermButton("terminal", new TermButtonAction() {
            @Override
            public void run() {

            }
        }, true, "red"));
        actionPane.add(new TermButton("send", null));
        actionPane.add(new TermButton("recv", null));
        actionPane.add(new TermButton("quit", new TermButtonAction() {
            @Override
            public void run() {
                // FIXME : better
                System.exit(0);
            }
        }, false, "#7799AA"));
        mainPane.add(actionPane, BorderLayout.NORTH);

        output = new JTextArea(25, 80);
        JScrollPane scrollPane = new JScrollPane(output);
        mainPane.add(scrollPane, BorderLayout.CENTER);

        editLine = new JTextField();
        editLine.addKeyListener(new KeyAdapter() {
            @Override
            public void keyTyped(KeyEvent e) {
                super.keyTyped(e);
                char ch = e.getKeyChar();
                ExpanderClient.getInstance().write(ch);
                if ( ch == '\n' ) {
                    editLine.setText("");
                }
            }
        });
        mainPane.add(editLine, BorderLayout.SOUTH);

        win.getContentPane().add(mainPane);


        win.pack();
        win.setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
        win.setVisible(true);
        return win;
    }
    // ********************************************************

    protected abstract class TermButtonAction implements Runnable {
        protected TermButton source = null;

        protected void setSource(TermButton source) {
            this.source = source;
        }

        protected TermButton getSource() {
            return source;
        }
    }

    protected class TermButton extends JButton {
        public TermButton(String text, final TermButtonAction action) {
            this(text, action, false, null);
        }

        public TermButton(String text, final TermButtonAction action, final boolean threadAction, String color) {
            super(color == null ? text : "<html><b color=\"" + color + "\">" + text + "</b></html>");
            if (action != null) {
                action.setSource(this);
                addActionListener(new ActionListener() {
                    @Override
                    public void actionPerformed(ActionEvent e) {
                        if (threadAction) {
                            new Thread(action).start();
                        } else {
                            action.run();
                        }
                    }
                });
            }
        }
    }


}
