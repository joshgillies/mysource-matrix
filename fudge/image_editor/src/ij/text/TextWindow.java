package ij.text;

import java.awt.*;
import java.io.*;
import java.awt.event.*;
import ij.*;
import ij.io.*;
import ij.gui.*;
import ij.plugin.filter.Analyzer;
import javax.swing.*;

/** Uses a TextPanel to displays text in a window.
	@see TextPanel
*/
public class TextWindow extends JFrame implements ActionListener, FocusListener {

	private TextPanel textPanel;

	/**
	Opens a new single-column text window.
	@param title	the title of the window
	@param str		the text initially displayed in the window
	@param width	the width of the window in pixels
	@param height	the height of the window in pixels
	*/
	public TextWindow(String title, String data, int width, int height) {
		this(title, "", data, width, height);
	}

	/**
	Opens a new multi-column text window.
	@param title	the title of the window
	@param headings	the tab-delimited column headings
	@param data		the text initially displayed in the window
	@param width	the width of the window in pixels
	@param height	the height of the window in pixels
	*/
	public TextWindow(String title, String headings, String data, int width, int height) {
		super(title);
		enableEvents(AWTEvent.WINDOW_EVENT_MASK);
		textPanel = new TextPanel(title);
		textPanel.setTitle(title);
		getContentPane().add(textPanel, BorderLayout.CENTER);
		textPanel.setColumnHeadings(headings);
		if (data!=null && !data.equals(""))
			textPanel.append(data);
		System.out.println("Data was "+data);
		ImageJ ij = IJ.getInstance();
		if (ij!=null) {
			addKeyListener(ij);
		}
 		addFocusListener(this);
 		addMenuBar();
		setSize(width, height);
		GUI.center(this);
		show();
	}

	/**
	Opens a new text window containing the contents
	of a text file.
	@param path		the path to the text file
	@param width	the width of the window in pixels
	@param height	the height of the window in pixels
	*/
	public TextWindow(String path, int width, int height) {
		super("");
		enableEvents(AWTEvent.WINDOW_EVENT_MASK);
		textPanel = new TextPanel();
		getContentPane().add(textPanel, BorderLayout.CENTER);
		if (openFile(path)) {
			setSize(width, height);
			show();
		} else
			dispose();
	}
	
	void addMenuBar() {
		JMenuBar mb = new JMenuBar();
		JMenu m = new JMenu("File");
		m.add(new JMenuItem("Save As..."/*, new MenuShortcut(KeyEvent.VK_S)*/));
		m.addActionListener(this);
		mb.add(m);
		m = new JMenu("Edit");
		m.add(new JMenuItem("Cut"/*, new MenuShortcut(KeyEvent.VK_X)*/));
		m.add(new JMenuItem("Copy"/*, new MenuShortcut(KeyEvent.VK_C)*/));
		m.add(new JMenuItem("Copy All"));
		m.add(new JMenuItem("Clear"));
		m.add(new JMenuItem("Select All"/*, new MenuShortcut(KeyEvent.VK_A)*/));
		if (getTitle().equals("Results")) {
			m.addSeparator();
			m.add(new JMenuItem("Clear Results"));
			m.add(new JMenuItem("Summarize"));
			m.add(new JMenuItem("Set Measurements..."));
		}
		m.addActionListener(this);
		mb.add(m);
		setJMenuBar(mb);
	}

	/**
	Adds one or lines of text to the window.
	@param text		The text to be appended. Multiple
					lines should be separated by \n.
	*/
	public void append(String text) {
		textPanel.append(text);
	}
	
	/** Set the font that will be used to display the text. */
	public void setFont(Font font) {
		textPanel.setFont(font);
	}
  
	boolean openFile(String path) {
		OpenDialog od = new OpenDialog("Open Text File...", path);
		String directory = od.getDirectory();
		String name = od.getFileName();
		if (name==null)
			return false;
		path = directory + name;
		
		IJ.showStatus("Opening: " + path);
		try {
			BufferedReader r = new BufferedReader(new FileReader(directory + name));
			load(r);
			r.close();
		}
		catch (Exception e) {
			IJ.error(e.getMessage());
			return true;
		}
		textPanel.setTitle(name);
		setTitle(name);
		IJ.showStatus("");
		return true;
	}
	
	/** Returns a reference to this TextWindow's TextPanel. */
	public TextPanel getTextPanel() {
		return textPanel;
	}

	/** Appends the text in the specified file to the end of this TextWindow. */
	public void load(BufferedReader in) throws IOException {
		int count=0;
		while (true) {
			String s=in.readLine();
			if (s==null) break;
			textPanel.appendLine(s);
		}
	}

	public void actionPerformed(ActionEvent evt) {
		String cmd = evt.getActionCommand();
		textPanel.doCommand(cmd);
	}

	public void processWindowEvent(WindowEvent e) {
		super.processWindowEvent(e);
		int id = e.getID();
		if (id==WindowEvent.WINDOW_CLOSING)
			close();	
	}

	public void close() {
		if (getTitle().equals("Results")) {
			if (!Analyzer.resetCounter())
				return;
			IJ.setTextPanel(null);
		}
		if (getTitle().equals("Log")) {
			IJ.debugMode = false;
			IJ.log("$Closed");
		}
		setVisible(false);
		dispose();
		textPanel.flush();
	}
	
	public void focusGained(FocusEvent e) {
	}

	public void focusLost(FocusEvent e) {}

}