package ij;
import ij.gui.*;
import ij.process.*;
import ij.text.*;
import ij.io.*;
import ij.plugin.*;
import ij.plugin.filter.*;
import ij.util.Tools;
import ij.plugin.frame.Recorder;
import java.awt.event.*;
import java.text.*;
import java.util.Locale;	
import java.awt.*;	
import java.applet.Applet;
import java.io.*;
import java.lang.reflect.*;
import javax.swing.*;

/** This class consists of static utility methods. */
public class IJ {
	public static boolean debugMode = false;
	    
    public static final char micronSymbol = (char)181;
    public static final char angstromSymbol = (char)197;
    public static final char degreeSymbol = (char)176;

	private static ImageJ ij;
	private static java.applet.Applet applet;
	private static ProgressBar progressBar;
	private static TextPanel textPanel;
	private static String osname;
	private static boolean isMac, isWin, isJava2, isJava14;
	private static boolean altDown, spaceDown, shiftDown;
	private static boolean macroRunning;
	private static Thread previousThread;
	private static TextPanel logPanel;
	private static boolean notVerified = true;		
	private static PluginClassLoader classLoader;
	private static boolean memMessageDisplayed;
	private static long maxMemory;
			
	static {
		osname = System.getProperty("os.name");
		isWin = osname.startsWith("Windows");
		isMac = !isWin && osname.startsWith("Mac");
		String version = System.getProperty("java.version").substring(0,3);
		// JVM on Sharp Zaurus PDA claims to be "3.1"!
		isJava2 = version.compareTo("1.1")>0 && version.compareTo("2.9")<=0;
		isJava14 = version.compareTo("1.3")>0 && version.compareTo("2.9")<=0;
	}
			
	static void init(ImageJ imagej, Applet theApplet) {
		ij = imagej;
		applet = theApplet;
		progressBar = ij.getProgressBar();
	}

	/**Returns a reference to the "ImageJ" frame.*/
	public static ImageJ getInstance() {
		return ij;
	}
	
	/** Runs the specified plugin and returns a reference to it. */
	public static Object runPlugIn(String className, String arg) {
		return runPlugIn("", className, arg);
	}
	
	/** Runs the specified plugin and returns a reference to it. */
	static Object runPlugIn(String commandName, String className, String arg) {
		if (IJ.debugMode)
			IJ.log("runPlugin: "+className+" "+arg);
		Object thePlugIn=null;
		try {
			Class c = Class.forName(className);
 			thePlugIn = c.newInstance();
 			if (thePlugIn instanceof PlugIn) {
				((PlugIn)thePlugIn).run(arg);
 			} else {
				runFilterPlugIn(thePlugIn, commandName, arg);
			}
		}
		catch (ClassNotFoundException e) {
			if (IJ.getApplet()==null)
				System.err.println("Plugin not found: " + className);
		}
		catch (InstantiationException e) {log("Unable to load plugin (ins)");}
		catch (IllegalAccessException e) {log("Unable to load plugin, possibly \nbecause it is not public.");}
		return thePlugIn;
	}
	       
