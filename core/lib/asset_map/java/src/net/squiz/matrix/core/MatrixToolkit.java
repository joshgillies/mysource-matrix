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
* $Id: MatrixToolkit.java,v 1.1 2005/02/18 05:20:07 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.core;

import java.util.*;
import java.net.*;
import javax.swing.*;
import java.awt.Component;
import java.io.UnsupportedEncodingException;

/**
 *
 * MatrixToolkit also defines methods that provide functionality similar to
 * their PHP counterparts so that the Java Asset Map and the Matrix PHP source
 * can Interoperate
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixToolkit {

	// cannot instantiate
	private MatrixToolkit() {}

	/**
	 * Encodes a string using the UTF-8 encoding format.
	 * Useful for encoding strings for urls
	 *
	 * @param url the string to encode
	 * @param encodeSpaces if TRUE spaces will be encoded to plus signs(+)
	 * which will produce a string that is a legal url
	 * @return the URL encoded String
	 */
	public static String rawUrlEncode(String url, boolean encodeSpaces) {
		try {
			url = URLEncoder.encode(url, "UTF-8");
		} catch (UnsupportedEncodingException uee) {
			System.err.println("Could not encode url");
			uee.printStackTrace();
		}
		if (!encodeSpaces)
			url = url.replace('+', ' ');
		return url;
	}

	/**
	 * Decodes a string from the UTF-8 encoding standard to a normal string
	 *
	 * @param url the string to decode
	 * @return the decoded String
	 */
	public static String rawUrlDecode(String url) {
		try {
			url = URLDecoder.decode(url, "UTF-8");
		} catch (UnsupportedEncodingException uee) {
			System.err.println("Could not decode url");
			uee.printStackTrace();
		}
		return url;
	}

	/**
	 * Implodes a Vector with a specified delimiter, creating a string.
	 * Currently, this method can handle Vectors occupied with Integer and/or String objects.
	 *
	 * @param delimiter the delimiter that each element in the vector will
	 * be sererated by
	 * @param elements the elements to implode
	 * @return String the imploded elements string
	 */
	public static String implode(String delimiter, List elements) {
		StringBuffer buff = new StringBuffer();
		Iterator iterator = elements.iterator();
		while (iterator.hasNext()) {
			Object element = iterator.next();
			if (element instanceof Integer)
				buff.append(((Integer) element).toString());
			else
				buff.append((String) element);
			buff.append(delimiter);
		}
		return buff.toString();
	}

	public static void addAssetToXML(StringBuffer xml, Asset asset) {
		addAssetToXML(xml, asset.getId());
	}

	/**
	 * Adds an XML node to the specified string buffer. The xml can be
	 * used for requests for assets.
	 * @param xml the string buffer that contains the xml
	 * @param asset the asset to add.
	 */
	public static void addAssetToXML(StringBuffer xml, String assetid) {
		xml.append("<asset assetid=\"").append(
				rawUrlEncode(assetid, false)).append("\" />");
	}
}
