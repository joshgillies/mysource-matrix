
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
import java.awt.image.*;

public class AssetMap extends JApplet implements InitialisationListener {

	private BasicView view1;
	private BasicView view2;
	private BasicView view3;
	private MatrixTabbedPane pane;
	protected static JSObject window;
	public static AssetMap applet;
	private javax.swing.Timer timer;

	public AssetMap() {
		try {
			UIManager.setLookAndFeel(new MatrixLookAndFeel());
		} catch (UnsupportedLookAndFeelException ulnfe) {
			ulnfe.printStackTrace();
		}
		applet = this;
	}

	// MM: find a better solution for doing this
	// there is a way to get the root container (this)
	public static AssetMap getApplet() {
		return applet;
	}

	public static void getURL(String url) {
		try {
			applet.getAppletContext().showDocument(new URL(url), "sq_main");
		} catch (MalformedURLException mue) {
			mue.printStackTrace();
		}
	}

	public static void openWindow(String url, String title, String options) {
		if (window == null) {
			window = JSObject.getWindow(applet);
		}
		window.call("open_hipo", new Object[] { url } );
	}

	public void start() {}

	public void initialisationComplete(InitialisationEvent evt) {
	//	Asset site = AssetManager.getAsset("30");
	//	MatrixTreeNode siteNode = null;

	//	Iterator nodes = site.getTreeNodes();
	//	while (nodes.hasNext()) {
	//		siteNode = (MatrixTreeNode) nodes.next();
	//	}
	//	((DefaultTreeModel) view2.getTree().getModel()).setRoot(siteNode);
	//	view2.getTree().loadChildAssets(siteNode);

	//	Asset sm = AssetManager.getAsset("3");
	//	MatrixTreeNode smNode = null;

	//	nodes = sm.getTreeNodes();
	//	while (nodes.hasNext()) {
	//		smNode = (MatrixTreeNode) nodes.next();
	//	}
	//	((DefaultTreeModel) view3.getTree().getModel()).setRoot(smNode);
	//	view3.getTree().loadChildAssets(smNode);
	}


	public void init() {
		// load the parameters from the applet tag and add them to
		// our property set
		String paramStr = getParameter("parameter.params");
		String[] params = paramStr.split(",");

		for (int i = 0; i < params.length; i++) {
			Matrix.setProperty(params[i], getParameter(params[i]));
		}
		window = JSObject.getWindow(this);

		createApplet();
		getContentPane().add(createApplet());
		AssetManager.addInitialisationListener(this);

		SwingWorker worker = new SwingWorker() {
			public Object construct() {
				AssetManager.init();
				return null;
			}
		};
		worker.start();

		ActionListener taskPerformer = new ActionListener() {
			public void actionPerformed(ActionEvent evt) {

				ActionListener taskPerformer = new ActionListener() {
					public void actionPerformed(ActionEvent evt) {

						if (window == null)
							JSObject.getWindow(AssetMap.this);

						String assetids = (String) window.getMember("SQ_REFRESH_ASSETIDS");
						if (!assetids.equals("")) {
							AssetManager.refreshAssets(assetids);
							window.eval("SQ_REFRESH_ASSETIDS = '';");
						}
					}
				};

				timer = new javax.swing.Timer(2000, taskPerformer);
				timer.start();
			}
		};
	//	javax.swing.Timer t = new javax.swing.Timer(5000, taskPerformer);
	//	t.setRepeats(false);
	//	t.start();
	}

	public void stop() {
		timer.stop();
		timer = null;
	}

	private JTabbedPane createApplet() {
		getContentPane().setBackground(new Color(0x342939));

		pane = new MatrixTabbedPane(JTabbedPane.LEFT);
		view1 = new BasicView();
		view2 = new BasicView();

		pane.addView("Tree One", view1);
		pane.addView("Tree Two", view2);

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
		}
	}

	private void processAssetFinder(String command, String params) {
		if (command.equals("assetFinderStarted")) {
			String[] assetTypes = params.split(",");
			MatrixTreeBus.startAssetFinderMode(assetTypes);
		} else if (command.equals("assetFinderStopped")) {
			MatrixTreeBus.stopAssetFinderMode();
		}
	}

}
