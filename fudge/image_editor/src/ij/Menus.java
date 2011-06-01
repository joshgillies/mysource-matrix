package ij;
import ij.process.*;
import ij.util.*;
import java.awt.*;
import java.awt.image.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;
import java.applet.Applet;
import java.awt.event.*;
import java.util.zip.*;
import javax.swing.*;

/**
This class installs and updates ImageJ's menus. Note that menu labels,
even in submenus, must be unique. This is because ImageJ uses a single
hash table for all menu labels. If you look closely, you will see that
File->Import->Text Image... and File->Save As->Text Image... do not use
the same label. One of the labels has an extra space.

@see ImageJ
*/

public class Menus {

	public static final char PLUGINS_MENU = 'p';
	public static final char IMPORT_MENU = 'i';
	public static final char SAVE_AS_MENU = 's';
	public static final char SHORTCUTS_MENU = 'h'; // 'h'=hotkey
	public static final char ABOUT_MENU = 'a';
	public static final char FILTERS_MENU = 'f';
	public static final char TOOLS_MENU = 't';
	public static final char UTILITIES_MENU = 'u';

	public static final int WINDOW_MENU_ITEMS = 5; // fixed items at top of Window menu

	public static final int NORMAL_RETURN = 0;
	public static final int COMMAND_IN_USE = -1;
	public static final int INVALID_SHORTCUT = -2;
	public static final int SHORTCUT_IN_USE = -3;
	public static final int NOT_INSTALLED = -4;
	public static final int COMMAND_NOT_FOUND = -5;

	private static JMenuBar mbar;
	private static JCheckBoxMenuItem gray8Item,gray16Item,gray32Item,
			color256Item,colorRGBItem;
	private static JPopupMenu popup;

	private static ImageJ ij;
	private static Applet applet;
	private static Hashtable demoImagesTable = new Hashtable();
	private static String pluginsPath;
	private static JMenu pluginsMenu, importMenu, saveAsMenu, shortcutsMenu,
		aboutMenu, filtersMenu, toolsMenu, utilitiesMenu, optionsMenu;
	private static Hashtable pluginsTable;

	static JMenu window;
	int nPlugins;
	private static Hashtable shortcuts = new Hashtable();
	private static Vector pluginsPrefs = new Vector(); // commands saved in IJ_Prefs
	static int windowMenuItems2; // non-image windows listed in Window menu + separator
	private static String error;
	private String jarError;
	private boolean isJarErrorHeading;
	private boolean installingJars, duplicateCommand;
	private static Vector jarFiles;  // JAR files in plugins folder with "_" in their name
	private int importCount, saveAsCount, toolsCount, optionsCount;
	private static Hashtable menusTable; // Submenus of Plugins menu
	private int userPluginsIndex; // First user plugin or submenu in Plugins menu
	private boolean addSorted;

	Menus(ImageJ ijInstance, JApplet appletInstance) {
		ij = ijInstance;
		applet = appletInstance;
	}

