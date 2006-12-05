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
* $Id: XML.java,v 1.3 2006/12/05 05:26:36 bcaldwell Exp $
*
*/

package net.squiz.matrix.core;

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

	/* the static Document Builder instance */
	private static DocumentBuilder builder;

	// cannot instantiate
	private XML() {}

	/**
	 * Retrieves the XML DOM parser for parsing xml
	 * @return the parser, or <code>null</code>
	 * if the parser factory is not configured correctly
	 */
	public static DocumentBuilder getParser() throws SAXException {

		if (builder == null) {
			try {
				DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
				factory.setIgnoringElementContentWhitespace(false);
				builder = factory.newDocumentBuilder();
			} catch (ParserConfigurationException pce) {
				throw new SAXException("Error creating the document builder : "
					+ pce.getMessage());
			}
		}
		return builder;
	}

	/**
	 * Returns a dom  <code>Document</code> from a input stream to an xml file
	 * @param is the input stream
	 * @return The <code>Document</code> from the parsed <code>InputStream</code>
	 * @throws Exception is thrown if there is an error parsing the <code>InputStream</code>
	 */
	public static synchronized Document getDocumentFromInputStream(InputStream is)
		throws SAXException {

		Document doc = null;
		DocumentBuilder db = getParser();

		if (is == null)
			throw new SAXException("Input Stream of XML source cannot be null");

		try {
			doc = db.parse(is);
		} catch (IOException se) {
			throw new SAXException("Could not create the document from the " +
				"Input Stream : " + se.getMessage());
		}

		if (doc.getDocumentElement().getTagName().equals("error"))
			throw new SAXException("Could not create the document");

		return doc;
	}

	/**
	 * Parses a XML string and returns a <code>Document</code> from that XML string
	 *
	 * @param xmlStr the xml string
	 * @return The <code>Document</code> from the parsed XML string
	 * @throws Exception is thrown if there is an error parsing the string
	 */
	public static synchronized Document getDocumentFromString(String xmlStr)
		throws SAXException {

		byte [] xmlArray = xmlStr.getBytes();
		ByteArrayInputStream xmlStream = new ByteArrayInputStream(xmlArray);

		StreamSource ss = new StreamSource(xmlStream);
		Document doc = null;

		try {
			doc = getDocumentFromInputStream(ss.getInputStream());
		} catch (SAXException se) {
			throw new SAXException("Could not create document from String: "
				+ se.getMessage());
		}

		return doc;
	}
}