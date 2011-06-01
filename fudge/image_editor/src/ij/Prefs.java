package ij;
import java.io.*;
import java.util.*;
import java.applet.*;
import java.net.URL;
import java.awt.Color;
import java.applet.Applet;
import ij.io.*;
import ij.util.Tools;
import ij.gui.*;
import ij.plugin.filter.*;
import ij.process.ImageConverter;
import ij.process.FloatBlitter;
import ij.plugin.JpegWriter;
import ij.process.ColorProcessor;

/**
This class contains the ImageJ preferences, which are 
loaded from the "IJ_Props.txt" and "IJ_Prefs.txt" files.
@see ij.ImageJ
*/
public class Prefs {

	public static final String PROPS_NAME = "IJ_Props.txt";
	public static final String PREFS_NAME = "IJ_Prefs.txt";
	public static final String DIR_IMAGE = "dir.image";
	public static final String FCOLOR = "fcolor";
	public static final String BCOLOR = "bcolor";
	public static final String ROICOLOR = "roicolor";
	public static final String JPEG = "jpeg";
	public static final String FPS = "fps";
    public static final String DIV_BY_ZERO_VALUE = "div-by-zero";
    public static final String NOISE_SD = "noise.sd";
    public static final String KEY_PREFIX = ".";
 
	private static final int USE_POINTER=1, ANTIALIASING=2, INTERPOLATE=4, ONE_HUNDRED_PERCENT=8,
		BLACK_BACKGROUND=16, JFILE_CHOOSER=32, UNWEIGHTED=64, BLACK_CANVAS=128;  
    public static final String OPTIONS = "prefs.options";

	/** file.separator system property */
	public static String separator = System.getProperty("file.separator");
	/** Use pointer cursor instead of cross */
	public static boolean usePointerCursor;
	/** Display antialiased text */
	public static boolean antialiasedText;
	/** Display images scaled <100% using bilinear interpolation */
	public static boolean interpolateScaledImages;
	/** Open images at 100% magnification*/
	public static boolean open100Percent;
	/** Backgound is black in binary images*/
	public static boolean blackBackground;
	/** Color to grayscale conversion is not weighted if the variable is true. */
	public static boolean unweightedColor;
	/** Use black image border. */
	public static boolean blackCanvas;

	static Properties ijPrefs = new Properties();
	static Properties props = new Properties(ijPrefs);
	static String prefsDir;
	static String imagesURL;
	static String homeDir; // ImageJ folder

	/** Finds and loads the ImageJ configuration file, "IJ_Props.txt".
		@return	an error message if "IJ_Props.txt" not found.
	*/
	public static String load(Object ij, Applet applet) {
		InputStream f = ij.getClass().getResourceAsStream("/"+PROPS_NAME);
		if (applet!=null)
			return loadAppletProps(f,applet);
		homeDir = System.getProperty("user.dir");
		String userHome = System.getProperty("user.home");
		String osName = System.getProperty("os.name");
		if (osName.indexOf("Windows",0)>-1)
			prefsDir = homeDir; //ImageJ folder on Windows
		else {
			prefsDir = userHome; // Mac Preferences folder or Unix home dir
			if (IJ.isMacOSX())
				prefsDir += "/Library/Preferences";
		} 
		if (f==null) {
			try {f = new FileInputStream(homeDir+"/"+PROPS_NAME);}
			catch (FileNotFoundException e) {f=null;}
		}
		if (f==null)
			return PROPS_NAME+" not found in ij.jar or in "+homeDir;
		f = new BufferedInputStream(f);
		try {props.load(f); f.close();}
		catch (IOException e) {return("Error loading "+PROPS_NAME);}
		imagesURL = props.getProperty("images.location");
		loadPreferences();
		loadOptions();
		return null;
	}

	/*
	static void dumpPrefs(String title) {
		IJ.log("");
		IJ.log(title);
		Enumeration e = ijPrefs.keys();
		while (e.hasMoreElements()) {
			String key = (String) e.nextElement();
			IJ.log(key+": "+ijPrefs.getProperty(key));
		}
	}
	*/