	String addMenuBar() {
		error = null;
		pluginsTable = new Hashtable();

		JMenu file = new JMenu("File");
		addItem(file, "Open Local", KeyEvent.VK_O, false);
		importMenu = addSubMenu(file, "Import");
		file.addSeparator();
		//addItem(file, "Save Local",  KeyEvent.VK_S, false);
		addItem(file, "Revert", KeyEvent.VK_R,  false);

		JMenu edit = new JMenu("Edit");
		addItem(edit, "Undo", KeyEvent.VK_Z, false);
		edit.addSeparator();
		addItem(edit, "Cut", KeyEvent.VK_X, false);
		addItem(edit, "Copy", KeyEvent.VK_C, false);
		addItem(edit, "Paste", KeyEvent.VK_V, false);
		edit.addSeparator();
		addPlugInItem(edit, "Clear", "ij.plugin.filter.Filler(\"clear\")", 0, false);
		addPlugInItem(edit, "Clear Outside", "ij.plugin.filter.Filler(\"outside\")", 0, false);
		addPlugInItem(edit, "Fill", "ij.plugin.filter.Filler(\"fill\")", KeyEvent.VK_F, false);
		addPlugInItem(edit, "Draw", "ij.plugin.filter.Filler(\"draw\")", KeyEvent.VK_D, false);
		addPlugInItem(edit, "Invert", "ij.plugin.filter.Filters(\"invert\")", KeyEvent.VK_I, true);
		edit.addSeparator();
		addSubMenu(edit, "Selection");
		optionsMenu = addSubMenu(edit, "Options");

		JMenu image = new JMenu("Image");
		JMenu imageType = new JMenu("Set Type");
			gray8Item = addCheckboxItem(imageType, "8-bit Greyscale", "ij.plugin.Converter(\"8-bit\")");
			gray16Item = addCheckboxItem(imageType, "16-bit Greyscale", "ij.plugin.Converter(\"16-bit\")");
			gray32Item = addCheckboxItem(imageType, "32-bit Greyscale", "ij.plugin.Converter(\"32-bit\")");
			color256Item = addCheckboxItem(imageType, "8-bit Colour", "ij.plugin.Converter(\"8-bit Color\")");
			colorRGBItem = addCheckboxItem(imageType, "RGB Colour", "ij.plugin.Converter(\"RGB Color\")");
			imageType.add(new JMenuItem("-"));
			image.add(imageType);

		image.addSeparator();
		addSubMenu(image, "Adjust");
		image.addSeparator();
		addPlugInItem(image, "Set Size...", "ij.plugin.filter.Resizer", 0, false);
		addPlugInItem(image, "Scale Size...", "ij.plugin.filter.Scaler", KeyEvent.VK_E, false);
		addPlugInItem(image, "Resize Canvas", "ij.plugin.CanvasResizer", 0, false);
		addPlugInItem(image, "Crop", "ij.plugin.filter.Resizer(\"crop\")", 0, false);
		addSubMenu(image, "Rotate");

		JMenu process = new JMenu("Process");
		addPlugInItem(process, "Smooth", "ij.plugin.filter.Filters(\"smooth\")", KeyEvent.VK_S, true);
		addPlugInItem(process, "Sharpen", "ij.plugin.filter.Filters(\"sharpen\")", 0, false);
		addPlugInItem(process, "Find Edges", "ij.plugin.filter.Filters(\"edge\")", KeyEvent.VK_F, true);
		addPlugInItem(process, "Enhance Contrast", "ij.plugin.ContrastEnhancer", 0, false);
		addSubMenu(process, "Noise");
		addSubMenu(process, "Shadows");
		addSubMenu(process, "Binary");
		addSubMenu(process, "Math");
		process.addSeparator();
		addPlugInItem(process, "Subtract Background...", "ij.plugin.filter.BackgroundSubtracter", 0, false);
		addItem(process, "Repeat Command", KeyEvent.VK_R, true);
		JMenu help = new JMenu("Help");
		addPlugInItem(help, "About Image Editor", "ij.plugin.AboutBox", 0, false);

		mbar = new JMenuBar();
		mbar.setBorder(BorderFactory.createMatteBorder(1, 1, 0, 1, Color.BLACK));
		mbar.add(file);
		mbar.add(edit);
		mbar.add(image);
		mbar.add(process);
		mbar.add(help);
		if (ij!=null)
			ij.setJMenuBar(mbar);

		if (jarError!=null)
			error = error!=null?error+="\n"+jarError:jarError;
		return error;
	}

	// TB Removed keyboard shortcut support when moving to Swing
	void addItem(JMenu menu, String label, int shortcut, boolean shift) {
		if (menu==null)
			return;
		JMenuItem item;
		item = new JMenuItem(label);
		if (addSorted && menu==pluginsMenu)
			addItemSorted(menu, item, userPluginsIndex);
		else
			menu.add(item);
		item.addActionListener(ij);
	}

	void addPlugInItem(JMenu menu, String label, String className, int shortcut, boolean shift) {
		pluginsTable.put(label, className);
		nPlugins++;
		addItem(menu, label, shortcut, shift);
	}

