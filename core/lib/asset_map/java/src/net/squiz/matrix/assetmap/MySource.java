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
* $Id: MySource.java,v 1.8 2004/09/24 04:13:44 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import net.squiz.matrix.xml.XML;

import java.net.*;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.*;
import org.xml.sax.SAXException;
import java.io.*;
import java.security.*;


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
	
	/** The unique instance of the MySource object. */
	public static final MySource INSTANCE = new MySource();

	/** base URL for the Matrix system */
	private URL baseURL;
	
	/** execution URL - URL for talking to the asset map */
	private URL execURL;

	/**
	 * Constructs the Mysource Object
	 */
	private MySource() {
		AssetMap am = AssetMap.getApplet();
		
	//	String basePath = "http://192.168.0.2/marc_matrix/";
	//	String execPath = "http://192.168.0.2/marc_matrix/?SQ_ACTION=asset_map_request";

		String basePath = am.getParameter("BASE_URL");
		String execPath = basePath + am.getParameter("BACKEND_SUFFIX") 
			+ "/?SQ_ACTION=asset_map_request&SESSION_ID=" 
			+ am.getParameter("SESSION_ID") + "&SESSION_KEY=" + am.getParameter("SESSION_KEY");;

		try {
			baseURL = new URL(basePath);
			execURL = new URL(execPath);
		} catch (MalformedURLException mue) {
			System.err.println ("Could not set url : " + mue.getMessage());
			baseURL = null;
			execURL = null;
		}
	}

	/** 
	 * Returns the base URL for the Matrix system.
	 * 
	 * @return the base URL for the Matrix system, 
	 * or <code>null</code> if we don't have one.
	 */
	public URL getBaseURL() {
		return baseURL;
	}

	/**
	 * Performs an XML request to <code>asset_map.inc</code>.
	 * 
	 * @param request The String of the XML request 
	 * @return	the XML DOM <code>Document</code> response from the Matrix
	 * @throws IOException	if connection can't be opened, XML parse error, 
	 * or an error given by the Matrix
	 */
	public Document doRequest(String request) throws IOException {

		if (execURL == null)
			throw new IOException("ExecURL is null");

		URLConnection conn = null;
		try {
			conn = execURL.openConnection();
			conn.setUseCaches(false);
			conn.setDoOutput(true);
			
			// Grows if necessary
			ByteArrayOutputStream byteStream = new ByteArrayOutputStream(512);
			PrintWriter out = new PrintWriter(byteStream, true);
			String postData = request;
			out.print(postData);
			out.flush();

			conn.setRequestProperty("Content-Length", String.valueOf(byteStream.size()));
			conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
			conn.setRequestProperty("Cache-Control", "no-store, no-cache, " +
					"must-revalidate, post-check=0, pre-check=0");

			byteStream.writeTo(conn.getOutputStream());
		} catch (IOException ioe) {
			throw new IOException("error while getting request connection : " + ioe.getMessage());
		} catch (AccessControlException ace) {
			Permission perm = ace.getPermission();
			throw new AccessControlException("Permission Exception while " +
					"connecting to '" + perm.getName() + "' " + perm.toString() 
					+ ". The Following Action could not Performed: " + perm.getActions());
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
			// we will get into here if no XML is returned from 
			// conn (the DB daemon hasn't started?)
			throw new IOException("error while parsing : \n" + se.getMessage());
		}

		if (document.getDocumentElement().getTagName().equals("error")) {
			Element errorElement = (Element) document.getDocumentElement();
			throw new IOException("error while getting response: " 
					+ errorElement.getFirstChild().getNodeValue());
		}
		
		return document;

	}
}
