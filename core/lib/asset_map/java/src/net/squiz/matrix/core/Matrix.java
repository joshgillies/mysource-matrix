/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: Matrix.java,v 1.11 2013/07/25 23:23:50 lwright Exp $
*
*/

package net.squiz.matrix.core;

import java.net.*;
import javax.xml.parsers.DocumentBuilder;
import org.w3c.dom.*;
import org.xml.sax.SAXException;
import java.io.*;
import java.security.*;
import java.util.*;
import java.text.MessageFormat;

/**
 * An interface to the Matrix system.
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
public class Matrix {

	private static Properties properties = new Properties();
	private static Properties translations = new Properties();

	// cannot instantiate
	private Matrix() {}

	/**
	 * Returns the translation with the specfied key
	 * @param key the key of the wanted translation
	 */
	public static final String translate(String key) {
		if (!translations.containsKey(key))
			System.out.println("Translation " + key + " not found");
		return translations.getProperty(key, key);
	}

	/**
	 * Returns the translation with the specfied key
	 * @param key the key of the wanted translation
	 * @param arguments the variable values to be formatted into the translation
	 */
	public static final String translate(String key, Object[] arguments) {
		if (!translations.containsKey(key))
			System.out.println("Translation " + key + " not found");
		String rawTranslation = translations.getProperty(key, key);
		return MessageFormat.format(rawTranslation, arguments);
	}

	/**
	 * Sets the translation with the specifed key to the specified value
	 * @param key the key of the translation to set
	 * @param value the value of the translation
	 */
	public static final void setTranslationFile(String propertiesFile) {
		try {
			translations.load(new ByteArrayInputStream(propertiesFile.getBytes()));
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	/**
	 * Returns the property with the specfied key
	 * @param key the key of the wanted property
	 */
	public static final String getProperty(String key) {
		return properties.getProperty(key);
	}

	/**
	 * Returns the property with the specfied key or the default value
	 * if no property is found with that specified key
	 * @param key the key of the wanted property
	 * @param defaultValue the value to return if no property is found
	 */
	public static final String getProperty(String key, String defaultValue) {
		return properties.getProperty(key, defaultValue);
	}

	/**
	 * Sets the property with the specifed key to the specified value
	 * @param key the key of the property to set
	 * @param value the value of the property
	 */
	public static final void setProperty(String key, String value) {
		properties.setProperty(key, value);
	}

	/**
	 * Performs an XML request to <code>asset_map.inc</code>.
	 * @param request The String of the XML request
	 * @return	the XML DOM <code>Document</code> response from the Matrix
	 * @throws IOException	if connection can't be opened, XML parse error,
	 * or an error given by the Matrix
	 */
	public static Document doRequest(String xml) throws IOException {

		final String myxml = xml;
		// Use provileged mode when making request to asset_map.inc because jsToJavaCall() fails on default security policy
		URLConnection conn =  AccessController.doPrivileged(
				new PrivilegedAction<URLConnection>() {
					public URLConnection run() {
						URL execURL = null;
						URLConnection conn = null;
						try {
								String basePath = getProperty("parameter.url.baseurl");
								String execPath = basePath + getProperty("parameter.backendsuffix")
								+ "/?SQ_ACTION=asset_map_request";
								// post session is required when httponly cookie is used which blocks java applet access cookie
								String postSession = getProperty("parameter.postsession");
								if (postSession.equals("1")) {
								    execPath += "&SESSION_ID=" + getProperty("parameter.sessionid") + "&SESSION_KEY=" + getProperty("parameter.sessionkey");
								}
								execURL = new URL(execPath);
						} catch (MalformedURLException mue) {
								mue.printStackTrace(); 
						}       
		                
						try {
								conn = execURL.openConnection();
								conn.setUseCaches(false);
								conn.setDoOutput(true);

								ByteArrayOutputStream byteStream = new ByteArrayOutputStream(512);
								PrintWriter out = new PrintWriter(byteStream, true);
								out.print(myxml);
								out.flush();

								conn.setRequestProperty("Content-Length", String.valueOf(byteStream.size()));
								conn.setRequestProperty("Content-Type", "application/x-www-form-urlencoded");
								conn.setRequestProperty("Cache-Control", "no-store, no-cache, " +
												"must-revalidate, post-check=0, pre-check=0");
								byteStream.writeTo(conn.getOutputStream());

						} catch (IOException ioe) {
		                		//throw new IOException("error while getting request connection : " + ioe.getMessage());
		                		ioe.printStackTrace();
						}
						return conn;
					
					}}
		);
		
		Document document = null;
		DocumentBuilder builder = null;

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
			throw new IOException("error while parsing : " + se.getMessage());
		}

		if (document.getDocumentElement().getTagName().equals("error")) {
			Element errorElement = (Element) document.getDocumentElement();
			throw new IOException("error while getting response: "
					+ errorElement.getFirstChild().getNodeValue());
		}
		return document;
	}
}