	JCheckBoxMenuItem addCheckboxItem(JMenu menu, String label, String className) {
		pluginsTable.put(label, className);
		nPlugins++;
		JCheckBoxMenuItem item = new JCheckBoxMenuItem(label);
		menu.add(item);
		item.addItemListener(ij);
		item.setState(false);
		return item;
	}

	JMenu addSubMenu(JMenu menu, String name)
	{
		String value;
		String key = name.toLowerCase(Locale.US);
		int index;
		JMenu submenu=new JMenu(name.replace('_', ' '));

		index = key.indexOf(' ');
		if (index > 0) {
			key = key.substring(0, index);
		}
		for (int count=1; count<100; count++) {
			value = Prefs.getString(key + (count/10)%10 + count%10);
			if (value==null)
				break;
			if (count==1)
				menu.add(submenu);
			if (value.equals("-"))
				submenu.addSeparator();
			else
				addPluginItem(submenu, value);
		}
		return submenu;

	}//end addSubMenu()

	void addPluginItem(JMenu submenu, String s) {
		if (s.equals("\"-\"")) {
			// add menu separator if command="-"
			addSeparator(submenu);
			return;
		}
		int lastComma = s.lastIndexOf(',');
		if (lastComma<=0)
			return;
		String command = s.substring(1,lastComma-1);
		int keyCode = 0;
		boolean shift = false;
		if (command.endsWith("]")) {
			int openBracket = command.lastIndexOf('[');
			if (openBracket>0) {
				String shortcut = command.substring(openBracket+1,command.length()-1);
				keyCode = convertShortcutToCode(shortcut);
				boolean functionKey = keyCode>=KeyEvent.VK_F1 && keyCode<=KeyEvent.VK_F12;
				if (keyCode>0 && !functionKey)
					command = command.substring(0,openBracket);
				//IJ.write(command+": "+shortcut);
			}
		}
		if (keyCode>=KeyEvent.VK_F1 && keyCode<=KeyEvent.VK_F12) {
			shortcuts.put(new Integer(keyCode),command);
			keyCode = 0;
		} else if (keyCode>200) {
			keyCode -= 200;
			shift = true;
		}
		addItem(submenu,command,keyCode,shift);
		while(s.charAt(lastComma+1)==' ' && lastComma+2<s.length())
			lastComma++; // remove leading spaces
		String className = s.substring(lastComma+1,s.length());
		//IJ.log(command+"  "+className);
		if (installingJars)
			duplicateCommand = pluginsTable.get(command)!=null;
		pluginsTable.put(command, className);
		nPlugins++;
	}

	void checkForDuplicate(String command) {
		if (pluginsTable.get(command)!=null) {
		}
	}

	void addPluginsMenu() {
		String value,label,className;
		int index;
		pluginsMenu = new JMenu("Plugins");
		for (int count=1; count<100; count++) {
			value = Prefs.getString("plug-in" + (count/10)%10 + count%10);
			if (value==null)
				break;
			char firstChar = value.charAt(0);
			if (firstChar=='-')
				pluginsMenu.addSeparator();
			else if (firstChar=='>') {
				String submenu = value.substring(2,value.length()-1);
				JMenu menu = addSubMenu(pluginsMenu, submenu);
				if (submenu.equals("Shortcuts"))
					shortcutsMenu = menu;
				else if (submenu.equals("Utilities"))
					utilitiesMenu = menu;
			} else
				addPluginItem(pluginsMenu, value);
		}
		userPluginsIndex = pluginsMenu.getItemCount();
		if (userPluginsIndex<0) userPluginsIndex = 0;
	}

