package ij;

import java.awt.*;
import java.net.URL;
import java.awt.image.*;
import java.io.*;
import java.util.*;
import java.awt.event.KeyEvent;
import ij.io.*;
import ij.process.*;
import ij.gui.*;
import ij.util.*;
import ij.text.TextWindow;
import ij.plugin.frame.*;
import javax.swing.*;

/** Runs menu commands in a separate thread.*/
public class Executer implements Runnable {

	private static String previousCommand;

	private String command;
	private ImagePlus iplus;
	private ImageJ ij;
	private Thread thread;
	
	/** Create an Executer to run the specified menu command
		in this thread using the active image. */
	public Executer(String cmd) {
		command = cmd;
		iplus = IJ.getInstance().getImagePlus();
		ij = IJ.getInstance();
	}

	/** Create an Executer that runs the specified menu command
		in a separate thread using the specified image. */
	public Executer(String cmd, ImagePlus imp) {
		iplus = imp;
		if (cmd.startsWith("Repeat"))
			command = previousCommand;
		else {
			command = cmd;
			if (!(cmd.equals("Undo")||cmd.equals("Close")))
				previousCommand = cmd;
		}
			
		ij = IJ.getInstance();
		thread = new Thread(this, cmd);
		thread.setPriority(Math.max(thread.getPriority()-2, Thread.MIN_PRIORITY));
		thread.start();
	}

	public void run() {
		if (command==null) return;
		ImagePlus imp = iplus;
		iplus = null; // maybe this will help get the image GC'd

		try {
			runCommand(command, imp);
		} catch(Throwable e) {
			IJ.showStatus("");
			IJ.showProgress(1.0);
			if (imp!=null) imp.unlock();
			String msg = e.getMessage();
			if (e instanceof OutOfMemoryError)
				IJ.outOfMemory(command);
			else if (e instanceof RuntimeException && msg!=null && msg.equals("Macro canceled"))
				; //do nothing
			else {
				CharArrayWriter caw = new CharArrayWriter();
				PrintWriter pw = new PrintWriter(caw);
				e.printStackTrace(pw);
				String s = caw.toString();
				if (IJ.isMacintosh()) {
					if (s.indexOf("ThreadDeath")>0)
						return;
					s = Tools.fixNewLines(s);
				}
				JOptionPane.showMessageDialog(IJ.getInstance(), s, "Exception", JOptionPane.ERROR_MESSAGE);
			}
		}
	}
	
	
	void save(String s) {
		PrintWriter pw = null;
		try {
			FileOutputStream fos = new FileOutputStream("exception.txt");
			BufferedOutputStream bos = new BufferedOutputStream(fos);
			pw = new PrintWriter(bos);
		}
		catch (IOException e) {
			IJ.error("" + e);
			return;
		}
		pw.println(s);
		pw.close();
	}
	

    public void runCommand(String cmd, ImagePlus imp) {

		if (cmd.equals("New..."))
			new NewImage();
		else if (cmd.equals("Open Local")) {
			new Opener().open();
		} else if (cmd.equals("Close"))
			close(imp);
		else if (cmd.equals("Cut"))
			copy(imp, true);
		else if (cmd.equals("Copy"))
			copy(imp, false);
		else {
			Hashtable table = Menus.getCommands();
			String plugIn = (String)table.get(cmd);
			if (plugIn!=null)
				runPlugIn(cmd, plugIn);
			else
				runImageCommand(cmd, imp);
		}
		ij.getContentPane().repaint();
    }

   /** Run commands that process images. */
    public void runImageCommand(String cmd, ImagePlus imp) {
    	
    	if (imp!=null) {
			if (!imp.lock())
				return;   // exit if image is in use
			ImageCanvas canvas = imp.getCanvas();

			if (cmd.equals("Revert"))
				{imp.revert(); }
			else if (cmd.equals("Save Local"))
				{new FileSaver(imp).save(); }
			else if (cmd.equals("Paste"))
				{ canvas.paste(); }
			else if (cmd.equals("Undo"))
				{ Undo.undo(); }
			else
				IJ.error("Unrecognized command: " + cmd);

			imp.unlock();
		} 
	}

	void runPlugIn(String cmd, String className) {
		String arg = "";
		if (className.endsWith("\")")) {
			// extract string argument (e.g. className("arg"))
			int argStart = className.lastIndexOf("(\"");
			if (argStart>0) {
				arg = className.substring(argStart+2, className.length()-2);
				className = className.substring(0, argStart);
			}
		}
		IJ.runPlugIn(cmd, className, arg);
	}
	
	void roiRequired() {
		IJ.error("Selection required");
	}

	void copy(ImagePlus imp, boolean cut) {
		if (imp==null) {
	 		IJ.noImage();
	 		return;
	 	} else
	 		imp.getCanvas().copy(cut);
	}
	
	void close(ImagePlus imp) {
	}

	/** Returns the last command executed. Returns null
		if no command has been executed. */
	public static String getCommand() {
		return previousCommand;
	}
	
}