	static void runFilterPlugIn(Object theFilter, String cmd, String arg) {
		ImagePlus imp = IJ.getInstance().getImagePlus();
		int capabilities = ((PlugInFilter)theFilter).setup(arg, imp);
		if ((capabilities&PlugInFilter.DONE)!=0)
			return;
		if ((capabilities&PlugInFilter.NO_IMAGE_REQUIRED)!=0)
			{((PlugInFilter)theFilter).run(null); return;}
		if (imp==null)
			{IJ.noImage(); return;}
		if ((capabilities&PlugInFilter.ROI_REQUIRED)!=0 && imp.getRoi()==null)
			{IJ.error("Selection required"); return;}
		int type = imp.getType();
		switch (type) {
			case ImagePlus.GRAY8:
				if ((capabilities&PlugInFilter.DOES_8G)==0)
					{wrongType(capabilities, cmd); return;}
				break;
			case ImagePlus.COLOR_256:
				if ((capabilities&PlugInFilter.DOES_8C)==0)
					{wrongType(capabilities, cmd); return;}
				break;
			case ImagePlus.GRAY16:
				if ((capabilities&PlugInFilter.DOES_16)==0)
					{wrongType(capabilities, cmd); return;}
				break;
			case ImagePlus.GRAY32:
				if ((capabilities&PlugInFilter.DOES_32)==0)
					{wrongType(capabilities, cmd); return;}
				break;
			case ImagePlus.COLOR_RGB:
				if ((capabilities&PlugInFilter.DOES_RGB)==0)
					{wrongType(capabilities, cmd); return;}
				break;
		}
		if (!imp.lock())
			return; // exit if image is in use
		imp.startTiming();
		IJ.showStatus(cmd + "...");
		ImageProcessor ip;
		ImageProcessor mask = null;
		float[] cTable = imp.getCalibration().getCTable();
		ip = imp.getProcessor();
		mask = imp.getMask();
		if ((capabilities&PlugInFilter.NO_UNDO)!=0)
			Undo.reset();
		else {
			Undo.setup(Undo.FILTER, imp);
			ip.snapshot();
		}
		//ip.setMask(mask);
		ip.setCalibrationTable(cTable);
		((PlugInFilter)theFilter).run(ip);
		if ((capabilities&PlugInFilter.SUPPORTS_MASKING)!=0)
			ip.reset(ip.getMask());  //restore image outside irregular roi
		IJ.showTime(imp, imp.getStartTime(), cmd + ": ", 1);
		if ((capabilities&PlugInFilter.NO_CHANGES)==0) {
			imp.changes = true;
	 		imp.updateAndDraw();
	 	}
		imp.unlock();
	}
        
	static Object runUserPlugIn(String commandName, String className, String arg, boolean createNewLoader) {
		if (applet!=null)
			return null;
		String pluginsDir = Menus.getPlugInsPath();
		if (pluginsDir==null)
			return null;
		if (notVerified) {
			// check for duplicate classes in the plugins folder
			IJ.runPlugIn("ij.plugin.ClassChecker", "");
			notVerified = false;
		}
		PluginClassLoader loader;
		if (createNewLoader)
			loader = new PluginClassLoader(pluginsDir);
		else {
			if (classLoader==null)
				classLoader = new PluginClassLoader(pluginsDir);
			loader = classLoader;
		}
		Object thePlugIn = null;
		try { 
			thePlugIn = (loader.loadClass(className)).newInstance(); 
 			if (thePlugIn instanceof PlugIn)
				((PlugIn)thePlugIn).run(arg);
 			else if (thePlugIn instanceof PlugInFilter)
				runFilterPlugIn(thePlugIn, commandName, arg);
		}
		catch (ClassNotFoundException e) {
			if (className.indexOf('_')!=-1)
				IJ.error("Plugin not found: "+className);
		}
		catch (InstantiationException e) {IJ.error("Unable to load plugin (ins)");}
		catch (IllegalAccessException e) {IJ.error("Unable to load plugin, possibly \nbecause it is not public.");}
		return thePlugIn;
	} 

	static void wrongType(int capabilities, String cmd) {
		String s = "\""+cmd+"\" requires an image of type:\n \n";
		if ((capabilities&PlugInFilter.DOES_8G)!=0) s +=  "    8-bit grayscale\n";
		if ((capabilities&PlugInFilter.DOES_8C)!=0) s +=  "    8-bit color\n";
		if ((capabilities&PlugInFilter.DOES_16)!=0) s +=  "    16-bit grayscale\n";
		if ((capabilities&PlugInFilter.DOES_32)!=0) s +=  "    32-bit (float) grayscale\n";
		if ((capabilities&PlugInFilter.DOES_RGB)!=0) s += "    RGB color\n";
		IJ.error(s);
	}
	