	/** Install plugins using "pluginxx=" keys in IJ_Prefs.txt.
		Plugins not listed in IJ_Prefs are added to the end
		of the Plugins menu. */
	void installPlugins() {
		String value, className;
		char menuCode;
		JMenu menu;
		String[] plugins = getPlugins();
		String[] plugins2 = null;
		Hashtable skipList = new Hashtable();
		for (int index=0; index<100; index++) {
			value = Prefs.getString("plugin" + (index/10)%10 + index%10);
			if (value==null)
				break;
			menuCode = value.charAt(0);
			switch (menuCode) {
				case PLUGINS_MENU: default: menu = pluginsMenu; break;
				case IMPORT_MENU: menu = importMenu; break;
				case SAVE_AS_MENU: menu = saveAsMenu; break;
				case SHORTCUTS_MENU: menu = shortcutsMenu; break;
				case ABOUT_MENU: menu = aboutMenu; break;
				case FILTERS_MENU: menu = filtersMenu; break;
				case TOOLS_MENU: menu = toolsMenu; break;
				case UTILITIES_MENU: menu = utilitiesMenu; break;
			}
			String prefsValue = value;
			value = value.substring(2,value.length()); //remove menu code and coma
			className = value.substring(value.lastIndexOf(',')+1,value.length());
			boolean found = className.startsWith("ij.");
			if (!found && plugins!=null) { // does this plugin exist?
				if (plugins2==null)
					plugins2 = getStrippedPlugins(plugins);
				for (int i=0; i<plugins2.length; i++) {
					if (className.startsWith(plugins2[i])) {
						found = true;
						break;
					}
				}
			}
			if (found) {
				addPluginItem(menu, value);
				pluginsPrefs.addElement(prefsValue);
				if (className.endsWith("\")")) { // remove any argument
					int argStart = className.lastIndexOf("(\"");
					if (argStart>0)
						className = className.substring(0, argStart);
				}
				skipList.put(className, "");
			}
		}
		if (plugins!=null) {
			for (int i=0; i<plugins.length; i++) {
				if (!skipList.containsKey(plugins[i]))
					installUserPlugin(plugins[i]);
			}
		}
		installJarPlugins();
	}


	/** Install plugins located in JAR files. */
	void installJarPlugins() {
		if (jarFiles==null)
			return;
		installingJars = true;
		for (int i=0; i<jarFiles.size(); i++) {
			isJarErrorHeading = false;
			String jar = (String)jarFiles.elementAt(i);
			InputStream is = getConfigurationFile(jar);
			if (is==null) continue;
			LineNumberReader lnr = new LineNumberReader(new InputStreamReader(is));
			try {
				while(true) {
					String s = lnr.readLine();
					if (s==null) break;
					installJarPlugin(jar, s);
				}
			}
			catch (IOException e) {}
			finally {
				try {if (lnr!=null) lnr.close();}
				catch (IOException e) {}
			}
		}
	}

	/** Install a plugin located in a JAR file. */
	void installJarPlugin(String jar, String s) {
		//IJ.log(s);
		if (s.length()<3) return;
		char firstChar = s.charAt(0);
		if (firstChar=='#') return;
		addSorted = false;
		JMenu menu;
		if (s.startsWith("Plugins>")) {
			int firstComma = s.indexOf(',');
			if (firstComma==-1 || firstComma<=8)
				menu = null;
			else {
				String name = s.substring(8, firstComma);
				menu = getPluginsSubmenu(name);
			}
		} else if (firstChar=='"' || s.startsWith("Plugins")) {
			String name = getSubmenuName(jar);
			if (name!=null)
				menu = getPluginsSubmenu(name);
			else {
				menu = pluginsMenu;
				addSorted = true;
			}
		} else if (s.startsWith("File>Import")) {
			menu = importMenu;
			if (importCount==0) addSeparator(menu);
			importCount++;
		} else if (s.startsWith("File>Save")) {
			menu = saveAsMenu;
			if (saveAsCount==0) addSeparator(menu);
			saveAsCount++;
		} else if (s.startsWith("Analyze>Tools")) {
			menu = toolsMenu;
			if (toolsCount==0) addSeparator(menu);
			toolsCount++;
		} else if (s.startsWith("Help>About")) {
			menu = aboutMenu;
		} else if (s.startsWith("Edit>Options")) {
			menu = optionsMenu;
			if (optionsCount==0) addSeparator(menu);
			optionsCount++;
		} else {
			if (jarError==null) jarError = "";
			addJarErrorHeading(jar);
			jarError += "    Invalid menu: " + s + "\n";
			return;
		}
		int firstQuote = s.indexOf('"');
		if (firstQuote==-1)
			return;
		s = s.substring(firstQuote, s.length()); // remove menu
		if (menu!=null) {
			addPluginItem(menu, s);
			addSorted = false;
		}
		if (duplicateCommand) {
			if (jarError==null) jarError = "";
			addJarErrorHeading(jar);
			jarError += "    Duplicate command: " + s + "\n";
		}
		duplicateCommand = false;
	}