	static String loadAppletProps(InputStream f, Applet applet) {
		if (f==null)
			return PROPS_NAME+" not found in ij.jar";
		try {
			props.load(f);
			f.close();
		}
		catch (IOException e) {return("Error loading "+PROPS_NAME);}
		try {
			URL url = new URL(applet.getDocumentBase(), "images/");
			imagesURL = url.toString();
		}
		catch (Exception e) {}
		return null;
	}

	/** Returns the URL for the ImageJ sample images. */
	public static String getImagesURL() {
		return imagesURL;
	}

	/** Returns the path to the ImageJ directory. */
	public static String getHomeDir() {
		return homeDir;
	}

	/** Finds an string in IJ_Props or IJ_Prefs.txt. */
	public static String getString(String key) {
		return props.getProperty(key);
	}

	/** Finds an string in IJ_Props or IJ_Prefs.txt. */
	public static String getString(String key, String defaultString) {
		if (props==null)
			return defaultString;
		String s = props.getProperty(key);
		if (s==null)
			return defaultString;
		else
			return s;
	}

	/** Finds a boolean in IJ_Props or IJ_Prefs.txt. */
	public static boolean getBoolean(String key, boolean defaultValue) {
		if (props==null) return defaultValue;
		String s = props.getProperty(key);
		if (s==null)
			return defaultValue;
		else
			return s.equals("true");
	}

	/** Finds an int in IJ_Props or IJ_Prefs.txt. */
	public static int getInt(String key, int defaultValue) {
		if (props==null) //workaround for Netscape JIT bug
			return defaultValue;
		String s = props.getProperty(key);
		if (s!=null) {
			try {
				return Integer.decode(s).intValue();
			} catch (NumberFormatException e) {IJ.write(""+e);}
		}
		return defaultValue;
	}

	/** Looks up a real number in IJ_Props or IJ_Prefs.txt. */
	public static double getDouble(String key, double defaultValue) {
		if (props==null)
			return defaultValue;
		String s = props.getProperty(key);
		Double d = null;
		if (s!=null) {
			try {d = new Double(s);}
			catch (NumberFormatException e){d = null;}
			if (d!=null)
				return(d.doubleValue());
		}
		return defaultValue;
	}

	/** Finds a color in IJ_Props or IJ_Prefs.txt. */
	public static Color getColor(String key, Color defaultColor) {
		int i = getInt(key, 0xaaa);
		if (i == 0xaaa)
			return defaultColor;
		return new Color((i >> 16) & 0xFF, (i >> 8) & 0xFF, i & 0xFF);
	}

	/** Returns the file.separator system property. */
	public static String getFileSeparator() {
		return separator;
	}

	/** Opens the IJ_Prefs.txt file. */
	static void loadPreferences() {
		String path = prefsDir+separator+PREFS_NAME;
		boolean ok =  loadPrefs(path);
		if (!ok && IJ.isMacOSX()) {
			path = System.getProperty("user.home")+separator+PREFS_NAME;
			ok = loadPrefs(path); // look in home dir
			if (ok)
				new File(path).delete();
		}

	}

	static boolean loadPrefs(String path) {
		try {
			InputStream is = new BufferedInputStream(new FileInputStream(path));
			ijPrefs.load(is);
			is.close();
			return true;
		} catch (Exception e) {
			return false;
		}
	}

	/** Saves user preferences in the IJ_Prefs.txt properties file. */
	static void savePreferences() {
	}

	static void loadOptions() {
		int options = getInt(OPTIONS, ANTIALIASING);
		usePointerCursor = (options&USE_POINTER)!=0;
		antialiasedText = (options&ANTIALIASING)!=0;
		interpolateScaledImages = (options&INTERPOLATE)!=0;
		open100Percent = (options&ONE_HUNDRED_PERCENT)!=0;
		open100Percent = (options&ONE_HUNDRED_PERCENT)!=0;
		blackBackground = (options&BLACK_BACKGROUND)!=0;
		unweightedColor = (options&UNWEIGHTED)!=0;
		if (unweightedColor)
			ColorProcessor.setWeightingFactors(1d/3d, 1d/3d, 1d/3d);
		blackCanvas = (options&BLACK_CANVAS)!=0;
	}

