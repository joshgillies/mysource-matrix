package ij.io;
import ij.*;
import ij.gui.*;
import ij.plugin.frame.Recorder;
import ij.util.Java2;
import java.awt.*;
import java.io.*;
import javax.swing.*;
import javax.swing.filechooser.*;

/** This class displays a dialog window from 
	which the user can select an input file. */ 
 public class OpenDialog {

	private String dir;
	private String name;
	private boolean recordPath;
	private static String defaultDirectory;
	private static Frame sharedFrame;
	private String title;
	
	/** Displays a file open dialog with 'title' as
		the title. If 'path' is non-blank, it is
		used and the dialog is not displayed. Uses
		and updates the ImageJ default directory. */
	public OpenDialog(String title, String path) {
		if (path==null || path.equals("")) {
			jOpen(title, getDefaultDirectory(), null);
			if (name!=null) defaultDirectory = dir;
			this.title = title;
			recordPath = true;
		} else {
			decodePath(path);
		}
		IJ.register(OpenDialog.class);
	}
	
	/** Displays a file open dialog, using the specified 
		default directory and file name. */
	public OpenDialog(String title, String defaultDir, String defaultName) {
		String path = null;
		if (path!=null)
			decodePath(path);
		else {
			jOpen(title, defaultDir, defaultName);
			this.title = title;
			recordPath = true;
		}
	}
	
	// Uses the JFileChooser class to display the dialog box
	void jOpen(String title, String path, String fileName) {
		Java2.setSystemLookAndFeel();
		JFileChooser fc = new JFileChooser();
		File fdir = null;
		if (path!=null)
			fdir = new File(path);
		if (fdir!=null)
			fc.setCurrentDirectory(fdir);
		if (fileName!=null)
			fc.setSelectedFile(new File(fileName));
		int returnVal = fc.showOpenDialog(null);
		if (returnVal!=JFileChooser.APPROVE_OPTION)
			{ return;}
		File file = fc.getSelectedFile();
		if (file==null)
			{ return;}
		name = file.getName();
		dir = fc.getCurrentDirectory().getPath()+File.separator;
	}
	

	void decodePath(String path) {
		int i = path.lastIndexOf('/');
		if (i==-1)
			i = path.lastIndexOf('\\');
		if (i>0) {
			dir = path.substring(0, i+1);
			name = path.substring(i+1);
		} else {
			dir = "";
			name = path;
		}
	}

	/** Returns the selected directory. */
	public String getDirectory() {
		return dir;
	}
	
	/** Returns the selected file name. */
	public String getFileName() {
		if (Recorder.record && recordPath)
			Recorder.recordPath(title, dir+name);
		return name;
	}
		
	/** Returns the current working directory, which my be null. */
	public static String getDefaultDirectory() {
		if (defaultDirectory==null)
			defaultDirectory = Prefs.getString(Prefs.DIR_IMAGE);
		return defaultDirectory;
	}

	public static void setDefaultDirectory(String defaultDir) {
		defaultDirectory = defaultDir;
		IJ.register(OpenDialog.class);
	}

}