	void addJarErrorHeading(String jar) {
		if (!isJarErrorHeading) {
				if (!jarError.equals(""))
					jarError += " \n";
				jarError += "Plugin configuration error: " + jar + "\n";
				isJarErrorHeading = true;
			}
	}

	JMenu getPluginsSubmenu(String submenuName) {
		if (menusTable!=null) {
			JMenu menu = (JMenu)menusTable.get(submenuName);
			if (menu!=null)
				return menu;
		}
		JMenu menu = new JMenu(submenuName);
		//pluginsMenu.add(menu);
		addItemSorted(pluginsMenu, menu, userPluginsIndex);
		if (menusTable==null) menusTable = new Hashtable();
		menusTable.put(submenuName, menu);
		//IJ.log("getPluginsSubmenu: "+submenuName);
		return menu;
	}

	String getSubmenuName(String jarPath) {
		//IJ.log("getSubmenuName: "+jarPath);
		int index = jarPath.lastIndexOf(File.separatorChar);
		if (index<0) return null;
		String name = jarPath.substring(0, index);
		index = name.lastIndexOf(File.separatorChar);
		if (index<0) return null;
		name = name.substring(index+1);
		if (name.equals("plugins")) return null;
		//IJ.log("getSubmenuName: "+jarPath+"    \""+name+"\"");
		return name;
	}

	void addItemSorted(JMenu menu, JMenuItem item, int startingIndex) {
		String itemLabel = item.getText();
		int count = menu.getItemCount();
		boolean inserted = false;
		for (int i=startingIndex; i<count; i++) {
			JMenuItem mi = menu.getItem(i);
			String label = mi.getText();
			//IJ.log(i+ "  "+itemLabel+"  "+label + "  "+(itemLabel.compareTo(label)));
			if (itemLabel.compareTo(label)<0) {
				menu.insert(item, i);
				inserted = true;
				break;
			}
		}
		if (!inserted) menu.add(item);
	}

	void addSeparator(JMenu menu) {
		menu.addSeparator();
	}

	/** Opens the configuration file ("plugins.txt") from a JAR file and returns it as an InputStream. */
	InputStream getConfigurationFile(String jar) {
		try {
			ZipFile jarFile = new ZipFile(jar);
			Enumeration entries = jarFile.entries();
			while (entries.hasMoreElements()) {
				ZipEntry entry = (ZipEntry) entries.nextElement();
				if (entry.getName().endsWith("plugins.config"))
					return jarFile.getInputStream(entry);
			}
		}
		catch (Exception e) {}
		return null;
	}

	/** Returns a list of the plugins with directory names removed. */
	String[] getStrippedPlugins(String[] plugins) {
		String[] plugins2 = new String[plugins.length];
		int slashPos;
		for (int i=0; i<plugins2.length; i++) {
			plugins2[i] = plugins[i];
			slashPos = plugins2[i].lastIndexOf('/');
			if (slashPos>=0)
				plugins2[i] = plugins[i].substring(slashPos+1,plugins2[i].length());
		}
		return plugins2;
	}