    /** Starts executing a menu command in a separete thread and returns immediately. */
	public static void doCommand(String command) {
		if (ij!=null)
			ij.doCommand(command);
	}
	
    /** Runs an ImageJ command. Does not return until 
    	the command has finished executing. */
	public static void run(String command) {
		run(command, null);
	}
	
    /** Runs an ImageJ command, with options that are passed to the
		GenericDialog and OpenDialog classes. Does not return until
		the command has finished executing. */
	public static void run(String command, String options) {
		//IJ.log("run: "+command+" "+Thread.currentThread().getName());
		if (ij==null && Menus.getCommands()==null)
			init();
		Thread thread = Thread.currentThread();
		if (previousThread==null || thread!=previousThread) {
			String name = thread.getName();
			if (!name.startsWith("Run$_"))
				thread.setName("Run$_"+name);
		}
		if (command.equals("Miscellaneous..."))
			command = "Misc...";
		previousThread = thread;
		Executer e = new Executer(command);
		e.run();
		testAbort();
	}
	
	static void init() {
		Menus m = new Menus(null, null);
		Prefs.load(m, null);
		m.addMenuBar();
	}

	private static void testAbort() {
	}

	/** Returns true if either of the IJ.run() methods is executing. */
	public static boolean macroRunning() {
		return macroRunning;
	}

	/**Returns the Applet that created this ImageJ or null if running as an application.*/
	public static java.applet.Applet getApplet() {
		return applet;
	}
	
	/**Displays a message in the ImageJ status bar.*/
	public static void showStatus(String s) {
		if (ij!=null) ij.showStatus(s);
	}

	/** Displays a line of text in the "Results" window. Uses
		System.out.println if ImageJ is not present. */
	public static void write(String s) {
		if (textPanel==null)
			showResults();
		if (textPanel!=null)
				textPanel.append(s);
		else
			System.out.println(s);
	}

	private static void showResults() {
		TextWindow resultsWindow = new TextWindow("Results", "", 300, 200);
		textPanel = resultsWindow.getTextPanel();
		if (ij!=null)
			textPanel.addKeyListener(ij);
	}

	/** Displays a line of text in the "Log" window. Uses
		System.out.println if ImageJ is not present. */
	public static synchronized void log(String s) {
		System.out.println(s);
	}

	/** Clears the "Results" window and sets the column headings to
		those in the tab-delimited 'headings' String. */
	public static void setColumnHeadings(String headings) {
		if (textPanel==null)
			showResults();
		if (textPanel!=null)
			textPanel.setColumnHeadings(headings);
	}

	/** Returns true if the "Results" window is open. */
	public static boolean isResultsWindow() {
		return textPanel!=null;
	}
	
	/** Returns a reference to the "Results" window TextPanel.
		Opens the "Results" window if it is currently not open. */
	public static TextPanel getTextPanel() {
		if (textPanel==null)
			showResults();
		return textPanel;
	}
	
	/** TextWindow calls this method with a null argument when the "Results" window is closed. */
	public static void setTextPanel(TextPanel tp) {
		textPanel = tp;
	}
    
    /**Displays a "no images are open" dialog box.*/
	public static void noImage() {
		showMessage("No Image", "There are no images open.");
	}

	/** Displays an "out of memory" message to the "Log" window. */
	public static void outOfMemory(String name) {
		Undo.reset();
		System.gc();
		String tot = Runtime.getRuntime().totalMemory()/1048576L+"MB";
		if (!memMessageDisplayed)
			log(">>>>>>>>>>>>>>>>>>>>>>>>>>>");
		log("<Out of memory>");
		if (!memMessageDisplayed) {
			log("<All available memory ("+tot+") has been>");
			log("<used. Instructions for making more>");
			log("<available can be found in the \"Memory\" >");
			log("<sections of the installation notes at>");
			log("<http://rsb.info.nih.gov/ij/docs/install/>");
			log(">>>>>>>>>>>>>>>>>>>>>>>>>>>");
			memMessageDisplayed = true;
		}
	}

