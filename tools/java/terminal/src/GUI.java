import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.io.File;
import java.util.Locale;

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
        actionPane.add(new TermButton("send", new TermButtonAction() {
            @Override
            public void run() {
                buildSendingPanel();
            }
        }, true, null));
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
                if (ch == '\n') {
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

    protected String openFileDlg(String currentFile) {
        final JFileChooser fc = new JFileChooser();
        fc.setCurrentDirectory(new File(Config.getInstance().getLastDir()));
        if (currentFile != null) {
            File f = new File(currentFile);
            if (f.exists()) {
                fc.setSelectedFile(f);
            }
        }

        int returnVal = fc.showOpenDialog(win);

        if (returnVal == JFileChooser.APPROVE_OPTION) {
            File file = fc.getSelectedFile();
            //This is where a real application would open the file.
            addTextToConsole("Opening: " + file.getName() + "." + "\n");
            return file.getPath();
        } else {
            addTextToConsole("Open command cancelled by user." + "\n");
        }
        return null;
    }

    protected boolean isNativeFormatFile(String currentFile) {
        if (currentFile == null) {
            return false;
        }
        String lname = currentFile.toLowerCase().trim();
        if (lname.isEmpty()) {
            return false;
        }
        String lnameWoLastPart = lname.substring(0, lname.length() - 1);
        if (lnameWoLastPart.endsWith(".92") || lnameWoLastPart.endsWith(".9x") || lnameWoLastPart.endsWith(".v2") || lnameWoLastPart.endsWith(".89")) {
            // regular Texas Variable Files
            return true;
        }
        // own storage
        return false;
    }

    public void buildSendingPanel() {
        JOptionPane optPane = new JOptionPane("Send to Expander", JOptionPane.INFORMATION_MESSAGE, JOptionPane.OK_CANCEL_OPTION);
        final JDialog dlg = optPane.createDialog(win, "Sending to Expander");
        Container panel = dlg.getRootPane();
        panel.removeAll();
        panel.setLayout(new GridLayout(-1, 2));

        final JCheckBox nativeFile = new JCheckBox();
        final JCheckBox sendToTiToo = new JCheckBox();
        final JTextField fileField = new JTextField("file to send ...");


        final TermButton okBtn = new TermButton("OK", new TermButtonAction() {
            @Override
            public void run() {
                addTextToConsole("OK will send " + fileField.getText().trim());
                dlg.setVisible(false);
            }
        }, true, "#999966");


        Runnable fileChecker = new Runnable() {
            @Override
            public void run() {
                String choosenFile = fileField.getText();
                if (choosenFile == null && !choosenFile.trim().isEmpty()) {
                    nativeFile.setSelected(false);
                    sendToTiToo.setSelected(false);
                    okBtn.setEnabled(false);
                } else {
                    nativeFile.setSelected(isNativeFormatFile(choosenFile));
                    sendToTiToo.setSelected(false); // could save a default value
                    okBtn.setEnabled(true);
                }
            }
        };

        fileField.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                super.mouseClicked(e);
                String choosenFile = openFileDlg(fileField.getText());
                if (choosenFile != null) {
                    fileField.setText(choosenFile);
                    Config.getInstance().setLastDir( new File(choosenFile).getParent() );
                }
                fileChecker.run();
            }
        });
        fileField.addFocusListener(new FocusAdapter() {
            @Override
            public void focusLost(FocusEvent e) {
                super.focusLost(e);
                fileChecker.run();
            }
        });

        okBtn.setEnabled(false);

        panel.add(new JLabel("File"));
        panel.add(fileField);
        panel.add(new JLabel("Native ?"));
        panel.add(nativeFile);
        panel.add(new JLabel("Send to Ti too ?"));
        panel.add(sendToTiToo);

        panel.add(okBtn);
        panel.add(new TermButton("CANCEL", new TermButtonAction() {
            @Override
            public void run() {
                dlg.setVisible(false);
            }
        }));
        dlg.setVisible(true);
    }


}
