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
* $Id: AssetMap.java,v 1.7 2004/09/01 00:47:52 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import javax.swing.*;
import java.awt.*;
import javax.swing.tree.*;
import java.awt.BorderLayout;
import java.io.IOException;
import java.net.URL;
import netscape.javascript.*;

import java.awt.event.*;

/**
 * The base class for the Asset Map applet for the MySource Matrix System.
 * This class is only instatiated when the asset map is in the simple mode.
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class AssetMap extends JApplet {

	/** The tree object */
	protected AssetTree tree;

	/** The javascript DOM object for the window */
	protected JSObject window;

	/** An instance of the asset map*/
	public static AssetMap INSTANCE;
	
	/** An identifier for the main frame of the matrix system */
	public static final String SQ_MAIN_FRAME = "sq_main";
	
	/**The background color of the asset map */
	public static final Color BACKGROUND_COLOUR = new Color(0x342939);
	
	/**
	 * Constructs an Asset Map
	 */
	public AssetMap() {
		INSTANCE = this;
	}

	/**
	 * Returns a reference to this Applet
	 *
	 * @return the applet
	 */
	public static AssetMap getApplet() {
		return AssetMap.INSTANCE;
	}

	/**
	 * Retrurns the <code>AssetTree</code> object
	 * 
	 * @return the <code>AssetTree</code> object. If the <code>AssetMap</code>
	 *  is in the simple mode an instance of the <code>AssetTree</code> is 
	 * returned, else a <code>ComplexAssetTree</code> instance is returned
	 */
	public JTree getTree() {
		return this.tree;
	}
	
	/**
	 * Redirects the specified frame to a location speicifed. If
	 * target is null, the target will default to the sq_main frame
	 * 
	 * @param location the location where the specified frame is
	 * 			to redirect to
	 * @param target the target frame
	 * @return void
	 */
	public static void getUrl(URL location, String target) {

		if (location == null)
			throw new IllegalArgumentException("location is null");
		AssetMap.getApplet().getAppletContext().showDocument(location, target);
	}
	
	/**
	 * Redirects the SQ_MAIN_FRAME of the matrix system to the specified URL
	 * 
	 * @param location the location to redirect to
	 */
	public static void getUrl(URL location) {
		getUrl(location, SQ_MAIN_FRAME);
	}
	
	/**
	 * Opens a new browser window via a javascript call on the 
	 * <code>window</code> object within the DOM interface. 
	 * Options should be in the save format as a javascript call
	 * to the native method <code>window.open</code>, ie: <br>
	 * 
	 * <code>
	 *   window.open('http://www.example.com', 'windowName', 'toolbar=yes, 
	 *   resizable=yes, width=500, height=500');
	 * </code>
	 * Therefore, the options argument would look like:
	 * <code>
	 *  "toolbar=yes, resizable=yes, width=500, height=500"
	 * </code>
	 * 
	 * 
	 * @param url the URL in string form
	 * @param title the title of the window
	 * @param options the options to pass to the open javascript function
	 */
	public void openWindow(String url, String title, String options) {
		
		if (window == null) {
			window = JSObject.getWindow(AssetMap.getApplet());
		}
		window.call("open_hipo", new Object[] { url } );
	}

	/**
	 * Initialises the Asset Tree. This must be called after AssetManager.initialise
	 */
	protected void initTree() {
		tree = new AssetTree(new DefaultTreeModel(AssetManager.INSTANCE.getRootNode()));
		tree.initialise();
		tree.setAssetFinderMode(false);
	}
	
	
	/**
	 * Initialises the asset map
	 */
	public void init() {
		getContentPane().setBackground(BACKGROUND_COLOUR);
		window = JSObject.getWindow(this);
		
		try {
			AssetManager.INSTANCE.initialise();
		} catch (IOException ioe) {
			System.err.println("Error calling initialise on the Asset Manager" +
					": " + ioe.getMessage());
		}
		initTree();
		AssetManager.INSTANCE.setTree(getTree());
	}
	
	/**
	 * inits the GUI
	 */
	public void start() {
		getContentPane().setLayout(new BorderLayout());
		getContentPane().add(new JScrollPane(tree));
	}
	
	/**
	 * Deletes the cached objects and stops the asset tree
	 */
	public void stop() {
		AssetManager.INSTANCE.deleteCachedObjects();
		((AssetTree) getTree()).stop();
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
		try {
			JsEventManager.getInstance().jsToJavaCall(type, command, params);
		} catch (JsEventNotFoundException enfe) {
			System.err.println("Error when process js call: " + enfe.getMessage());
		}
	}


}