	/**	Updates the progress bar, where 0<=progress<=1.0. The progress bar is 
	not displayed if the time between the first and second calls to this method
	is less than 30 milliseconds. It is erased if progress>=1.0. Does nothing 
	if the ImageJ window is not present. */
	public static void showProgress(double progress) {
		if (progressBar!=null) progressBar.show(progress);
	}

	/**	Updates the progress bar, where bar-length = (currentIndex/finalIndex)*total-length.
	The bar is erased if currentIndex>=finalIndex. Does nothing 
	if the ImageJ window is not present. */
	public static void showProgress(int currentIndex, int finalIndex) {
		if (progressBar!=null) progressBar.show(currentIndex, finalIndex);
	}

	/**	Displays a message in a dialog box with the specified title.
		Writes the Java console if ImageJ is not present. */
	public static void showMessage(String title, String msg) {
		if (ij!=null) {
			if (msg.startsWith("<html>") && isJava2())
				new HTMLDialog(title, msg);
			else {
				JOptionPane.showMessageDialog(null, msg);
				ij.repaint();
			}
		} else {
			System.out.println(msg);
		}
	}

	/** Displays a message in a dialog box titled "Message".
		Writes the Java console if ImageJ is not present. */
	public static void showMessage(String msg) {
		showMessage("Message", msg);
	}

	/** Displays a message in a dialog box titled "ImageJ". Writes
		to the Java console if the ImageJ window is not present. */
	public static void error(String msg) {
		if (ij!=null) {
			JOptionPane.showMessageDialog(null, msg);
			ij.repaint();
		}
		else
			System.err.println(msg);
	}
	
	/** Displays a message in a dialog box with the specified title.
	   Returns false if the user pressed "Cancel". */
	public static boolean showMessageWithCancel(String title, String msg) {
		GenericDialog gd = new GenericDialog(title);
		gd.addMessage(msg);
		gd.showDialog();
		return !gd.wasCanceled();
	}

	public static final int CANCELED = Integer.MIN_VALUE;

	/** Allows the user to enter a number in a dialog box. Returns the	
	    value IJ.CANCELED (-2,147,483,648) if the user cancels the dialog box. 
	    Returns 'defaultValue' if the user enters an invalid number. */
	public static double getNumber(String prompt, double defaultValue) {
		GenericDialog gd = new GenericDialog("");
		int decimalPlaces = (int)defaultValue==defaultValue?0:2;
		gd.addNumericField(prompt, defaultValue, decimalPlaces);
		gd.showDialog();
		if (gd.wasCanceled())
			return CANCELED;
		double v = gd.getNextNumber();
		if (gd.invalidNumber())
			return defaultValue;
		else
			return v;
	}

	/** Allows the user to enter a string in a dialog box. Returns
	    "" if the user cancels the dialog box. */
	public static String getString(String prompt, String defaultString) {
		GenericDialog gd = new GenericDialog("");
		gd.addStringField(prompt, defaultString, 20);
		gd.showDialog();
		if (gd.wasCanceled())
			return "";
		return gd.getNextString();
	}

	/**Delays 'msecs' milliseconds.*/
	public synchronized static void wait(int msecs) {
		try {Thread.sleep(msecs);}
		catch (InterruptedException e) { }
	}
	
	/** Emits an audio beep. */
	public static void beep() {
		java.awt.Toolkit.getDefaultToolkit().beep();
	}
	