	/** Returns a list of the plugins in the plugins menu. */
	public static synchronized String[] getPlugins() {
		String homeDir = Prefs.getHomeDir();
		if (homeDir==null)
			return null;
		if (homeDir.endsWith("plugins"))
			pluginsPath = homeDir;
		else {
			String pluginsDir = System.getProperty("plugins.dir");
			if (pluginsDir==null)
				pluginsDir = homeDir;
			else if (pluginsDir.equals("user.home"))
				pluginsDir = System.getProperty("user.home");
			pluginsPath = pluginsDir+Prefs.separator+"plugins"+Prefs.separator;
		}
		File f = pluginsPath!=null?new File(pluginsPath):null;
		if (f==null || (f!=null && !f.isDirectory())) {
			error = "Plugins folder not found at "+pluginsPath;
			pluginsPath = null;
			return null;
		}
		String[] list = f.list();
		if (list==null)
			return null;
		Vector v = new Vector();
		jarFiles = null;
		for (int i=0; i<list.length; i++) {
			String name = list[i];
			boolean isClassFile = name.endsWith(".class");
			boolean hasUnderscore = name.indexOf('_')>=0;
			if (hasUnderscore && isClassFile && name.indexOf('$')<0 ) {
				name = name.substring(0, name.length()-6); // remove ".class"
				v.addElement(name);
			} else if (hasUnderscore && (name.endsWith(".jar") || name.endsWith(".zip"))) {
				if (jarFiles==null) jarFiles = new Vector();
				jarFiles.addElement(pluginsPath + name);
			} else if (hasUnderscore && (name.endsWith(".txt"))) {
			} else {
				if (!isClassFile)
					checkSubdirectory(pluginsPath, name, v);
			}
		}
		list = new String[v.size()];
		v.copyInto((String[])list);
		StringSorter.sort(list);
		return list;
	}

	/** Looks for plugins and jar files in a subdirectory of the plugins directory. */
	static void checkSubdirectory(String path, String dir, Vector v) {
		if (dir.endsWith(".java"))
			return;
		File f = new File(path, dir);
		if (!f.isDirectory())
			return;
		String[] list = f.list();
		if (list==null)
			return;
		dir += "/";
		for (int i=0; i<list.length; i++) {
			String name = list[i];
			boolean hasUnderscore = name.indexOf('_')>=0;
			if (hasUnderscore && name.endsWith(".class") && name.indexOf('$')<0) {
				name = name.substring(0, name.length()-6); // remove ".class"
				v.addElement(dir+name);
				//IJ.write("File: "+f+"/"+name);
			} else if (hasUnderscore && (name.endsWith(".jar") || name.endsWith(".zip"))) {
				if (jarFiles==null) jarFiles = new Vector();
				jarFiles.addElement(f.getPath() + File.separator + name);
			} else if (hasUnderscore && (name.endsWith(".txt"))) {
			}
		}
	}

	static String submenuName;
	static JMenu submenu;

	/** Installs a plugin in the Plugins menu using the class name,
		with underscores replaced by spaces, as the command. */
	void installUserPlugin(String className) {
		JMenu menu = pluginsMenu;
		int slashIndex = className.indexOf('/');
		if (slashIndex>0) {
			String dir = className.substring(0, slashIndex);
			className = className.substring(slashIndex+1, className.length());
			//className = className.replace('/', '.');
			if (submenu==null || !submenuName.equals(dir)) {
				submenuName = dir;
				submenu = new JMenu(submenuName);
				pluginsMenu.add(submenu);
				if (menusTable==null) menusTable = new Hashtable();
				menusTable.put(submenuName, submenu);
			}
			menu = submenu;
		//IJ.write(dir + "  " + className);
		}
		String command = className.replace('_',' ');
		command.trim();
		JMenuItem item = new JMenuItem(command);
		menu.add(item);
		item.addActionListener(ij);
		pluginsTable.put(command, className);
		nPlugins++;
	}

	void installPopupMenu(ImageJ ij) {
		String s;
		int count = 0;
		JMenuItem mi;
		popup = new JPopupMenu("");

		while (true) {
			count++;
			s = Prefs.getString("popup" + (count/10)%10 + count%10);
			if (s==null)
				break;
			if (s.equals("-"))
				popup.addSeparator();
			else if (!s.equals("")) {
				mi = new JMenuItem(s);
				mi.addActionListener(ij);
				popup.add(mi);
			}
		}
	}

