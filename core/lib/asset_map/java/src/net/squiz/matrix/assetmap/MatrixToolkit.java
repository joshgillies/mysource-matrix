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
* $Id: MatrixToolkit.java,v 1.4 2004/06/30 05:33:28 mmcintyre Exp $
* $Name: not supported by cvs2svn $
*/

package net.squiz.matrix.assetmap;

import java.util.*;
import java.net.*;
import javax.swing.*;

/**
 *
 * MatrixToolkit also defines methods that provide functionality similar to 
 * their PHP counterparts so that the Java Asset Map and the Matrix PHP source
 * can Interoperate
 * 
 * @author Marc McIntyre <mmcintyre@squiz.net>
 */
public class MatrixToolkit {
	
	/**
	 * A store for Icons. We use this because hitting the server
	 * for an icon everytime it is needed is expensive.
	 */
	private static Map icons = new HashMap();
	
	/** The URL to the root directory of the icon in the Matrix System */
	public static final String ICON_URL = "__lib/web/images/icons";
	
	/** The URL to type code icons */
	public static final String TYPE_CODE_URL = "__data/asset_types";
	
	/** the not accessible icon */
	public static final String NOT_ACCESSIBLE_ICON = "asset_map/not_accessible.png";
	
	/** the type 2 icon */
	public static final String TYPE_2_ICON = "asset_map/not_visible.png";
	
	/** The asset map icon URL */
	public static final String ASSET_MAP_ICON_URL = ICON_URL + "/asset_map";
	
	/**
	 * Returns an <code>Icon</code> from the generic <code>Icon</code> factory.
	 * Once an <code>Icon</code> has been obtained From the specified source, 
	 * it is stored so future resource can obtain the <code>Icon</code> 
	 * without hitting the server where the <code>Icon</code> exists.
	 * 
	 * @param path the path to the specified <code>Icon</code> source relative to 
	 * the Icon directory in the mysource system (/core/lib/web/images/icons). 
	 * For example if you wanted an icon for internal messages, this parameter
	 *  should be internal_messages/icon.png
	 * @return the <code>Icon</code> or <code>null</code> if the icon 
	 * does not exist at the specified url. If null is returned and is
	 * inserted as the image in a <code>JLabel</code> for example, the image 
	 * will be blank and will not affect the execution at all, so consider this 
	 * a failsafe device. 
	 */
	public static Icon getIcon(String path) {
		if (!icons.containsKey(path)) {
			try {
				Icon icon = new ImageIcon(
						new URL(MySource.INSTANCE.getBaseURL().toString() +  path));
				icons.put(path, icon);
				return icon;
			} catch (MalformedURLException mue) {
				return null;
			}
		}
		return (Icon) icons.get(path);
	}

	/**
	 * Returns an icon from the LIB/web/images/icons/asset_map directory
	 * in the mysource matrix system. 
	 * 
	 * @param iconName the name of the icon to get
	 * @return the Icon
	 */
	public static Icon getAssetMapIcon(String iconName) {
		return getIcon(ASSET_MAP_ICON_URL + "/" + iconName);
	}
	
	/**
	 * Returns the icon for the given type code
	 * 
	 * @param typeCode the name of the type code of the wanted icon
	 * @return the Icon
	 */
	public static Icon getIconForTypeCode(String typeCode) {
		return getIcon(TYPE_CODE_URL + "/" + typeCode + "/" + "icon.png");
	}
	
	/***
	 * Returns the compound icon for the given type code
	 * 
	 * @param typeCode the type code of the wanted icon
	 * @param compoundIconName the compound icon name
	 * @return the Icon
	 * 
	 * @see CompundIcon
	 */
	public static Icon getCompoundIconForTypeCode(
			String typeCode, 
			String compoundIconName) {
		
		String key = "__compound_icon_" + typeCode;
		if (!icons.containsKey(key)) {
			Icon baseIcon = getIconForTypeCode(typeCode);
			Icon overlayIcon = getIcon(ICON_URL + "/" + compoundIconName);
			
			CompoundIcon icon = new CompoundIcon(
								baseIcon,
								overlayIcon,
								SwingConstants.LEFT,
								SwingConstants.BOTTOM
				);
			icons.put(key, icon);
			return icon;
		}
		return (Icon) icons.get(key);
	}
	
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
		//try {
			url = URLEncoder.encode(url/*, "UTF-8" */);
		//} catch (UnsupportedEncodingException uee) {
		//	System.out.println("Could not encode url");
		//}
		if (!encodeSpaces)
			url = url.replace('+' , ' ');
	
		return url;

	}

	/**
	 * Decodes a string from the UTF-8 encoding standard to a normal string
	 *
	 * @param url the string to decode
	 * @return the decoded String 
	 */	
	public static String rawUrlDecode(String url) {
		//try {
			url = URLDecoder.decode(url/*, "UTF-8"*/);
		//} catch (UnsupportedEncodingException uee) {
	//		System.out.println("Could not encode url");
	//	}

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

	/**
	 * Legacy method for the <code>String.split()</code> method that was
	 * added in 1.4. Use this method in favour of String,split() to keep
	 * 1.3 compatibility.
	 * 
	 * @param str the String to split up
	 * @param splitAt the string to split at
	 * @return an array of tokens, or an array with the first element being the
	 * original string if the token was not found
	 * 
	 */
	public static String[] split(String str, String splitAt) {
		StringTokenizer stok = new StringTokenizer(str, splitAt);
		String[] tokens = new String[stok.countTokens()];
		for (int i = 0; stok.hasMoreTokens(); i++) {
			tokens[i] = stok.nextToken();
		}
		return tokens;
	}
	
}
