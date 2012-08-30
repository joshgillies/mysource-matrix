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
* $Id: MatrixToolkit.java,v 1.7 2012/08/30 01:09:20 ewang Exp $
*
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

	public static void addAssetToXML(StringBuffer xml, Asset asset, int start, int limit) {
		addAssetToXML(xml, asset.getId(), "0", start, limit);
	}

	/**
	 * Adds an XML node to the specified string buffer. The xml can be
	 * used for requests for assets.
	 * @param xml the string buffer that contains the xml
	 * @param asset the asset to add.
	 */
	public static void addAssetToXML(StringBuffer xml, String assetid, String linkid, int start, int limit) {
		if (assetid.equals("1")) {
			start = 0;
			limit = 0;
			linkid = "0";
		}
		String limitstr = "start=\""+start+"\" limit=\""+limit+"\"";

		xml.append("<asset assetid=\"").append(
				rawUrlEncode(assetid, false)).append("\" "+limitstr).append(" linkid=\"").append(rawUrlEncode(linkid, false)).append("\" />");
	}
}
