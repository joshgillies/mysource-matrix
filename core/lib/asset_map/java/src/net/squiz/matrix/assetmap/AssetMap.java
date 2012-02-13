/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: AssetMap.java,v 1.27 2012/02/12 23:51:25 ewang Exp $
*
*/

package net.squiz.matrix.assetmap;

import javax.swing.*;
import javax.swing.tree.*;
import net.squiz.matrix.ui.*;
import net.squiz.matrix.core.*;
import net.squiz.matrix.matrixtree.*;
import net.squiz.matrix.plaf.*;
import java.util.*;
import java.awt.event.*;
import java.awt.*;
import java.net.*;
import netscape.javascript.*;
import javax.swing.plaf.metal.*;
import javax.swing.border.*;
import net.squiz.matrix.debug.*;
import java.io.IOException;
import org.w3c.dom.*;
import java.security.AccessControlException;

/**
 * The main applet class
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetMap extends JApplet implements InitialisationListener, KeyListener, ContainerListener {

	private BasicView view1;
	private BasicView view2;
	private MatrixTabbedPane pane;
	protected static JSObject window;
	public static AssetMap applet;
	private javax.swing.Timer timer;
	public static final int POLLING_TIME = 2000;

	/**
	 * Constructs the Asset Map
	 */
	public AssetMap() {
		try {
			UIManager.setLookAndFeel(new MatrixLookAndFeel());
		} catch (UnsupportedLookAndFeelException ulnfe) {
			ulnfe.printStackTrace();
		}
		applet = this;

		addKeyAndContainerListenerRecursively(this);
	}

	// MM: find a better solution for doing this
	// there is a way to get the root container (this)
	public static AssetMap getApplet() {
		return applet;
	}

	/**
	 * Opens the specfied url in the sq_main frame of the matrix system.
	 * @param url the url to open in the sq_main frame
	 */
	public static void getURL(String url) throws MalformedURLException {
		applet.getAppletContext().showDocument(new URL(url), "sq_main");
	}

	/**
	 * Operns a new browser window to the specified url
	 * @param url the url to open
	 * @param title the title to show in the browser window
	 * @see getURL(String)
	 */
	public static void openWindow(String url, String title) {
		if (window == null)
			window = JSObject.getWindow(applet);
		window.call("open_hipo", new Object[] { url } );
	}

	public void initialisationComplete(InitialisationEvent evt) {}


	/**
	 * Initialises the asset map.
	 * @see start()
	 * @see init()
	 */
	public void init() {
		window = JSObject.getWindow(this);
		loadParameters();
		loadTranslations();
		getContentPane().add(createApplet());

		addKeyListener(this);
	}

	/**
	 * Starts the applet.
	 * This is where the initial request is made to the matrix system for
	 * the current assets and asset types. The request happens in a swing safe
	 * worker thread and the GUI is updated upon completion of the request.
	 * @see JApplet.start()
	 * @see stop()
	 * @see init()
	 */
	public void start() {

		initAssetMap();
		startPolling();
	}

	/**
	* Checks the JRE version of the client
	*/
	private void javaVersionCheck() {

		boolean supportedVersion = false;

		String version = System.getProperty("java.version");
		String[] supVersions = (Matrix.getProperty("parameter.java.supportedversion")).split("\\,");

		// compare versions
		for (int i=0; i< supVersions.length; i++) {
			if (version.startsWith(supVersions[i])) {
				supportedVersion = true;
				break;
			}
		}

		if (!supportedVersion) {
			Object[] options = { Matrix.translate("ok"), Matrix.translate("cancel") };
			Object[] transArgs = {
							version,
							Matrix.getProperty("parameter.java.supportedversion").replaceAll(",",", ")
						};

			int ret = JOptionPane.showOptionDialog(null, Matrix.translate("asset_map_error_java_version", transArgs),
			Matrix.translate("asset_map_error_java_version_title"),
			JOptionPane.DEFAULT_OPTION, JOptionPane.WARNING_MESSAGE, null, options, options[0]);

			if (ret == 0) {
				// go to the SUN Java download page
				try {
					getAppletContext().showDocument(new URL(Matrix.getProperty("parameter.java.sunurl")), "_blank");
				} catch (java.net.MalformedURLException exp) {
					System.out.println(exp.getMessage());
				}
			}
		}

	}

	/**
	 * Initialises the asset map by making a request to the matrix system for
	 * the current assets and asset types in the system.
	 */
	protected void initAssetMap() {
		// get a swing worker to call init in AssetManager
		// when it returns set the root node to all trees
		MatrixStatusBar.setStatus(Matrix.translate("asset_map_status_bar_init"));
		MatrixSwingWorker worker = new MatrixSwingWorker() {
			public Object construct() {
				MatrixTreeNode root = null;
				try {
					root = AssetManager.init();
					GUIUtilities.getAssetMapIcon(MatrixMenus.DEFAULT_ASSET_ICON);
				} catch (IOException ioe) {
					return ioe;
				}
				return root;
			}

			public void finished() {
				Object get = get();
				// success
				if (get instanceof MatrixTreeNode) {

					// Check if root folder asset is the actual root node for this user
					// if not use the specified asset as the root node
					String newRoot = Matrix.getProperty("parameter.rootlineage");
					MatrixTreeModelBus.setRoot((MatrixTreeNode) get());

					if (newRoot.length() > 0) {
						// root folder asset is not the root node
						String[] info = newRoot.split("~");
						String[] assetIds = info[0].split("\\|");
						String[] sort_orders = info[1].split("\\|");

						// update tree root nodes
						Iterator trees = MatrixTreeBus.getTrees();
						while (trees.hasNext()) {
							MatrixTree tree = (MatrixTree) trees.next();
							// collapse the current root so we dont see its kids while switching root nodes
							tree.collapsePath(tree.getPathToRoot((MatrixTreeNode)tree.getModel().getRoot()));
							// find the specified asset/link and switch root node
							tree.loadChildAssets(assetIds, sort_orders, false, true);


						}
					}

					try {
						// if we have an initial lineage selected (i.e. from /_admin) then expand the tree
						String initial_lineage = Matrix.getProperty("parameter.initialselection");
						if (initial_lineage.length() > 0) {
							String[] init_info = initial_lineage.split("~");
							String[] init_assetIds = init_info[0].split("\\|");
							String[] init_sort_orders = init_info[1].split("\\|");
							Iterator trees = MatrixTreeBus.getTrees();
							while (trees.hasNext()) {
								MatrixTree tree = (MatrixTree) trees.next();
								tree.loadChildAssets(init_assetIds, init_sort_orders, true, false);
							}
						}
					} catch (Exception exp) {}
					MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_success"), 1000);
				// error
				} else if (get instanceof IOException) {
					IOException ioe = (IOException) get;
					GUIUtilities.error(
						AssetMap.this, ioe.getMessage(), Matrix.translate("asset_map_dialog_title_init_failed"));
					MatrixStatusBar.setStatusAndClear(Matrix.translate("asset_map_status_bar_init_failed"), 1000);
					Log.log("Could not initialise the asset map", AssetMap.class, ioe);
				}
			}
		};
		worker.start();
	}

	/**
	 * Starts the polling operation to refresh assets that Matrix deems stale.
	 * The polling operation is polled at POLLING_TIME intervals. When javascript
	 * array is not empty, the assetids are used in a refresh operation.
	 * @see POLLING_TIME
	 */
	protected void startPolling() {
		// Polling timer
		ActionListener taskPerformer = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {
				if (window == null)
					JSObject.getWindow(AssetMap.this);

				String assetidsStr = (String) window.getMember("SQ_REFRESH_ASSETIDS");
				// if the string isn't empty, we have some work to do.
				// split the string for the asset ids and get the refresh worker
				// to start the refresh operation
				if (assetidsStr != null && !assetidsStr.equals("")) {
					String[] assetids = assetidsStr.split("\\|");
					AssetRefreshWorker worker = new AssetRefreshWorker(assetids, false) {
						// return a custom message for the wait message
						protected String getStatusBarWaitMessage() {
							return Matrix.translate("asset_map_status_bar_auto_refresh");
						}
					};
					worker.start();
					// clear the assets that we have refreshed
					window.eval("SQ_REFRESH_ASSETIDS = '';");
				}
			}
		};
		timer = new javax.swing.Timer(POLLING_TIME, taskPerformer);
		timer.start();
	}

	/**
	 * Load the parameters from the applet tag and add them to
	 * our property set. Parameters are stored in the Matrix property set
	 * and can be accessed with:
	 * <pre>
	 *  String prop = Matrix.getParameter("parameter." + propertyName);
	 * </pre>
	 * @see Matrix.getParameter(String)
	 */
	public void loadParameters() {
		// get the list of parameters availble to the asset map
		// and store them in the matrix property set.
		String paramStr = getParameter("parameter.params");
		String[] params = paramStr.split(",");

		for (int i = 0; i < params.length; i++)
			Matrix.setProperty(params[i], getParameter(params[i]));
	}

	/**
	 * Request the translations strings for use in the asset map and pass them
	 * off to a Matrix ResourceBundle.
	 */
	public void loadTranslations() {
		Document response = null;

		try {
			response = Matrix.doRequest("<command action=\"get translations\" />");
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}

		Element rootElement = response.getDocumentElement();

		// Set the Java VM locale to the Matrix locale if available
		String currentLocale = rootElement.getAttribute("locale");
		Locale availLocales[] = Locale.getAvailableLocales();
		for (int j = 0; j < availLocales.length; j++) {
			if (availLocales[j].toString().equals(currentLocale)) {
				try {
					Locale.setDefault(availLocales[j]);
				} catch (AccessControlException ace) {
					Log.log("Error setting locale, invalid permisions", AssetMap.class, ace);
				}
			}
		}

		// Grab the .properties file from the XML CDATA
		NodeList children = rootElement.getChildNodes();
		for (int i = 0; i < children.getLength(); i++) {
			if (!(children.item(i) instanceof CDATASection))
				continue;

			CDATASection cdata = (CDATASection) children.item(i);
			Matrix.setTranslationFile(cdata.getData());
		}
	}

	/**
	 * Stops the asset map and polling.
	 * @see start()
	 * @see init()
	 */
	public void stop() {
		timer.stop();
		timer = null;
	}

	/**
	 * Creates the asset map's interface.
	 * @return the compoent to add to the content pane.
	 */
	protected JComponent createApplet() {
		getContentPane().setBackground(new Color(0x342939));

		pane = new MatrixTabbedPane(JTabbedPane.LEFT);
		view1 = new BasicView();
		view2 = new BasicView();

		pane.addView(Matrix.translate("asset_map_tree1_name"), view1);
		pane.addView(Matrix.translate("asset_map_tree2_name"), view2);

		return pane;
	}

	/**
	 * Method stub for javascript calls to java.
	 * Passes on the work to the <code>JsEventManager</code> to invoke
	 * the method in the listeners.
	 *
	 * @param	type		The type of event
	 * @param	command		The event command. This is actually the
	 * name of the method that will eventually get  called in the listener.
	 * <br /><br />
	 * For Example.<br /> When javascript invokes the asset finder,
	 * a command of AssetFinderStarted is passed as this argument,
	 * which will call the method AssetFinderStarted in any listeners
	 * implementing the interface <code>AssetFinderListener</code>.
	 * @param params a serialised javascript array using the serialiseArray()
	 * function in <code>fudge/var_serialise/var_serialize.js</code>
	 */
	public void jsToJavaCall(String type, String command, String params) {
		if (type.equals("asset_finder")) {
			processAssetFinder(command, params);
		} else if (type.equals("asset_locator")) {
			processAssetLocator(params);
		}
	}

	public void processAssetLocator(String params) {
		// we need to create 2 arrays
		String[] info = params.split("~");
		String[] assetIds = info[0].split("\\|");
		String[] sort_orders = info[1].split("\\|");
		MatrixTreeBus.startAssetLocator(assetIds, sort_orders);
	}

	private void processAssetFinder(String command, String params) {
		if (command.equals("assetFinderStarted")) {
			String[] assetTypes = params.split("~");
			MatrixTreeBus.startAssetFinderMode(assetTypes);
		} else if (command.equals("assetFinderStopped")) {
			MatrixTreeBus.stopAssetFinderMode();
		}
	}

	public void keyPressed(KeyEvent e) {
		if (e.getKeyCode() == KeyEvent.VK_ESCAPE) {
			Iterator trees = MatrixTreeBus.getTrees();

			while (trees.hasNext()) {
				MatrixTree tree = (MatrixTree) trees.next();
				tree.stopCueMode();
			}
		}
	}

	public void keyTyped(KeyEvent e) {}
	public void keyReleased(KeyEvent e) {}

	public void componentAdded(ContainerEvent e) {
		addKeyAndContainerListenerRecursively(e.getChild());
	}

	public void componentRemoved(ContainerEvent e) {
		removeKeyAndContainerListenerRecursively(e.getChild());
	}

	public void addKeyAndContainerListenerRecursively(Component c) {
		c.addKeyListener(this);

		if (c instanceof Container) {
			Container cont = (Container)c;
			cont.addContainerListener(this);

			Component[] children = cont.getComponents();
			for (int i = 0; i < children.length; i++) {
				addKeyAndContainerListenerRecursively(children[i]);
			}
		}
	}

	public void removeKeyAndContainerListenerRecursively(Component c) {
		c.removeKeyListener(this);

		if (c instanceof Container) {
			Container cont = (Container)c;
			cont.removeContainerListener(this);

			Component[] children = cont.getComponents();
			for (int i = 0; i < children.length; i++) {
				removeKeyAndContainerListenerRecursively(children[i]);
			}
		}
	}
}
