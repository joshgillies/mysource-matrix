package ij.io;
import java.awt.*;
import java.io.*;
import javax.swing.*;
import javax.swing.filechooser.*;
import ij.*;
import ij.plugin.frame.Recorder;
import ij.util.Java2;

/** This class displays a dialog window from 
	which the user can save a file. */ 
public class SaveDialog {

	private String dir;
	private String name;
	private String title;
	
	/** Displays a file save dialog with 'title' as the 
		title, 'defaultName' as the initial file name, and
		'extension' (e.g. ".tif") as the default extension.
	*/
	public SaveDialog(String title, String defaultName, String extension) {
		this.title = title;
		String defaultDir = OpenDialog.getDefaultDirectory();
		defaultName = addExtension(defaultName, extension);
		jsave(title, defaultDir, defaultName);
		if (name!=null && dir!=null)
			OpenDialog.setDefaultDirectory(dir);
		IJ.showStatus(title+": "+dir+name);
	}
	
	/** Displays a file save dialog, using the specified 
		default directory and file name and extension. */
	public SaveDialog(String title, String defaultDir, String defaultName, String extension) {
		this.title = title;
		defaultName = addExtension(defaultName, extension);
		jsave(title, defaultDir, defaultName);
		IJ.showStatus(title+": "+dir+name);
	}
	
	boolean isMacro() {
		return false;
	}
	
	String addExtension(String name, String extension) {
		if (name!=null && extension!=null) {
			int dotIndex = name.lastIndexOf(".");
			if (dotIndex>=0)
				name = name.substring(0, dotIndex) + extension;
			else
				name += extension;
		}
		return name;
	}
	
	// Save using JFileChooser
	void jsave(String title, String defaultDir, String defaultName) {
		Java2.setSystemLookAndFeel();
		JFileChooser fc = new JFileChooser();
		if (defaultDir!=null) {
			File f = new File(defaultDir);
			if (f!=null)
				fc.setCurrentDirectory(f);
		}
		if (defaultName!=null)
			fc.setSelectedFile(new File(defaultName));
		int returnVal = fc.showSaveDialog(IJ.getInstance());
		if (returnVal!=JFileChooser.APPROVE_OPTION)
			{ return;}
		File f = fc.getSelectedFile();
		if(f.exists()) {
			int ret = JOptionPane.showConfirmDialog (fc,
				"The file "+ f.getName() + " already exists, \nwould you like to overwrite it?",
				"Overwrite?",
				JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE);
			if (ret!=JOptionPane.OK_OPTION) f = null;
		}
		dir = fc.getCurrentDirectory().getPath()+File.separator;
		name = fc.getName(f);
	}
	
	/** Returns the selected directory. */
	public String getDirectory() {
		return dir;
	}
	
	/** Returns the selected file name. */
	public String getFileName() {
		if (Recorder.record)
			Recorder.recordPath(title, dir+name);
		return name;
	}
	
}