	static void saveOptions(Properties prefs) {
		int options = (usePointerCursor?USE_POINTER:0) + (antialiasedText?ANTIALIASING:0)
			+ (interpolateScaledImages?INTERPOLATE:0) + (open100Percent?ONE_HUNDRED_PERCENT:0)
			+ (blackBackground?BLACK_BACKGROUND:0) 
			+ (unweightedColor?UNWEIGHTED:0) + (blackCanvas?BLACK_CANVAS:0);
		prefs.put(OPTIONS, Integer.toString(options));
	}

	/** Saves the value of the string <code>text</code> in the preferences
		file using the keyword <code>key</code>. This string can be 
		retrieved using the appropriate <code>get()</code> method. */
	public static void set(String key, String text) {
		if (key.indexOf('.')<1)
			throw new IllegalArgumentException("Key must have a prefix");
		ijPrefs.put(KEY_PREFIX+key, text);
	}

	/** Saves <code>value</code> in the preferences file using 
		the keyword <code>key</code>. This value can be retrieved 
		using the appropriate <code>getPref()</code> method. */
	public static void set(String key, double value) {
		set(key, ""+value);
	}

	/** Saves the boolean variable <code>value</code> in the preferences
		 file using the keyword <code>key</code>. This value can be retrieved 
		using the appropriate <code>getPref()</code> method. */
	public static void set(String key, boolean value) {
		set (key, ""+value);
	}

	/** Uses the keyword <code>key</code> to retrieve a string from the
		preferences file. Returns <code>defaultValue</code> if the key
		is not found. */
	public static String get(String key, String defaultValue) {
		String value = ijPrefs.getProperty(KEY_PREFIX+key);
		if (value == null)
			return defaultValue;
		else
			return value;
	}

	/** Uses the keyword <code>key</code> to retrieve a number from the
		preferences file. Returns <code>defaultValue</code> if the key
		is not found. */
	public static double get(String key, double defaultValue) {
		String s = ijPrefs.getProperty(KEY_PREFIX+key);
		Double d = null;
		if (s!=null) {
			try {d = new Double(s);}
			catch (NumberFormatException e) {d = null;}
			if (d!=null)
				return(d.doubleValue());
		}
		return defaultValue;
	}

	/** Uses the keyword <code>key</code> to retrieve a boolean from
		the preferences file. Returns <code>defaultValue</code> if
		the key is not found. */
	public static boolean get(String key, boolean defaultValue) {
		String value = ijPrefs.getProperty(KEY_PREFIX+key);
		if (value==null)
			return defaultValue;
		else
			return value.equals("true");
	}

	/** Save plugin preferences. */
	static void savePluginPrefs(Properties prefs) {
		Enumeration e = ijPrefs.keys();
		while (e.hasMoreElements()) {
			String key = (String) e.nextElement();
			if (key.indexOf(KEY_PREFIX) == 0)
				prefs.put(key, ijPrefs.getProperty(key));
		}
	}

	public static void savePrefs(Properties prefs, String path) throws IOException{
		FileOutputStream fos = new FileOutputStream(path);
		BufferedOutputStream bos = new BufferedOutputStream(fos);
		PrintWriter pw = new PrintWriter(bos);
		pw.println("# ImageJ "+ImageJ.VERSION+" Preferences");
		pw.println("# "+new Date());
		pw.println("");
		for (Enumeration e=prefs.keys(); e.hasMoreElements();) {
			String key = (String)e.nextElement();
			pw.print(key);
			pw.write('=');
			pw.println((String)prefs.get(key));
		}
		pw.close();
	}

	static String escapeBackSlashes (String s) {
		StringBuffer sb = new StringBuffer(s.length()+10);
		char[] chars = s.toCharArray();
		for (int i=0; i<chars.length; i++) {
			sb.append(chars[i]);
			if (chars[i]=='\\')
				sb.append('\\');
		}
		return sb.toString();
	}

}

