/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: AssetMap.java,v 1.10 2005/02/20 22:41:14 mmcintyre Exp $
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
import java.awt.image.*;

/**
 * The main applet class
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetMap extends JApplet implements InitialisationListener {

	private BasicView view1;
	private BasicView view2;
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

	public void initialisationComplete(InitialisationEvent evt) {}


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

		// Polling timer
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
		timer.setDelay(5000);
		timer.start();
		
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
