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
* $Id: MySource.java,v 1.2.2.1 2004/02/18 11:39:07 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.xml.XML;

import java.net.URL;
import java.net.URLConnection;
import java.net.MalformedURLException;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.*;

import java.util.HashMap;
import java.util.Map;
import javax.xml.transform.stream.StreamSource;

import org.xml.sax.SAXException;
import java.io.ByteArrayOutputStream;
import java.util.StringTokenizer;
import java.io.*;

import java.security.Permission;
import java.security.AccessControlException;
import javax.swing.Icon;
import javax.swing.ImageIcon;


/**
* An interface to the MySource system. There is only one interface, 
* accessible through the static method <code>getInstance()</code>.
*
* <p>
* Clients of this class call <code>doRequest</code> to send messages
* to MySource, which is controlled on that side by 
* <code>core/lib/asset_map/asset_map.inc</code>.
* </p>
* 
* <p>
* This would be where we broadcast our pirate signal and hack into 
* the Matrix...
* </p>
*
* @author Marc McIntyre <mmcintyre@squiz.net>
*/
public class MySource {
	
	/**
	* The unique instance of the MySource object.
	*/
	private static MySource instance = null;

	/**
	* base URL for the Matrix system
	*/
	private URL baseURL;
	
	
	/**
	* execution URL - URL for talking to the asset map
	*/
	private URL execURL;

	/**
	* A hashmap of the icons that are general to the asset map
	*/
	private HashMap icons = new HashMap();


	/** 
	* Gets the unique instance of the MySource object. 
	* 
	* @return the <code>MySource</code> singleton instance
	*/
	public static MySource getInstance() {
		if (MySource.instance == null) {
			AssetMap am = AssetMap.getApplet();

			URL base = am.getBaseURL();
			System.out.println("base : " + base.toString());
			String basePath = base.getProtocol() + "://" + base.getHost() + MySource.getPath(base.getPath()) + "/";
			String execPath = basePath + "?SQ_ACTION=asset_map_request";
			
			System.out.println(basePath);
			System.out.println(execPath);

			MySource.instance = new MySource(basePath, execPath);
		}
		return MySource.instance;

	}//end getInstance()


	/**
	* Returns a list of special paths that can be used for the matrix sysyem
	*
	* @return an array of special paths that can be used in the matrix system
	*/
	public static String[] getSpecialPaths() {
		int length = 5;
		String [] specialPaths = new String[length];
		specialPaths[0] = "/_admin";
		specialPaths[1] = "/_edit";
		specialPaths[2] = "/__lib";
		specialPaths[3] = "/__fudge";
		specialPaths[4] = "/__data";

		return specialPaths;

	}//end getSpecialPaths()


	/**
	* returns the special path used in the passed in path
	* 
	* @param the path to get the special path from
	* @return the special path used in the passed in path
	*/
	public static String getSpecialPath(String path) {
		String[] specialPaths = MySource.getSpecialPaths();
		for (int i = 0; i < specialPaths.length; i++) {
			if (path.indexOf(specialPaths[i]) != -1) {
				return specialPaths[i];
			}
		}
		return "";

	}//end getSpecialPath()


	/**
	* Returns where in the path the secial path exists
	* 
	* @param the path to get the offset out of
	* @return the position of the special path 
	*/
	public static int getPathOffset(String path) {
		String[] specialPaths = MySource.getSpecialPaths();
		for (int i = 0; i < specialPaths.length; i++) {
			int pos = path.indexOf(specialPaths[i]);
			// if we have something here...
			if (pos != -1) {
				return pos;
			}
		}
		return -1;

	}//end getPathOffset


	/**
	* Returns the root path of a url
	*
	* @param path the url
	* @return the root path
	*/
	public static String getPath(String path) {

		String realPath = path.substring(0, getPathOffset(path));
		return realPath;

	}//end getPath()


