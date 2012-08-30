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
 * $Id: AssetTypeScreen.java,v 1.4 2012/08/30 01:09:20 ewang Exp $
 *
 */

package net.squiz.matrix.core;

import java.io.*;

/**
 * An object that represents a screen for an <code>AssetType</code>.
 *
 * @author Marc McIntyre <mmcintyre@squiz.net>
 * @see AssetType
 */
public class AssetTypeScreen implements Serializable {
	/* the code name for this screen */
	private String codeName;

	/* the screen name for this screen */
	private  String screenName;

	/**
	 * Constructor
	 *
	 * @param codeName the screen code name
	 * @param screenName the screen's pretty name
	 */
	public AssetTypeScreen(String codeName, String screenName) {
		this.codeName   = codeName;
		this.screenName = screenName;
	}

	/**
	 * Returns the code name for this Screen
	 * @return the code name
	 */
	public String getCodeName() {
		return codeName;
	}

	/**
	 * Returns the screen name for this Screen
	 * @return the screen name
	 */
	public String getScreenName() {
		return screenName;
	}
}
