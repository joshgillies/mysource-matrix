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
* $Id: AssetMap.java,v 1.1 2004/01/13 00:43:33 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.HashSet;
import javax.swing.*;
import java.awt.BorderLayout;
import java.io.IOException;
import javax.swing.UIManager;
import java.net.URL;

/**
* The Base Asset Map class
* 
* This class is responsible for handling asset finder requests. This tree does not have any of the drag and drop capabilities 
* of the complex asset map, and mearly supplies functionality for expanding the tree to find a particulat asset
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*
* @see AssetTree
*/
public class AssetMap extends JApplet {

	/**
	* The Jtree to draw and manager the tree structure
	*/
	protected AssetTree tree;

	/**
	* A reference to the asset map applet
	*/
	private static AssetMap applet = null;

	/**
	* the base url of the matrix system
	*/
	private URL baseURL = null;

	/**
	* Constructor
	*/
	public AssetMap() {
		// set the static applet instance to this
		applet = this;

		// set the look and feel of the application
	/*	try {
			UIManager.setLookAndFeel("com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("problem setting look and feel : " + e.getMessage());
		}
*/
	}//end AssetMap()


	/**
	* Returns a reference to this Applet
	*
	* @return the applet
	*/
	public static AssetMap getApplet() {
		return AssetMap.applet;
	
	}//end getApplet()


	/**
	* Returns a reference to the tree that this applet is currently using
	*
	* @return the AssetTree
	*/
	public AssetTree getTree() {
		return this.tree;

	}//end getTree()


	/**
	* Returns the URL where this applet is
	*/
	public URL getBaseURL() {
		return baseURL;
	
	}//end getBaseURL();
	
	
	/**
	* Initialises the tree
	*/
	public void initTree() {
		
		try {
			tree = new AssetTree();
			tree.init();
			tree.setAssetFinderMode(false);
		} catch (IOException ioe) {
			System.err.println("Could not initialise the asset tree: " + ioe.getMessage());
		}

	}//end initTree()


	/**
	* Initialises the applet
	*
	* @access public
	* @return void
	*/
	public void init() {

		baseURL = getDocumentBase();
		initTree();
		getContentPane().setLayout(new BorderLayout());
		getContentPane().add(new JScrollPane(tree));

	}//end init()


	/**
	* Method stub for javascript calls to java.
	* Passes on the work to the <code>JsEventManager</code> to invoke the method in the listeners.
	*
	* @param	type		The type of event
	* @param	command		The event command. This is actually the name of the method that will eventually get 
	* called in the listener.<br /><br />
	* for Example.<br /> When javascript invokes the asset finder, a command of AssetFinderStarted is passed as 
	* this argument, which will call the method AssetFinderStarted in any listeners implementing the interface 
	* <code>AssetFinderListener</code>.
	* @param	params		a serialised javascript array using the serialiseArray() function in <code>fudge/var_serialise/var_serialize.js</code>
	*/
	public void jsToJavaCall(String type, String command, String params) {
		
		try {
			JsEventManager.getInstance().jsToJavaCall(type, command, params);
		} catch (JsEventNotFoundException enfe) {
			System.err.println("Error when process js call: " + enfe.getMessage());
		}
	
	}//end jsToJavaCall()


}//end class