	public static JMenuBar getMenuBar() {
		return mbar;
	}


	/** Updates the Image/Type and Window menus. */
	public static void updateMenus() {

		if (ij==null) return;
		gray8Item.setState(false);
		gray16Item.setState(false);
		gray32Item.setState(false);
		color256Item.setState(false);
		colorRGBItem.setState(false);
		ImagePlus imp = IJ.getInstance().getImagePlus();
		if (imp==null)
			return;
		int type = imp.getType();
		if (type==ImagePlus.GRAY8) {
			ImageProcessor ip = imp.getProcessor();
			if (ip!=null && ip.getMinThreshold()==ImageProcessor.NO_THRESHOLD && ip.isColorLut()) {
				type = ImagePlus.COLOR_256;
				if (!ip.isPseudoColorLut())
					imp.setType(ImagePlus.COLOR_256);
			}
		}
		switch (type) {
			case ImagePlus.GRAY8:
				gray8Item.setState(true);
				break;
			case ImagePlus.GRAY16:
				gray16Item.setState(true);
				break;
			case ImagePlus.GRAY32:
				gray32Item.setState(true);
				break;
			case ImagePlus.COLOR_256:
				color256Item.setState(true);
				break;
			case ImagePlus.COLOR_RGB:
				colorRGBItem.setState(true);
				break;
		}

	}

	static boolean isColorLut(ImagePlus imp) {
		ImageProcessor ip = imp.getProcessor();
		IndexColorModel cm = (IndexColorModel)ip.getColorModel();
		if (cm==null) return false;
		int mapSize = cm.getMapSize();
		byte[] reds = new byte[mapSize];
		byte[] greens = new byte[mapSize];
		byte[] blues = new byte[mapSize];
		cm.getReds(reds);
		cm.getGreens(greens);
		cm.getBlues(blues);
		boolean isColor = false;
		for (int i=0; i<mapSize; i++) {
			if ((reds[i] != greens[i]) || (greens[i] != blues[i])) {
				isColor = true;
				break;
			}
		}
		return isColor;
	}


	/** Returns the path to the user plugins directory or
		null if the plugins directory was not found. */
	public static String getPlugInsPath() {
		return pluginsPath;
	}


	/** Returns the hashtable that associates commands with plugins. */
	public static Hashtable getCommands() {
		return pluginsTable;
	}

	/** Returns the hashtable that associates shortcuts with commands. The keys
		in the hashtable are Integer keycodes, or keycode+200 for uppercase. */
	public static Hashtable getShortcuts() {
		return shortcuts;
	}


	public static JPopupMenu getPopupMenu() {
		return popup;
	}

