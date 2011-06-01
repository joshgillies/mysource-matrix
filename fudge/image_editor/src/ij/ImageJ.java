package ij;

import java.awt.*;
import java.util.*;
import java.awt.event.*;
import java.io.*;
import java.net.URL;
import java.awt.image.*;
import ij.gui.*;
import ij.process.*;
import ij.io.*;
import ij.plugin.*;
import ij.plugin.filter.*;
import ij.text.*;
import javax.swing.*;
import java.applet.*; 
import java.awt.*;

/**
*
* IMAGEJ IMAGE UPLOAD APPLET
* 
* This version of ImageJ was modified by Tom Barrett for squiz.net, for use
* as a single-frame image editing applet for CMSes.  The original intro
* comment for the ImageJ application is below:
* 
*   ImageJ is a work of the United States Government. It is in the public domain 
*   and open source. There is no copyright. You are free to do anything you want 
*   with this source but I like to get credit for my work and I would like you to 
*   offer your changes to me so I can possibly add them to the "official" version.
* 
* @author Wayne Rasband (wayne@codon.nih.gov)
* @author Tom Barrett (tbarrett@squiz.net)
*/

public class ImageJ extends javax.swing.JApplet implements ActionListener, 
	MouseListener, KeyListener, ItemListener {

	public static final String VERSION = "1.32j";
	public static Color backgroundColor = new Color(200, 189, 203); //224,226,235

	private static final String IJ_X="ij.x",IJ_Y="ij.y";
	private static final String RESULTS_X="results.x",RESULTS_Y="results.y",
		RESULTS_WIDTH="results.width",RESULTS_HEIGHT="results.height";
	
	private Toolbar toolbar;
	private JPanel statusBar;
	private ImageCanvas imageCanvas;
	private ProgressBar progressBar;
	private JButton submitButton = new JButton("Commit");
	private JPanel mainPane;
	private Label statusLine;
	private boolean firstTime = true;
	private Vector classes = new Vector();
	private boolean exitWhenQuiting;
	private boolean quitting;
	private long keyPressedTime, actionPerformedTime;
	boolean hotkey;
	private JScrollPane imageScrollPane;

	/* Text field to enter the name for the asset */
	private JTextField assetNameField;

	/* Dropdown box to select the filetype */
	private JComboBox fileTypeField;
	
	/* Length of asset name field */
	private static final int NAME_FIELD_LENGTH = 20;

	/** Creates a new ImageJ frame. */
	public ImageJ() {
		super();
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}
		
		String err1 = Prefs.load(this, null);
		
		mainPane = new JPanel(new BorderLayout());

		// Tool bar
		toolbar = new Toolbar();
		toolbar.setBackground(backgroundColor);
		toolbar.addKeyListener(this);
		JPanel jp = new JPanel();
		jp.setLayout(new BorderLayout());
		jp.add(toolbar, BorderLayout.CENTER);
		mainPane.add(jp, BorderLayout.NORTH);

		
		// Status bar
		statusBar = new JPanel();
		statusBar.setLayout(new BorderLayout());
		statusBar.setForeground(Color.black);
		statusBar.setBackground(backgroundColor);
		statusLine = new Label();
		statusLine.addKeyListener(this);
		statusLine.addMouseListener(this);
		statusBar.add("Center", statusLine);
		progressBar = new ProgressBar(100, 18);
		progressBar.addKeyListener(this);
		progressBar.addMouseListener(this);
		statusBar.add("East", progressBar);

		statusBar.setSize(toolbar.getPreferredSize());
		mainPane.add(statusBar, BorderLayout.SOUTH);
		mainPane.addKeyListener(this);

		Menus m = new Menus(this, null);
		String err2 = m.addMenuBar();
		m.installPopupMenu(this);

		IJ.init(this, null);
 		addKeyListener(this);

		imageScrollPane = new JScrollPane();
		mainPane.add(imageScrollPane, BorderLayout.CENTER);

		mainPane.setBorder(BorderFactory.createMatteBorder(0, 1, 1, 1, Color.BLACK));
		((JComponent)(getContentPane())).setBorder(BorderFactory.createMatteBorder(1, 0, 0, 0, new Color(172, 168, 153)));

		getContentPane().add(mainPane, BorderLayout.CENTER);

		// Submit button and name field
		JPanel bottomPanel = new JPanel(new BorderLayout());
		bottomPanel.setBorder(BorderFactory.createEmptyBorder(4,2,2,2));
		bottomPanel.setBackground(Color.WHITE);

		JPanel submitPanel = new JPanel();
		String showSubmitParam = null;
		try { showSubmitParam = getParameter("SHOW_SUBMIT"); } catch (Exception e) {}
		if (showSubmitParam != null) {
			submitPanel.add(submitButton, BorderLayout.EAST);
			submitPanel.setBorder(BorderFactory.createEmptyBorder());
			submitPanel.setBackground(Color.WHITE);
		}

		JPanel namePanel = new JPanel();
		String[] typeOptions = {".jpg", ".gif"};
 		assetNameField = new JTextField(NAME_FIELD_LENGTH);
		fileTypeField = new JComboBox(typeOptions);
		fileTypeField.setSize(100, assetNameField.getHeight());
		JLabel nameLabel = new JLabel("File Name: ");
		nameLabel.setFont(nameLabel.getFont().deriveFont(Font.BOLD));
		namePanel.add(nameLabel);
		namePanel.add(assetNameField);
		namePanel.add(fileTypeField);
		namePanel.setBackground(Color.WHITE);
		namePanel.setBorder(BorderFactory.createEmptyBorder());
		
		bottomPanel.add(namePanel, BorderLayout.WEST);
		if (showSubmitParam != null) bottomPanel.add(submitPanel, BorderLayout.EAST);
		getContentPane().add(bottomPanel, BorderLayout.SOUTH);

		setCursor(Cursor.getDefaultCursor()); // work-around for JDK 1.1.8 bug
		setVisible(true);
		if (err1!=null)
			IJ.error(err1);
		if (err2!=null)
			IJ.error(err2);
		if (IJ.isJava2()) {
			IJ.runPlugIn("ij.plugin.DragAndDrop", "");
		}
	}

	public void init() {
		try {
			submitButton.addActionListener(new ServerSubmitter(this));
		} catch (Exception e) {
			JOptionPane.showMessageDialog(ImageJ.this, "Problem adding action listener for submit button", "Error", JOptionPane.WARNING_MESSAGE);
		}
		try {
			assetNameField.addFocusListener(new FocusListener() {
				public void focusGained(FocusEvent ev) {
					updateFileTypes();
				}
				public void focusLost(FocusEvent ev) {}
			});
		} catch (Exception e) {
			JOptionPane.showMessageDialog(ImageJ.this, "Problem adding focus listener for name field", "Error", JOptionPane.WARNING_MESSAGE);
		}
		
		String openURL = null;
		if ((openURL = getParameter("OPEN_URL")) != null) {
			try {
				setImagePlus(new ImagePlus(openURL));
				String filename = openURL.substring(openURL.lastIndexOf("/")+1);
				int queryBegin = filename.indexOf("?");
				if (queryBegin != -1) {
					filename = filename.substring(0, queryBegin);
				}
				if (filename.toLowerCase().endsWith(".gif")) {
					fileTypeField.setSelectedItem(".gif");
				} else {
					fileTypeField.setSelectedItem(".jpg");
				}
				assetNameField.setText(filename.substring(0, filename.lastIndexOf(".")));
			} catch (Exception e) {

			}
		}
	}
    	

	public void showStatus(String s) {
        statusLine.setText(s);
	}

	public ProgressBar getProgressBar() {
        return progressBar;
	}

    /** Starts executing a menu command in a separate thread. */
    void doCommand(String name) {
		new Executer(name, IJ.getInstance().getImagePlus());
    }
        
	public void runFilterPlugIn(Object theFilter, String cmd, String arg) {
		IJ.runFilterPlugIn(theFilter, cmd, arg);
	}
        
	public Object runUserPlugIn(String commandName, String className, String arg, boolean createNewLoader) {
		return IJ.runUserPlugIn(commandName, className, arg, createNewLoader);	
	} 
	
	/** Return the current list of modifier keys. */
	public static String modifiers(int flags) { //?? needs to be moved
		String s = " [ ";
		if (flags == 0) return "";
		if ((flags & Event.SHIFT_MASK) != 0) s += "Shift ";
		if ((flags & Event.CTRL_MASK) != 0) s += "Control ";
		if ((flags & Event.META_MASK) != 0) s += "Meta ";
		if ((flags & Event.ALT_MASK) != 0) s += "Alt ";
		s += "] ";
		return s;
	}

	/** Handle menu events. */
	public void actionPerformed(ActionEvent e) {
		if ((e.getSource() instanceof JMenuItem)) {
			JMenuItem item = (JMenuItem)e.getSource();
			String cmd = e.getActionCommand();
			hotkey = false;
			actionPerformedTime = System.currentTimeMillis();
			long ellapsedTime = actionPerformedTime-keyPressedTime;
			if (cmd!=null && ellapsedTime>=10L)
				doCommand(cmd);
			if (IJ.debugMode) IJ.log("actionPerformed: "+ellapsedTime+" "+e);
		}
	}

	/** Handles CheckboxMenuItem state changes. */
	public void itemStateChanged(ItemEvent e) {
		JMenuItem item = (JMenuItem)e.getSource();
		JComponent parent = (JComponent)item.getParent();
		String cmd = ((JCheckBoxMenuItem)(e.getItem())).getText().toString();
		doCommand(cmd);
	}

	public void mousePressed(MouseEvent e) {
		Undo.reset();
		IJ.showStatus("Memory: "+IJ.freeMemory());
	}
	
	public void mouseReleased(MouseEvent e) {}
	public void mouseExited(MouseEvent e) {}
	public void mouseClicked(MouseEvent e) {}
	public void mouseEntered(MouseEvent e) {}

 	public void keyPressed(KeyEvent e) {
		int keyCode = e.getKeyCode();
		IJ.setKeyDown(keyCode);
		hotkey = false;
		if (keyCode==e.VK_CONTROL || keyCode==e.VK_SHIFT)
			return;
		char keyChar = e.getKeyChar();
		int flags = e.getModifiers();
		if (IJ.debugMode) IJ.log("keyCode=" + keyCode + " (" + KeyEvent.getKeyText(keyCode)
			+ ") keyChar=\"" + keyChar + "\" (" + (int)keyChar + ") "
			+ KeyEvent.getKeyModifiersText(flags));
		boolean shift = (flags & e.SHIFT_MASK) != 0;
		boolean control = (flags & e.CTRL_MASK) != 0;
		boolean alt = (flags & e.ALT_MASK) != 0;
		String c = "";
		ImagePlus imp = IJ.getInstance().getImagePlus();
		boolean isStack = (imp!=null) && (imp.getStackSize()>1);
		
		if (control && (keyCode == e.VK_D)) {
			// hard coding the control-d shortcut until we fix the menu shortcuts
			IJ.run("Draw");
			imp.killRoi();
		}

		if (imp!=null && !control && ((keyChar>=32 && keyChar<=255) || keyChar=='\b' || keyChar=='\n')) {
			Roi roi = imp.getRoi();
			if (roi instanceof TextRoi) {
				if ((flags & e.META_MASK)!=0 && IJ.isMacOSX()) return;
				if (alt)
					switch (keyChar) {
						case 'u': case 'm': keyChar = IJ.micronSymbol; break;
						case 'A': keyChar = IJ.angstromSymbol; break;
						default:
					}
				((TextRoi)roi).addChar(keyChar);
				return;
			}
		}
        		
		Hashtable shortcuts = Menus.getShortcuts();
		if (shift)
			c = (String)shortcuts.get(new Integer(keyCode+200));
		else
			c = (String)shortcuts.get(new Integer(keyCode));
		
		if (c==null)
			switch(keyCode) {
				case KeyEvent.VK_BACK_SPACE: c="Clear"; hotkey=true; break; // delete
				case KeyEvent.VK_EQUALS: case 0xbb: c="Start Animation [=]"; break;
				case KeyEvent.VK_SLASH: case 0xbf: c="Reslice [/]..."; break;
				case KeyEvent.VK_COMMA: case 0xbc: c="Previous Slice [<]"; break;
				case KeyEvent.VK_PERIOD: case 0xbe: c="Next Slice [>]"; break;
				case KeyEvent.VK_LEFT: case KeyEvent.VK_RIGHT: case KeyEvent.VK_UP: case KeyEvent.VK_DOWN: // arrow keys
					Roi roi = null;
					if (imp!=null) roi = imp.getRoi();
					if (roi==null) return;
					if ((flags & KeyEvent.ALT_MASK) != 0)
						roi.nudgeCorner(keyCode);
					else
						roi.nudge(keyCode);
					return;
				default: break;
			}
		if (c!=null && !c.equals("")) {
			if (c.equals("Fill"))
				hotkey = true;
			else {
				doCommand(c);
				keyPressedTime = System.currentTimeMillis();
			}
		}
	}

	public void keyReleased(KeyEvent e) {
		IJ.setKeyUp(e.getKeyCode());
	}
		
	public void keyTyped(KeyEvent e) {}

	public void register(Class c) {
		if (!classes.contains(c))
			classes.addElement(c);
	}

	public ImagePlus getImagePlus() {
		if (imageCanvas != null)
			return imageCanvas.getImagePlus();
		else
			return null;
	}

	public void setImagePlus(ImagePlus ip) {
		mainPane.remove(imageScrollPane);
		imageCanvas = new ImageCanvas(ip);
		ip.setCanvas(imageCanvas);
		imageCanvas.addKeyListener(this);
		imageCanvas.setFocusable(true);
		imageScrollPane = new JScrollPane(imageCanvas);
		mainPane.add(imageScrollPane, BorderLayout.CENTER);
		updateFileTypes();
		getContentPane().validate();
		getContentPane().repaint();
		requestFocus();
	}

	public void clearImagePlus() {
		getContentPane().remove(imageScrollPane);
		imageCanvas = null;
		getContentPane().validate();
	}

	public ImageCanvas getImageCanvas() {
		return imageCanvas;
	}

	/**
	* Get the asset name that the user typed
	*/
	String getAssetName() 
	{
		return assetNameField.getText();
	
	}//end getAssetName();


	/**
	* Get the file type that the user selected
	*/
	String getFileType() 
	{
		return (String)fileTypeField.getSelectedItem();

	}//end getFileType()

	/**
	* Enable or disable the .gif option according to the current image
	*/	
	void updateFileTypes()
	{
		if (getImagePlus() == null) return;
		if (!FileSaver.okForGif(getImagePlus())) {
			if (fileTypeField.getItemCount() > 1) {
				fileTypeField.removeItem(".gif");
			}
		} else {
			if (fileTypeField.getItemCount() < 2) {
				fileTypeField.addItem(".gif");
			}
		}
	}//end updateFileTypes()

	public void setCurrentType(String type)
	{
		fileTypeField.setSelectedItem(type);
	}

	public Dimension getViewportSize()
	{
		return imageScrollPane.getViewport().getExtentSize();
	}

	public String doUpload()
	{
		ServerSubmitter ss = new ServerSubmitter(this);
		ss.actionPerformed(null);
		return ss.getTempFileName();
	}

	public String getFilename()
	{
		return getAssetName() + getFileType();
	}

	public void setFilename(String path)
	{
		String baseName = new File(path).getName();
		if (-1 != baseName.toLowerCase().indexOf(".gif")) {
			setCurrentType(".gif");
		} else {
			setCurrentType(".jpg");
		}
		if (-1 != baseName.lastIndexOf(".")) {
			baseName = baseName.substring(0, baseName.lastIndexOf("."));
		}
		assetNameField.setText(baseName);
	}
	
	public boolean isFocusable() 
	{
		return true;
	}
	
} //class ImageJ