	public static String freeMemory() {
		System.gc();
		long freeMem = Runtime.getRuntime().freeMemory();
		long totMem = Runtime.getRuntime().totalMemory();
		long inUse = (int)(totMem-freeMem);
		String inUseStr = inUse<10000*1024?inUse/1024L+"K":inUse/1048576L+"MB";
		String maxStr="";
		long max = maxMemory();
		if (max>0L) {
			double percent = inUse*100/max;
			maxStr = " of "+max/1048576L+"MB ("+(percent<1.0?"<1":d2s(percent,0)) + "%)";
		}
		return  inUseStr + maxStr;
	}
	
	/** Returns the maximum amount of memory available to ImageJ or
		zero if ImageJ is unable to determine this limit. */
	public static long maxMemory() {
		if (maxMemory==0L) {
			Memory mem = new Memory();
			maxMemory = mem.getMemorySetting();
			if (maxMemory==0L) maxMemory = mem.maxMemory();
		}
		return maxMemory;
	}
	
	public static void showTime(ImagePlus imp, long start, String str) {
		showTime(imp, start, str, 1);
	}
	
	static void showTime(ImagePlus imp, long start, String str, int nslices) {
	    long elapsedTime = System.currentTimeMillis() - start;
		double seconds = elapsedTime / 1000.0;
		long pixels = imp.getWidth() * imp.getHeight();
		int rate = (int)((double)pixels*nslices/seconds);
		String str2;
		if (rate>1000000000)
			str2 = "";
		else if (rate<1000000)
			str2 = ", "+rate+" pixels/second";
		else
			str2 = ", "+d2s(rate/1000000.0,1)+" million pixels/second";
		showStatus(str+seconds+" seconds"+str2);
	}

	/** Converts a number to a formatted string using
		2 digits to the right of the decimal point. */
	public static String d2s(double n) {
		return d2s(n, 2);
	}
	
	private static DecimalFormat df =
		new DecimalFormat("0.00", new DecimalFormatSymbols(Locale.US));
	private static int dfDigits = 2;

	/** Converts a number to a rounded formatted string.
		The 'decimalPlaces' argument specifies the number of
		digits to the right of the decimal point. */
	public static String d2s(double n, int decimalPlaces) {
		if (n==Float.MAX_VALUE) // divide by 0 in FloatProcessor
			return "3.4e38";
		boolean negative = n<0.0;
		if (negative)
			n = -n;
		double whole = Math.round(n * Math.pow(10, decimalPlaces));
		double rounded = whole/Math.pow(10, decimalPlaces);
		if (negative)
			rounded = -rounded;
		if (decimalPlaces!=dfDigits)
			switch (decimalPlaces) {
				case 0: df.applyPattern("0"); dfDigits=0; break;
				case 1: df.applyPattern("0.0"); dfDigits=1; break;
				case 2: df.applyPattern("0.00"); dfDigits=2; break;
				case 3: df.applyPattern("0.000"); dfDigits=3; break;
				case 4: df.applyPattern("0.0000"); dfDigits=4; break;
				case 5: df.applyPattern("0.00000"); dfDigits=5; break;
				case 6: df.applyPattern("0.000000"); dfDigits=6; break;
				case 7: df.applyPattern("0.0000000"); dfDigits=7; break;
				case 8: df.applyPattern("0.00000000"); dfDigits=8; break;
			}
		String s = df.format(rounded);
		return s;
	}

	/** Adds the specified class to a Vector to keep it from being garbage
	collected, which would cause the classes static fields to be reset. */
	public static void register(Class c) {
		if (ij!=null) ij.register(c);
	}
	
	/** Returns true if the space bar is down. */
	public static boolean spaceBarDown() {
		return spaceDown;
	}

	/** Returns true if the alt key is down. */
	public static boolean altKeyDown() {
		return altDown;
	}

	/** Returns true if the shift key is down. */
	public static boolean shiftKeyDown() {
		return shiftDown;
	}

	public static void setKeyDown(int key) {
		switch (key) {
			case KeyEvent.VK_ALT:
				altDown=true;
				break;
			case KeyEvent.VK_SHIFT:
				shiftDown=true;
				break;
		}
	}
	
