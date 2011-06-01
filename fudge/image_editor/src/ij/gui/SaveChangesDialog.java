package ij.gui;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

/** A modal dialog box with a one line message and
	"Don't Save", "Cancel" and "Save" buttons. */
public class SaveChangesDialog extends JDialog implements ActionListener {
	private Button dontSave, cancel, save;
	private boolean cancelPressed, savePressed;

	public SaveChangesDialog(JApplet parent, String fileName) {
		super(new JFrame(), "Save?", true);
		setLayout(new BorderLayout());
		Panel panel = new Panel();
		panel.setLayout(new FlowLayout(FlowLayout.LEFT, 10, 10));
		Component message;
		if (fileName.startsWith("Save "))
			message = new Label(fileName);
		else {
			if (fileName.length()>22)
				message = new MultiLineLabel("Save changes to\n" + "\"" + fileName + "\"?");
			else
				message = new Label("Save changes to \"" + fileName + "\"?");
		}
		message.setFont(new Font("Dialog", Font.BOLD, 12));
		panel.add(message);
		add("Center", panel);
		
		panel = new Panel();
		panel.setLayout(new FlowLayout(FlowLayout.CENTER, 8, 8));
		save = new Button("  Save  ");
		save.addActionListener(this);
		cancel = new Button("  Cancel  ");
		cancel.addActionListener(this);
		dontSave = new Button("Don't Save");
		dontSave.addActionListener(this);
		if (ij.IJ.isMacintosh()) {
			panel.add(dontSave);
			panel.add(cancel);
			panel.add(save);
		} else {
			panel.add(save);
			panel.add(dontSave);
			panel.add(cancel);
		}
		add("South", panel);
		if (ij.IJ.isMacintosh())
			setResizable(false);
		pack();
		GUI.center(this);
		show();
	}
    
	public void actionPerformed(ActionEvent e) {
		if (e.getSource()==cancel)
			cancelPressed = true;
		else if (e.getSource()==save)
			savePressed = true;
		setVisible(false);
		dispose();
	}
	
	/** Returns true if the user dismissed dialog by pressing "Cancel". */
	public boolean cancelPressed() {
		return cancelPressed;
	}
	
	/** Returns true if the user dismissed dialog by pressing "Save". */
	public boolean savePressed() {
		return savePressed;
	}
	
}