	/** Adds a plugin based command to the end of a specified menu.
	* @param plugin			the plugin (e.g. "Inverter_", "Inverter_("arg")")
	* @param menuCode		PLUGINS_MENU, IMPORT_MENU, SAVE_AS_MENU or HOT_KEYS
	* @param command		the menu item label (set to "" to uninstall)
	* @param shortcut		the keyboard shortcut (e.g. "y", "Y", "F1")
	* @param ij				ImageJ (the action listener)
	*
	* @return				returns an error code(NORMAL_RETURN,COMMAND_IN_USE_ERROR, etc.)
	*/
	public static int installPlugin(String plugin, char menuCode, String command, String shortcut, ImageJ ij) {
		if (command.equals("")) { //uninstall
			//Object o = pluginsPrefs.remove(plugin);
			//if (o==null)
			//	return NOT_INSTALLED;
			//else
				return NORMAL_RETURN;
		}

		if (commandInUse(command))
			return COMMAND_IN_USE;
		if (!validShortcut(shortcut))
			return INVALID_SHORTCUT;
		if (shortcutInUse(shortcut))
			return SHORTCUT_IN_USE;

		JMenu menu;
		switch (menuCode) {
			case PLUGINS_MENU: menu = pluginsMenu; break;
			case IMPORT_MENU: menu = importMenu; break;
			case SAVE_AS_MENU: menu = saveAsMenu; break;
			case SHORTCUTS_MENU: menu = shortcutsMenu; break;
			case ABOUT_MENU: menu = aboutMenu; break;
			case FILTERS_MENU: menu = filtersMenu; break;
			case TOOLS_MENU: menu = toolsMenu; break;
			case UTILITIES_MENU: menu = utilitiesMenu; break;
			default: return 0;
		}
		int code = convertShortcutToCode(shortcut);
		JMenuItem item;
		boolean functionKey = code>=KeyEvent.VK_F1 && code<=KeyEvent.VK_F12;
		if (code==0)
			item = new JMenuItem(command);
		else if (functionKey) {
			command += " [F"+(code-KeyEvent.VK_F1+1)+"]";
			shortcuts.put(new Integer(code),command);
			item = new JMenuItem(command);
		}else {
			shortcuts.put(new Integer(code),command);
			int keyCode = code;
			boolean shift = false;
			if (keyCode>200) {
				keyCode -= 200;
				shift = true;
			}
			item = new JMenuItem(command);
		}
		menu.add(item);
		item.addActionListener(ij);
		pluginsTable.put(command, plugin);
		shortcut = code>0 && !functionKey?"["+shortcut+"]":"";
		//IJ.write("installPlugin: "+menuCode+",\""+command+shortcut+"\","+plugin);
		pluginsPrefs.addElement(menuCode+",\""+command+shortcut+"\","+plugin);
		return NORMAL_RETURN;
	}

	/** Deletes a command installed by installPlugin. */
	public static int uninstallPlugin(String command) {
		boolean found = false;
		for (Enumeration en=pluginsPrefs.elements(); en.hasMoreElements();) {
			String cmd = (String)en.nextElement();
			if (cmd.indexOf(command)>0) {
				pluginsPrefs.removeElement((Object)cmd);
				found = true;
				break;
			}
		}
		if (found)
			return NORMAL_RETURN;
		else
			return COMMAND_NOT_FOUND;

	}

	public static boolean commandInUse(String command) {
		if (pluginsTable.get(command)!=null)
			return true;
		else
			return false;
	}

	public static int convertShortcutToCode(String shortcut) {
		int code = 0;
		int len = shortcut.length();
		if (len==2 && shortcut.startsWith("F")) {
			code = KeyEvent.VK_F1+(int)shortcut.charAt(1)-49;
			if (code>=KeyEvent.VK_F1 && code<=KeyEvent.VK_F9)
				return code;
			else
				return 0;
		}
		if (len==3 && shortcut.startsWith("F")) {
			code = KeyEvent.VK_F10+(int)shortcut.charAt(2)-48;
			if (code>=KeyEvent.VK_F10 && code<=KeyEvent.VK_F12)
				return code;
			else
				return 0;
		}
		if (len!=1)
			return 0;
		int c = (int)shortcut.charAt(0);
		if (c>=65&&c<=90) //A-Z
			code = KeyEvent.VK_A+c-65 + 200;
		else if (c>=97&&c<=122) //a-z
			code = KeyEvent.VK_A+c-97;
		else if (c>=48&&c<=57) //0-9
			code = KeyEvent.VK_0+c-48;
		return code;
	}

	static boolean validShortcut(String shortcut) {
		int len = shortcut.length();
		if (shortcut.equals(""))
			return true;
		else if (len==1)
			return true;
		else if (shortcut.startsWith("F") && (len==2 || len==3))
			return true;
		else
			return false;
	}

	public static boolean shortcutInUse(String shortcut) {
		int code = convertShortcutToCode(shortcut);
		if (shortcuts.get(new Integer(code))!=null)
			return true;
		else
			return false;
	}

	/** Called once when ImageJ quits. */
	public static void savePreferences(Properties prefs) {
		int index = 0;
		for (Enumeration en=pluginsPrefs.elements(); en.hasMoreElements();) {
			String key = "plugin" + (index/10)%10 + index%10;
			prefs.put(key, (String)en.nextElement());
			index++;
		}
	}

}