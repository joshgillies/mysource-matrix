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
* $Id: XML.java,v 1.1 2004/06/30 05:27:49 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.xml;

import javax.xml.transform.stream.StreamSource;
import javax.xml.parsers.*;
import org.w3c.dom.Document;
import org.xml.sax.SAXException;
import java.io.*;


/**
 * Generic class for parsing and processing XML
 * Note that if this class is used in conjunction with an Applet, 
 * A DocumentBuilderFactory needs to be supplied with the DocumentBuilderFactory
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class XML {

	/**
	 * the static Document Builder instance
	 */
	private static DocumentBuilder builder;


	/**
	 * Retrieves the XML DOM parser for parsing xml
	 *
	 * @return the parser, or <code>null</code> if the parser factory is not configured correctly
	 */
	public static DocumentBuilder getParser() throws SAXException {

		if (builder == null) {
			try {
				DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
				factory.setIgnoringElementContentWhitespace(false);
				builder = factory.newDocumentBuilder();
			} catch (ParserConfigurationException pce) {
				throw new SAXException("Error creating the document builder : " + pce.getMessage());
			}
		}//end if

		return builder;

	}//end getParser()


	/**
	 * Returns a dom  <code>Document</code> from a input stream to an xml file
	 * @param is the input stream
	 * @return The <code>Document</code> from the parsed <code>InputStream</code>
	 * @throws Exception is thrown if there is an error parsing the <code>InputStream</code>
	 */
	public static synchronized Document getDocumentFromInputStream(InputStream is) throws SAXException {
		
		Document doc = null;
		DocumentBuilder db = XML.getParser();

		if (is == null)
			throw new SAXException("Input Stream of XML source cannot be null");

		try {
			doc = db.parse(is);
		} catch (IOException se) {
			throw new SAXException("Could not Create the document from the Input Stream : " + se.getMessage());
		}

		if (doc.getDocumentElement().getTagName().equals("error")) 
			throw new SAXException("Could not create the document");

		return doc;

	}//end getDocumentFromInputStream()


	/**
	 * Parses a XML string and returns a <code>Document</code> from that XML string
	 *
	 * @param xmlStr the xml string
	 *
	 * @return The <code>Document</code> from the parsed XML string
	 * @throws Exception is thrown if there is an error parsing the string
	 */
	public static synchronized Document getDocumentFromString(String xmlStr) throws SAXException {

		// create a new stream to the data string
		byte [] xmlArray = xmlStr.getBytes();
		ByteArrayInputStream xmlStream = new ByteArrayInputStream(xmlArray);
		
		// create a stream source to the data stream
		StreamSource ss = new StreamSource(xmlStream);
		Document doc = null;

		try {
			doc = XML.getDocumentFromInputStream(ss.getInputStream());
		} catch (SAXException se) {
			throw new SAXException("Could not create document from String: " + se.getMessage());
		}

		return doc;
	
	}//end getDocumentFromString()


}//end class