	public static void setKeyUp(int key) {
		switch (key) {
			case KeyEvent.VK_ALT: altDown=false; break;
			case KeyEvent.VK_SHIFT: shiftDown=false; break;
		}
	}
	
	public static void setInputEvent(InputEvent e) {
		altDown = e.isAltDown();
		shiftDown = e.isShiftDown();
	}

	/** Returns true if this machine is a Macintosh. */
	public static boolean isMacintosh() {
		return isMac;
	}
	
	/** Returns true if this machine is a Macintosh running OS X. */
	public static boolean isMacOSX() {
		return isMacintosh() && isJava2();
	}

	/** Returns true if this machine is running Windows. */
	public static boolean isWindows() {
		return isWin;
	}
	
	/** Returns true if ImageJ is running on Java 2. */
	public static boolean isJava2() {
		return isJava2;
	}
	
	/** Returns true if ImageJ is running on a Java 1.4 or greater JVM. */
	public static boolean isJava14() {
		return isJava14;
	}

	/** Displays an error message and returns false if the
		ImageJ version is less than the one specified. */
	public static boolean versionLessThan(String version) {
		boolean lessThan = ImageJ.VERSION.compareTo(version)<0;
		if (lessThan)
			error("This plugin or macro requires ImageJ "+version+" or later.");
		return lessThan;
	}
	
	/** Displays a "Process all slices?" dialog. Returns
		'flags'+PlugInFilter.DOES_STACKS if the user selects "Yes",
		'flags' if the user selects "No" and PlugInFilter.DONE
		if the user selects "Cancel".
	*/
	public static int setupDialog(ImagePlus imp, int flags) {
		return flags;
	}
	
	/** Creates a rectangular selection. Removes any existing 
		selection if width or height are less than 1. */
	public static void makeRectangle(int x, int y, int width, int height) {
		if (width<=0 || height<0)
			getImage().killRoi();
		else
			getImage().setRoi(x, y, width, height);
	}
	
	/** Creates an elliptical selection. Removes any existing 
		selection if width or height are less than 1. */
	public static void makeOval(int x, int y, int width, int height) {
		if (width<=0 || height<0)
			getImage().killRoi();
		else
			getImage().setRoi(new OvalRoi(x, y, width, height));
	}
	
	/** Creates a straight line selection. */
	public static void makeLine(int x1, int y1, int x2, int y2) {
		getImage().setRoi(new Line(x1, y1, x2, y2));
		//wait(100);
	}
	
	/** Sets the minimum and maximum displayed pixel values. */
	public static void setMinAndMax(double min, double max) {
		ImagePlus img = getImage();
		img.getProcessor().setMinAndMax(min, max);
		img.updateAndDraw();
	}

	/** Resets the minimum and maximum displayed pixel values
		to be the same as the min and max pixel values. */
	public static void resetMinAndMax() {
		ImagePlus img = getImage();
		img.getProcessor().resetMinAndMax();
		img.updateAndDraw();
	}

	/** Sets the lower and upper threshold levels. */
	public static void setThreshold(double lowerThreshold, double upperThresold) {
		ImagePlus img = getImage();
		img.getProcessor().setThreshold(lowerThreshold,upperThresold,ImageProcessor.RED_LUT);
		img.updateAndDraw();
	}
	
	/** Disables thresholding. */
	public static void resetThreshold() {
		ImagePlus img = getImage();
		img.getProcessor().resetThreshold();
		img.updateAndDraw();
	}
	
	/** For IDs less than zero, activates the image with the specified ID.
		For IDs greater than zero, activates the Nth image. */
	public static void selectWindow(int id) {
	}
	
	static void selectWindow(Frame frame) {

	}
	
	/** Sets the foreground color. */
	public static void setForegroundColor(int red, int green, int blue) {
		setColor(red, green, blue, true);
	}