	/**
	* Constructor.
	*
	* @param baseURLString The base URL for the Matrix system
	* @param execURLString The URL for the PHP asset map object of the Matrix
	*/
	private MySource(String baseURLString, String execURLString) {
		try {
			baseURL = new URL(baseURLString);
			execURL = new URL(execURLString);
		} catch (MalformedURLException mue) {
			System.err.println ("Could not set url : " + mue.getMessage());
			baseURL = null;
			execURL = null;
		}

	}//end constructor


	/** 
	* Returns the base URL for the Matrix system.
	* 
	* @return the base URL for the Matrix system, or <code>null</code> if we don't have one.
	*/
	public URL getBaseURL() {
		if (baseURL != null) {
			try {
				return new URL(baseURL.toString());
			} catch (MalformedURLException mue) {
				return null;
			}
		}
		return null;

	}//end getBaseURL()


	/**
	* Returns an <code>javax.swing.Icon</code> for a web path
	*
	* The Web path should be relative to <code>core/lib/web</code>
	*/
	public Icon getIcon(String webPath) {
		
		// don't keep hitting the server for each icon every time it is needed
		if (!(icons.containsKey(webPath))) {
			URL url = null;
			try {
				url = new URL(baseURL + "__lib/images/icons" + webPath);
			} catch (MalformedURLException mfue) {
				System.err.println("There does not seem to be an icon for web path " + webPath + ": " + mfue.getMessage());
			}
			icons.put(webPath, new ImageIcon(url));
		}
		return (Icon) icons.get(webPath);

	}//end getIcon()


	/**
	* Performs an XML request to <code>asset_map.inc</code>.
	* 
	* @param request The String of the XML request
	* 
	* @return	the XML DOM <code>Document</code> response from the Matrix
	* @throws IOException	if connection can't be opened, XML parse error, or an error given by the Matrix
	*/
	public Document doRequest(String request) throws IOException {

		if (execURL == null) {
			throw new IOException("ExecURL is null");
		}

		URLConnection conn = null;
		try {
			conn = execURL.openConnection();
			conn.setUseCaches(false);
			conn.setDoOutput(true);
			ByteArrayOutputStream byteStream = new ByteArrayOutputStream(512); // Grows if necessary
			PrintWriter out = new PrintWriter(byteStream, true);
			String postData = request;
			out.print(postData);
			out.flush();

			conn.setRequestProperty("Content-Length", String.valueOf(byteStream.size()));
			conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");

			byteStream.writeTo(conn.getOutputStream());
		} catch (IOException ioe) {
			throw new IOException("error while getting request connection : " + ioe.getMessage());
		} catch (AccessControlException ace) {
			Permission perm = ace.getPermission();
			throw new AccessControlException("Permission Exception while connecting to '" + perm.getName() + "' " + perm.toString() + ". The Following Action could not Performed: " + perm.getActions());
		}

		Document document = null;
		DocumentBuilder builder = null;

	/*	String line = "";
		BufferedReader b = new BufferedReader(new InputStreamReader(conn.getInputStream()));
		while(true) {
			try {
				line = b.readLine();
				if (line == null)
					break;
				System.out.println(line);
			} catch (IOException ioe) {
				System.out.println("error reading stream");
			}
		}
	*/	
		try {
			builder = XML.getParser();
		} catch (SAXException se) {
			throw new IOException("Could not do the mysource request: " + se.getMessage());
		}
		try {
			document = builder.parse(conn.getInputStream());
		} catch (SAXException se) {
			// we will get into here if no XML is returned from conn (the DB daemon hasn't started?)
			throw new IOException("error while parsing : \n" + se.getMessage());
		}

		if (document.getDocumentElement().getTagName().equals("error")) {
			Element errorElement = (Element)document.getDocumentElement();
			throw new IOException("error while getting response - error returned: " + errorElement.getFirstChild().getNodeValue());
		}

		return document;

	}//end doRequest()

}//end class