	/** Sets the background color. */
	public static void setBackgroundColor(int red, int green, int blue) {
		setColor(red, green, blue, false);
	}
	
	static void setColor(int red, int green, int blue, boolean foreground) {
	    if (red<0) red=0; if (green<0) green=0; if (blue<0) blue=0; 
	    if (red>255) red=255; if (green>255) green=255; if (blue>255) blue=255;  
		Color c = new Color(red, green, blue);
		if (foreground) {
			Toolbar.setForegroundColor(c);
			ImagePlus img = IJ.getInstance().getImagePlus();
			if (img!=null)
				img.getProcessor().setColor(c);
		} else
			Toolbar.setBackgroundColor(c);
	}

	/** Switches to the specified tool, where id = Toolbar.RECTANGLE (0),
		Toolbar.OVAL (1), etc. */
	public static void setTool(int id) {
		Toolbar.getInstance().setTool(id);
	}

	/** Equivalent to clicking on the current image at (x,y) with the
		wand tool. Returns the number of points in the resulting ROI. */
	public static int doWand(int x, int y) {
		ImagePlus img = getImage();
		ImageProcessor ip = img.getProcessor();
		Wand w = new Wand(ip);
		double t1 = ip.getMinThreshold();
		if (t1==ip.NO_THRESHOLD)
			w.autoOutline(x, y);
		else
			w.autoOutline(x, y, (int)t1, (int)ip.getMaxThreshold());
		if (w.npoints>0) {
			Roi roi = new PolygonRoi(w.xpoints, w.ypoints, w.npoints, Roi.TRACED_ROI);
			img.setRoi(roi);
			if (shiftKeyDown() || altKeyDown())
				roi.modifyRoi(); 
		}
		return w.npoints;
	}
	
	/** Sets the transfer mode used by the <i>Edit/Paste</i> command, where mode is "Copy", "Blend", "Average", "Difference", 
		"Transparent", "AND", "OR", "XOR", "Add", "Subtract", "Multiply", or "Divide". */
	public static void setPasteMode(String mode) {
		mode = mode.toLowerCase(Locale.US);
		int m = Blitter.COPY;
		if (mode.startsWith("ble") || mode.startsWith("ave"))
			m = Blitter.AVERAGE;
		else if (mode.startsWith("diff"))
			m = Blitter.DIFFERENCE;
		else if (mode.startsWith("tran"))
			m = Blitter.COPY_TRANSPARENT;
		else if (mode.startsWith("and"))
			m = Blitter.AND;
		else if (mode.startsWith("or"))
			m = Blitter.OR;
		else if (mode.startsWith("xor"))
			m = Blitter.XOR;
		else if (mode.startsWith("sub"))
			m = Blitter.SUBTRACT;
		else if (mode.startsWith("add"))
			m = Blitter.ADD;
		else if (mode.startsWith("div"))
			m = Blitter.DIVIDE;
		else if (mode.startsWith("mul"))
			m = Blitter.MULTIPLY;
		Roi.setPasteMode(m);
	}

	/** Returns a reference to the active image. Displays an error
		message and aborts the macro if no images are open. */
	public static ImagePlus getImage() {
		ImagePlus img = IJ.getInstance().getImagePlus();
		if (img==null) {
			IJ.noImage();
			abort();
		}
		return img;
	}
	
	/** Returns the ImageJ version number as a string. */
	public static String getVersion() {
		return ImageJ.VERSION;
	}
	
	/** Returns the path to the plugins, macros, temp or image directory if <code>title</code> 
		is "plugins", "macros", "temp" or "image", otherwise, displays a dialog 
		and returns the path to the directory selected by the user. 
		Returns null if the specified directory is not found or the user
		cancels the dialog box. Also aborts the macro if the user cancels
		the dialog box.*/
	public static String getDirectory(String title) {
		// Not applicable to applet version
		return "";
	}
	
	static void abort() {
		throw new RuntimeException("Macro canceled");
	}
	
}